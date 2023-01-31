#include <sys/types.h>
#include <stdint.h>
#include <stdio.h>

#define foreach(t, e, a) for (t *e = a; *(char *)e; ++e)
#define arrlength(a) sizeof(a) / sizeof(*a)

typedef void buffer;

typedef struct {
    char *value;
    long sz;
} charsz;

void errnox(int cond, char *extra);
long filesz(char *filename);
void cat(char *filename);
buffer *buffer_new(long sz);
void buffer_append(buffer **_buf, void *data, long sz);
void buffer_print(buffer *_buf);
void buffer_dump(buffer *_buf);
buffer *buffer_new_file(char *path);
buffer *buffer_new_fp(FILE *fp);
void *buffer_raw(buffer *_buf);
long buffer_get_length(buffer *_buf);
uint32_t match(char **patterns, int length, uint32_t mask, char ch, int column);
uint32_t match_patterns(char **patterns, int length, char *match);
int first_set_bit(uint32_t mask);
int match_extensions(char **extensions, int length, char *filename);
void mkpath(char *_path, int length, mode_t mode);

charsz *markdown_to_html(char *bin, long sz);
char *get_output_file(char *inputfile, char *extension, char *outdir);
