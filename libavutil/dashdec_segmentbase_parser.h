#ifndef AVFORMAT_DASHDEC_SEGMENTBASE_PARSER_H
#define AVFORMAT_DASHDEC_SEGMENTBASE_PARSER_H

#include "libavformat/avio.h"

// TODO: webm segment base parser could also be moved here.

typedef struct FMP4Segment {
    uint64_t begin;
    uint64_t size;
    double duration;
    struct FMP4Segment* next;
} FMP4Segment;


int dashdec_fmp4_parse_init(AVIOContext* s, FMP4Segment** head, uint64_t* moov_end);
void dashdec_fmp4_free(FMP4Segment* s);


#endif // AVFORMAT_DASHDEC_SEGMENTBASE_PARSER_H
