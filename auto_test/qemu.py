# qemu.py
import subprocess, os, signal, time

class Qemu:
    def __init__(self, config, cmd):
        self.cmd = cmd
        self.proc = None
        self.output = []
        self.fs_path = config["fs_path"] or None
        self.fs_base_path = config["fs_base_path"] or None

        if not self.fs_base_path or not os.path.exists(self.fs_base_path):
            raise Exception("Not configured a valid fs base path for filesystem!")

    def reset(self):
        # example: reset disk image / kernel
        if self.fs_path:
            if (os.path.exists(self.fs_path)):
                os.system(f"rm {self.fs_path}")
            os.system(f"cp {self.fs_base_path} {self.fs_path}")

    def start(self):
        self.reset_env()
        self.proc = subprocess.Popen(
            self.cmd,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            bufsize=0
        )


    def read_line(self, timeout=0.1):
        if not self.proc:
            return None
        r, _, _ = select.select([self.proc.stdout], [], [], timeout)
        if not r:
            return None
        line = self.proc.stdout.readline()
        if not line:
            return None
        decoded = line.decode(errors="replace")
        self.output.append(decoded)
        return decoded

    def kill(self):
        if self.proc:
            self.proc.terminate()
            try:
                self.proc.wait(timeout=2)
            except subprocess.TimeoutExpired:
                self.proc.kill()
