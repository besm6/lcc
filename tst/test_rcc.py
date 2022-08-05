import pytest
import os
import subprocess
import pathlib

@pytest.mark.parametrize("case", [
    "8q",
    "array",
    "cf",
    "cq",
    "cvt",
    "fields",
    "front",
    "incr",
    "init",
    "limits",
    "paranoia",
    "sort",
    "spill",
    "stdarg",
    "struct",
    "switch",
    "wf1",
    "yacc",
])
@pytest.mark.parametrize("target", [
    "x86/linux",
])
def test_case(case, target):
    # Go to parent directory.
    os.chdir(pathlib.Path(__file__).parent / "..")
    #print("---", target, case)

    #
    # Run preprocessor.
    #
    preproc = subprocess.Popen(["build/cpp", f"tst/{case}.c"], stdout=subprocess.PIPE)
    c_code = preproc.stdout.read().decode('utf-8')

    #
    # Run compiler.
    #
    compiler = subprocess.Popen(["build/rcc", f"-target={target}", "-g0"],
                                stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    compiler.stdin.write(c_code.encode('utf-8'))
    compiler.stdin.close()

    #
    # Read asm output.
    #
    actual_asm = compiler.stdout.read().decode('utf-8')
    #print("--- actual_asm=", actual_asm)
    with open(f"{target}/tst/{case}.sbk") as file:
        expected_asm = file.read()
        #print("--- expected_asm=", expected_asm)

    assert actual_asm == expected_asm

    #
    # Read stderr output.
    #
    actual_err = compiler.stderr.read().decode('utf-8')
    #print("--- actual_err=", actual_err)

    with open(f"{target}/tst/{case}.2bk") as file:
        expected_err = file.read()
        #print("--- expected_err=", expected_err)

    assert actual_err == expected_err
