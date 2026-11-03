# Casino card fill and initialization

`func_80082658` (248 bytes) fills a byte array with 255, then draws random
values until each passes the existing occurrence-limit check. The retry
loop preserves the original signed remainder operation and rejection behavior.
It does not add a retry limit or alter invalid-input behavior.

`func_8008D39C` (296 bytes) loads the game-five VRAM stages at timer values
zero, one and two, initializes sound and display state at three, then advances
to state eighteen at timer nineteen. Its timer tests remain independent
because the called routines can affect shared state.

Both functions require 100% objdiff and all eleven JP1 linked binary SHA-256
checks using the pinned original compiler before acceptance.
