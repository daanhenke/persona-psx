# Casino grid exit match

`func_80090A50` refunds eleven stored balances to casino money, clears and redraws each balance, refreshes palette state, and starts the exit animations before changing the casino state.

The 520-byte C implementation uses an indexed payout loop and an explicit money temporary. Together these preserve the original compiler register allocation and load-delay scheduling. Pointer-walking forms retained three differing instructions.

Validation: objdiff reports 100%; all eleven JP1 linked binary SHA-256 checks pass, independently rehashed against the saved reference hashes. See the local `hard-gridexit-validation.log`. No emulator or native-port gameplay claim is made.
