#!/bin/bash

set -e

./compile-all.sh
./dump-all.sh
./measure-all.sh > tmp1

./compile-all.sh -O
./dump-all.sh
./measure-all.sh --nofilename > tmp2

paste tmp1 tmp2
