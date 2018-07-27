/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mea.h"
#include "mea_internal.h"

void fill_buffer(uint8_t *buf, int len) {
    for (int i = 0; i < len; i++) {
        buf[i] = rand();
    }
}

int main(int argc, char *argv[]) {
    MeaPlane p1, p2;
    int w = 64;
    int h = 64;
    uint8_t ref[w * h], rec[w * h];

    srand(argc > 1 ? atoi(argv[1]) : 1);

    fill_buffer(ref, sizeof(ref));
    memcpy(rec, ref, sizeof(ref));

    p1.data = ref;
    p1.stride = w;
    p1.width = w;
    p1.height = h;

    p2.data = rec;
    p2.stride = w;
    p2.width = w;
    p2.height = h;

    printf("psnr %f\n", psnr(&p1, &p2));
    printf("mssim %f\n", msssim(&p1, &p2));

    for (size_t i = 0; i < sizeof(rec); i+=4) {
        rec[i] += rand() > RAND_MAX / 2;
    }

    printf("psnr %f\n", psnr(&p1, &p2));
    printf("mssim %f\n", msssim(&p1, &p2));

    memset(rec, 0, sizeof(rec));

    printf("psnr %f\n", psnr(&p1, &p2));
    printf("mssim %f\n", msssim(&p1, &p2));

    return 0;
}
