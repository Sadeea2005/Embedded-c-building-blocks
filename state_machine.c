#include "state_machine.h"

void fsm_init(frame_fsm_t *fsm) {
    fsm->state = ST_IDLE;
    fsm->len   = 0;
}

fsm_state_t fsm_feed(frame_fsm_t *fsm, char byte) {
    switch (fsm->state) {

    case ST_COMPLETE:
    case ST_ERROR:
        /* A previous frame/error is latched until the caller re-inits;
         * once re-armed we treat a start byte as a fresh frame. */
        if (byte == FRAME_START) {
            fsm->len   = 0;
            fsm->state = ST_RECEIVING;
        }
        break;

    case ST_IDLE:
        if (byte == FRAME_START) {
            fsm->len   = 0;
            fsm->state = ST_RECEIVING;
        }
        /* ignore noise before a start byte */
        break;

    case ST_RECEIVING:
        if (byte == FRAME_END) {
            fsm->state = ST_COMPLETE;          /* frame delimited */
        } else if (byte == FRAME_START) {
            fsm->len   = 0;                    /* restart on new start byte */
        } else if (fsm->len >= FRAME_MAX_LEN) {
            fsm->state = ST_ERROR;             /* payload overflow */
        } else {
            fsm->payload[fsm->len++] = byte;   /* accumulate */
        }
        break;
    }
    return fsm->state;
}

const char *fsm_state_name(fsm_state_t s) {
    switch (s) {
        case ST_IDLE:      return "IDLE";
        case ST_RECEIVING: return "RECEIVING";
        case ST_COMPLETE:  return "COMPLETE";
        case ST_ERROR:     return "ERROR";
        default:           return "?";
    }
}
