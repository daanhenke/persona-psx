# Manual reel line scan

`func_80085F08` at CASINO `0x80085F08` is a 1,092-byte routine that constructs twelve possible lines from the four outer reel groups and tests each active line against symbols 0–6. Bets below five select that many lines; larger bets add two lines per additional bet. The first four lines have five positions; the remaining lines have four positions plus a -1 sentinel.

The scan fills the synthetic positions with the symbol being tested, writes the matching-position flags, and calls `func_80087818` for candidates with at least three hits. Accepted lines enable the corresponding symbol sprites after clearing their synthetic-position flags. The result byte is reset for each symbol, and the scan continues through all seven symbols; this is distinct from the ordinary line scanner's early exit.

The inline sprite helper copies the original twelve-by-five table locally and takes a signed byte row. The outer scan uses an integer row and an unsigned byte symbol. These data types, the inline helper's local table, and the grouped synthetic-position assignments reproduce the original compiler's loops and register lifetimes. The call to `func_80087818` uses its existing four-integer signature.

Acceptance requires all eleven JP1 binary SHA-256 checks. This function still uses the original PS1 sprite interface and does not establish native Linux startup or gameplay.
