# A Retargetable C Compiler

This repo is a development of LCC compiler.
We started from version 4.2, as released by Chris Fraser and Dr. David Hanson
at [github.com/drh/lcc](https://github.com/drh/lcc).
A goal is to adapt the compiler to unusual architectures like BESM-6.

# Build
To build LCC from sources, you need to have C compiler, Git and CMake available on your system.
For example, on Ubuntu the required packages can be installed by command:

    sudo apt install build-essential git cmake

To build LCC, run:

    git clone https://github.com/sergev/lcc.git
    cd lcc
    cmake -B build
    make -C build

The following binaries will be created in the build directory:

 * rcc - backend of the C compiler
 * lcc - frontend to invoke the compiler from command line
 * cpp - C preprocessor

# Testing
Unit tests help to make sure the compiler is working as expected.
You need to install pytest first:

    pip3 install pytest

Run all unit tests:

    cd lcc
    pytest

Expected output:

    =================== test session starts ====================
    ...
    collected 36 items

    tst/test_rcc.py .................................... [100%]

    ==================== 36 passed in 0.39s ====================
