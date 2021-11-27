import asyncio
import argparse
from pathlib import Path


async def run(release_build_dir: str):
    producer_binary = str(Path(release_build_dir) / 'sample_apps/test_producer/test_producer_app')
    consumer_binary = str(Path(release_build_dir) / 'sample_apps/test_consumer/test_consumer_app')

    producer_cmd = f'taskset 0x00000001 {producer_binary} 11 no_log'

    consumer_cmds = list()
    consumer_cmds += [f'taskset 0x00000010 {consumer_binary} 10 no_log'] * 10

    coros = [run_command(producer_cmd, 'producer', is_consumer=False)]
    for i in range(len(consumer_cmds)):
        coros.append(run_command(consumer_cmds[i], f'consumer-{i}', is_consumer=True))
    await asyncio.gather(*coros)


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
        print(f'[stdout {desc}]\n{stdout.decode()}')
    if stderr:
        print(f'[stderr {desc}]\n{stderr.decode()}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('release_build_dir')
    args = parser.parse_args()

    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(run(args.release_build_dir))
    except KeyboardInterrupt:
        print('Stopped (KeyboardInterrupt)')


if __name__ == '__main__':
    main()