import asyncio
import argparse


async def run(producer_binary: str, consumer_binary: str):
    cmds = [f'./{producer_binary}']
    cmds += [f'./{consumer_binary}'] * 10

    coros = list()
    for i in range(len(cmds)):
        coros.append(run_command(cmds[i], i))
    await asyncio.gather(*coros)


async def run_command(cmd, index):
    print(f'running: {cmd}')
    proc = await asyncio.create_subprocess_shell(
        cmd,
        stdout=asyncio.subprocess.PIPE,
        stderr=asyncio.subprocess.PIPE)

    stdout, stderr = await proc.communicate()

    print(f'[{cmd!r} exited with {proc.returncode}]')
    if stdout:
        print(f'[stdout {index}]\n{stdout.decode()}')
    if stderr:
        print(f'[stderr {index}]\n{stderr.decode()}')


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--producer_binary', default='../cmake-build-debug/sample_apps/test_producer/test_producer_app')
    parser.add_argument('--consumer_binary', default='../cmake-build-debug/sample_apps/test_consumer/test_consumer_app')
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