# Hard blackjack control-flow matches

Two previously deferred functions now recover the original instruction streams:

- `func_8008230C` (52 bytes), card-pair classifier: return 2 directly when both bytes are at least 22, otherwise return 1. A shared result variable generated an extra return-register move; separate early failure returns generated a different branch structure.
- `func_800815D8` (268 bytes), blackjack confirm/advance input: assign the state directly inside the two input branches. GCC shares the stores while retaining the original v0 allocation. The previous temporary/goto formulation allocated the state in a0 instead.

Validation uses the original GCC 2.6.0 pipeline and all eleven JP1 linked binary SHA-256 checks. These are PS1 matching implementations; no native Linux runtime or gameplay testing is implied.
