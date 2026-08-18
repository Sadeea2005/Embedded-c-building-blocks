/*
 * state_machine.h  --  Explicit finite state machine (FSM)
 *
 * Firmware is full of protocols and control flows best modelled as an FSM:
 * a UART frame parser, a button debouncer, a charging controller. Writing
 * the states and transitions EXPLICITLY (rather than a tangle of flags)
 * makes behaviour reviewable and testable.
 *
 * This example models a tiny serial-frame receiver:
 *     IDLE --('$')--> RECEIVING --(0..N data)--> ('\n') --> COMPLETE
 * Any framing violation drops back to IDLE. Feed it one byte at a time;
 * it never blocks and holds no dynamic memory.
 */
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include <stdint.h>
#include <stdbool.h>

#define FRAME_MAX_LEN 32
#define FRAME_START   '$'
#define FRAME_END     '\n'

typedef enum {
    ST_IDLE,        /* waiting for start byte            */
    ST_RECEIVING,   /* accumulating payload             */
    ST_COMPLETE,    /* a full frame is ready to read     */
    ST_ERROR        /* overflow / framing error          */
} fsm_state_t;

typedef struct {
    fsm_state_t state;
    char        payload[FRAME_MAX_LEN];
    uint32_t    len;
} frame_fsm_t;

void        fsm_init(frame_fsm_t *fsm);

/* Drive the machine with one received byte; returns the new state. */
fsm_state_t fsm_feed(frame_fsm_t *fsm, char byte);

const char *fsm_state_name(fsm_state_t s);

#endif /* STATE_MACHINE_H */
