
from .monitor import LineMonitor
from .output_accumulator import OutputAccumulator


def test_simple_write_to_stdout(program):
	acc = OutputAccumulator()
	lines_to_send = [
		"run_main",
		"echo HOLA MUNDO",
		"exit"
	]

	for line in lines_to_send:
		print(f"Run '{line}'")
		program.send_line(line)

	monitor = LineMonitor()
	monitor.expect(
		r"user>\secho\sHOLA\sMUNDO\n\nHOLA MUNDO\nuser>\sexit")
	
	return acc, monitor
