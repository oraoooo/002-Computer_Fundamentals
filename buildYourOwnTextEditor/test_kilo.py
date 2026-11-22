#!/usr/bin/env python3
"""kilo butuh TTY asli buat raw mode-nya, jadi gabisa langsung di-pipe.
makanya drive lewat pty beneran, terus cek file KILO_KEYLOG-nya kecatet
ada burst ketikan cepet, ada pause lama, sama backspace ke-flag bener."""
import pty
import os
import sys
import time
import threading
import struct
import fcntl
import termios
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
KILO_BIN = os.path.join(HERE, "kilo")


def main():
    if not os.path.exists(KILO_BIN):
        print(f"FAIL: {KILO_BIN} not built (run `make kilo` first)")
        return 1

    with tempfile.TemporaryDirectory() as tmp:
        scratch = os.path.join(tmp, "scratch.txt")
        with open(scratch, "w") as f:
            f.write("line one\n")
        log_path = os.path.join(tmp, "keystrokes.log")

        captured = bytearray()
        pid, fd = pty.fork()
        if pid == 0:
            os.environ["KILO_KEYLOG"] = log_path
            os.execvp(KILO_BIN, [KILO_BIN, scratch])

        fcntl.ioctl(fd, termios.TIOCSWINSZ, struct.pack("HHHH", 24, 80, 0, 0))
        stop = threading.Event()

        def drain():
            while not stop.is_set():
                try:
                    data = os.read(fd, 65536)
                    if not data:
                        break
                    captured.extend(data)
                except OSError:
                    break

        t = threading.Thread(target=drain, daemon=True)
        t.start()

        time.sleep(0.3)
        for ch in "hi":
            os.write(fd, ch.encode())
            time.sleep(0.03)
        time.sleep(0.6)  # sengaja pause lama, biar kaya mikir
        os.write(fd, b"\x7f")  # backspace
        time.sleep(0.1)
        # KILO_QUIT_TIMES = 3, jadi 3x Ctrl-Q pertama cuma warning doang,
        # baru yang ke-4 beneran keluar.
        os.write(fd, bytes([17, 17, 17, 17]))

        exited = False
        for _ in range(40):  # polling ~2s, jangan blocking wait biar ga hang
            time.sleep(0.05)
            try:
                if os.waitpid(pid, os.WNOHANG)[0] == pid:
                    exited = True
                    break
            except ChildProcessError:
                exited = True
                break

        stop.set()
        t.join(timeout=1)

        if not exited:
            os.kill(pid, 9)
            try:
                os.waitpid(pid, 0)
            except ChildProcessError:
                pass
            print("FAIL: kilo did not exit on its own after 4x Ctrl-Q "
                  f"(terminal output tail: {bytes(captured)[-300:]!r})")
            return 1

        if not os.path.exists(log_path):
            print(f"FAIL: no keylog written. terminal output tail: {bytes(captured)[-300:]!r}")
            return 1

        lines = [l.split("\t") for l in open(log_path).read().splitlines() if l.strip()]
        deltas = [float(l[0]) for l in lines]
        backspace_flags = [int(l[2]) for l in lines]

        ok = (
            len(lines) >= 4
            and any(d > 400 for d in deltas)
            and any(f == 1 for f in backspace_flags)
        )
        if ok:
            print(f"PASS: keystroke logger captured {len(lines)} events, "
                  f"including a >400ms pause and a flagged backspace")
            return 0
        print(f"FAIL: unexpected log contents: {lines}")
        return 1


if __name__ == "__main__":
    sys.exit(main())
