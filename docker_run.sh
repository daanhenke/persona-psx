#!/bin/bash
# Run a command in the build container, with the repo mounted at /app.
#   ./docker_run.sh make check
set -e
if [ $# -eq 0 ]; then
    echo "Error: No command provided."
    echo "Usage: $0 <command> (e.g., make, make clean)"
    exit 1
fi
docker run --platform linux/amd64 -it --rm -v "$(pwd)":/app -w /app \
    persona-psx-buildenv "$@"
