# ADV overlay data

The ADV overlay drives the isometric rooms. What it reads off the disc, and
what it does with it.

## Files

Eight paths go through `AdvSelectFile`, which switches on a 1-based kind. The
mapping is read off the jump table at `0x800647A0` and the template each case
copies, not inferred from the names:

| kind | path | contents |
|---|---|---|
| 1 | `\ADV\MES.BIN;1` | script text |
| 2 | `\ADV\EBG.BIN;1` | event backgrounds |
| 3 | `\ADV\BST.BIN;1` | not established |
| 6 | `\ADV\KAGE.BIN;1` | sound sequences - see below |
| 7 | `\ADV\BGM.BIN;1` | music sequences |
| 8 | `\ADV\BVB.BIN;1` | the VAB BGM plays through |
| 9 | `\ADV\SE.BIN;1` | sound effect sequences |
| 10 | `\ADV\SVB.BIN;1` | the VAB SE play through |

Kinds 4 and 5 are dead: both jump-table entries point at the default exit.

`KAGE` reads as 影, shadow, but `AdvSoundCommand` loads its entries into
`0x80118000` - the blob `SoundPlaySeq` indexes through the offset table at
`0x80118020`. So it holds sound data whatever the name suggests. Treat every
name in this table as the path and nothing more.

Another eight paths are loaded directly rather than through `AdvSelectFile`:

    \ADV\ADVCMD.BIN;1     (two copies of the path in the overlay)
    \ADV\ADVCHR.BIN;1     also queued by main's PreloadAdv
    \ADV\TYNCHR.BIN;1     read to 0x80120000 by the scene setup
    \ADV\TYN01.BIN;1
    \ADV\TYNSE.BIN;1
    \ADV\EV01.BGD;1  EV02.BGD  EV03.BGD     .BGD, not .BIN

and five shared ones from another directory:

    \CD0\P_DATA.BIN;1  B_DATA  S_DATA  C_DATA  D_DATA

## Scene packs

`E0.BIN` through `E3.BIN` are not script alone - a pack carries a whole scene:
the script, the room collision, and the actor tables. Main's `AdvResolveSceneLoc`
picks the pack and the entry within it from a u16 start-sector table, where the
gap between entry *i* and *i+1* is that scene's length.

The pack is read to `0x80100000`. What is known of the layout:

| offset | contents |
|---|---|
| `+0x06` | event flag A, `0xFFFF` for "no condition" |
| `+0x08` | event flag B |
| `+0x0A` | scene id when neither condition holds |
| `+0x0C` | scene id when both flags are set |
| `+0x0E` | scene id when A is clear and B is set |
| `+0x1F0` | actor definitions, `g_adv_actor_defs` |
| `+0xAF0` | second object table |
| `+0xBD0` | third object table |
| `+0xC20` | fourth object table |

`AdvPickSceneByFlags` reads the header as a truth table over its two flag
conditions and loads the `MES.BIN` for whichever branch wins, so the branch
decision and the load that follows it are the same routine.

### Actors

`g_adv_actor_defs` is indexed `[room * 0x120 + actor * 0x24]` - eight `0x24`-byte
records per room. Word 0 of a record is an event flag id, and it selects between
the record's *two* alternative descriptions: `0xFFFF` means unconditional and
takes the first, otherwise the low 9 bits index `g_event_flags` and a set bit
takes the second. That is the mechanism behind a room showing a different actor
once the story has moved past a point.

`AdvBuildActors` expands the current room's eight records into `g_adv_actors`,
a `0x2C`-per-entry runtime array. That array lives in the save-game work area
rather than in the scene buffer, so it outlives the buffer being reused.

### Four object tables, not one

The same routine builds four groups, each eight entries, each producing
`0x2C`-byte runtime records in the save-game work area. They differ in the size
of the packed record they come from:

| scene offset | record | runtime array | conditional |
|---|---|---|---|
| `+0x1F0` | `0x24` | `0x801F15D8` | yes, two variants |
| `+0xAF0` | `0x1C` | `0x801F1898` | yes, two variants |
| `+0xBD0` | `0x0A` | `0x801F1754` | no |
| `+0xC20` | `0x04` | `0x801F1A40` | no |

Only the first two carry an event-flag id and alternative descriptions; the
last two are read straight through. All four end up in the same `0x2C` runtime
shape and feed the display slots, so they are four kinds of scene object rather
than four unrelated tables - but which kind each is has not been established,
and the group names above are positions, not claims.

## The scene at run time

`g_adv_scene` (0x800BBB28) is set to 0x80100034 - 0x34 into the pack - and what
the overlay reaches through it are (count, table) pairs pointing back into the
pack:

| offset | record | key |
|---|---|---|
| `+0x08` / `+0x0C` | 12 bytes | a tile x and y |
| `+0x10` / `+0x14` | 8 bytes | three bytes |
| `+0x18` / `+0x1C` | 14 bytes | a u16 |
| `+0x20` | the room grid, one byte a tile | rows 32 bytes apart |

A tile record carries an event flag at `+0x04` that makes the trigger
conditional, mode bits at `+0x06`, and at `+0x08` the script pointer a caller
runs when the player steps on it. The room grid's stride is 32 while
`RoomRotatePoint` puts the far edge at 23, so it is the power of two above the
room rather than its width.

`g_dir_x` and `g_dir_y` are {0, 0, -1, 1} and {-1, 1, 0, 0}, which fixes the
four facings as up, down, left and right. Both are stored as bytes and added to
an unsigned coordinate, so the 0xFF entries are the -1s.

### Actor records

`g_adv_actors` (0x801F15D8) is 0x2C bytes an entry. `ActorsSetDepth` walks 25
of them, more than the eight a room's actor definitions expand to, so the array
runs on past them. What is pinned down so far:

| offset | contents |
|---|---|
| `+0x08` | the animation in play, from the pack's table at `+0xF0` |
| `+0x0C` | id, 0xFFFF while the slot is unused |
| `+0x0E` | world x and y; the renderer subtracts the camera from them |
| `+0x12` | base sort depth |
| `+0x14` | added to it: 0, or 0x20 for an actor standing behind another |
| `+0x16` | the previous value of `+0x17` |
| `+0x17` | a state the dispatchers switch on, values 0 to 3 |
| `+0x1C` | tile x, y |
| `+0x1E` | where the step in progress is taking it |
