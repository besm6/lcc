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
    "bytecode",
])
def test_case(case, target):
    # Go to parent directory.
    os.chdir(pathlib.Path(__file__).parent / "..")

    #
    # Run preprocessor.
    #
    preproc = subprocess.Popen(["build/cpp",
                               f"-Itst/{target}",
                               f"tst/{case}.c"],
                               stdout=subprocess.PIPE)
    c_code = preproc.stdout.read().decode('utf-8')

    #
    # Run compiler.
    #
    compiler = subprocess.Popen(["build/rcc",
                                f"-target={target}",
                                "-g0"],
                                stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    compiler.stdin.write(c_code.encode('utf-8'))
    compiler.stdin.close()

    #
    # Read asm output.
    #
    actual_asm = compiler.stdout.read().decode('utf-8')
    #with open(f"tst/{target}/{case}.s", "w") as file:
    #    file.write(actual_asm)
    with open(f"tst/{target}/{case}.sbk") as file:
        expected_asm = file.read()

    #
    # Read stderr output.
    #
    actual_err = compiler.stderr.read().decode('utf-8')
    #with open(f"tst/{target}/{case}.2", "w") as file:
    #    file.write(actual_err)
    with open(f"tst/{target}/{case}.2bk") as file:
        expected_err = file.read()
        #print("--- expected_err=", expected_err)

    assert actual_err == expected_err
    assert actual_asm == expected_asm
