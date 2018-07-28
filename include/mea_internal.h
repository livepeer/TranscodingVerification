/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */

#ifndef MEA_INTERNAL_H
#define MEA_INTERNAL_H
#include "mea.h"

double psnr(const MeaPlane *plane1, const MeaPlane *plane2);
double msssim(const MeaPlane *plane1, const MeaPlane *plane2);

double frame_psnr(const MeaFrame *frame1, const MeaFrame *frame2);

// More complex metrics
struct MeaContext {
    double qual;
    double min_frame_qual;
    double max_frame_qual;
    unsigned int frames;
};

#endif    // MEA_INTERNAL_H
