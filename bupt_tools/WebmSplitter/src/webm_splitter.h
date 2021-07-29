#include <vector>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "webm/callback.h"
#include "webm/file_reader.h"
#include "webm/buffer_reader.h"
#include "webm/status.h"
#include "webm/webm_parser.h"

typedef struct Cue {
    Cue(uint64_t ct, uint64_t cp) : cue_time(ct),
                                    cue_pos(cp) {}
    uint64_t cue_time;
    uint64_t cue_pos;
} Cue;

class SimpleWebmParser : public webm::Callback {
    public:
        SimpleWebmParser();
        ~SimpleWebmParser();

        // For User
        void Parse(FILE* file);
        uint64_t GetInfoDuration();
        uint64_t GetInfoTimescale();
        uint64_t GetSegmentStart();
        uint64_t GetSegmentEnd();
        const std::vector<Cue>& GetCues();
        void Reset();

       // For Callback
        webm::Status OnCuePoint(const webm::ElementMetadata& meta, const webm::CuePoint& cp) override;
        webm::Status OnInfo(const webm::ElementMetadata& metadata, const webm::Info& info) override;
        webm::Status OnSegmentBegin(const webm::ElementMetadata& metadata, webm::Action* action) override;

    private:
        uint64_t info_duration;
        uint64_t info_timescale;
        uint64_t segment_start;
        uint64_t segment_end;

        std::vector<Cue> cues;
};
