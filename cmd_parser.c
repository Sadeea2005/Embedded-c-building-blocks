#include "cmd_parser.h"
#include <string.h>

static int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

int cmd_tokenise(char *line, char *argv[CMD_MAX_TOKENS]) {
    int argc = 0;
    char *p = line;

    while (*p != '\0' && argc < CMD_MAX_TOKENS) {
        while (is_space(*p)) {         /* skip leading separators */
            *p++ = '\0';
        }
        if (*p == '\0') {
            break;
        }
        argv[argc++] = p;              /* start of a token */
        while (*p != '\0' && !is_space(*p)) {
            p++;                       /* walk to end of token */
        }
    }
    return argc;
}

int cmd_dispatch(const cmd_entry_t *table, size_t n,
                 int argc, char *argv[]) {
    if (argc == 0) {
        return -1;
    }
    for (size_t i = 0; i < n; ++i) {
        if (strcmp(table[i].name, argv[0]) == 0) {
            return table[i].handler(argc, argv);
        }
    }
    return -1;                         /* unknown command */
}
