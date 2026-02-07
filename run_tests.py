# test_basic.py
# from qemu import Qemu
from auto_test.runner import run_test,TestFailure
from auto_test.sh_runner import KernelRun

from auto_test.sample_tests import *

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
        use_gdb = False,
        fs_base_path = ".kernel_disk/disk_base.img",
        fs_path = ".kernel_disk/disk_test.img",
    )

    run_test(program, test_simple_write_to_stdout)

if __name__ == "__main__":
    try:
        test_boot()
        print("OK")
    except TestFailure as e:
        print("FAIL:", e)

