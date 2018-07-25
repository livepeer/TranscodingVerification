/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */
#include <assert.h>
#include <math.h>

#include "mea.h"
#include "mea_internal.h"

#define TWENTY_LOG_TEN_8BIT (48.13080361)

double psnr(const MeaPlane *plane1, const MeaPlane *plane2) {
    const unsigned height = plane1->height;
    const unsigned width = plane1->width;
    // SSE requires the same frame size.
    assert(height == plane2->height);
    assert(width == plane2->width);

    const ptrdiff_t p1_str = plane1->stride;
    const ptrdiff_t p2_str = plane2->stride;

    int64_t result = 0;
    const uint8_t *p1 = plane1->data;
    const uint8_t *p2 = plane2->data;
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            const int diff = p1[x] - p2[x];
            result += diff * diff;
        }
        p1 += p1_str;
        p2 += p2_str;
    }
    const double mse = result / ((double)(height * width));
    return TWENTY_LOG_TEN_8BIT - 10 * log10(mse);
}
