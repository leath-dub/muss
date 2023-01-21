#include <stddef.h>
#include <stdio.h>
#include <sys/queue.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>

/**
 * Temp for development, lowdown.h is all you really want
 * #include "lowdown.h"
 */
#include "../lib/lowdown/lowdown.h"
// #include "lowdown.h"
#define BUFSIZE 0x2000
#define BUFMAX 0x5F5E100
#define MAXMSG 50
#define MSGBUFLEN 50

typedef struct {
    char *buf;
    size_t sz;
} Buf;

Buf *markdown_to_html(char *filename);
int free_buf(Buf *buf);

static char msgbuf[MSGBUFLEN] = {0};

/**
 * Function returns a reference to a heap allocated Buf type,
 * it is up to the caller to free Buf and Buf->buf
 */
Buf *
markdown_to_html(char *filename) {
    Buf *result;
    char *buf;
    size_t bufsz;
    FILE *fp;
    int rc;

    struct lowdown_opts opts;
    memset(&opts, 0, sizeof(struct lowdown_opts));
    opts.type = LOWDOWN_HTML;
    opts.feat = LOWDOWN_FOOTNOTES |
        LOWDOWN_AUTOLINK |
        LOWDOWN_TABLES |
        LOWDOWN_SUPER |
        LOWDOWN_STRIKE |
        LOWDOWN_FENCED |
        LOWDOWN_COMMONMARK |
        LOWDOWN_DEFLIST |
        LOWDOWN_IMG_EXT |
        LOWDOWN_METADATA;
    opts.oflags = LOWDOWN_HTML_HEAD_IDS |
        LOWDOWN_HTML_NUM_ENT |
        LOWDOWN_HTML_OWASP |
        LOWDOWN_SMARTY |
        LOWDOWN_STANDALONE;

    fp = fopen(filename, "r");
    if (fp == NULL) {
        strerror_r(errno, msgbuf, MSGBUFLEN);
        errx(EXIT_FAILURE, "%s: %s", msgbuf, filename);
    }
    rc = lowdown_file(&opts, fp, &buf, &bufsz, NULL);
    if (rc == 0) {
        fclose(fp);
        errx(EXIT_FAILURE, "error calling lowdown_file");
    }

    /* set the result */
    result = malloc(sizeof(Buf));
    result->buf = buf;
    result->sz = bufsz;

    /* clean up */
    fclose(fp);
    return result;
}

int
free_buf(Buf *buf) {
    if (buf == NULL) {
        warn("passed null reference to free_buf, not freeing");
        return 1;
    }
    /* free char buf */
    free(buf->buf);

    /* free the object */
    free(buf);

    return 0;
}

int
main(int argc, char **argv)
{
    if (argc > 1) {
        Buf *markdown = markdown_to_html(argv[1]);
        write(0, markdown->buf, markdown->sz);
        free_buf(markdown);
    } else {
        Buf *markdown = markdown_to_html("./test.md");
        write(0, markdown->buf, markdown->sz);
        free_buf(markdown);
    }
    return 0;
}
