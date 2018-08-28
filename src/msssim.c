#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "mea.h"

#if !defined(M_PI)
#define M_PI (3.141592653589793238462643)
#endif

#define KERNEL_SHIFT (10)
#define KERNEL_WEIGHT (1 << KERNEL_SHIFT)
#define KERNEL_ROUND ((1 << KERNEL_SHIFT) >> 1)

#define FS_MINI(_a, _b) ((_a) < (_b) ? (_a) : (_b))
#define FS_MAXI(_a, _b) ((_a) > (_b) ? (_a) : (_b))

typedef struct ssim_moments ssim_moments;

struct ssim_moments {
    int64_t mux;
    int64_t muy;
    int64_t x2;
    int64_t xy;
    int64_t y2;
    int64_t w;
};

#define SSIM_K1 0.01
#define SSIM_K2 0.03

/*Inside the border padding, SSIM will not be computed. The original paper used
  5, but later implementations use 0, which produces slightly better subjective
  correlation.*/
#define BORDER_PADDING (0)

static int gaussian_filter_init(unsigned **_kernel, double _sigma,
                                int _max_len) {
    const double scale = 1 / (sqrt(2 * M_PI) * _sigma);
    const double nhisigma2 = -0.5 / (_sigma * _sigma);
    /*Compute the kernel size so that the error in the first truncated
      coefficient is no larger than 0.5*KERNEL_WEIGHT.
      There is no point in going beyond this given our working precision.*/
    const double s = sqrt(0.5 * M_PI) * _sigma * (1.0 / KERNEL_WEIGHT);
    const double len = s < 1 ? floor(_sigma * sqrt(-2 * log(s))) : 0;
    const int kernel_len = len < _max_len ? (int)len : _max_len - 1;
    const int kernel_sz = kernel_len << 1 | 1;
    unsigned *kernel = (unsigned *)malloc(kernel_sz * sizeof(*kernel));
    unsigned sum = 0;
    for (int ci = kernel_len; ci > 0; ci--) {
        kernel[kernel_len - ci] = kernel[kernel_len + ci] =
            (unsigned)(KERNEL_WEIGHT * scale * exp(nhisigma2 * ci * ci) + 0.5);
        sum += kernel[kernel_len - ci];
    }
    kernel[kernel_len] = KERNEL_WEIGHT - (sum << 1);
    *_kernel = kernel;
    return kernel_sz;
}

static void calc_ssim(const int *_src, int _systride, const int *_dst, int _dystride,
                      int _w, int _h, int max, double *ssim_ret, double *cs_ret) {
    ssim_moments *line_buf;
    ssim_moments **lines;
    unsigned *hkernel;
    unsigned *vkernel;
    const int vkernel_sz = gaussian_filter_init(&vkernel, 1.5, 5);
    const int vkernel_offs = vkernel_sz >> 1;
    int line_sz = 1;
    for (int log_line_sz = 0; line_sz < vkernel_sz;
         line_sz <<= 1, log_line_sz++)
        ;
    const int line_mask = line_sz - 1;
    lines = (ssim_moments **)malloc(line_sz * sizeof(*lines));
    lines[0] = line_buf =
        (ssim_moments *)malloc(line_sz * _w * sizeof(*line_buf));
    for (int y = 1; y < line_sz; y++) lines[y] = lines[y - 1] + _w;
    const int hkernel_sz = gaussian_filter_init(&hkernel, 1.5, 5);
    const int hkernel_offs = hkernel_sz >> 1;
    double ssim = 0;
    double cs = 0;
    double ssimw = 0;
    for (int y = BORDER_PADDING; y < _h + vkernel_offs - BORDER_PADDING; y++) {
        if (y < _h) {
            ssim_moments *buf = lines[y & line_mask];
            for (int x = BORDER_PADDING; x < _w - BORDER_PADDING; x++) {
                ssim_moments m;
                memset(&m, 0, sizeof(m));
                const int k_min = hkernel_offs - x <= 0 ? 0 : hkernel_offs - x;
                const int k_max =
                    x + hkernel_offs - _w + 1 <= 0
                        ? hkernel_sz
                        : hkernel_sz - (x + hkernel_offs - _w + 1);
                for (int k = k_min; k < k_max; k++) {
                    const int32_t s = _src[x - hkernel_offs + k];
                    const int32_t d = _dst[x - hkernel_offs + k];
                    const int32_t window = hkernel[k];
                    m.mux += window * s;
                    m.muy += window * d;
                    m.x2 += (int64_t)window * s * s;
                    m.xy += (int64_t)window * s * d;
                    m.y2 += (int64_t)window * d * d;
                    m.w += window;
                }
                *(buf + x) = *&m;
            }
            _src += _systride;
            _dst += _dystride;
        }
        if (y >= vkernel_offs + BORDER_PADDING) {
            const int k_min = vkernel_sz - y - 1 <= 0 ? 0 : vkernel_sz - y - 1;
            const int k_max =
                y + 1 - _h <= 0 ? vkernel_sz : vkernel_sz - (y + 1 - _h);
            for (int x = BORDER_PADDING; x < _w - BORDER_PADDING; x++) {
                ssim_moments m;
                memset(&m, 0, sizeof(m));
                for (int k = k_min; k < k_max; k++) {
                    int32_t window = vkernel[k];
                    ssim_moments *buf =
                        lines[(y + 1 - vkernel_sz + k) & line_mask] + x;
                    m.mux += window * buf->mux;
                    m.muy += window * buf->muy;
                    m.x2 += window * buf->x2;
                    m.xy += window * buf->xy;
                    m.y2 += window * buf->y2;
                    m.w += window * buf->w;
                }
                const double w = m.w;
                const double c1 = SSIM_K1 * SSIM_K1 * max * max * w * w;
                const double c2 = SSIM_K2 * SSIM_K2 * max * max * w * w;
                const double mx2 = m.mux * (double)m.mux;
                const double mxy = m.mux * (double)m.muy;
                const double my2 = m.muy * (double)m.muy;
                const double cs_tmp = m.w * (c2 + 2 * (m.xy * w - mxy)) /
                                      (m.x2 * w - mx2 + m.y2 * w - my2 + c2);
                cs += cs_tmp;
                ssim += cs_tmp * (2 * mxy + c1) / (mx2 + my2 + c1);
                ssimw += m.w;
            }
        }
    }
    free(line_buf);
    free(lines);
    free(vkernel);
    free(hkernel);
    *ssim_ret = ssim / ssimw;
    *cs_ret = cs / ssimw;
}

/*These come from the original MS-SSIM implementation paper:
https://ece.uwaterloo.ca/~z70wang/publications/msssim.pdf
They don't add up to 1 due to rounding done in the paper. */
static const double WEIGHT[] = { 0.0448, 0.2856, 0.3001, 0.2363, 0.1333 };

static void downsample_2x(int32_t *_src1, int _s1ystride, int32_t *_src2,
                          int _s2ystride, int _w, int _h) {
    const int w = _w >> 1;
    const int h = _h >> 1;
    for (int j = 0; j < h; j++) {
        const int j0 = 2 * j;
        const int j1 = FS_MINI(j0 + 1, _h - 1);
        for (int i = 0; i < w; i++) {
            const int i0 = 2 * i;
            const int i1 = FS_MINI(i0 + 1, _w - 1);
            _src1[j * _s1ystride + i] =
                (_src1[j0 * _s1ystride + i0] + _src1[j0 * _s1ystride + i1] +
                 _src1[j1 * _s1ystride + i0] + _src1[j1 * _s1ystride + i1]);
            _src2[j * _s2ystride + i] =
                (_src2[j0 * _s2ystride + i0] + _src2[j0 * _s2ystride + i1] +
                 _src2[j1 * _s2ystride + i0] + _src2[j1 * _s2ystride + i1]);
        }
    }
}

double msssim(const MeaPlane *plane1, const MeaPlane *plane2) {
    unsigned height = plane1->height;
    unsigned width = plane1->width;
    // MSSSIM requires the same frame size.
    assert(height == plane2->height);
    assert(width == plane2->width);

    int max = 255;
    double ssim[5];
    double cs[5];

    const ptrdiff_t p1_str = plane1->stride;
    const ptrdiff_t p2_str = plane2->stride;
    const int cstride = width;

    int32_t *const c1 = malloc(width * height * sizeof(*c1));
    int32_t *const c2 = malloc(width * height * sizeof(*c2));
    for (unsigned y = 0; y < height; y++) {
        for (unsigned x = 0; x < width; x++) {
            c1[y * cstride + x] = plane1->data[y * p1_str + x];
            c2[y * cstride + x] = plane2->data[y * p2_str + x];
        }
    }

    calc_ssim(c1, cstride, c2, cstride, width, height, max, &ssim[0], &cs[0]);

    for (int i = 1; i < 5; i++) {
        downsample_2x(c1, cstride, c2, cstride, width, height);
        width >>= 1;
        height >>= 1;
        max *= 4;
        calc_ssim(c1, cstride, c2, cstride, width, height, max, &ssim[i],
                  &cs[i]);
    }
    const double overall_msssim =
        pow(cs[0], WEIGHT[0]) * pow(cs[1], WEIGHT[1]) * pow(cs[2], WEIGHT[2]) *
        pow(cs[3], WEIGHT[3]) * pow(ssim[4], WEIGHT[4]);

    free(c1);
    free(c2);

    return overall_msssim;
}
