# runner.py
import time

class TestFailure(Exception):
    pass


STARTED_LINE= "[TEST] interactive test shell ready"
EXITED_LINE = "[TEST] interactive test shell exited"
LEN_STARTED = len(STARTED_LINE)
CAN_EXEC_COMMAND_LINE = "[tester command]>"
EXECUTED_COMAND_LINE = "[TEST] interactive test shell command executed:"


def wait_start(program, timeout):
    start = time.time()

    while time.time() - start < timeout:
        line = program.read_line()

        if line:
            if line == STARTED_LINE:
                print("")
                print(line)
                return

            # print(f"!'{line}' {len(line)} vs {len(STARTED_LINE)}")
            print(f"!{line[:-1]}", end="\r")

def wait_can_exec(program, timeout):
    start = time.time()
    while time.time() - start < timeout:
        line = program.read_line()
        if line == CAN_EXEC_COMMAND_LINE:
            print(line)
            return
        elif line:
            print(f"!!{line}", end="\r")

def wait_executed_line(program, monitor, timeout):
    start = time.time()
    while time.time() - start < timeout:
        line = program.read_line()
        if line == EXECUTED_COMAND_LINE:
            print("EXECUTED!")
            return
        elif line:
            monitor.feed(line+"\n")
            if monitor.done():
                return
    
    raise TestFailure("Timeout")

def run_test(program, monitor, timeout=5):
    program.reset()
    program.start()

    try:
        wait_start(program, timeout);

        wait_can_exec(program, timeout);
        print("Now start test, send 'run_tests'");
        program.send_line("run_tests")
        wait_executed_line(program, monitor, timeout);

    finally:
        program.kill()
