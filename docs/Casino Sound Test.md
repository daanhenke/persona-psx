# Casino sound test

`CasinoSoundTestStep` (`0x8006D100`, 348 bytes) is the unused development sound-test handler. Up/down change a signed byte selection; the play button selects either the field or casino sound bank, closes sequence handle 14, opens the selected sequence at volume 127, and starts it. A sequence pointer of -1 marks the end of the list: selection zero moves to one, otherwise the selection moves back one entry.

The C uses a one-byte array view of the existing cursor symbol to preserve address reuse for byte updates. The shared inline player accepts a full-width bank value before the original sequence API narrows it to a signed halfword. Both bank branches retain their player call, which reproduces the original common call tail. The sentinel correction reads through the byte pointer directly; copying it to a local changes register allocation.

Acceptance requires all eleven complete JP1 binary SHA-256 checks. This debug handler uses the original PlayStation sequence APIs; it does not add native Linux audio or gameplay.
