import asyncio
import argparse
import json
from pathlib import Path
from typing import Dict, List
from sys import platform

performance_metrics: Dict = dict()
all_tests: List = list()
failed_tests: List = list()


async def run(release_build_dir: str,
              num_consumer_cores: int,
              num_consumers: int,
              first_core: int):
    global performance_metrics
    performance_metrics = dict()
    benchmark_binary = str(Path(release_build_dir) / 'sample_apps/benchmark_app/benchmark_app')
    producer_binary = str(Path(release_build_dir) / 'sample_apps/test_producer/test_producer_app')
    consumer_binary = str(Path(release_build_dir) / 'sample_apps/test_consumer/test_consumer_app')

    benchmark_cmd = f'{set_cpu_affinity(first_core)} {benchmark_binary} 10'
    producer_cmd = f'{set_cpu_affinity(first_core + 1)} {producer_binary} 11 no_log'

    coros = list()
    coros += [run_command(benchmark_cmd, 'benchmark', is_consumer=False)]
    coros += [run_command(producer_cmd, 'producer', is_consumer=False)]

    for i in range(num_consumers):
        core = (i % num_consumer_cores) + 2 + first_core
        consumer_cmd = f'{set_cpu_affinity(core)} {consumer_binary} 10 no_log'
        coros.append(run_command(consumer_cmd, f'consumer-{i + 1}', is_consumer=True))

    print(f'running performance tests with 1 producer and {num_consumers} consumers')
    await asyncio.gather(*coros)
    print_metrics(num_consumers)


async def run_command(cmd: str, desc: str, is_consumer: bool):
    if is_consumer:
        await asyncio.sleep(0.5)

    print(f'running: {cmd}')
    proc = await asyncio.create_subprocess_shell(
        cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE)

    stdout, stderr = await proc.communicate()

    print(f'[{cmd!r} exited with {proc.returncode}]')
    if stdout:
        collect_metrics(desc, stdout.decode())
    if stderr:
        print(f'[stderr {desc}] process crashed with error: \n{stderr.decode()}')


def support_cpu_affinity() -> bool:
    if platform == "darwin":
        return False # tasket not supported on OSX
    return True


def set_cpu_affinity(core: int) -> str:
    if not support_cpu_affinity():
        return ""
    return f"taskset -c {core}"


def parse_metric_line(line: str) -> json:
    metric = json.loads(line.split('_metric]')[-1].strip())
    return metric


def collect_metrics(desc: str, stdout: str):
    global performance_metrics
    lines = stdout.split('\n')
    lines = [line for line in lines if '_metric' in line]
    performance_metrics[desc] = dict()
    for line in lines:
        performance_metrics[desc].update(parse_metric_line(line))


def print_metric_for_app(app_name: str, benchmark_mb_per_sec: int, num_consumers: int):
    global all_tests, failed_tests
    for name, metrics in performance_metrics.items():
        if not name == app_name:
            continue

        finished = int(metrics['finished']) == 1
        performance_percent = metrics['mb_per_sec'] * 1.0 / benchmark_mb_per_sec
        if finished and performance_percent > 0.7:
            status = 'PASSED'
        else:
            status = "FAIL"
            failed_tests.append(f'[FAIL] test-{num_consumers}-consumers: {app_name}')

        all_tests.append(f'[{status}] test-{num_consumers}-consumers: {app_name}')

        print(f"{name} [{status}]:")
        print(f"  percent_throughput_of_benchmark: {(performance_percent * 100):.2f}%")
        for metric_name, metric_value in metrics.items():
            print(f"  {metric_name}: {metric_value}")


def print_metrics(num_consumers: int):
    benchmark_mb_per_sec = 0
    for name, metrics in performance_metrics.items():
        if name == 'benchmark':
            benchmark_mb_per_sec = metrics['mb_per_sec']

    print("==============================================")
    print(f"RESULTS: benchmark with 1 producer and {num_consumers} consumers")
    print("")
    print_metric_for_app('benchmark', benchmark_mb_per_sec, num_consumers)
    print_metric_for_app('producer', benchmark_mb_per_sec, num_consumers)
    for name, metrics in performance_metrics.items():
        if 'consumer' in name:
            print_metric_for_app(name, benchmark_mb_per_sec, num_consumers)
    print("==============================================")


def print_final_results():
    print("==============================================")
    print(f" ALL RESULTS")
    print("")
    for result in all_tests:
        print(result)
    print("")

    if len(all_tests) == 0:
        print("[FAIL] No test ran.")
    elif len(failed_tests) == 0:
        print("[SUCCESS] All tests passed.")
    else:
        print("[FAIL] There were failures. The following tests failed:")

    for result in failed_tests:
        print(result)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('release_build_dir')
    parser.add_argument('num_cores')
    parser.add_argument('start_at_core')
    args = parser.parse_args()

    assert int(args.num_cores) >= 3, "at least 3 cores are required for the performance tests"
    assert int(args.start_at_core) >= 0, "start_at_core has to be greater than 0"

    print("starting fastq performance tests")
    if not support_cpu_affinity():
        print("======================================================================================")
        print(" ** WARNING ** : CPU affinity is not supported on this system, benchmarks will be near useless!")
        print("======================================================================================")

    num_cores: int = int(args.num_cores)
    first_core: int = int(args.start_at_core)
    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(run(args.release_build_dir, num_cores - 2, 1, first_core))
        loop.run_until_complete(run(args.release_build_dir, num_cores - 2, 5, first_core))
        loop.run_until_complete(run(args.release_build_dir, num_cores - 2, 10, first_core))
        loop.run_until_complete(run(args.release_build_dir, num_cores - 2, num_cores - 2, first_core))
        print_final_results()
    except KeyboardInterrupt:
        print('Stopped (KeyboardInterrupt)')


if __name__ == '__main__':
    main()
