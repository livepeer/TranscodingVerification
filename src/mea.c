/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */

#include <stdlib.h>
#include <string.h>

#include "mea.h"
#include "mea_internal.h"

MeaContext *mea_context_new() {
    MeaContext *ctx = calloc(1, sizeof(*ctx));

    return ctx;
}

void mea_context_drop(MeaContext **ctx) {
    free(*ctx);
    *ctx = NULL;
}

int mea_sequence_start(MeaContext *ctx) {
    memset(ctx, 0, sizeof(*ctx));

    return 0;
}

int mea_sequence_status(MeaContext *ctx, MeaSequenceQuality *qual) {
    qual->ranking = ctx->qual / ctx->frames;
    return 0;
}

int mea_frame_process(MeaContext *ctx, MeaFrame *ref, MeaFrame *rec, MeaFrameQuality *qual) {
    ctx->frames += 1;

    for (int i = 0; i < 3; i++) {
        double p = psnr(&ref->planes[i], &rec->planes[i]);
        qual->psnr[i] = p > 60 ? 60 : p;
    }

    qual->ssim = msssim(&ref->planes[0], &rec->planes[0]);

    ctx->qual += (qual->psnr[0] + qual->psnr[1] + qual->psnr[2]) * qual->ssim;

    return 0;
}
