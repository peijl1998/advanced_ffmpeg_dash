#ifndef AVFORMAT_ABR_H
#define AVFORMAT_ABR_H

#include <stdlib.h>

typedef enum ABRAlgorithm {
    AlwaysFirst,
    SimpleThroughput,
    BBA
} ABRAlgorithm;

typedef struct ABRContext {
    int max_history_len;
    float* throughput_history;
    float buffer_level;
    ABRAlgorithm algorithm;

    // Other parameters needed
} ABRContext;

void abr_add_metric(ABRContext* ac, float tpt, float buffer_level);
int  abr_get_stream(ABRContext* ac, int* bandwidth, int size); // if return -1, that means no-change.

#endif /* AVFORMAT_ABR_H */
