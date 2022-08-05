typedef unsigned long jmp_buf[16];

void longjmp(jmp_buf env, int val);
int _setjmp(jmp_buf env);

#define setjmp _setjmp
