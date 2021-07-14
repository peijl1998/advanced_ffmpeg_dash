#ifndef AVFORMAT_MATROSKADEC_H
#define AVFORMAT_MATROSKADEC_H

typedef struct CuePosList {
    uint64_t begin;
    uint64_t end;
    struct CuePosList* next;
} CuePosList;

uint64_t dashdec_webm_parse_init(AVFormatContext* s);
CuePosList* dashdec_webm_parse_cue(AVFormatContext* s);
void dashdec_webm_free(CuePosList* c);

#endif /* AVFORMAT_MATROSKADEC_H */
