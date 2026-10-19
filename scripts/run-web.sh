#!/usr/bin/env bash

set -euo pipefail

repository_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
build_directory="${repository_root}/build/web"
server_port="${PORT:-0}"

if [[ ! "${server_port}" =~ ^[0-9]+$ ]] || ((server_port < 0 || server_port > 65535)); then
    echo "Error: PORT must be an integer between 0 and 65535." >&2
    exit 1
fi

for required_tool in emcmake emcc cmake python3; do
    if ! command -v "${required_tool}" >/dev/null 2>&1; then
        echo "Error: ${required_tool} is required for the browser build." >&2
        echo "Install and activate the Emscripten SDK, then rerun this script." >&2
        echo "See: https://emscripten.org/docs/getting_started/downloads.html" >&2
        exit 1
    fi
done

emcmake cmake -S "${repository_root}" -B "${build_directory}" \
    -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=OFF
cmake --build "${build_directory}" --parallel

# Binding and serving happen in one process, so an automatically selected port
# cannot be claimed by another process between selection and server startup.
exec python3 -c '
import functools
import http.server
import sys

directory = sys.argv[1]
requested_port = int(sys.argv[2])
handler = functools.partial(http.server.SimpleHTTPRequestHandler, directory=directory)

try:
    with http.server.ThreadingHTTPServer(("127.0.0.1", requested_port), handler) as server:
        actual_port = server.server_address[1]
        print(f"Serving ARENA CLASH at http://127.0.0.1:{actual_port}/arena-clash.html", flush=True)
        print("Press Ctrl+C to stop the server.", flush=True)
        server.serve_forever()
except KeyboardInterrupt:
    print("\nServer stopped.", flush=True)
except OSError as error:
    print(f"Error: unable to start the web server: {error}", file=sys.stderr)
    raise SystemExit(1)
' "${build_directory}" "${server_port}"
