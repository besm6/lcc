extern struct _iobuf {
    int             _cnt;
    unsigned char   *_ptr;
    unsigned char   *_base;
    int             _bufsiz;
    int             _flag;
    int             _file;
    int             _unused1;
    int             _unused2;
} _iob[];

#define	_IOREAD  01
#define	_IOWRT   02
#define	_IONBF   04
#define	_IOMYBUF 010
#define	_IOEOF   020
#define	_IOERR   040
#define	_IOSTRG  0100
#define	_IOLBF   0200
#define	_IORW    0400

#ifndef	NULL
#define	NULL	0
#endif

#define	FILE	struct _iobuf
#define	EOF	(-1)

#define	stdin	(&_iob[0])
#define	stdout	(&_iob[1])
#define	stderr	(&_iob[2])

#define	getc(p)		(--(p)->_cnt>=0 ? (int)(*(p)->_ptr++) : _filbuf(p))

#define putc(x, p)	(--(p)->_cnt >= 0 ?\
                        (int)(*(p)->_ptr++ = (unsigned char)(x)) :\
                        _flsbuf((unsigned char)(x), p))

int _filbuf(FILE *);
int _flsbuf(unsigned char, FILE *);
int fprintf(FILE *, const char *, ...);
int printf(const char *, ...);
int sprintf(char *, const char *, ...);
int fscanf(FILE *, const char *, ...);
int scanf(const char *, ...);
int sscanf(const char *, const char *, ...);
int fflush(FILE *);

void abort(void);
