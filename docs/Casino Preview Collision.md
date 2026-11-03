# Casino preview collision

`func_80091EB8` resolves contact between the two preview objects and their
horizontal and vertical boundary responses. It preserves the separate 25,
24 and 21 unit contact thresholds, signed active-state checks, collision
latches, shared contact flags, sequence calls and the order of the two
vertical-response callbacks.

The first velocity exchange scales both original x velocities by 1.2. The
second exchanges y velocities and negates each object's value at offset
0x98. A horizontal boundary response first multiplies x velocity by -1.0,
then by 0.13, then adds it to that value. These remain separate floating-point
operations. Two double temporaries and explicit scalar views of the shared
state reproduce the original compiler's loads and stores; no assembly body
or artificial stack padding is used. The first 112 bytes of the motion-tail
view contain the previous transform and three vectors and are not modified.

The recovered function is 1,244 bytes with the original compiler. Acceptance
requires all eleven complete JP1 binary hashes, beyond the isolated linked
comparison. The native test covers 389,376 combinations of contact and boundary
thresholds, positive and negative positions, signed flag values including
128–255, velocity signs and signed zero. Controlled callbacks change state
to verify reloads and ordering; complete storage and event traces are checked.
It passes optimized Linux and AddressSanitizer/UndefinedBehaviorSanitizer
builds with recovery disabled.

The fixture supplies contiguous storage and linker aliases for the overlapping
PS1 data views. Compile the recovered source separately from the fixture so
host interprocedural optimization cannot treat those externally aliased views
as independent objects. The callbacks replace audio and vertical response
execution; these tests do not demonstrate native game startup or rendering.

```sh
cc -std=c99 -O2 -fno-builtin -funsigned-char -fno-strict-aliasing \
  -Iinclude -Iinclude/psyq tests/native/casino_previewcollision.c src/casino/game/previewcollision.c \
  -o /tmp/previewcollision
/tmp/previewcollision
```
