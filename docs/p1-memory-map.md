# Persona 1 (JP, SLPS-00500) — memory map and program structure

All facts below were recovered from the binary in Ghidra, not from documentation.

## Programs on disc

`SLPS_005.00` is only the resident core. The game is split across a resident main
EXE, six mutually-exclusive overlays, and four sub-EXEs launched via BIOS `Exec()`.

| binary | kind | vram | size | sha1 |
|---|---|---|---|---|
| `SLPS_005.00` | PS-EXE, resident | `0x80010000` | `0x46000` | `fd0af59ae98db83db3bb48a4e8e6e449e29a8e6f` |
| `DNG.BIN` | overlay id 4 | `0x800643a0` | `0x3c4f0` | `035300c55c8f468e3946e35aaa6e2018df36d7f5` |
| `BTLP.BIN` | overlay id 5 | `0x800643a0` | `0x91f0c` | `1b5253963b28369cfa5d0edf6a13bbd2224a1567` |
| `S2D.BIN` | overlay id 6 | `0x800643a0` | `0x551e0` | `fe25e5c709a046523af272449c9c1f7ee7d1b563` |
| `ADV.BIN` | overlay id 7 | `0x800643a0` | `0x58274` | `e843c4ec1cf3f8f7a0b362cca4f4691e7e836383` |
| `CASINO.BIN` | overlay id 8 | `0x800643a0` | `0x50408` | `45e69f62648c32f00a31d23504a9cbbe312b2766` |
| `NAME.BIN` | overlay id 9 | `0x800643a0` | `0x16900` | `2b14f81398bf13ce0ac50a8219d51ff1dabf3a92` |
| `EXE/ATLUS.EXE` | PS-EXE via `Exec()` | `0x80080000` | `0x25800` | `438655a6eb3b588e4b051df693040b513a4c56cf` |
| `EXE/OPEN.EXE` | PS-EXE via `Exec()` | `0x80080000` | `0x34800` | `b5a2b10cd7ed4ec2d01fddef304191a44c37769f` |
| `EXE/MOVIE.EXE` | PS-EXE via `Exec()` | `0x80080000` | `0x2a000` | `c94ca8e212628a7f5d1553a006341d76d5dcd28f` |
| `EXE/END.EXE` | PS-EXE via `Exec()` | `0x80080000` | `0x2e000` | `cb84681e52760c11e14b164f8bf6c828820d7bd9` |

## Overlays

Every overlay is `CdRead` to the **same base, `0x800643a0`**, so only one is resident
at a time. Layout of an overlay file:

    +0x00  u32 overlay_id      (4..9, matches the index in g_overlay_table)
    +0x04  code / data, loaded contiguously

`g_overlay_table` @ `0x8001004c` is an array of `{const char *name; void *entry;}`
indexed by `overlay_id - 3` (index 0 is a duplicate DNG entry used as the default):

| idx | id | name | entry | entry - base |
|---|---|---|---|---|
| 0 | - | `\DNG.BIN;1` | `0x80064d2c` | `0x98c` |
| 1 | 4 | `\DNG.BIN;1` | `0x80064d2c` | `0x98c` |
| 2 | 5 | `\BTLP.BIN;1` | `0x8007f408` | `0x1b068` |
| 3 | 6 | `\S2D.BIN;1` | `0x80089c04` | `0x25864` |
| 4 | 7 | `\ADV.BIN;1` | `0x8007d9ec` | `0x1964c` |
| 5 | 8 | `\CASINO.BIN;1` | `0x80065dac` | `0x1a0c` |
| 6 | 9 | `\NAME.BIN;1` | `0x800643a4` | `0x4` |

`NAME.BIN`'s entry is base+4 — immediately after the id dword, landing on an
`addiu $sp,$sp,-0x40` prologue. That is what pins the base address.

`LoadOverlay` @ `0x800119dc`:

```c
while (g_cd_busy != -1) {}
CdSearchFileLoc(&loc, entry->name);
while (!CdControlB(CdlSetloc, &loc, NULL)) {}
while (!CdRead((size + 0x7ff) >> 11, (u_long *)&g_overlay_base, 0x80)) {}
CdReadSync(0, NULL);
entry->entry();          /* never returns to the caller normally */
```

## Sub-EXEs

`LoadAndExecPsExe` @ `0x80011864` — `CdLoadPsExe(name, &exec)`, then `DrawSync`,
`ResetGraph`, `PadStop`, `StopCallback`, BIOS `Exec(&exec, 1, 0)`. Control **returns**,
after which `main` re-runs `ResetCallback` / `CdInit` / `PadInit` / `SsEnd`.

All four load at `0x80080000`, which **overlaps the overlay region**
(`0x800643a0`..`~0x800f6000`). That is why an overlay is always reloaded after a
sub-EXE returns.

Roles, from `main`'s state machine:

- `ATLUS.EXE` — publisher logo (boot, state `-1`)
- `OPEN.EXE` — title/opening; preceded by `LoadFileToAddr("\OPEN.BIN;1", 0x80180000)`
- `MOVIE.EXE` — FMV player, dispatched from **state 6 during gameplay**, not just boot
- `END.EXE` — endings; uses `EXE/END.BIN` (5 MB)

## Non-overlay data blobs

`OPEN.BIN` and `NAMEDT.BIN` are *not* overlays despite the `.BIN` extension - they
have an offset-table header (entry counts `0x2c` and `0x54`) rather than an id dword.
`OPEN.BIN` is loaded to `0x80180000`.

## Named so far

| addr | name |
|---|---|
| `0x8001128c` | `main` |
| `0x800119dc` | `LoadOverlay` |
| `0x80011864` | `LoadAndExecPsExe` |
| `0x80012bec` | `CdLoadPsExe` |
| `0x80011e3c` | `LoadFileToAddr` |
| `0x800622f0` | `g_cd_queue` (0x24-byte entries) |
| `0x80011cfc` / `0x80011dd0` | `CdQueueSubmit` / `CdQueueSubmitResolved` |
| `0x80012794` | `CdQueueDispatch` |
| `0x800126b8` | `CdQueueNextCallback` |
| `0x80012754` | `CdQueueReadyCallback` |
| `0x80012974` | `CdQueueClearCallback` |
| `0x80055c1c` / `0x80055c20` | `g_cd_stream_sectors` / `g_cd_stream_mode` (write-only) |
| `0x80012090` | `CdReadFileToAddr` |
| `0x80012144` | `CdReadToAddr` |
| `0x80012b2c` | `CdSearchFileLoc` |
| `0x8001004c` | `g_overlay_table` |
| `0x800643a0` | `g_overlay_base` |
| `0x80055c04` / `0x80055c38` / `0x80055c30` | `g_state_cur` / `g_state_next` / `g_state_prev` |
| `0x80055c10` | `g_cd_busy` |

## Toolchain evidence

Ghidra's Psy-Q signature database matched real SDK objects in the binary, confirming
a Psy-Q build: `LIBGS/GS_101,102,103,106,124,125.OBJ`, `LIBGTE/REG13.OBJ`,
`LIBSND/VS_MONO,VM_DOFF,VM_DON,SSQUIT,SCNOFF.OBJ`, `LIBSN/CLOSE,FSINIT,_OP_VNEW,_OP_VDEL.OBJ`.

The bundled `cc1-psx-26` self-identifies as
`GNU C 2.6.3 [AL 1.1, MM 40] Sony Playstation` — the correct compiler family.
Exact Psy-Q version still needs to be pinned empirically by matching.
