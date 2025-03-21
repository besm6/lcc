#include "c.h"

// clang-format off
#undef yy
#define yy \
xx(alpha/osf,     alphaIR) \
xx(besm6,         besm6IR) \
xx(bytecode,      bytecodeIR) \
xx(mips/irix,     mipsebIR) \
xx(sparc/sun,     sparcIR) \
xx(sparc/solaris, solarisIR) \
xx(symbolic/osf,  symbolic64IR) \
xx(symbolic/irix, symbolicIR) \
xx(symbolic,      symbolicIR) \
xx(x86/win32,     x86IR) \
xx(x86/linux,     x86linuxIR) \
xx(null,          nullIR)
// clang-format on

#undef xx
#define xx(a, b) extern Interface b;
yy

#undef xx
#define xx(a, b) { #a, &b },

Binding bindings[] = {
    // clang-format off
    yy
    { NULL, NULL },
    // clang-format on
};
#undef yy
#undef xx
