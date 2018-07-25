/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>

#include "mea.h"
#include "mea_internal.h"

int main(void) {
    uint8_t data[16] = { 242, 129, 25, 10, 99, 33, 111, 137,
                         242, 129, 25, 10, 99, 33, 111, 137 };
    MeaPlane p1;
    p1.data = data;
    p1.stride = 4;
    p1.width = 2;
    p1.height = 2;

    printf("%f", psnr(&p1, &p1));
    return 0;
}
