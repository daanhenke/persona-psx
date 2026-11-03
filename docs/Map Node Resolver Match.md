# Map node resolver match

`func_8009BF9C` in the JP1 S2D overlay now matches its original 224-byte implementation. It reads the current map's node table, emits zero for small literal entries, and resolves pointer entries through an optional event condition before copying the selected node's output byte. The table's 255 sentinel and 256-entry bound stop traversal.

The previous typed draft differed in five instruction words. Moving the counter increment into the non-sentinel path recovers the original branch-delay scheduling; the local counter has no observable use after termination. Node field offsets and the existing event-test ABI are preserved.

Validation uses the pinned offline GCC 2.6.0 environment and all eleven linked JP1 binary SHA-256 checks. This is matching PS1 source; native Linux runtime work remains separate.
