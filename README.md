# Embedded C Building Blocks

Core low-level data structures that show up in almost every firmware codebase,
written in **portable C with zero dynamic memory** and backed by a host-side
**unit-test suite (359 checks)**. Built to demonstrate the pointer, memory, and
constraint fundamentals that embedded / firmware work depends on.

## What's inside

| Module | What it is | Why firmware needs it |
|--------|-----------|-----------------------|
| `ring_buffer` | Fixed-size circular FIFO (single-producer / single-consumer) | Decouples a fast interrupt producer from a slower main-loop consumer (e.g. UART RX/TX) without locks |
| `mem_pool` | Fixed-block allocator using an intrusive free list | Replaces `malloc`/`free`, which firmware avoids due to heap fragmentation and non-deterministic timing |
| `cmd_parser` | In-place line tokeniser + command dispatch table | The serial command shell almost every device grows |
| `state_machine` | Explicit finite state machine for a serial frame | Protocol parsing / debouncing / control flow, made reviewable and testable |

## Build & run

Requires a C compiler (`gcc`) and `make`.

```bash
make test
```

Expected output:

```
=== Embedded C building-blocks test suite ===
[ring_buffer]
  ok
[mem_pool]
  ok
[cmd_parser]
  ok
[state_machine]
  ok

ALL TESTS PASSED  (359 checks)
```

The suite compiles with `-Wall -Wextra -Werror`, so it builds warning-clean.

### Without `make`

```bash
gcc -std=c11 -Wall -Wextra ring_buffer.c mem_pool.c cmd_parser.c state_machine.c tests.c -o tests
./tests
```

> On Windows, install a toolchain via [MSYS2](https://www.msys2.org)
> (`pacman -S mingw-w64-ucrt-x86_64-gcc make`) and run the commands above from
> the MSYS2 UCRT64 terminal.

## Design highlights

- **Ring buffer** capacity is a power of two, so index wrap is a branch-free
  `& (size - 1)` instead of a modulo. One slot is intentionally left empty so
  `head == tail` means *empty* without a separate count — safe to share between
  an ISR and the main loop with no lock.
- **Memory pool** stores each free block's "next" pointer *inside the block's
  own memory* (an intrusive free list), so bookkeeping costs zero extra bytes
  and allocation/free are O(1) with no fragmentation.
- **State machine** keeps states and transitions explicit (an `enum` + `switch`)
  rather than a tangle of boolean flags, which makes the behaviour easy to
  review and unit-test.

## Files

```
ring_buffer.c / .h      mem_pool.c / .h
cmd_parser.c / .h       state_machine.c / .h
tests.c                 Makefile
```

## License

Released under the MIT License — free to use and adapt.
