/*
 * tests.c  --  Host-side unit tests for all four building blocks.
 *
 * These modules are pure, portable C with no hardware dependency, so they
 * compile and run on your laptop with gcc -- which is exactly how you unit
 * test firmware logic before it ever touches a board. Run: `make test`.
 */
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "ring_buffer.h"
#include "mem_pool.h"
#include "cmd_parser.h"
#include "state_machine.h"

static int g_checks = 0;
#define CHECK(cond) do {                                   \
        g_checks++;                                        \
        if (!(cond)) {                                     \
            printf("  FAIL: %s  (line %d)\n", #cond, __LINE__); \
            return 1;                                      \
        }                                                  \
    } while (0)

static int test_ring_buffer(void) {
    printf("[ring_buffer]\n");
    uint8_t storage[8];
    ring_buffer_t rb;

    CHECK(rb_init(&rb, storage, 8) == true);
    CHECK(rb_init(&rb, storage, 7) == false);   /* not power of two */
    CHECK(rb_is_empty(&rb));
    CHECK(rb_count(&rb) == 0);

    /* capacity is size-1 == 7 usable slots */
    for (int i = 0; i < 7; ++i) {
        CHECK(rb_put(&rb, (uint8_t)i) == true);
    }
    CHECK(rb_is_full(&rb));
    CHECK(rb_put(&rb, 99) == false);            /* rejected when full */
    CHECK(rb_count(&rb) == 7);

    uint8_t out;
    for (int i = 0; i < 7; ++i) {
        CHECK(rb_get(&rb, &out) == true);
        CHECK(out == (uint8_t)i);               /* FIFO order preserved */
    }
    CHECK(rb_is_empty(&rb));
    CHECK(rb_get(&rb, &out) == false);          /* empty */

    /* wrap-around: write, read, write past the end */
    for (int i = 0; i < 100; ++i) {
        CHECK(rb_put(&rb, (uint8_t)i));
        CHECK(rb_get(&rb, &out));
        CHECK(out == (uint8_t)i);
    }
    printf("  ok\n");
    return 0;
}

static int test_mem_pool(void) {
    printf("[mem_pool]\n");
    /* 4 blocks of 16 bytes */
    static uint8_t backing[4 * 16];
    mem_pool_t mp;

    CHECK(mp_init(&mp, backing, 16, 4) == true);
    CHECK(mp_init(&mp, backing, 2, 4) == false);   /* block < sizeof(void*) */
    CHECK(mp_free_count(&mp) == 4);

    void *a = mp_alloc(&mp);
    void *b = mp_alloc(&mp);
    void *c = mp_alloc(&mp);
    void *d = mp_alloc(&mp);
    CHECK(a && b && c && d);
    CHECK(a != b && b != c && c != d);             /* distinct blocks */
    CHECK(mp_free_count(&mp) == 0);
    CHECK(mp_alloc(&mp) == NULL);                  /* exhausted */

    /* the memory must actually be usable */
    memset(a, 0xAA, 16);
    memset(b, 0x55, 16);
    CHECK(((uint8_t *)a)[0] == 0xAA);
    CHECK(((uint8_t *)b)[0] == 0x55);

    mp_free(&mp, b);
    CHECK(mp_free_count(&mp) == 1);
    void *e = mp_alloc(&mp);
    CHECK(e == b);                                 /* LIFO reuse */
    CHECK(mp_free_count(&mp) == 0);
    printf("  ok\n");
    return 0;
}

static int last_led_state = -1;
static int cmd_led(int argc, char *argv[]) {
    if (argc >= 2 && strcmp(argv[1], "on") == 0)  { last_led_state = 1; return 0; }
    if (argc >= 2 && strcmp(argv[1], "off") == 0) { last_led_state = 0; return 0; }
    return 2;                                      /* bad usage */
}
static int cmd_ping(int argc, char *argv[]) { (void)argc; (void)argv; return 0; }

static int test_cmd_parser(void) {
    printf("[cmd_parser]\n");
    char line[] = "  led   on  \r\n";
    char *argv[CMD_MAX_TOKENS];
    int argc = cmd_tokenise(line, argv);
    CHECK(argc == 2);
    CHECK(strcmp(argv[0], "led") == 0);
    CHECK(strcmp(argv[1], "on") == 0);

    const cmd_entry_t table[] = {
        { "led",  cmd_led  },
        { "ping", cmd_ping },
    };
    CHECK(cmd_dispatch(table, 2, argc, argv) == 0);
    CHECK(last_led_state == 1);

    char line2[] = "led off";
    argc = cmd_tokenise(line2, argv);
    CHECK(cmd_dispatch(table, 2, argc, argv) == 0);
    CHECK(last_led_state == 0);

    char line3[] = "reboot now";
    argc = cmd_tokenise(line3, argv);
    CHECK(cmd_dispatch(table, 2, argc, argv) == -1);   /* unknown */
    printf("  ok\n");
    return 0;
}

static int test_state_machine(void) {
    printf("[state_machine]\n");
    frame_fsm_t fsm;
    fsm_init(&fsm);
    CHECK(fsm.state == ST_IDLE);

    CHECK(fsm_feed(&fsm, 'x') == ST_IDLE);      /* noise ignored */
    CHECK(fsm_feed(&fsm, '$') == ST_RECEIVING); /* start */
    CHECK(fsm_feed(&fsm, 'H') == ST_RECEIVING);
    CHECK(fsm_feed(&fsm, 'i') == ST_RECEIVING);
    CHECK(fsm_feed(&fsm, '\n') == ST_COMPLETE); /* delimited */
    CHECK(fsm.len == 2);
    CHECK(memcmp(fsm.payload, "Hi", 2) == 0);

    /* overflow path */
    fsm_init(&fsm);
    fsm_feed(&fsm, '$');
    for (int i = 0; i < FRAME_MAX_LEN; ++i) {
        fsm_feed(&fsm, 'A');
    }
    CHECK(fsm_feed(&fsm, 'A') == ST_ERROR);
    printf("  ok\n");
    return 0;
}

int main(void) {
    printf("=== Embedded C building-blocks test suite ===\n");
    int rc = 0;
    rc |= test_ring_buffer();
    rc |= test_mem_pool();
    rc |= test_cmd_parser();
    rc |= test_state_machine();
    if (rc == 0) {
        printf("\nALL TESTS PASSED  (%d checks)\n", g_checks);
    } else {
        printf("\nTESTS FAILED\n");
    }
    return rc;
}
