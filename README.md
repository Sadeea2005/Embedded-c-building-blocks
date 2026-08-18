# Project 3 — Embedded C Building Blocks (100% portable, unit-tested)

Four low-level data structures that show up in almost every firmware codebase,
written in portable C with **no dynamic memory** and a host-side test suite you
can run on your laptop. This is where you prove you understand pointers,
memory, and constraints — the daily reality of firmware.

## Modules
| Module | What it is | Why firmware needs it |
|--------|-----------|-----------------------|
| `ring_buffer` | Fixed-size circular FIFO (SPSC) | Decouples a fast ISR producer from a slower main-loop consumer; used for UART RX/TX |
| `mem_pool` | Fixed-block allocator (free list) | Replaces `malloc`/`free`, which firmware bans to avoid heap fragmentation |
| `cmd_parser` | In-place line tokeniser + dispatch table | The serial "shell" almost every device grows |
| `state_machine` | Explicit FSM for a serial frame | Protocol parsing / debouncing / control flow, made reviewable |

## Build & test (runs on any PC — verified)
```
make test
```
Output ends with:
```
ALL TESTS PASSED  (359 checks)
```
Compiled with `-Wall -Wextra -Werror`, so it builds warning-clean.

## Design points worth mentioning
- Ring buffer capacity is a **power of two** → index wrap is a cheap `& (size-1)`
  instead of a modulo. One slot is kept empty so `head==tail` means *empty*
  without a separate count — safe between an ISR and the main loop with no lock.
- Memory pool uses an **intrusive free list**: each free block stores the "next"
  pointer *inside its own memory*, so bookkeeping costs zero extra bytes and
  alloc/free are O(1) with no fragmentation.
- The FSM keeps states and transitions **explicit** (an `enum` + `switch`)
  rather than a tangle of boolean flags — easier to review and to test.
