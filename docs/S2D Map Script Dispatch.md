# S2D map script dispatch

`func_8009C07C` interprets one command from the current map-script pointer. The 652-byte routine handles event flags, conditional and random jumps, message windows, indicators, and scene transitions. Ordinary bytes advance the pointer; the stop command returns without saving a new pointer.

The generated 73-entry switch table occupies the original 292 bytes at `0x800650FC`. The shared unconditional-jump case sits inside the random-jump branch so the original compiler emits the same common destination block. This unusual case placement is intentional.

Isolated comparison uses the original jump-table address. Full acceptance additionally requires 100% objdiff code and all eleven linked JP1 SHA-256 checks; the local validation log records that result. No native Linux backend or gameplay testing is added.

`func_8009BCBC` (736 bytes) selects a conditional map-event record, checks directional and interaction restrictions, and dispatches map exits, dialogue, scripts, or state transitions. Its explicit no-op case 6 preserves the seven-entry table at `0x800650DC`; the following original four padding bytes remain separate. The fixed map-state addresses reflect the original PS1 memory layout.

Both functions passed 100% objdiff code matching and all eleven JP1 linked binary SHA-256 checks, independently rehashed after the combined build. The matching code total increases by 1,388 bytes. The jump tables are generated C data; the progress report retains its own conservative data-match accounting.
