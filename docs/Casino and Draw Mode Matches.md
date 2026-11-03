# Casino and draw-mode matching C

This batch replaces 3,980 bytes of assembly with 14 matching C routines.

| Area | Functions | Bytes |
| --- | --- | ---: |
| Casino setup | `func_80088818` | 316 |
| S2D draw-mode slots | `D_80097CD4`, `func_80097DBC`, `func_80097DD4` | 92 |
| Blackjack menu reveal | `func_8007B8B0` | 356 |
| Grid advance and prompt | `func_800899F8`, `func_8008C16C` | 736 |
| Blackjack results and natural exit | `func_8007C158`, `func_8007A10C` | 720 |
| Reel line classification and acceptance | `func_800876FC`, `func_80087818` | 616 |
| Blackjack bet settlement | `func_8007C8E4`, `func_8007CA5C` | 768 |
| Blackjack hand fade | `func_8007F7E8` | 376 |

The former 284-byte `D_80097CD4` symbol contained four routines. Its boundaries are now explicit: 40-byte clear, 192-byte draw queue, 24-byte set and 28-byte unset. The draw queue remains assembly; only the three slot helpers count as new matching C. This correction increases the reported function denominator by three without changing total code bytes.

Compiler-sensitive details include the independent grid index/timer addresses, a cached halfword timer load, five-column reel input records, signed inner-loop width, and the original branch and switch structure. Existing SDK and casino argument types are preserved. Jump-table branches were compared with local assembler labels to avoid false PC-relative relocation differences; the final linked hashes verify the tables and addresses.

Validation uses the pinned offline build environment and original compiler: `make generate`, `make -j4 report`, and `make -j4 CHECKSUM=1 build`. All 14 routines score 100% in objdiff and all 11 JP1 binary SHA-256 checks pass. The output files were also independently rehashed. No emulator gameplay or native Linux runtime testing is implied.

Nearby split-hand fade, hand selection and draw-queue experiments remain outside production because they do not yet match. The existing hard backlog is deferred.
