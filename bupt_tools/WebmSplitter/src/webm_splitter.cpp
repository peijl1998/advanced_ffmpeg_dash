#include "webm_splitter.h"

using namespace webm;

SimpleWebmParser::SimpleWebmParser() {
    this->Reset();
}
SimpleWebmParser::~SimpleWebmParser() {}

void SimpleWebmParser::Reset() {
    this->info_duration = -1;
    this->info_timescale = -1;
    this->segment_start = -1;
    this->segment_end = -1;
    std::vector<Cue>().swap(this->cues);
}

// NOTE: assume that there is only one track in a webm file.
Status SimpleWebmParser::OnCuePoint(const ElementMetadata& meta, const CuePoint& cp) {
    Cue cue(cp.time.value(), cp.cue_track_positions[0].value().cluster_position.value());
    this->cues.emplace_back(cue);
    return Status(Status::kOkCompleted);
}

Status SimpleWebmParser::OnInfo(const ElementMetadata& metadata, const Info& info) {
    this->info_timescale = info.timecode_scale.value();
    this->info_duration = info.duration.value();
    return Status(Status::kOkCompleted);
}

Status SimpleWebmParser::OnSegmentBegin(const ElementMetadata& metadata, Action* action) {
    uint64_t header_size = metadata.header_size;
    this->segment_start = metadata.position + header_size;
    this->segment_end = metadata.size;
    return Status(Status::kOkCompleted);
}

void SimpleWebmParser::Parse(FILE* file) {
    FileReader reader(file);
    WebmParser parser;
    Status status = parser.Feed(this, &reader);
}

uint64_t SimpleWebmParser::GetInfoDuration() {
    return this->info_duration;
}

uint64_t SimpleWebmParser::GetInfoTimescale() {
    return this->info_timescale;
}

uint64_t SimpleWebmParser::GetSegmentStart() {
    return this->segment_start;
}

uint64_t SimpleWebmParser::GetSegmentEnd() {
    return this->segment_end;
}

const std::vector<Cue>& SimpleWebmParser::GetCues() {
    return this->cues;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
      std::cerr << "Input Error\n";
      return EXIT_FAILURE;
    }
    
    FILE* file = std::fopen(argv[1], "rb");
    if (!file) {
      std::cerr << "File cannot be opened\n";
      return EXIT_FAILURE;
    }
    
    SimpleWebmParser* parser = new SimpleWebmParser();
    parser->Parse(file);

    std::cout << " Duration = " << parser->GetInfoDuration() \
              << " Timescale = " << parser->GetInfoTimescale() \
              << " SegmentStart = " << parser->GetSegmentStart() \
              << " SegmentEnd = " << parser->GetSegmentEnd() << std::endl;
    
    for (auto item : parser->GetCues()) {
        std::cout << "time=" << item.cue_time << " pos=" << item.cue_pos << std::endl;
    }

    return 0;
}
