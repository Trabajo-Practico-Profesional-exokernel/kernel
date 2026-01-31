# runner.py
import time

class TestFailure(Exception):
    pass

def run_test(program, monitor, timeout=5):
    program.reset()
    program.start()
    start = time.time()

    try:
        while time.time() - start < timeout:
            line = program.read_line()
            if line:
                print(line)
                monitor.feed(line)
                if monitor.done():
                    return
        raise TestFailure("Timeout")
    finally:
        program.kill()
