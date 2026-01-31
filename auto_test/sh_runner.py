# runscript.py
import subprocess
import os
import signal
import select

class KernelRun:
    def __init__(
        self,
        arch="riscv",
        ncpu=1,
        testing=False,
        use_gdb=False,
        clean=False,
        build_users=False,
        only_build=False,
        verbose=False,
        cwd=".",
        script="./run.sh",
        fs_path = None,
        fs_base_path = None,
    ):
        self.arch = arch
        self.ncpu = ncpu
        self.testing = testing
        self.use_gdb = use_gdb
        self.clean = clean
        self.build_users = build_users
        self.only_build = only_build
        self.verbose = verbose
        self.cwd = cwd
        self.script = script

        self.proc = None
        self._buffer = ""
        self.fs_path = fs_path
        self.fs_base_path = fs_base_path

        if not self.fs_base_path or not os.path.exists(self.fs_base_path):
            raise Exception("Not configured a valid fs base path for filesystem!")

    def reset_fs(self):
        # example: reset disk image / kernel
        if self.fs_path:
            if os.path.exists(self.fs_path):
                os.system(f"rm {self.fs_path}")

            os.system(f"cp {self.fs_base_path} {self.fs_path}")

    def _build_reset_cmd(self):
        cmd = [self.script]
        
        if self.build_users:
            cmd.append("-build_users")
        else:
            cmd.append("-c")

        if self.testing:
            cmd.append("-tests")

        cmd.append("-only_build")

        cmd.append(self.arch)
        cmd.append(str(self.ncpu))
        return cmd

    def _build_run_cmd(self):
        cmd = [self.script]

        if self.testing:
            cmd.append("-tests")

        if self.use_gdb:
            cmd.append("-d")
        
        if self.verbose:
            cmd.append("-v")

        cmd.append(self.arch)
        cmd.append(str(self.ncpu))
        return cmd


    # --------------------
    # SETUP PHASE
    # --------------------
    def reset(self):
        """
        Prepare build artifacts (clean/build/users/etc).
        Blocking. No streaming. No QEMU run.
        """
        cmd = self._build_reset_cmd()

        p = subprocess.Popen(
            cmd,
            cwd=self.cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
        )

        out, _ = p.communicate()
        if p.returncode != 0:
            raise RuntimeError(
                "Kernel reset/build failed:\n" + out
            )

        self.reset_fs()

    # --------------------
    # RUNTIME PHASE
    # --------------------
    def start(self):
        """
        Start kernel execution only.
        No clean/build flags allowed here.
        """
        cmd = self._build_run_cmd()

        self.proc = subprocess.Popen(
            cmd,
            cwd=self.cwd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            stdin=subprocess.PIPE,
            preexec_fn=os.setsid,
            bufsize=0,
            text=True,
        )

    def read_line(self, timeout=0.1):
        if not self.proc:
            return None

        rlist, _, _ = select.select([self.proc.stdout], [], [], timeout)
        if not rlist:
            return None

        ch = self.proc.stdout.read(1)
        if not ch:
            return None

        self._buffer += ch
        if "\n" not in self._buffer:
            return None

        line, self._buffer = self._buffer.split("\n", 1)
        return line + "\n"

    def kill(self):
        if self.proc:
            os.killpg(os.getpgid(self.proc.pid), signal.SIGTERM)
            self.proc = None
