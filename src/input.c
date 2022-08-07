#include "c.h"

static void pragma(void);
static void resynch(void);

static int bsize;
static unsigned char buffer[MAXLINE + 1 + BUFSIZE + 1];

unsigned char *incp;  /* current input character */
char *file;           /* current input file name */
char *firstfile;      /* first input file */
unsigned char *limit; /* points to last character + 1 */
char *line;           /* current line */
int lineno;           /* line number of current line */

void nextline(void)
{
    do {
        if (incp >= limit) {
            fillbuf();
            if (incp >= limit)
                incp = limit;
            if (incp == limit)
                return;
        } else {
            lineno++;
            for (line = (char *)incp; *incp == ' ' || *incp == '\t'; incp++)
                ;
            if (*incp == '#') {
                resynch();
                nextline();
            }
        }
    } while (*incp == '\n' && incp == limit);
}

void fillbuf(void)
{
    if (bsize == 0)
        return;
    if (incp >= limit)
        incp = &buffer[MAXLINE + 1];
    else {
        int n            = limit - incp;
        unsigned char *s = &buffer[MAXLINE + 1] - n;
        assert(s >= buffer);
        line = (char *)s - ((char *)incp - line);
        while (incp < limit)
            *s++ = *incp++;
        incp = &buffer[MAXLINE + 1] - n;
    }
    if (feof(stdin))
        bsize = 0;
    else
        bsize = fread(&buffer[MAXLINE + 1], 1, BUFSIZE, stdin);
    if (bsize < 0) {
        error("read error\n");
        exit(EXIT_FAILURE);
    }
    limit  = &buffer[MAXLINE + 1 + bsize];
    *limit = '\n';
}

void input_init(int argc, char *argv[])
{
    static int inited;

    if (inited)
        return;
    inited = 1;
    main_init(argc, argv);
    limit  = incp = &buffer[MAXLINE + 1];
    bsize  = -1;
    lineno = 0;
    file   = NULL;
    fillbuf();
    if (incp >= limit)
        incp = limit;
    nextline();
}

/* ident - handle #ident "string" */
static void ident(void)
{
    while (*incp != '\n' && *incp != '\0')
        incp++;
}

/* pragma - handle #pragma ref id... */
static void pragma(void)
{
    if ((curtok = gettok()) == ID && strcmp(token, "ref") == 0)
        for (;;) {
            while (*incp == ' ' || *incp == '\t')
                incp++;
            if (*incp == '\n' || *incp == 0)
                break;
            if ((curtok = gettok()) == ID && tsym) {
                tsym->ref++;
                use(tsym, src);
            }
        }
}

/* resynch - set line number/file name in # n [ "file" ], #pragma, etc. */
static void resynch(void)
{
    for (incp++; *incp == ' ' || *incp == '\t';)
        incp++;
    if (limit - incp < MAXLINE)
        fillbuf();
    if (strncmp((char *)incp, "pragma", 6) == 0) {
        incp += 6;
        pragma();
    } else if (strncmp((char *)incp, "ident", 5) == 0) {
        incp += 5;
        ident();
    } else if (*incp >= '0' && *incp <= '9') {
    line:
        for (lineno = 0; *incp >= '0' && *incp <= '9';)
            lineno = 10 * lineno + *incp++ - '0';
        lineno--;
        while (*incp == ' ' || *incp == '\t')
            incp++;
        if (*incp == '"') {
            file = (char *)++incp;
            while (*incp && *incp != '"' && *incp != '\n')
                incp++;
            file = stringn(file, (char *)incp - file);
            if (*incp == '\n')
                warning("missing \" in preprocessor line\n");
            if (firstfile == 0)
                firstfile = file;
        }
    } else if (strncmp((char *)incp, "line", 4) == 0) {
        for (incp += 4; *incp == ' ' || *incp == '\t';)
            incp++;
        if (*incp >= '0' && *incp <= '9')
            goto line;
        if (Aflag >= 2)
            warning("unrecognized control line\n");
    } else if (Aflag >= 2 && *incp != '\n')
        warning("unrecognized control line\n");
    while (*incp) {
        if (*incp++ == '\n') {
            if (incp == limit + 1) {
                nextline();
                if (incp == limit)
                    break;
            } else {
                break;
            }
        }
    }
}
