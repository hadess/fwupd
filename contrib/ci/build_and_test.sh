#!/bin/sh
set -e

meson .. $@
ninja

# Build, Test & Install
ninja
ninja install DEST=/tmp/install_root/
