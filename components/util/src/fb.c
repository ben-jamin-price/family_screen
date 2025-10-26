#include "util/fb.h"
#include "esp_heap_caps.h"

void *util_malloc_psram_dma(size_t bytes)
{
    void *p = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM | MALLOC_CAP_DMA);
    if (!p) p = heap_caps_malloc(bytes, MALLOC_CAP_SPIRAM);
    if (!p) p = heap_caps_malloc(bytes, MALLOC_CAP_DEFAULT);
    return p;
}
