#include "lib.h"
#include "option.h"

#include <stdlib.h>
#include <err.h>
#include <stdint.h>

/**
 * Lowdown headers
 */
#include <sys/queue.h>
#include <stdio.h>
#include "../lib/lowdown/lowdown.h"

#include "../lib/mustach/mustach-cjson.h"
#include "../lib/include/cjson/cJSON.h"

struct option_t commandline_options[] = {
    {"html",   ".",             dir_isvalid},
    {"markup", ".",             dir_isvalid},
    {"vars",   "./config.json", file_isvalid},
    {"out",    "./out",         dir_create},
    0
};

typedef char *(*callback) (FILE *);

char *apply_template(cJSON *cjson, char *filename, callback call);

static int mustach_flags = Mustach_With_ErrorUndefined;

char *
apply_template(cJSON *cjson, char *filename, callback call)
{
    int rc;
    FILE *tmp;
    buffer *contents;
    char *result;
    long resultsz;

    contents = buffer_new_file(filename);
    tmp = tmpfile();
    rc = mustach_cJSON_file(
        buffer_raw(contents),
        buffer_get_length(contents),
        cjson, mustach_flags, tmp
    );
    free(contents);

    if (call) {
        rewind(tmp);
        result = call(tmp);
    } else {
        fseek(tmp, 0, SEEK_END);
        resultsz = ftell(tmp);
        rewind(tmp);
        result = calloc(1, resultsz + 1);
        errnox(result == NULL, NULL);
        fread(result, 1, resultsz, tmp);
    }
    fclose(tmp);

    return result;
}

struct task_t {
    char *outputfile;
    char *contents;
};

char *extensions[] = {
    ".md", ".html"
};
callback extension_handler[] = {
    markdown_to_html,
    NULL
};

/* returns a task or NULL */
struct task_t *
build_task(cJSON *cjson, char *inputfile, struct option_t *options)
{
    int pos;
    callback handler;
    struct task_t *task;

    pos = match_extensions(extensions, arrlength(extensions), inputfile);
    if (pos < 0) return 0;

    task = calloc(1, sizeof(struct task_t));

    task->outputfile = get_output_file(inputfile, ".html", get_option("out", options)->value);
    task->contents = apply_template(cjson, inputfile, extension_handler[pos]);

    return task;
}

/**
 * 2023-01-29
 * TODO: so we have the basis of what we need, all we need now is to
 * generate the inputs from a directory - recursively attempting to make
 * tasks on each file encountered (adding those tasks to an array). Then
 * once you have an array of tasks which hold the outputs in memory,
 * you just validate the output destinations ( make sure your not overriting
 * files and the outputs are unique maybe ?, make sure path is valid, etc )
 * then finally you generate the paths needed and write to the outputfiles,
 * thats it !
 */

int
main(int argc, char **argv)
{
    parse_opts(argc, argv, commandline_options);
    validate_opts(commandline_options);

    buffer *contents;
    struct task_t *task;

    contents = buffer_new_file(get_option("vars", commandline_options)->value);
    cJSON *cjson = cJSON_Parse(buffer_raw(contents));
    free(contents);

    task = build_task(cjson, "test.html", commandline_options);

    cJSON_Delete(cjson);

    // puts(task->outputfile);
    puts(task->contents);

    free(task->outputfile);
    free(task->contents);
    free(task);

    /*

    buffer *contents;
    contents = buffer_new_file(get_option("vars", commandline_options)->value);
    cJSON *cjson = cJSON_Parse(buffer_raw(contents));
    apply_template(cjson, "test.html", NULL);
    cJSON_Delete(cjson);
    free(contents);
    */

// mkpath("hello/guys/this/is/a/path", 26, 0700);
return 0;
}
