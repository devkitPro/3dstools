#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"

#define BLZ_NORMAL    0          // normal mode
#define BLZ_BEST      1          // best mode

#ifdef __cplusplus
extern "C" {
#endif

u8 *BLZ_Code(u8 *raw_buffer, int raw_len, u32 *new_len, int best);

#ifdef __cplusplus
}
#endif
