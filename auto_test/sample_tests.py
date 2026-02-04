
from .monitor import LineMonitor
from .output_accumulator import OutputAccumulator


def test_simple_write_to_stdout(program):
	acc = OutputAccumulator()
	lines_to_send = [
		"run_shell",
		"write 1 HOLA MUNDO",
		"exit"
	]

	for line in lines_to_send:
		print(f"Run '{line}'")
		program.send_line(line)

	monitor = LineMonitor()
	monitor.expect(
		r"user>\swrite\s1\sHOLA\sMUNDO\nHOLA\sMUNDOWritten\s10\sbytes\sto\sFD\s1\nuser>\sexit")
	
	return acc, monitor
