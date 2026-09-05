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
| `+0xB00`, `+0xC00` | two further sections, not yet identified |

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
