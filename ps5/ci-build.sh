#!/bin/bash

cd ps5
make -j"$(nproc)" package || exit 1
