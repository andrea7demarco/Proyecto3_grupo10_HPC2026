#!/usr/bin/env python3
"""Ejecuta los programas C y guarda corridas, medianas y speedup."""
import argparse
import csv
from datetime import datetime, timezone
from pathlib import Path
import platform
import statistics
import subprocess

ROOT = Path(__file__).resolve().parents[1]
RECURSOS = {1: 2, 2: 6, 6: 24, 12: 48, 24: 96, 48: 192, 96: 384}


def lista_enteros(texto):
    valores = sorted(set(int(valor) for valor in texto.split(",")))
    if not valores or valores[0] < 1:
        raise argparse.ArgumentTypeError("Usar enteros positivos separados por comas")
    return valores


def probabilidad_teorica(k):
    if k > 365:
        return 1.0
    sin_coincidencia = 1.0
    for i in range(k):
        sin_coincidencia *= (365 - i) / 365
    return 1 - sin_coincidencia


def ejecutar(comando, archivo_log, etiqueta_tiempo):
    resultado = subprocess.run(comando, capture_output=True, text=True)
    archivo_log.write_text("Comando: " + str(comando) + "\n" + resultado.stdout
                           + resultado.stderr, encoding="utf-8")
    if resultado.returncode != 0:
        raise RuntimeError("Fallo la ejecucion. Revisar " + str(archivo_log))
    # Cada programa imprime una etiqueta seguida de dos puntos y su valor.
    datos = {}
    for linea in resultado.stdout.splitlines():
        if ":" in linea:
            nombre, valor = linea.split(":", 1)
            datos[nombre.strip()] = valor.strip()
    coincidencias = int(datos["Coincidencias"])
    tiempo = float(datos[etiqueta_tiempo].split()[0])
    if tiempo < 0:
        raise RuntimeError("Tiempo negativo. Revisar el reloj y repetir la campana")
    return coincidencias, tiempo


def resumir(tiempos, k, n):
    medianas = {}
    for configuracion, mediciones in tiempos.items():
        medianas[configuracion] = statistics.median(mediciones)
    filas = []
    for (version, p), mediana in medianas.items():
        speedup, eficiencia, speedup_mpi1 = "", "", ""
        if 1 < k <= 365 and mediana > 0:
            speedup = medianas[("sequential", 1)] / mediana
            eficiencia = speedup / p
            if version in ("mpi", "mpi_dynamic"):
                speedup_mpi1 = medianas[(version, 1)] / mediana
        filas.append([version, k, n, p, len(tiempos[(version, p)]), mediana,
                      speedup, eficiencia, speedup_mpi1])
    return filas


def main(argv=None):
    # 1. Elegir las pruebas. --cores debe coincidir con el panel del cluster.
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--cores", type=int, required=True, choices=RECURSOS)
    parser.add_argument("--profile", choices=["performance", "validation"], default="performance")
    parser.add_argument("--k", type=lista_enteros)
    parser.add_argument("--n", type=lista_enteros)
    parser.add_argument("--processes", type=lista_enteros)
    parser.add_argument("--repetitions", type=int, default=7)
    parser.add_argument("--seed", type=int, default=2026)
    parser.add_argument("--launcher", default="mpirun")
    parser.add_argument("--mpi-mode", choices=["static", "dynamic", "both"], default="static",
                        help="Version MPI a medir; both compara estatica y dinamica")
    parser.add_argument("--batch-size", type=int,
                        help="Simulaciones por lote dinamico; por defecto se calcula automaticamente")
    args = parser.parse_args(argv)
    procesos = args.processes or [p for p in RECURSOS if p <= args.cores]
    if 1 not in procesos or max(procesos) > args.cores:
        parser.error("Incluir MPI1 y no superar los cores asignados")
    if args.repetitions < 1 or not 0 <= args.seed <= 4294967295 - args.repetitions * args.cores + 1:
        parser.error("Repeticiones o semilla fuera de rango")
    if args.batch_size is not None:
        if not 1 <= args.batch_size <= 18446744073709551615:
            parser.error("El lote debe ser positivo y entrar en unsigned long long")
        if args.mpi_mode == "static":
            parser.error("--batch-size requiere --mpi-mode dynamic o both")
    if args.profile == "performance":
        valores_k = args.k or [23]
        valores_n = args.n or [100000000, 1000000000]
    else:
        valores_k = args.k or [1, 2, 10, 23, 30, 57, 100, 365, 366]
        valores_n = args.n or [1000, 10000, 100000, 1000000]
        procesos = sorted({1, max(procesos)})
    if max(valores_k) > 2147483647 or max(valores_n) > 18446744073709551615:
        parser.error("K o N fuera del rango de los programas C")

    # 2. Crear una carpeta nueva y compilar las versiones elegidas con -O3.
    fecha = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S_%fZ")
    carpeta = ROOT / "results" / f"cores{args.cores}_{args.profile}_{fecha}"
    carpeta.mkdir(parents=True)
    (carpeta / "logs").mkdir()
    (carpeta / "bin").mkdir()
    secuencial = carpeta / "bin/cumple_secuencial"
    fuentes_mpi = {}
    if args.mpi_mode in ("static", "both"):
        fuentes_mpi["mpi"] = "cumple_mpi.c"
    if args.mpi_mode in ("dynamic", "both"):
        fuentes_mpi["mpi_dynamic"] = "cumple_mpi_dinamico.c"
    paralelos = {version: carpeta / "bin" / Path(fuente).stem
                 for version, fuente in fuentes_mpi.items()}
    print("Resultados:", carpeta, flush=True)
    (carpeta / "info.txt").write_text(
        f"Equipo: {platform.node()}\nSistema: {platform.platform()}\n"
        f"Recurso declarado: {RECURSOS[args.cores]} GB / {args.cores} cores\n"
        f"Parametros: {vars(args)}\nCompilacion: -O3 -std=c11 -Wall -Wextra -Wpedantic\n",
        encoding="utf-8")
    compilaciones = [("gcc", "cumple_secuencial.c", secuencial)]
    compilaciones += [("mpicc", fuente, paralelos[version])
                      for version, fuente in fuentes_mpi.items()]
    for compilador, fuente, ejecutable in compilaciones:
        subprocess.run([compilador, "-O3", "-std=c11", "-Wall", "-Wextra", "-Wpedantic",
                        str(ROOT / "src" / fuente), "-o", str(ejecutable)], check=True)

    # 3. Guardar cada corrida y el resumen de cada conjunto completo.
    with (carpeta / "raw.csv").open("w", newline="", encoding="utf-8") as raw, \
         (carpeta / "summary.csv").open("w", newline="", encoding="utf-8") as summary:
        registro, resumen = csv.writer(raw), csv.writer(summary)
        registro.writerow(["implementation", "k", "n", "processes", "repetition", "seed",
                            "coincidences", "probability", "theory", "absolute_error", "kernel_seconds"])
        resumen.writerow(["implementation", "k", "n", "processes", "repetitions", "median_seconds",
                           "speedup_sequential", "efficiency_sequential", "speedup_mpi1"])
        configuraciones = [("sequential", 1)] + [(version, p)
                           for version in fuentes_mpi for p in procesos]
        for k in valores_k:
            for n in valores_n:
                tiempos = {configuracion: [] for configuracion in configuraciones}
                teoria = probabilidad_teorica(k)
                for repeticion in range(args.repetitions + 1):
                    # La repeticion 0 es un calentamiento: no entra en los CSV.
                    semilla = args.seed
                    if args.profile == "validation" and repeticion > 0:
                        semilla += (repeticion - 1) * args.cores
                    conteo_secuencial = None
                    for version, p in configuraciones:
                        if version == "sequential":
                            comando = [str(secuencial)]
                            etiqueta = "Tiempo secuencial"
                        else:
                            comando = [args.launcher, "-np", str(p), str(paralelos[version])]
                            etiqueta = "Tiempo paralelo"
                        comando += [str(k), str(n), str(semilla)]
                        if version == "mpi_dynamic" and args.batch_size is not None:
                            comando.append(str(args.batch_size))
                        log = carpeta / "logs" / f"k{k}_n{n}_{version}_p{p}_r{repeticion}.log"
                        print(f"K={k} N={n} {version} p={p} repeticion={repeticion}", flush=True)
                        conteo, tiempo = ejecutar(comando, log, etiqueta)
                        if not 0 <= conteo <= n or (k == 1 and conteo != 0) or (k > 365 and conteo != n):
                            raise RuntimeError("Conteo invalido; revisar " + str(log))
                        if version == "sequential":
                            conteo_secuencial = conteo
                        elif p == 1 and conteo != conteo_secuencial:
                            raise RuntimeError("Secuencial y MPI1 difieren con igual semilla")
                        if repeticion > 0:
                            probabilidad = conteo / n
                            registro.writerow([version, k, n, p, repeticion, semilla, conteo,
                                                probabilidad, teoria, abs(probabilidad - teoria), tiempo])
                            raw.flush()
                            tiempos[(version, p)].append(tiempo)
                resumen.writerows(resumir(tiempos, k, n))
                summary.flush()
    print("Campana completa. Ver raw.csv y summary.csv en", carpeta)


if __name__ == "__main__":
    main()
