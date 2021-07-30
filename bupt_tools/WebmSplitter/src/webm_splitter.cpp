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

void SimpleWebmParser::Parse(const std::vector<std::uint8_t>& buffer) {
    BufferReader reader(buffer);
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

void write_single_file(char* prefix, int idx, std::uint64_t start, std::uint64_t end, std::vector<std::uint8_t>& buffer) {
    char val[50];
    if (idx != -1)
        sprintf(val, "%s_%d.webm", prefix, idx);
    else
        sprintf(val, "%s_init.webm", prefix);
    FILE* file = std::fopen(val, "wb");
    for (int i = start; i < end; ++i) {
        fputc(buffer[i], file);
    }
    std::fclose(file);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
      std::cerr << "Input Error\n";
      return EXIT_FAILURE;
    }
    
    // Parsing
    FILE* file = std::fopen(argv[1], "rb");
    if (!file) {
      std::cerr << "File cannot be opened\n";
      return EXIT_FAILURE;
    }
    
    std::vector<std::uint8_t> buffer;
    uint8_t c;
    while (!std::feof(file)) {
        c = fgetc(file);
        buffer.push_back(c);
    }

    SimpleWebmParser* parser = new SimpleWebmParser();
    parser->Parse(buffer);

    std::cout << " Duration = " << parser->GetInfoDuration() \
              << " Timescale = " << parser->GetInfoTimescale() \
              << " SegmentStart = " << parser->GetSegmentStart() \
              << " SegmentEnd = " << parser->GetSegmentEnd() << std::endl;
    
    // for (auto item : parser->GetCues()) {
    //     std::cout << "time=" << item.cue_time << " pos=" << item.cue_pos << std::endl;
    // }
    
    // Writing
    std::uint64_t timescale = 1000000000 / parser->GetInfoTimescale();
    std::uint64_t segment_start = parser->GetSegmentStart();
    std::uint64_t segment_end = parser->GetSegmentEnd();
    double total_duration = 0;
    auto cues = parser->GetCues();
    double duration = parser->GetInfoDuration();
    write_single_file(argv[2], -1, 0, cues[0].cue_pos + segment_start, buffer);
    for (int i = 0; i < cues.size(); ++i) {
        int start = cues[i].cue_pos + segment_start;
        int end;
        if (i != cues.size() - 1) {
            total_duration += (float)(cues[i + 1].cue_time - cues[i].cue_time) / timescale;
            end = cues[i + 1].cue_pos + segment_start - 1;
        } else {
            total_duration += (float)(duration - cues[i].cue_time) / timescale;
            end = segment_end - 1;
        }
        std::cout << argv[2] << " " << i << " " << start << " " << end << std::endl;
        write_single_file(argv[2], i, start, end, buffer);
    }

    std::cout << "Duration total=" << total_duration << "s, avg=" << total_duration / cues.size() << "s" << std::endl;

    std::fclose(file);
    return 0;
}
