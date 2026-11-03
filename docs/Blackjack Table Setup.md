# Blackjack table setup

`func_8007DBC8` (1,860 bytes) builds blackjack layouts, frames and textured objects, binds their transforms, and queues palette updates. Its three repeated object groups contain six, three and three records.

The transform at `D_8009B7B0 + 12` uses a scalar linker alias to retain the original independent field load and object address construction. This is a data-symbol view; the function remains C. The isolated linked bytes match, objdiff reports 100%, and all eleven JP1 SHA-256 checks pass in the pinned compiler environment.
