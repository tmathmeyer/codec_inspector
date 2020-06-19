
#include <iostream>
#include <string>

#include "base/values.h"
#include "base/json/json_writer.h"

#include "media/filters/ffmpeg_glue.h"
#include "media/tools/codec_inspector/accepted_codecs.h"
#include "media/ffmpeg/ffmpeg_common.h"
#include "media/base/decoder_buffer.h"
#include "media/gpu/accelerated_video_decoder.h"
#include "media/tools/codec_inspector/vp9_struct_logger.h"

// forward declare the ffmpeg AVPacket
struct AVPacket;

class OnTheFilesystem : public media::FFmpegURLProtocol {
 public:
  OnTheFilesystem(std::string path) {
    ro_media_file = fopen(path.c_str(), "r");
    fseek(ro_media_file, 0, SEEK_END);
    filesize = ftell(ro_media_file);
    fseek(ro_media_file, 0, SEEK_SET);
    position = 0;
  }

  virtual ~OnTheFilesystem() {
    fclose(ro_media_file);
  }

  int Read(int size, uint8_t* data) override {
    int read_bytes = fread(data, 1, size, ro_media_file);
    position += read_bytes;
    return read_bytes;
  }

  bool GetPosition(int64_t* position_out) override {
    int pos = ftell(ro_media_file);
    if (pos != position) {
      puts("Uh oh, the position is out of sync...");
    }
    return pos;
  }

  bool SetPosition(int64_t pos) override {
    if (!fseek(ro_media_file, pos, SEEK_SET)) {
      position = pos;
      return true;
    }
    return false;
  }

  bool GetSize(int64_t* size_out) override {
    return filesize;
  }

  bool IsStreaming() override {
    return false;
  }

 private:
  FILE *ro_media_file;
  int64_t position;
  int64_t filesize;
};


int read_nonempty_packet(AVFormatContext* ctx, AVPacket* packet) {
  // Skip empty packets in a tight loop to avoid timing out fuzzers.
  int result;
  bool drop_packet;
  do {
    result = av_read_frame(ctx, packet);
    drop_packet = (!packet->data || !packet->size) && result >= 0;
    if (drop_packet) {
      av_packet_unref(packet);
    }
  } while (drop_packet);

  return result;
}







class StateMachine {
 public:
  StateMachine() {}
  ~StateMachine() {}

  bool ShowHelp() {
    std::cout << "Commands:" << std::endl;
    std::cout << "  file [media file]" << std::endl;
    std::cout << "  track [track num]" << std::endl;
    std::cout << "  frame / f" << std::endl;
    std::cout << "  skip [n frames]" << std::endl;
    std::cout << "  quit" << std::endl;
    std::cout << "  help" << std::endl;
    return true;
  }

  bool LoadFile(std::string file) {
    // What even happens if this fails???
    url_ = std::make_unique<OnTheFilesystem>(file);
    glue_ = std::make_unique<media::FFmpegGlue>(url_.get());
    track_codecs_.clear();
    return true;
  }

  bool FindAndEchoTracks() {
    glue_->OpenContext(true); // should be local to filesystem.
    if (!accepted_codec(glue_->container())) {
      puts("Unsupported container");
      return false;
    }

    // rewrite this.
    AVFormatContext *ctx = glue_->format_context();
    av_opt_set(ctx, "skip_clear", "1", AV_OPT_SEARCH_CHILDREN);
    track_count_ = ctx->nb_streams;
    std::cout << "[";
    bool needs_comma = false;
    for (int i = 0; i < track_count_; i++) {
      if (needs_comma) std::cout << ",";
      AVStream *stream = ctx->streams[i];
      std::string codec = CodecName(stream->codecpar->codec_id);
      track_codecs_.push_back(codec);
      std::cout << "{\"id\": " << i << ",\"type\":\"";
      std::cout << CodecType(stream->codecpar->codec_type);
      std::cout << "\",\"codec\":\"";
      std::cout << codec;
      std::cout << "\"}";
      needs_comma = true;
    }
    std::cout << "]" << std::endl;

    state_ = State::kNoStreamSelected;
    return true;
  }

  bool SelectTrack(int track) {
    if (track < 0 || track > track_count_) {
      std::cout << "{\"error\":\"track " << track << "out of range [0-";
      std::cout << (track_count_-1) << "]\"}" << std::endl;
    }
    selected_track_ = track;
    std::cout << "{\"message\":\"Selected track #" << track;
    std::cout << ". codec is " << track_codecs_[track] << "\"}" << std::endl;
    decoder_ = CreateDecoder(track_codecs_[selected_track_]);
    if (decoder_)
      state_ = State::kAwaitingTick;
    return !!decoder_;
  }

  bool interpret(std::string line) {
#define CHECK_STATE(STATE, ERR) if (state_ != State::STATE) {  \
    std::cout << "{\"error\":\"" << ERR << "\"}" << std::endl; \
    return false; }

    if (line == "help") {
      return ShowHelp();
    }

    if (line.rfind("file ", 0) == 0) {
      return LoadFile(line.substr(5)) && FindAndEchoTracks();
    }

    if (line.rfind("track ", 0) == 0) {
      CHECK_STATE(kNoStreamSelected, "You need to have just loaded a file");
      return SelectTrack(std::stoi(line.substr(6)));
    }

    if (line == "frame" || line == "f") {
      CHECK_STATE(kAwaitingTick, "You havent selected a track");
      return WriteNextFrame();
    }

    if (line.rfind("skip ", 0) == 0) {
      CHECK_STATE(kAwaitingTick, "You havent selected a track");
      return SkipFrames(std::stoi(line.substr(5)));
    }

    std::cout << "{\"error\":\"unrecognized command. try help.\"}" << std::endl;
    return true;
  }

  void AddFrameDict(base::Value frameDict) {
    backlog_.emplace_back(std::move(frameDict));
  }

  std::unique_ptr<media::AcceleratedVideoDecoder> CreateDecoder(std::string codec) {
    if (codec == "vp9") {
      return std::make_unique<media::VP9Decoder>(
        std::make_unique<VP9StructLogger>(
          base::BindRepeating(&StateMachine::AddFrameDict, base::Unretained(this))),
        media::VP9PROFILE_PROFILE2);  // YOLO who cares about profile 2 anyway? 
    }

    std::cout << "Can't create decoder for codec " << codec << std::endl;
    return nullptr;
  }

  std::string CodecType(AVMediaType type) {
    switch(type) {
      case AVMEDIA_TYPE_VIDEO: return "Video";
      case AVMEDIA_TYPE_AUDIO: return "Audio";
      case AVMEDIA_TYPE_SUBTITLE: return "Text";
      default: return "UNKNOWN";
    }
  }

  std::string CodecName(AVCodecID id) {
    const AVCodecDescriptor *desc = avcodec_descriptor_get(id);
    return desc ? desc->name : "UNKNOWN";
  }

  bool SkipFrames(unsigned int framecount) {
    while(backlog_.size() < framecount) {
      if (!ReadPacketIntoDecoder())
        return false;
    }
    backlog_.erase(backlog_.begin(), backlog_.begin()+framecount);
    std::cout << "{\"message\":\"Skipped " << framecount;
    std::cout << " frames " << "\"}" << std::endl;
    return true;
  }

  bool WriteNextFrame() {
    if (backlog_.empty()) {
      if (!ReadPacketIntoDecoder())
        return false;
    }
    std::string params_json;
    base::JSONWriter::Write(backlog_.front(), &params_json);
    std::cout << params_json << std::endl;
    backlog_.erase(backlog_.begin());

    return true;
  }

  bool ReadPacketIntoDecoder() {
    AVPacket packet;
    do {
      if (read_nonempty_packet(glue_->format_context(), &packet)) {
        puts("Error reading packet");
        return false;
      }
    } while (packet.stream_index != selected_track_);

    int side_data_size = 0;
    uint8_t* side_data = av_packet_get_side_data(
        &packet, AV_PKT_DATA_MATROSKA_BLOCKADDITIONAL, &side_data_size);

    scoped_refptr<media::DecoderBuffer> buffer;
    if (side_data_size > 0) {
      buffer = media::DecoderBuffer::CopyFrom(packet.data,
                                       packet.size, side_data,
                                       side_data_size);
    } else {
      buffer = media::DecoderBuffer::CopyFrom(packet.data,
                                       packet.size);
    }

    av_packet_unref(&packet);
    decoder_->SetStream(-1, *buffer);

#define ERROR(ERR) \
    std::cout << "{\"error\":\"" << ERR << "\"}" << std::endl; \
    return false;

    while(true) {
      switch(decoder_->Decode()) {
        case media::AcceleratedVideoDecoder::kRanOutOfStreamData:
          // This means we finished the packet.
          return true;
        case media::AcceleratedVideoDecoder::kRanOutOfSurfaces:
          ERROR("Ran out of surfaces - how did this happen?");
        case media::AcceleratedVideoDecoder::kConfigChange:
          // This is normal, if we guessed the profile wrong.
          break;
        case media::AcceleratedVideoDecoder::kTryAgain:
          ERROR("got told to piss off by the decoder, so bye.");
        case media::AcceleratedVideoDecoder::kDecodeError:
          ERROR("Decoder Error");
        case media::AcceleratedVideoDecoder::kNeedContextUpdate:
          ERROR("Context is for scrubs.");
      }
    }
  }

 private:
  enum class State {
    kNoFile,
    kNoStreamSelected,
    kAwaitingTick,
  };

  State state_ = State::kNoFile;

  // Glue must die before url.
  std::unique_ptr<OnTheFilesystem> url_;
  std::unique_ptr<media::FFmpegGlue> glue_;
  std::unique_ptr<media::AcceleratedVideoDecoder> decoder_;
  std::vector<std::string> track_codecs_;
  int selected_track_ = -1;
  int track_count_ = -1;

  std::vector<base::Value> backlog_;
};




void command_loop() {
  std::string line;
  auto state_machine = std::make_unique<StateMachine>();
  for(;;) {
    std::getline(std::cin, line);
    if (line == "quit") return;
    if (!state_machine->interpret(line)) {
      puts("state machine error");
      return;
    }
  }
}


int main(int argc, char** argv) {
  command_loop();
}