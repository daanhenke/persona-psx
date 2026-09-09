#!/bin/bash
# Build the container the CI workflows run in. Push it with:
#   echo "$GITHUB_TOKEN" | docker login ghcr.io -u <user> --password-stdin
#   docker push ghcr.io/daanhenke/persona-psx-buildenv:main
set -e
docker build --platform linux/amd64 -f Dockerfile \
    -t ghcr.io/daanhenke/persona-psx-buildenv:main \
    -t persona-psx-buildenv .
