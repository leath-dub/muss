#include "lib.h"
#include "option.h"
#include <err.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

void option_parse(struct option_t *o, int *i, char **args, int args_l)
{
    if (++(*i) < args_l) {
        o->value = args[*i];
    } else {
        errx(1, "argument required for option %s", o->name);
    }
}

int
dir_create(char *path)
{
    int rc;
    rc = mkdir(path, 0700);
    if (errno == EEXIST) {
        return 1;
    }
    errnox(rc < 0, path);
    return 1;
}

int
file_isvalid(char *path)
{
    int rc;
    struct stat st;
    rc = stat(path, &st);
    errnox(rc < 0, path);
    if (!S_ISREG(st.st_mode)) {
        errx(1, "%s is not a regular file", path);
    }
    return 1;
}

int
dir_isvalid(char *path)
{
    int rc;
    struct stat st;
    rc = stat(path, &st);
    errnox(rc < 0, path);
    if (!S_ISDIR(st.st_mode)) {
        errx(1, "%s is not a directory", path);
    }
    return 1;
}

int
parse_opts(int argc, char **argv, struct option_t *option_opts)
{
    if (!(argc > 1)) {
        return -1;
    } else {
        for (int i = 0; i < argc - 1; ++i) {
            for (int o = 0; *(int *)&option_opts[o] != 0; ++o) {
                if (!strcmp(argv[i + 1], option_opts[o].name)) {
                    option_parse(&option_opts[o], &i, argv + 1, argc - 1);
                }
            }
        }
    }
    return 1;
}

void
validate_opts(struct option_t *option_opts)
{
    foreach (struct option_t, o, option_opts) {
        o->valid(o->value);
    }
}

struct option_t *
get_option(char *option_name, struct option_t *option_opts)
{
    foreach (struct option_t, o, option_opts) {
        if (!strcmp(o->name, option_name)) {
            return o;
        }
    }
    return NULL;
}

/*
int
main(int argc, char **argv)
{
    parse_opts(argc, argv);

    foreach (struct option, o, validate_opts) {
        printf("%s, %s\n", o->name, o->value);
    }

    validate_opts(option_opts);

    return 0;
}
*/
