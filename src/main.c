#include <stddef.h>
#include <stdio.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <err.h>
#include <string.h>
#include <fcntl.h>

/**
 * Temp for development, lowdown.h is all you really want
 */
#include "../lib/lowdown/lowdown.h"
#include "../lib/mustach/mustach-cjson.h"
#include "../lib/include/cjson/cJSON.h"
// #include "lowdown.h"
// #include "mustach.h"
// #include "cJSON.h"

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
static int flags = Mustach_With_ErrorUndefined;

/**
 * Function returns a reference to a heap allocated Buf type,
 * it is up to the caller to free Buf and Buf->buf
 */
Buf *
markdown_to_html(char *filename)
{
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
    if (result == NULL) {
        fclose(fp);
        strerror_r(errno, msgbuf, MSGBUFLEN);
        errx(EXIT_FAILURE, "%s", msgbuf);
    }
    result->buf = buf;
    result->sz = bufsz;

    /* clean up */
    fclose(fp);
    return result;
}

int
free_buf(Buf *buf)
{
    if (buf == NULL) {
        warn("passed null reference to free_buf, not freeing");
        return 1;
    }

    free(buf->buf);
    free(buf);
    return 0;
}

int
file_to_buf(char *filename, char **buf)
{
    int rc;
    int fd;
    long fsz;
    struct stat inode;

    /* get stats about file, we need the size */
    rc = stat(filename, &inode);
    if (rc == -1) {
        strerror_r(errno, msgbuf, MSGBUFLEN);
        errx(EXIT_FAILURE, "%s: %s", msgbuf, filename);
    }
    fsz = inode.st_size;

    *buf = malloc(fsz);
    if (*buf == NULL) {
        strerror_r(errno, msgbuf, MSGBUFLEN);
        errx(EXIT_FAILURE, "%s", msgbuf);
    }

    fd = open(filename, O_RDONLY);
    read(fd, *buf, fsz);
    close(fd);
    return fsz;
}

void
parse_json(char *template, char *filename)
{
    int rc;
    long fsz;
    struct stat inode;
    char *buf;
    int fd;
    cJSON *json;

    /* first we need to open the file and read the contents to a string */
    fsz = file_to_buf(filename, &buf);

    fd = open(filename, O_RDONLY);
    read(fd, buf, fsz);
    close(fd);

    json = cJSON_Parse(buf);
    if (json == NULL) {
        close(fd);
        errx(EXIT_FAILURE, "\033[31merror parsing json %s\n%s", filename, cJSON_GetErrorPtr());
    }

    rc = mustach_cJSON_fd(template, 0, json, flags, 0);
    if (rc < 0) {
        close(fd);
        fputs("\033[31m<-- ", stderr);
        errx(EXIT_FAILURE, "error parsing template %s", filename);
    }

    free(buf);
    free(json);
}



int
main(int argc, char **argv)
{
    /*
    if (argc > 1) {
        Buf *markdown = markdown_to_html(argv[1]);
        write(0, markdown->buf, markdown->sz);
        free_buf(markdown);
    } else {
        Buf *markdown = markdown_to_html("./test.md");
        write(0, markdown->buf, markdown->sz);
        free_buf(markdown);
    }
    */
    char *template;
    int sz = file_to_buf("template.html", &template);
    parse_json(template, "test.json");
    free(template);
    return 0;
}
