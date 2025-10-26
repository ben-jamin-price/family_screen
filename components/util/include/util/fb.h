#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Allocate a framebuffer (or any large buffer) from PSRAM when possible,
 * falling back to default heap. The allocation is DMA-capable if available.
 *
 * Order:
 *  1) SPIRAM | DMA
 *  2) SPIRAM
 *  3) DEFAULT
 *
 * @param bytes  number of bytes to allocate
 * @return pointer to buffer, or NULL if allocation fails
 */
void *util_malloc_psram_dma(size_t bytes);

#ifdef __cplusplus
} // extern "C"
#endif
