# Casino vector and motion matches

Seven JP1 casino functions now use exact matching C, totaling 1,104 bytes.

| Function | Bytes | Behavior |
| --- | ---: | --- |
| `func_80092C44` | 108 | Convert a by-value double vector into three signed halfwords, truncating each component through an integer conversion. |
| `func_80092CB0` | 132 | Convert the first two components to words and the third to `762.0 - z`. |
| `func_80092D34` | 184 | Convert each double component to a word after multiplying by 4096.0. |
| `func_8009190C` | 144 | Multiply the double at 0xC0 by -0.2, add 7.7 at 0x78, and set 0x98 to 100.0 when the signed mode byte is zero. |
| `func_80092394` | 156 | On pending byte 1, restart sequence 10, negate doubles at 0xB8 and 0x98, set flag 0xE1 and clear the pending byte. |
| `func_800825A4` | 180 | Queue 20 UV records from a selected signed-short index strip, assigning consecutive destinations. |
| `func_80090DD4` | 200 | Animate the cursor's vertical displacement, restart sequence 11 and update idle transition flags. |

The three conversion functions receive a 24-byte vector by value. Three separate
double arguments have the same incoming register/stack words but generate different
register allocation and larger prologues. The aggregate signature preserves the
original spills and the original software floating-point calls. The signed-short
conversion deliberately goes through `int` before storing a halfword.

For the UV strip, increment the index pointer in its own statement before loading
the UV fields, and read the final field into a temporary before forming the call.
The first draft with an embedded postincrement and direct final-field expression
produced 184 rather than 180 bytes; these ordinary evaluation-order statements
recover the original load schedule. Records contain four signed halfwords and two
unsigned halfwords, stride 12, and the selection index is an unsigned byte.

All seven functions retain PS1 SDK interfaces and original data addresses. Their
C is reconstructed source, not a claim to be the original source. Object comparison
and all 11 linked executable/overlay SHA-256 checks validate the final batch. No
emulator gameplay testing or native Linux runtime is included.
