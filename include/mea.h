/*
 * Copyright 2018 Livepeer
 * SPDX-License-Identifier: MIT
 */

#ifndef MEA_H
#define MEA_H

#include <inttypes.h>
#include <stddef.h>

typedef struct MeaContext MeaContext;

typedef struct MeaPlane {
    /// Start of the Plane memory
    uint8_t *data;
    /// Size in bytes of a plane line
    ptrdiff_t stride;
    unsigned int width;
    unsigned int height;
} MeaPlane;

typedef struct MeaFrame {
    /** Start of the whole Frame memory
     *
     * if NULL the memory is allocated
     * per-plane
     */
    uint8_t *data;

    /// Per Plane information
    MeaPlane planes[3];
} MeaFrame;

typedef struct MeaSequenceQuality {
    double ranking;
} MeaSequenceQuality;

typedef struct MeaFrameQuality {
    double psnr[3];
    double ssim;
} MeaFrameQuality;

/// Allocate a default MeaContext
MeaContext *mea_context_new(void);
/// Free all the memory used by the context and set it to NULL.
void mea_context_drop(MeaContext **ctx);

/// Initialize the MeaContext to evaluate a new sequence
int mea_sequence_start(MeaContext *ctx);
/**
 * Fill `qual` with the current Sequence quality information.
 *
 * May be called multiple times.
 */
int mea_sequence_status(MeaContext *ctx, MeaSequenceQuality *qual);

/**
 * Compute the quality measurement and fills `qual` with the per-frame
 * information.
 *
 * The per-sequence quality information is updated as well.
 *
 * @see mea_sequence_status
 */
int mea_frame_process(MeaContext *ctx, MeaFrame *ref, MeaFrame *rec,
                      MeaFrameQuality *qual);

#endif    // MEA_H
