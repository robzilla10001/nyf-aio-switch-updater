#!/usr/bin/env bash
# Packages the contents of output/ into output/nyf-aio-switch-updater.zip

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
cd "$DIR/output" || exit 1

cp nyf-aio-switch-updater.elf release-nyf-aio-switch-updater.elf
rm -rf switch/aio-switch-updater/
mkdir -p switch/aio-switch-updater/
cp nyf-aio-switch-updater.nro switch/aio-switch-updater/
zip -FSr nyf-aio-switch-updater.zip switch/
