#!/bin/bash
# Installs the toolchain and the PS5 payload SDK (with SDL2) into /opt/ps5-payload-sdk.
# Meant to run inside an ubuntu:24.04 container (see ci-docker-run.sh).

set -e

PACBREW_RELEASE=v0.41.1

apt-get update
DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    bash clang-18 lld-18 llvm-18 make pkg-config wget ca-certificates zip file

wget -q -O /tmp/ps5-payload-dev.tar.gz \
    "https://github.com/ps5-payload-dev/pacbrew-repo/releases/download/${PACBREW_RELEASE}/ps5-payload-dev.tar.gz"
tar xf /tmp/ps5-payload-dev.tar.gz -C /
rm /tmp/ps5-payload-dev.tar.gz
