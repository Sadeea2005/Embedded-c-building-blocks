/*
 * cmd_parser.h  --  In-place tokeniser for a UART/CLI command line
 *
 * Almost every embedded device grows a serial "shell": you type
 * "led on 3\r" and firmware acts on it. The parser here tokenises a
 * mutable C string IN PLACE (no allocation) by replacing separators with
 * '\0' and recording pointers to each token -- the same idea as strtok,
 * but reentrant and bounded. It then dispatches to a command table.
 */
#ifndef CMD_PARSER_H
#define CMD_PARSER_H

#include <stdint.h>
#include <stddef.h>

#define CMD_MAX_TOKENS 8

/* Split `line` in place into whitespace-separated tokens.
 * `argv` is filled with pointers into `line`; returns the token count
 * (argc), capped at CMD_MAX_TOKENS. `line` is modified. */
int cmd_tokenise(char *line, char *argv[CMD_MAX_TOKENS]);

/* A command handler receives argc/argv (argv[0] is the command name)
 * and returns 0 on success, non-zero on error. */
typedef int (*cmd_handler_t)(int argc, char *argv[]);

typedef struct {
    const char   *name;
    cmd_handler_t handler;
} cmd_entry_t;

/* Look up argv[0] in `table` (of `n` entries) and invoke its handler.
 * Returns the handler's result, or -1 if the command is unknown. */
int cmd_dispatch(const cmd_entry_t *table, size_t n,
                 int argc, char *argv[]);

#endif /* CMD_PARSER_H */
