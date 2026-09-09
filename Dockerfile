# Build environment for the Persona 1 (JP) PSX decompilation.
#
# Everything the build needs that is not committed to the repo. The compiler
# itself is committed - tools/bin/gcc-2.6.0 - and is a statically linked 32-bit
# i386 binary, so it runs here with no extra libraries; it does need the kernel
# to allow 32-bit executables, which linux/amd64 does.
#
# Nothing from the disc lives in this image. The executables the build is
# checked against come from a separate private repository, so this one can stay
# public and cacheable.
FROM python:3.12-slim-bookworm

ARG USERNAME=user
ARG UID=1000
ARG GID=1000

# binutils-mipsel: little-endian, which is what the PSX is. The big-endian
# `mips` packages the upstream Silent Hill image carries are no use here - the
# Makefile drives mipsel-linux-gnu-{as,ld,objcopy,objdump} and
# mipsel-linux-gnu-cpp by name.
RUN apt-get update && apt-get install -y --no-install-recommends \
    make \
    gcc \
    git \
    binutils-mipsel-linux-gnu \
    cpp-mipsel-linux-gnu \
    bchunk \
    p7zip-full \
    progress \
    sudo \
    && apt-get autoremove -y \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/* \
    && groupadd -g $GID $USERNAME \
    && useradd -m -u $UID -g $GID -s /bin/bash $USERNAME \
    && echo "$USERNAME ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/$USERNAME \
    && chmod 0440 /etc/sudoers.d/$USERNAME

# splat, spimdisasm, the permuter's dependencies and the rest, system-wide.
COPY requirements.txt /tmp/requirements.txt
RUN pip install --no-cache-dir -r /tmp/requirements.txt && rm /tmp/requirements.txt

# The Makefile defaults PYTHON to the local .venv, which a fresh checkout does
# not have. It takes this from the environment instead, so point it at the
# interpreter that already has splat and spimdisasm in it.
ENV PYTHON=python3

USER $USERNAME
WORKDIR /app
