import socket

class GDBClient:
    def __init__(self, port):
        self.sock = socket.create_connection(("localhost", port))

    def send(self, cmd):
        pkt = f"${cmd}#{sum(cmd.encode()) % 256:02x}"
        self.sock.send(pkt.encode())

    def cont(self):
        self.send("c")
