# simctty - Toy OpenRISC 1000 simulation.
#
# This is a toy project, it does just enough to boot the bundled
# Linux binary.

# Copyright (c) 2014-2016 Tom Harwood

--------------------------------------------------------------------------------
# Requirements:

sudo apt-get install libgtest-dev libgflags-dev libgflags2

--------------------------------------------------------------------------------
# Native build:

mkdir build && cd build
cmake ..
make

--------------------------------------------------------------------------------
# Running the Linux binary (from the build dir):

simctty/simctty ../linux/vmlinux.bin

To exit the simuation, run "poweroff".

--------------------------------------------------------------------------------
# Tests:

ctest -V
