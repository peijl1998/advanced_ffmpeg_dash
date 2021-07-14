#include "abr.h"
#include "stdio.h"
#include "limits.h"

/* THIS IS NOT THREADING SAFE */

static int abr_get_stream_alwaysfirst(ABRContext* ac, int* bandwidth, int size) {
    return 0;
}


static int abr_get_stream_simplethroughput(ABRContext* ac, int* bandwidth, int size) {
    float avg_tpt = 0, tpt_size = 0;
    int best_idx = -1, best_bw = -1, least_idx = -1, least_bw = INT_MAX, cur_bw;
    
    for (int i = 0; i < ac->max_history_len; ++i) {
        if (ac->throughput_history[i] == -1) {
            break;
        }
        avg_tpt += ac->throughput_history[i];
        tpt_size++;
    }
    if (tpt_size > 0) {
        avg_tpt /= tpt_size;
    } else {
        return -1;
    }
    
    for (int i = 0; i < size; ++i) {
        cur_bw = bandwidth[i];
        if (cur_bw <= least_bw) {
            least_bw = cur_bw;
            least_idx = i;
        }
        if (avg_tpt >= cur_bw && best_bw <= cur_bw) {
            best_bw = cur_bw;
            best_idx = i;
        }
    }
    if (best_idx == -1) {
        best_idx = least_idx;
        best_bw = least_bw;
    }

    // printf("abr tpt=%f, best_idx=%d, best_bw=%d\n", avg_tpt, best_idx, bandwidth[best_idx]); 
    return best_idx;
}

// TODO: implement it later.
static int abr_get_stream_bba(ABRContext* ac, int* bandwidth, int size) {
    return 0;
}

void abr_add_metric(ABRContext* ac, float tpt, float buffer_level) {
    // TODO(pjl): can be optimized by Queue here. implement it later.
    // NOTE: it's not threading-safe.
    ac->buffer_level = buffer_level;
    
    // from Mbps to bps
    tpt = tpt * 1024 * 1024;

    if (!ac->throughput_history) {
        ac->throughput_history = (float*)malloc(sizeof(float) * ac->max_history_len);
        for (int i = 0; i < ac->max_history_len; ++i) {
            ac->throughput_history[i] = -1;
        }
    }
    
    if (tpt == -1) {
        return ;
    }

    int last_empty_pos = 0;
    for (int i = ac->max_history_len - 1; i >= 0; --i) {
        if (ac->throughput_history[i] != -1) {
            last_empty_pos = i + 1;
            break;
        }
    }
    
    if (last_empty_pos != ac->max_history_len) {
        ac->throughput_history[last_empty_pos] = tpt;
    } else {
        for (int i = 0; i < ac->max_history_len - 1; ++i) {
            ac->throughput_history[i] = ac->throughput_history[i + 1];
        }
        ac->throughput_history[ac->max_history_len - 1] = tpt;
    }
}

int abr_get_stream(ABRContext* ac, int* bandwidth, int size) {
    switch (ac->algorithm) {
        case AlwaysFirst: return abr_get_stream_alwaysfirst(ac, bandwidth, size);
        case SimpleThroughput: return abr_get_stream_simplethroughput(ac, bandwidth, size); 
        case BBA: return abr_get_stream_bba(ac, bandwidth, size);
        default: return 0;
    }
}

