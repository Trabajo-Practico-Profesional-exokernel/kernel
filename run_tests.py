# test_basic.py
# from qemu import Qemu
from auto_test.monitor import LineMonitor
from auto_test.runner import run_test,TestFailure
from auto_test.sh_runner import KernelRun

def test_boot():
    # program = Qemu([
    #     "qemu-system-x86_64",
    #     "-kernel", "kernel.bin",
    #     "-nographic"
    # ])

    program = KernelRun(
        arch="riscv",
        ncpu=1,
        testing=True,
        clean=True,
        use_gdb = False
    )

    mon = LineMonitor()
    mon.expect(r"Interactive shell ready")
    mon.expect(r">")

    run_test(program, mon)

if __name__ == "__main__":
    try:
        test_boot()
        print("OK")
    except TestFailure as e:
        print("FAIL:", e)

