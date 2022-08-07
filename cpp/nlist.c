#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpp.h"

extern int getopt(int, char *const *, const char *);
extern char *optarg;
extern int optind;
extern int verbose;
extern int Cplusplus;

Nlist *kwdefined;

#define NLSIZE 128

static Nlist *nlist[NLSIZE];

struct kwtab {
    char *kw;
    int val;
    int flag;
} kwtab[] = {
    { "if",       KIF,      ISKW },
    { "ifdef",    KIFDEF,   ISKW },
    { "ifndef",   KIFNDEF,  ISKW },
    { "elif",     KELIF,    ISKW },
    { "else",     KELSE,    ISKW },
    { "endif",    KENDIF,   ISKW },
    { "include",  KINCLUDE, ISKW },
    { "define",   KDEFINE,  ISKW },
    { "undef",    KUNDEF,   ISKW },
    { "line",     KLINE,    ISKW },
    { "error",    KERROR,   ISKW },
    { "pragma",   KPRAGMA,  ISKW },
    { "eval",     KEVAL,    ISKW },
    { "defined",  KDEFINED, ISDEFINED + ISUNCHANGE },
    { "ident",    KPRAGMA,  ISKW }, /* treat like pragma (ignored) */
    { "__LINE__", KLINENO,  ISMAC + ISUNCHANGE },
    { "__FILE__", KFILE,    ISMAC + ISUNCHANGE },
    { "__DATE__", KDATE,    ISMAC + ISUNCHANGE },
    { "__TIME__", KTIME,    ISMAC + ISUNCHANGE },
    { "__STDC__", KSTDC,    ISUNCHANGE },
    { NULL }
};

unsigned long namebit[077 + 1];
Nlist *np;

void setup_kwtab(void)
{
    struct kwtab *kp;
    Nlist *p;
    Token t;
    static Token deftoken[1] = { { NAME, 0, 0, 0, 7, (uchar *)"defined" } };
    static Tokenrow deftr    = { deftoken, deftoken, deftoken + 1, 1 };

    for (kp = kwtab; kp->kw; kp++) {
        t.t     = (uchar *)kp->kw;
        t.len   = strlen(kp->kw);
        p       = lookup(&t, 1);
        p->flag = kp->flag;
        p->val  = kp->val;
        if (p->val == KDEFINED) {
            kwdefined = p;
            p->val    = NAME;
            p->vp     = &deftr;
            p->ap     = 0;
        }
    }
}

Nlist *lookup(Token *tp, int install)
{
    unsigned int h;
    Nlist *p;
    uchar *cp, *cpe;

    h = 0;
    for (cp = tp->t, cpe = cp + tp->len; cp < cpe;)
        h += *cp++;
    h %= NLSIZE;
    p = nlist[h];
    while (p) {
        if (*tp->t == *p->name && tp->len == p->len &&
            strncmp((char *)tp->t, (char *)p->name, tp->len) == 0)
            return p;
        p = p->next;
    }
    if (install) {
        p        = new (Nlist);
        p->vp    = NULL;
        p->ap    = NULL;
        p->flag  = 0;
        p->val   = 0;
        p->len   = tp->len;
        p->name  = newstring(tp->t, tp->len, 0);
        p->next  = nlist[h];
        nlist[h] = p;
        quickset(tp->t[0], tp->len > 1 ? tp->t[1] : 0);
        return p;
    }
    return NULL;
}
