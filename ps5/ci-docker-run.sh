#!/bin/bash
docker run --rm -v "$(pwd)":/src -w /src ubuntu:24.04 "$@"
