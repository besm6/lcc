#ifndef __FLOAT
#define __FLOAT

#define FLT_ROUNDS (__flt_rounds())
#define FLT_RADIX  2

#define FLT_DIG        6
#define FLT_EPSILON    1.19209289550781250000e-07
#define FLT_MANT_DIG   24
#define FLT_MAX        3.40282346638528860000e+38
#define FLT_MAX_10_EXP 38
#define FLT_MAX_EXP    128
#define FLT_MIN        1.17549435082228750000e-38
#define FLT_MIN_10_EXP -37
#define FLT_MIN_EXP    -125

#define DBL_DIG        15
#define DBL_EPSILON    2.22044604925031310000e-16
#define DBL_MANT_DIG   53
#define DBL_MAX        1.79769313486231570000e+308
#define DBL_MAX_10_EXP 308
#define DBL_MAX_EXP    1024
#define DBL_MIN        2.22507385850720140000e-308
#define DBL_MIN_10_EXP -307
#define DBL_MIN_EXP    -1021

#define LDBL_MANT_DIG   DBL_MANT_DIG
#define LDBL_EPSILON    DBL_EPSILON
#define LDBL_DIG        DBL_DIG
#define LDBL_MIN_EXP    DBL_MIN_EXP
#define LDBL_MIN        DBL_MIN
#define LDBL_MIN_10_EXP DBL_MIN_10_EXP
#define LDBL_MAX_EXP    DBL_MAX_EXP
#define LDBL_MAX        DBL_MAX
#define LDBL_MAX_10_EXP DBL_MAX_10_EXP

#endif /* __FLOAT */
