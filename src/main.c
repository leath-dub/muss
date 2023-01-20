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

struct lowdown_buf *file_to_buf(char *filename);

struct lowdown_buf *
file_to_buf(char *filename)
{
    char *buf;
    size_t bufsz;
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

    FILE *fp = fopen(filename, "r");
    lowdown_file(&opts, fp, &buf, &bufsz, NULL);
    fwrite(buf, 1, bufsz, stdout);
    fclose(fp);
    free(buf);

    return NULL;
}

int
main(int argc, char **argv)
{
    /* void *lowdown_html_new(const struct lowdown_opts *); */
    // lowdown_buf_new(10);
    file_to_buf("./test.md");
    return 0;
}
