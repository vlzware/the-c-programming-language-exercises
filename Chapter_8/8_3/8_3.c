/*
Exercise 8-3. Design and write _flushbuf , fflush , and fclose .

- Based on 8_2
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>


#define OPEN_MAX 20 /* max #files open at once */

typedef struct _iobuf {
    int cnt;        /* characters left */
    char *ptr;      /* next character position */
    char *base;     /* location of buffer */
    int flag;       /* mode of file access */
    int fd;         /* file descriptor */
} MFILE;

MFILE _iob[OPEN_MAX];

#define mstdin  (&_iob[0])
#define mstdout (&_iob[1])
#define mstderr (&_iob[2])

enum _flags {
    _READ   = 01,   /* file open for reading */
    _WRITE  = 02,   /* file open for writing */
    _UNBUF  = 04,   /* file is unbuffered */
    _EOF    = 010,  /* EOF has occurred on this file */
    _ERR    = 020   /* error occurred on this file */
};

int _fillbuf(MFILE *);
// int _flushbuf(int, MFILE *);

#define mfeof(p)        ((p)->flag & _EOF) != 0)
#define mferror(p)      ((p)->flag & _ERR) != 0)
#define mfileno(p)      ((p)->fd)

#define mgetc(p)        (--(p)->cnt >= 0 \
                            ? (unsigned char) *(p)->ptr++ : _fillbuf(p))
// #define mputc(x,p)      (--(p)->cnt >= 0 \
//                             ? *(p)->ptr++ = (x) : _flushbuf((x),p))
#define mgetchar()      mgetc(mstdin)
// #define mputchar(x)     mputc((x), mstdout)


#define PERMS 0666  /* RW for owner, group, others */

MFILE *mfopen(char *, char *);
void error(char *, ...);
char *prog;                                 /* program name for errors */



/* play with a file using our routines */
int main(int argc, char *argv[]) {
    MFILE _iob[OPEN_MAX] = {                /* stdin, stdout, stderr */
        { 0, (char *) 0, (char *) 0, _READ, 0 },
        { 0, (char *) 0, (char *) 0, _WRITE, 1 },
        { 0, (char *) 0, (char *) 0, _WRITE | _UNBUF, 2 }
    };

    prog = argv[0];

    if (argc == 1) {
        printf("Usage: %s file\n", prog);
        exit(1);
    }

    int c;
    MFILE *fp;
/*
    if ((fp = mfopen(argv[1], "a")) == NULL)
        error("can't open %s", argv[1]);

    char *toappend = "this is new\n";
    while (toappend) {
        mputc(*toappend, fp);
        toappend++;
    }

    close(fp->fd);
*/
    if ((fp = mfopen(argv[1], "r")) == NULL)
        error("can't open %s", argv[1]);

    while ((c = mgetc(fp)) != EOF)
        putchar(c);

    free(fp->base);
    close(fp->fd);

    return 0;
}


MFILE *mfopen(char *name, char *mode)
{
    int fd;
    MFILE *fp;

    if (*mode != 'r' && *mode != 'w' && *mode != 'a')
        return NULL;
    for (fp = _iob; fp < _iob + OPEN_MAX; fp++)
        if ((fp->flag & (_READ | _WRITE)) == 0)
            break;                          /* found free slot */
    if (fp >= _iob + OPEN_MAX)              /* no free slots */
        return NULL;

    if (*mode == 'w')
        fd = creat(name, PERMS);
    else if (*mode == 'a') {
        if ((fd = open(name, O_WRONLY, 0)) == -1)
            fd = creat(name, PERMS);
        lseek(fd, 0L, 2);
    } else
        fd = open(name, O_RDONLY, 0);
    if (fd == -1)                           /* couldn't access name */
        return NULL;

    fp->fd = fd;
    fp->cnt = 0;
    fp->base = NULL;
    fp->flag = (*mode == 'r') ? _READ : _WRITE;

    return fp;
}

/* _fillbuf: allocate and fill input buffer */
int _fillbuf(MFILE *fp)
{
    int bufsize;

    if ((fp->flag&(_READ | _EOF | _ERR)) != _READ)
        return EOF;
    bufsize = (fp->flag & _UNBUF) ? 1 : BUFSIZ;
    if (fp->base == NULL)                   /* no buffer yet */
        if ((fp->base = (char *) malloc(bufsize)) == NULL)
            return EOF;                     /* can't get buffer */
    fp->ptr = fp->base;
    fp->cnt = read(fp->fd, fp->ptr, bufsize);
    if (--fp->cnt < 0) {
        if (fp->cnt == -1)
            fp->flag |= _EOF;
        else
            fp->flag |= _ERR;
        fp->cnt = 0;
        return EOF;
    }
    return (unsigned char) *fp->ptr++;
}

/* error: print an error message and die */
void error(char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    fprintf(stderr, "%s error: ", prog);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}
