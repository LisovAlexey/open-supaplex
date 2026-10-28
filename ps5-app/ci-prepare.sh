#!/bin/bash
# Extra tools needed by ps5-native-app-boilerplate on top of the payload toolchain.

set -e

./ps5/ci-prepare.sh
DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
    git curl python3 python3-venv tar unzip
