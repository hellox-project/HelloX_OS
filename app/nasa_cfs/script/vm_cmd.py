#!/usr/bin/env python3
"""Helper to send a command to the HelloX VM CLI over telnet and print the reply."""
import socket
import sys
import time

HOST = "192.168.1.204"
PORT = 23

def main():
    cmd = sys.argv[1] if len(sys.argv) > 1 else "help"
    wait = float(sys.argv[2]) if len(sys.argv) > 2 else 3.0

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.settimeout(2)
    s.connect((HOST, PORT))

    # Send a couple of newlines first (per HelloX CLI quirk).
    s.sendall(b"\r\n\r\n")
    time.sleep(1)

    # Send the command.
    s.sendall(cmd.encode() + b"\r\n")

    # Read whatever comes back until timeout.
    s.settimeout(1.0)
    out = b""
    end = time.time() + wait
    while time.time() < end:
        try:
            chunk = s.recv(4096)
            if not chunk:
                break
            out += chunk
        except socket.timeout:
            continue
        except Exception:
            break

    s.close()
    sys.stdout.write(out.decode(errors="replace"))

if __name__ == "__main__":
    main()
