import asyncio
import argparse
import time


async def run(producer_binary: str, consumer_binary: str):
    producer_cmd = f'taskset 0x00000001 {producer_binary} 10 no_log'

    consumer_cmds = list()
    consumer_cmds += [f'taskset 0x00000010 {consumer_binary} 10 no_log'] * 1

    coros = [run_command(producer_cmd, 'producer', is_consumer=False)]
    for i in range(len(consumer_cmds)):
        coros.append(run_command(consumer_cmds[i], f'consumer-{i}', is_consumer=True))
    await asyncio.gather(*coros)


async def run_command(cmd: str, desc: str, is_consumer: bool):
    if is_consumer:
        await asyncio.sleep(2)

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
    parser.add_argument('--producer_binary', default='../cmake-build-release/sample_apps/test_producer/test_producer_app')
    parser.add_argument('--consumer_binary', default='../cmake-build-release/sample_apps/test_consumer/test_consumer_app')
    args = parser.parse_args()

    producer = args.producer_binary
    consumer = args.consumer_binary

    loop = asyncio.get_event_loop()
    try:
        loop.run_until_complete(run(producer, consumer))
    except KeyboardInterrupt:
        print('Stopped (KeyboardInterrupt)')


if __name__ == '__main__':
    main()