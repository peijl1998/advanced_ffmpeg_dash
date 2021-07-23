#include "dashdec_segmentbase_parser.h"



int dashdec_fmp4_parse_init(AVIOContext* pb, FMP4Segment** head, uint64_t* moov_end) {
    unsigned type, size;
    unsigned track_id, timescale, fo, ref_count;
    uint32_t seg_size, seg_dur;
    uint64_t pos, start;
    int version, i, is_media_sidx;
    FMP4Segment *cursor;
    *head = NULL;
    *moov_end = -1;

    while (!avio_feof(pb)) {
        pos = avio_tell(pb);
        size = avio_rb32(pb);
        type = avio_rl32(pb);
        if (avio_feof(pb)) {
            break;
        }
        if (type != MKTAG('s','i','d','x')) {
            if (type == MKTAG('m', 'o', 'o', 'v')) {
                *moov_end = pos + size + 1;
            }
            avio_skip(pb, size - 8);
        } else {
            version = avio_r8(pb); //version
            avio_rb24(pb);  // flags
            track_id = avio_rb32(pb); // reference id
            timescale = avio_rb32(pb); // timescale
            if (version == 0) {
                avio_rb32(pb); // earliest pts
                fo = avio_rb32(pb); // first_offset
            } else {
                avio_rb64(pb);
                fo = avio_rb64(pb);
            }
            avio_rb16(pb); // reserved
            ref_count = avio_rb16(pb);
            
            is_media_sidx = 1;
            start = pos + size + fo;
            for (i = 0; i < ref_count; ++i) {
                seg_size = avio_rb32(pb);
                seg_dur = avio_rb32(pb);

                if (seg_size & 0x80000000) {
                    is_media_sidx = 0;
                }
                seg_size = (seg_size << 1) >> 1;
                avio_rb32(pb); // SAP related.

                if (is_media_sidx == 1) {
                    FMP4Segment* next = (FMP4Segment*)malloc(sizeof(FMP4Segment));
                    next->duration = (double)seg_dur / timescale;
                    next->begin = start;
                    next->size = seg_size;
                    next->next = NULL;
                    if (!(*head)) {
                        cursor = (*head) = next;
                    } else {
                        cursor->next = next;
                        cursor = cursor->next;
                    }
                }
                start += seg_size;
            }
        }
    }

    return 0;
}

void dashdec_fmp4_free(FMP4Segment* s) {
    while (s) {
        FMP4Segment* tmp = s;
        s = s->next;
        free(tmp);
    }
}
