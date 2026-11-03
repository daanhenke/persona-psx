# Coin shop screen

`CoinShopOpen` now builds from C by default. Its 1,420 bytes match the original JP1 function, including linked addresses; all 11 complete game binary SHA-256 checks pass.

The routine builds eight stock rows, a sixteen-cell separator, item and price details, the coin balance, the count selector and the scrolling arrows. It captures the selected stock index before opening the screen, while later cursor, arrow and scroll positions read the current menu state.

The separator loop retains two equivalent writes under an index condition. GCC 2.6 removes the branch, but the extra index use changes register allocation for the two setup loops. Removing the condition leaves thirteen instruction differences. No assembly or instruction data is embedded in the C.

The Linux fixture executes the actual source with synthetic data and observed UI calls. Its 432 cases cover every visible row at nine scroll positions, three coin balances, both arrow boundaries, and a callback that changes the menu after selection is captured. Optimized and UndefinedBehaviorSanitizer builds pass 109,729 checks. The test also checks that separator writes leave neighbouring cells untouched and arrow changes preserve other attributes.

```sh
cc -std=gnu99 -O2 -Wno-error=implicit-function-declaration \
  -fno-builtin -funsigned-char -fno-strict-aliasing \
  -ffunction-sections -fdata-sections -Wl,--gc-sections \
  -Iinclude -Iinclude/psyq tests/native/coin_shop.c -o /tmp/persona-coin-shop
/tmp/persona-coin-shop
```

For UndefinedBehaviorSanitizer, replace `-O2` with `-O1 -g -fsanitize=undefined -fno-sanitize-recover=all`. The surrounding legacy source still produces declaration warnings. This fixture uses a private, non-replacing mapping at the original PS1 data addresses; it exits with status 77 if that range is occupied. AddressSanitizer reserves this range for shadow memory, so this fixture is not an ASan result. Hardware and asset loading are not exercised, and this is not a native game boot test.
