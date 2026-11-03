# Scene alert lighting

`func_800967DC` recovers 296 bytes of scene lighting and sound control as C.
Each invocation establishes neutral lighting. Mode 1 pulses on timer value 3;
mode 2 pulses on odd timer values. Both active modes decrement and clamp the
timer. Other modes leave it unchanged. A pulse changes the ambient color and
fade RGB, stops the current alert sequence, sets its volume and restarts it.

The two ordinary switch cases intentionally contain the same pulse body. The
original compiler merges them while retaining the required shared RGB pointer.
Sound callbacks can change the timer or sequence handle, so those values are
read again where the original code reads them. `PERSONA_ALERT_SEQUENCE`
defaults to the original fixed address and can be replaced by native fixtures.

The production C passes 4,320 native fixtures, both normally and with ASan/UBSan
and sanitizer recovery disabled. Coverage includes active and inactive modes,
timer boundaries, negative timers, signed sequence handles, callback changes,
call order and untouched neighboring fields. Acceptance also requires all
eleven complete JP1 SHA-256 checks; these are not gameplay or Linux boot tests.
