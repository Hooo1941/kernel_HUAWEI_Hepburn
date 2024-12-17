#ifndef AVFORMAT_HDR_VIVID_H
#define AVFORMAT_HDR_VIVID_H

#ifdef HDR_VIVID_VIDEO
#include "avformat.h"
#include "libavutil/log.h"
#include "movenc.h"
#include "avio.h"

typedef struct CUVVConfiguration {
    int cuva_version_map;
    int terminal_provide_code;
    int terminal_provide_oriented_code;
} CUVVConfiguration;

CUVVConfiguration hdr_get_cuva_from_metadata(AVFormatContext *s, MOVMuxContext *mov, MOVTrack *track);
int hdr_write_cuvv_tag(AVIOContext *pb, CUVVConfiguration *cuvv_config);
#endif

#endif /* AVFORMAT_HDR_VIVID_H */