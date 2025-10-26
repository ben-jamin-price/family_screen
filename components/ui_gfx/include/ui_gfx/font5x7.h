#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* Glyph metrics (fixed width). */
#define UI_GFX_FONT5x7_WIDTH   5
#define UI_GFX_FONT5x7_HEIGHT  7

/* Pointer to 5 columns (LSB = top row) for ASCII codepoint.
 * Returns '?' if glyph is missing.
 */
const uint8_t *ui_gfx_font5x7_glyph(char c);

#ifdef __cplusplus
} // extern "C"
#endif
