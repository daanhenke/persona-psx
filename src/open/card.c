/* open's object for the shared unit in src/common.
 *
 * Each executable compiles its own: the card's globals sit at different
 * addresses, and splat resolves symlinks when it writes the linker script,
 * so a link here would collapse back to one shared object.
 */
#include "../common/card/card.c"
