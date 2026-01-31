# runner.py
import time

class TestFailure(Exception):
    pass


STARTED_LINE= "[TEST] interactive test shell ready\n"
EXITED_LINE = "[TEST] interactive test shell exited\n"

CAN_EXEC_COMMAND_LINE = "[tester command]>"
EXECUTED_COMAND_LINE = "[TEST] interactive test shell command executed:\n"


def wait_start(program, timeout):
    start = time.time()
    while time.time() - start < timeout:
        line = program.read_line()
        if line == STARTED_LINE:
            print("")
            print(line)
            return
        elif line:
            print(f"!{line[:-1]}", end="\r")

def wait_can_exec(program, timeout):
    start = time.time()
    while time.time() - start < timeout:
        line = program.read_line()
        if line == CAN_EXEC_COMMAND_LINE:
            print(line)
            return
        elif line:
            print(f"!!{line}")

def run_test(program, monitor, timeout=5):
    program.reset()
    program.start()

    try:
        wait_start(program, timeout);

        wait_can_exec(program, timeout);

        start = time.time()
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
