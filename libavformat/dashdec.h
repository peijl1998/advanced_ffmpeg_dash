#ifndef AVFORMAT_DASHDEC_H
#define AVFORMAT_DASHDEC_H
#include "avformat.h"
#include "abr.h"

struct fragment {
    int64_t url_offset;
    int64_t size;
    char *url;
    double duration; // seconds, only for webm-segmentbase.
};

/*
 * reference to : ISO_IEC_23009-1-DASH-2012
 * Section: 5.3.9.6.2
 * Table: Table 17 — Semantics of SegmentTimeline element
 * */
struct timeline {
    /* starttime: Element or Attribute Name
     * specifies the MPD start time, in @timescale units,
     * the first Segment in the series starts relative to the beginning of the Period.
     * The value of this attribute must be equal to or greater than the sum of the previous S
     * element earliest presentation time and the sum of the contiguous Segment durations.
     * If the value of the attribute is greater than what is expressed by the previous S element,
     * it expresses discontinuities in the timeline.
     * If not present then the value shall be assumed to be zero for the first S element
     * and for the subsequent S elements, the value shall be assumed to be the sum of
     * the previous S element's earliest presentation time and contiguous duration
     * (i.e. previous S@starttime + @duration * (@repeat + 1)).
     * */
    int64_t starttime;
    /* repeat: Element or Attribute Name
     * specifies the repeat count of the number of following contiguous Segments with
     * the same duration expressed by the value of @duration. This value is zero-based
     * (e.g. a value of three means four Segments in the contiguous series).
     * */
    int64_t repeat;
    /* duration: Element or Attribute Name
     * specifies the Segment duration, in units of the value of the @timescale.
     * */
    int64_t duration;
};

/*
 * Each playlist has its own demuxer. If it is currently active,
 * it has an opened AVIOContext too, and potentially an AVPacket
 * containing the next packet from this stream.
 */
struct representation {
    char *url_template;
    AVIOContext pb;
    AVIOContext *input;
    AVFormatContext *parent;
    AVFormatContext *ctx;
    AVFormatContext *last_ctx;
    int stream_index;

    char *id;
    char *lang;
    int bandwidth;
    AVRational framerate;
    AVStream *assoc_stream; /* demuxer stream associated with this representation */

    int n_fragments;
    struct fragment **fragments; /* VOD list of fragment for profile */

    int n_timelines;
    struct timeline **timelines;

    int64_t first_seq_no;
    int64_t last_seq_no;
    int64_t start_number; /* used in case when we have dynamic list of segment to know which segments are new one*/

    int64_t fragment_duration;
    int64_t fragment_timescale;

    int64_t presentation_timeoffset;

    int64_t cur_seq_no;
    int64_t cur_seg_offset;
    int64_t cur_seg_size;
    struct fragment *cur_seg;

    /* Currently active Media Initialization Section */
    struct fragment *init_section;
    uint8_t *init_sec_buf;
    uint32_t init_sec_buf_size;
    uint32_t init_sec_data_len;
    uint32_t init_sec_buf_read_offset;
    int64_t cur_timestamp;
    int is_restart_needed;
};

typedef struct Period {
    int n_videos;
    struct representation **videos;
    int n_audios;
    struct representation **audios;
    int n_subtitles;
    struct representation **subtitles;
    
    uint64_t period_duration;
    uint64_t period_start;

    char *adaptionset_lang;

    int is_init_section_common_video;
    int is_init_section_common_audio;
    int is_init_section_common_subtitle;
} Period;

typedef struct DASHContext {
    const AVClass *class;
    char *base_url;

    // int n_videos;
    // struct representation **videos;
    // int n_audios;
    // struct representation **audios;
    // int n_subtitles;
    // struct representation **subtitles;

    /* MediaPresentationDescription Attribute */
    uint64_t media_presentation_duration;
    uint64_t suggested_presentation_delay;
    uint64_t availability_start_time;
    uint64_t availability_end_time;
    uint64_t publish_time;
    uint64_t minimum_update_period;
    uint64_t time_shift_buffer_depth;
    uint64_t min_buffer_time;

    /* Period Attribute */
    // uint64_t period_duration;
    // uint64_t period_start;

    /* AdaptationSet Attribute */
    // char *adaptionset_lang;

    int is_live;
    AVIOInterruptCB *interrupt_callback;
    char *allowed_extensions;
    AVDictionary *avio_opts;
    int max_url_size;

    /* Flags for init section*/
    // int is_init_section_common_video;
    // int is_init_section_common_audio;
    // int is_init_section_common_subtitle;

    // BUPT
    int n_periods;
    int current_period;
    struct Period **periods;
    
    ABRContext* audio_abr;
    ABRContext* video_abr;

    void (*dashdec_add_metric)(AVFormatContext *s, enum AVMediaType type, float tpt, float buffer_level,
                               int buffer_max, int buffer_r, int buffer_c);
    int (*dashdec_get_stream)(AVFormatContext *s, enum AVMediaType type);
} DASHContext;

#endif /* AVFORMAT_DASHDEC_H */
