/**
 * Every option has a name, value and validater function that is
 * applied on the value
 */
int dir_create(char *path);
int file_isvalid(char *path);
int dir_isvalid(char *path);

struct option_t {
    char *name;
    char *value;
    int (*valid) (char *);
};

int parse_opts(int argc, char **argv, struct option_t *option_opts);
void validate_opts(struct option_t *option_opts);

void option_parse(struct option_t *o, int *i, char **args, int args_l);
struct option_t *get_option(char *option_name, struct option_t *option_opts);
