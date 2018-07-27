/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>
#include <string.h>

#include "mea.h"
#include "mea_internal.h"

int main(void) {
    uint8_t ref[16] = { 242, 129, 25, 10, 99, 33, 111, 137,
                         242, 129, 25, 10, 99, 33, 111, 137 };
    uint8_t rec[16] = { 242, 120, 25, 10, 99, 33, 111, 131,
                         242, 129, 25, 10, 99, 33, 111, 131 };
    MeaPlane p1, p2;
    p1.data = ref;
    p1.stride = 4;
    p1.width = 2;
    p1.height = 2;

    p2.data = rec;
    p2.stride = 4;
    p2.width = 2;
    p2.height = 2;

    printf("psnr %f\n", psnr(&p1, &p1));
    printf("psnr %f\n", psnr(&p1, &p2));
    memset(rec, 0, sizeof(rec));
    printf("psnr %f\n", psnr(&p1, &p2));

    return 0;
}
