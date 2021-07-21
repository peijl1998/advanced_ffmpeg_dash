#ifndef AVFORMAT_MATROSKADEC_H
#define AVFORMAT_MATROSKADEC_H

typedef struct CuePosList {
    uint64_t begin;
    uint64_t end;
    uint64_t cue_time;
    struct CuePosList* next;
} CuePosList;

uint64_t dashdec_webm_parse_init(AVFormatContext* s, int* start, int* timescale, int* duration);
CuePosList* dashdec_webm_parse_cue(AVFormatContext* s);
void dashdec_webm_free(CuePosList* c);

#endif /* AVFORMAT_MATROSKADEC_H */
