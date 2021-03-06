/*
 * Android MediaCodec NDK helper functions.
 *
 * Copyright (c) 2015-2016, NVIDIA CORPORATION. All rights reserved.
 *
 * This file is part of FFmpeg.
 *
 * FFmpeg is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * FFmpeg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with FFmpeg; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#include <dlfcn.h>

#include "libavutil/atomic.h"
#include "libavutil/avassert.h"
#include "libavutil/thread.h"
#include "mediacodecndk.h"

static const struct {
    enum AVPixelFormat lav;
    enum FFMediaCodecNDKColorFormat ndk;
} pix_fmt_map[] = {
    { AV_PIX_FMT_NV12,     COLOR_FormatYUV420SemiPlanar },
    { AV_PIX_FMT_YUV420P,  COLOR_FormatYUV420Planar },
    { AV_PIX_FMT_YUV422P,  COLOR_FormatYUV422Flexible },
    { AV_PIX_FMT_YUV444P,  COLOR_FormatYUV444Flexible },
    { AV_PIX_FMT_RGB8,     COLOR_FormatRGBFlexible },
    { AV_PIX_FMT_BGR24,    COLOR_Format24bitBGR888 },
    { AV_PIX_FMT_ABGR,     COLOR_Format32bitABGR8888 },
    { AV_PIX_FMT_RGBA,     COLOR_FormatRGBAFlexible },
    { AV_PIX_FMT_RGB565BE, COLOR_Format16bitRGB565 },
    { AV_PIX_FMT_NONE,     COLOR_FormatSurface },
};

enum FFMediaCodecNDKColorFormat ff_mediacodecndk_get_color_format(enum AVPixelFormat lav)
{
    unsigned i;
    for (i = 0; pix_fmt_map[i].lav != AV_PIX_FMT_NONE; i++) {
        if (pix_fmt_map[i].lav == lav)
            return pix_fmt_map[i].ndk;
    }
    return COLOR_FormatSurface;
}

enum AVPixelFormat ff_mediacodecndk_get_pix_fmt(enum FFMediaCodecNDKColorFormat ndk)
{
    unsigned i;
    for (i = 0; pix_fmt_map[i].lav != AV_PIX_FMT_NONE; i++) {
        if (pix_fmt_map[i].ndk == ndk)
            return pix_fmt_map[i].lav;
    }
    return AV_PIX_FMT_NONE;
}

const char* ff_mediacodecndk_get_mime(enum AVCodecID codec_id)
{
    switch (codec_id) {
    case AV_CODEC_ID_H264:
        return "video/avc";
    case AV_CODEC_ID_HEVC:
        return "video/hevc";
    case AV_CODEC_ID_MPEG2VIDEO:
        return "video/mpeg2";
    default:
        av_assert0(!"Unsupported codec ID");
        return NULL;
    }
}

static volatile int ret = 0;
static AVOnce ff_mediacodec_init_once = AV_ONCE_INIT;

static void ff_mediacodecndk_init_binder_once(void)
{
    void *lib = dlopen("ndkbinderutil.so", RTLD_NOW | RTLD_GLOBAL);
    void (*thread_pool_start)(void);
    if (!lib)
        lib = dlopen("nvtranscode.so", RTLD_NOW | RTLD_GLOBAL);
    if (!lib) {
        av_log(NULL, AV_LOG_ERROR, "Binder initialization library not found\n");
        avpriv_atomic_int_set(&ret, AVERROR_ENCODER_NOT_FOUND);
        return;
    }
    thread_pool_start = dlsym(lib, "NdkBinderUtilThreadCreate");
    if (!thread_pool_start)
        thread_pool_start = dlsym(lib, "NvTranscodeThreadCreate");
    if (!thread_pool_start) {
        av_log(NULL, AV_LOG_ERROR, "Binder initialization function not found\n");
        avpriv_atomic_int_set(&ret, AVERROR_ENCODER_NOT_FOUND);
        return;
    }
    thread_pool_start();
}

int ff_mediacodecndk_init_binder(void)
{
    ff_thread_once(&ff_mediacodec_init_once, ff_mediacodecndk_init_binder_once);
    return avpriv_atomic_int_get(&ret);
}
