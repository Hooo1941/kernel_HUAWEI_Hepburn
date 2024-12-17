#ifdef HDR_VIVID_VIDEO
#include "hdr_vivid.h"
#include "avio_internal.h"

CUVVConfiguration hdr_get_cuva_from_metadata(AVFormatContext *s, MOVMuxContext *mov, MOVTrack *track)
{
    CUVVConfiguration cuvv_config;
    cuvv_config.cuva_version_map = 0;
    cuvv_config.terminal_provide_code = 0;
    cuvv_config.terminal_provide_oriented_code = 0;
    if (s == NULL || mov == NULL || track == NULL) {
        av_log(mov->fc, AV_LOG_WARNING, "Not cuvv info. Parameters are NULL!\n");
        return cuvv_config;
    }

    int i = 0;
    for (i = 0; i < mov->nb_streams; ++i) {
        if (track == &mov->tracks[i]) {
            break;
        }
    }
    if (i == mov->nb_streams) {
        av_log(mov->fc, AV_LOG_WARNING, "Not cuvv info. The track is not in the mov!\n");
        return cuvv_config;
    }
    AVStream *st = i < s->nb_streams ? s->streams[i] : NULL;
    if (st && st->metadata) {
        AVDictionaryEntry *t = av_dict_get(st->metadata, "hdr_type", NULL, 0);
        if (t && t->value && strcmp(t->value, "hdr_vivid") == 0) {
            cuvv_config.cuva_version_map = 1;
            cuvv_config.terminal_provide_code = 4;
            cuvv_config.terminal_provide_oriented_code = 5;
        }
    }
    return cuvv_config;
}

int hdr_write_cuvv_tag(AVIOContext *pb, CUVVConfiguration *cuvv_config)
{
    int64_t pos = avio_tell(pb);
    avio_wb32(pb, 0); /* size */
    ffio_wfourcc(pb, "cuvv");
    avio_wb16(pb, cuvv_config->cuva_version_map);
    avio_wb16(pb, cuvv_config->terminal_provide_code);
    avio_wb16(pb, cuvv_config->terminal_provide_oriented_code);
    avio_wb32(pb, 0); // reserved
    avio_wb32(pb, 0); // reserved
    avio_wb32(pb, 0); // reserved
    avio_wb32(pb, 0); // reserved

    int64_t curpos = avio_tell(pb);
    avio_seek(pb, pos, SEEK_SET);
    avio_wb32(pb, curpos - pos); /* rewrite size */
    avio_seek(pb, curpos, SEEK_SET);

    return curpos - pos;
}
#endif