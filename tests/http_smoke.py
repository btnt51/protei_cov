"""Launch the real server, check HTTP routing and one completed call."""
import http.client
import json
import pathlib
import socket
import subprocess
import sys
import tempfile
import time

executable = str(pathlib.Path(sys.argv[1]).resolve())
with tempfile.TemporaryDirectory(prefix="protei_cov-http-") as directory:
    config = pathlib.Path(directory) / "config.json"
    config.write_text(json.dumps({"RMin": 4, "RMax": 5,
                                  "AmountOfOperators": 2, "SizeOfQueue": 15}))
    with socket.socket() as probe:
        probe.bind(("127.0.0.1", 0))
        port = probe.getsockname()[1]
    with open(pathlib.Path(directory) / "server.log", "w+") as log:
        process = subprocess.Popen([executable, str(port), str(config)],
                                   cwd=directory, stdout=log, stderr=log)
        try:
            deadline = time.monotonic() + 5
            while True:
                if process.poll() is not None:
                    raise RuntimeError("Server exited before accepting requests")
                try:
                    with socket.create_connection(("127.0.0.1", port), timeout=0.2):
                        break
                except OSError:
                    if time.monotonic() >= deadline:
                        raise RuntimeError("Server did not start in 5 seconds")
                    time.sleep(0.05)
            for path, expected_status, expected_body in [
                ("/missing", 404, "Not Found"),
                ("/phone=1234567", 200, "Completed"),
            ]:
                connection = http.client.HTTPConnection("127.0.0.1", port, timeout=10)
                try:
                    connection.request("GET", path)
                    response = connection.getresponse()
                    body = response.read().decode()
                    if response.status != expected_status or expected_body not in body:
                        raise AssertionError(f"{path}: {response.status}: {body}")
                finally:
                    connection.close()
        except Exception:
            log.flush()
            log.seek(0)
            print(log.read(), file=sys.stderr)
            raise
        finally:
            process.terminate()
            try:
                process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                process.kill()
                process.wait()
