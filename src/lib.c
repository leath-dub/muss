#include "lib.h"

#include <sys/queue.h>
#include <stdio.h>
#include "../lib/lowdown/lowdown.h"

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>

#define mbuf_l 50
static char mbuf[mbuf_l];
void
errnox(int cond, char *extra)
{
    if (cond) {
        strerror_r(errno, mbuf, mbuf_l);
        errx(1, "%s %s", mbuf, extra);
    }
}

long
filesz(char *filename)
{
    int rc;
    struct stat st;
    rc = stat(filename, &st);
    errnox(rc < 0, filename);
    return st.st_size;
}

void
cat(char *filename)
{
    long sz;
    char *buf;
    FILE *fp;

    sz = filesz(filename);
    buf = calloc(sz, 1);
    fp = fopen(filename, "r");

    fread(buf, sz, 1, fp);
    fwrite(buf, sz, 1, stdout);

    fclose(fp);
    free(buf);
}

struct buffer {
    long _length;
    long _sz;
    uint8_t _data[]; // variable length byte array
};

void *
buffer_new(long sz)
{
    struct buffer *new = calloc(sizeof(struct buffer) + sz, 1);
    errnox(new == NULL, NULL);
    new->_sz = sz;
    new->_length = 0;
    return new;
}

void
buffer_append(buffer **_buf, void *data, long sz)
{
    long available;
    long n_sz;
    uint8_t *w_addr;
    struct buffer *buf = *_buf;

    available = buf->_sz - buf->_length;
    if (sz > available) {
        n_sz = buf->_sz + (2 * sz); // double what where writing
        buf = realloc(buf, sizeof(struct buffer) + n_sz);
        buf->_sz = n_sz;
        errnox(buf == NULL, NULL);
    }

    w_addr = (uint8_t *)(buf->_data + buf->_length);
    memmove(w_addr, data, sz); // copy the data
    buf->_length += sz; // new length

    *_buf = buf;
}

void
buffer_print(buffer *_buf)
{
    struct buffer *buf = _buf;
    fwrite(buf->_data, 1, buf->_length, stdout);
}

void
buffer_dump(buffer *_buf)
{
    struct buffer *buf = _buf;
    {
        int i = 0;
        uint8_t *iter = buf->_data;
        for (; i < buf->_sz; ++i, ++iter) {
            if (i + 1 == buf->_sz) {
                printf("%d", *iter);
            } else {
                printf("%d ", *iter);
            }
        }
    }
}

buffer *
buffer_new_file(char *path)
{
    FILE *fp;
    long sz;
    struct buffer *buf;

    sz = filesz(path);
    buf = buffer_new(sz);

    fp = fopen(path, "r");
    fread(buf->_data, 1, sz, fp);
    buf->_length = sz;

    fclose(fp);
    return buf;
}

buffer *
buffer_new_fp(FILE *fp)
{
    long sz;
    struct buffer *buf;

    fseek(fp, 0, SEEK_END);
    sz = ftell(fp);
    rewind(fp);

    buf = buffer_new(sz);
    fread(buf->_data, 1, sz, fp);
    buf->_length = sz;

    return buf;
}

void *
buffer_raw(buffer *_buf)
{
    return ((struct buffer *)_buf)->_data;
}

long
buffer_get_length(buffer *_buf)
{
    return ((struct buffer *)_buf)->_length;
}

uint32_t
match(char **patterns, int length, uint32_t mask, char ch, int column)
{
    uint32_t bit;
    for (int j = 0; j < length; j += 1) {
        bit = 1 << j;
        if (bit & mask) {
            if (patterns[j][column] != ch) {
                mask &= ~bit;
            }
        }
    }
    return mask;
}

/* returns the mask where each set bit corresponds to position of matching
 * string ( can be multiple ) */
uint32_t
match_patterns(char **patterns, int length, char *pattern)
{
    if (length > 32) {
        errx(1, "extension list is restricted to 32 items");
    }

    uint32_t mask;

    mask = ~(~0 << length);

    {   int col;
        char *ch = pattern;
        do {
            if (mask == 0) return 0; // this is whats makes this so fast
            mask = match(patterns, length, mask, *ch, col);
            ch += 1;
            col += 1;
        } while (*ch);
        mask = match(patterns, length, mask, *ch, col);
    }

    return mask;
}

/* return position of first set bit or 0 if none set */
int
first_set_bit(uint32_t mask)
{
    for (int i = 0; i < 32; i += 1) {
        if ((1 << i) & mask) {
            return i;
        }
    }
    return -1;
}

/* wrapper around match_patterns and first_set_bit */
int
match_extensions(char **extensions, int length, char *filename)
{
    uint32_t mask;
    foreach (char, sub, filename) {
        if (*sub == '.') {
            mask = match_patterns(extensions, length, sub);
            return first_set_bit(mask);
        }
    }
    return -1;
}

void
mkpath(char *_path, int length, mode_t mode)
{
    int rc;
    char *path;

    path = calloc(length + 1, sizeof(*_path));
    memmove(path, _path, length);
    foreach (char, ch, path) {
        if (*ch == '/') {
            *ch = 0;
            rc = mkdir(path, mode);
            errnox((rc < 0) && (errno != EEXIST), NULL);
            *ch = '/';
        }
    }
    rc = mkdir(path, mode);
    free(path);
    errnox((rc < 0) && (errno != EEXIST), NULL);
}

char *
markdown_to_html(FILE *tmpfile_ptr)
{
    int rc;
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
        LOWDOWN_DEFLIST |
        LOWDOWN_IMG_EXT |
        LOWDOWN_METADATA;
    opts.oflags = LOWDOWN_HTML_HEAD_IDS |
        LOWDOWN_HTML_NUM_ENT |
        LOWDOWN_HTML_OWASP |
        LOWDOWN_SMARTY |
        LOWDOWN_STANDALONE;

    rc = lowdown_file(&opts, tmpfile_ptr, &buf, &bufsz, NULL);
    if (rc == 0) {
        errx(1, "error calling lowdown_file");
    }

    return buf;
}

char *
get_output_file(char *inputfile, char *extension, char *outdir)
{
    int bufsz;
    int outdir_length;
    int extension_length;
    int pathsz;
    char *buf;
    char *writer;

    outdir_length = strlen(outdir);
    extension_length = strlen(extension);

    bufsz = outdir_length; // we will prepend with outdir, so add to size

    for (pathsz = 0; inputfile[pathsz]; pathsz += 1) {
        if (inputfile[pathsz] == '.') {
            bufsz += pathsz;
            break;
        }
    }

    bufsz += extension_length; // final length, just add extension
    buf = calloc(1, bufsz + 2); // 2 allows for redundency, we made need extra
                                // chars
    errnox(buf == NULL, NULL);

    writer = buf;

    if (outdir[outdir_length - 1] != '/') {
        memmove(writer, outdir, outdir_length);
        writer += outdir_length;
        *writer = '/';
        writer += 1;
    } else {
        memmove(writer, outdir, outdir_length);
        writer += outdir_length;
    }

    memmove(writer, inputfile, pathsz);
    writer += pathsz;

    memmove(writer, extension, extension_length);

    return buf;
}
