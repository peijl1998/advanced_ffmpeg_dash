#include "abr.h"
#include "stdio.h"
#include "limits.h"

/* !!! THIS IS MAY NOT THREADING SAFE !!! */

static float abr_get_avg_tpt(ABRContext* ac) {
    float avg_tpt = 0;
    int tpt_size = 0;
    for (int i = 0; i < ac->max_history_len; ++i) {
        if (ac->throughput_history[i] == -1) {
            break;
        }
        avg_tpt += ac->throughput_history[i];
        tpt_size++;
    }

    if (tpt_size > 0) {
        return avg_tpt / tpt_size;
    } else {
        return -1;
    }
}

static int abr_get_stream_alwaysfirst(ABRContext* ac, int* bandwidth, int size) {
    return 0;
}


static int abr_get_stream_simplethroughput(ABRContext* ac, int* bandwidth, int size) {
    int best_idx = -1, best_bw = -1, least_idx = -1, least_bw = INT_MAX, cur_bw;
    float avg_tpt = abr_get_avg_tpt(ac);
    if (avg_tpt < 0) return -1;
    
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

    return best_idx;
}

/*
 * one example parameter
 * buffer_r(the size of reservoir) is 37.5% of the buffer_max, but at least 1 chunk duration
 * buffer_c(the size of cushion) if 37.5%~90% of the buffer_max
 * the rate map is piece-wise
*/
static int abr_get_stream_bba(ABRContext* ac, int* bandwidth, int size) {
    int rate_min = INT_MAX, rate_max = 0, rate_min_idx, rate_max_idx;
    int best_bw, best_idx = -1;
    int rc, i;
    for (i = 0; i < size; ++i) {
        if (bandwidth[i] < rate_min) {
            rate_min = bandwidth[i];
            rate_min_idx = i;
        }
        if (bandwidth[i] > rate_max) {
            rate_max = bandwidth[i];
            rate_max_idx = i;
        }
    }
    if (ac->buffer_level <= ac->buffer_r) {
        return rate_min_idx;
    } else if (ac->buffer_level >= ac->buffer_r + ac->buffer_c) {
        return rate_max_idx;
    } else {
        rc = rate_min + 1.0 * (rate_max - rate_min) / (ac->buffer_c) * (ac->buffer_level - ac->buffer_r);
        if (rc == rate_min) return rate_min_idx;
        if (rc == rate_max) return rate_max_idx;
        for (i = 0; i < size; ++i) {
            if (bandwidth[i] <= rc) {
                if (best_idx == -1 || bandwidth[i] > best_bw) {
                    best_idx = i;
                    best_bw = bandwidth[i];
                }
            }
        }

        return best_idx;
    }

    return -1;
}

void abr_add_metric(ABRContext* ac, float tpt, float buffer_level,
                    int buffer_max, int buffer_r, int buffer_c) {
    ac->buffer_level = buffer_level;
    ac->buffer_max = buffer_max;
    ac->buffer_r = buffer_r;
    ac->buffer_c = buffer_c;
    
    // from Mbps to bps
    tpt = tpt * 1024 * 1024;
    

    // TODO(pjl): array can be optimized by queue here. implement it later.
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
        case BBA_0: return abr_get_stream_bba(ac, bandwidth, size);
        default: return 0;
    }
}

