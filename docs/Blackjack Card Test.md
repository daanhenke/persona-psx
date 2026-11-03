# Blackjack card-test screen

`func_80080270` at CASINO `0x80080270` is a 1,640-byte card-test and selection routine. It deals the initial player/dealer cards, animates the card objects into position, moves the selection cursor, and cycles the selected card through all 52 card values with repeat input and a sequence sound. Leaving the screen advances the casino state to 61.

The C reuses the same inline card-cycle operation as the adjacent card-selection routine. Its short dealing loop uses a separate counter from the animation loops. The staggered animation uses a linear negative step, and each selected-card address is built from the shoe position before adding the cursor. These ordinary C lifetime and address-expression choices reproduce the original GCC 2.6 register allocation.

Acceptance requires the full JP1 build to reproduce all 11 reference SHA-256 values. This is matching PS1 code; this change does not establish that the test screen is reachable in ordinary gameplay or provide a native Linux runtime.
