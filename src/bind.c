#include "c.h"

// clang-format off
#undef yy
#define yy \
xx(alpha/osf,    alphaIR) \
xx(mips/irix,    mipsebIR) \
xx(sparc/sun,    sparcIR) \
xx(sparc/solaris,solarisIR) \
xx(x86/win32,    x86IR) \
xx(x86/linux,    x86linuxIR) \
xx(bytecode,     bytecodeIR) \
xx(null,         nullIR)
// clang-format on
//xx(symbolic/osf, symbolic64IR)
//xx(symbolic/irix,symbolicIR)
//xx(symbolic,     symbolicIR)

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
