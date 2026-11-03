# Complete grid rotation

`func_8008B10C` at CASINO `0x8008B10C` is a 624-byte routine that updates the visible faces of all twelve grid models and starts their half-turn animations. The first two rows use the vertical model group and rotate about Y; the last two rows use the horizontal group and rotate about X. The speed argument determines the animation duration as `2048 / speed`.

Direction 1 selects face 1, texture indices 0 and 1, and an angle of -2048. Direction -1 selects face 0, texture indices 4 and 8, and an angle of 2048. Other direction values retain the zero-initialized parameters. Before walking the models, the routine restarts casino sequence 10.

The texture indices, face selectors and angle are signed shorts; the model index is an unsigned byte. Their types and initializer order reproduce the original compiler's register allocation without assembly or compiler overrides. Vertical and horizontal animation calls retain their original axis-specific arguments.

Acceptance requires all eleven JP1 binary SHA-256 checks. The routine still depends on the original PS1 graphics and sound interfaces; it does not establish native Linux gameplay.
