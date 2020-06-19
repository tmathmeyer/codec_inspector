
#include "media/filters/ffmpeg_glue.h"

bool accepted_codec(media::container_names::MediaContainerName name) {
  switch(name) {
    case media::container_names::MediaContainerName::CONTAINER_UNKNOWN:
      puts(" Unknown");
      break;
    case media::container_names::MediaContainerName::CONTAINER_AAC:
      puts("AAC (Advanced Audio Coding)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_AC3:
      puts("AC-3");
      break;
    case media::container_names::MediaContainerName::CONTAINER_AIFF:
      puts("AIFF (Audio Interchange File Format)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_AMR:
      puts("AMR (Adaptive Multi-Rate Audio)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_APE:
      puts("APE (Monkey's Audio)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_ASF:
      puts("ASF (Advanced / Active Streaming Format)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_ASS:
      puts("SSA (SubStation Alpha) subtitle");
      break;
    case media::container_names::MediaContainerName::CONTAINER_AVI:
      puts("AVI (Audio Video Interleaved)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_BINK:
      puts("Bink");
      break;
    case media::container_names::MediaContainerName::CONTAINER_CAF:
      puts("CAF (Apple Core Audio Format)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_DTS:
      puts("DTS");
      break;
    case media::container_names::MediaContainerName::CONTAINER_DTSHD:
      puts("DTS-HD");
      break;
    case media::container_names::MediaContainerName::CONTAINER_DV:
      puts("DV (Digital Video)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_DXA:
      puts("DXA");
      break;
    case media::container_names::MediaContainerName::CONTAINER_EAC3:
      puts("Enhanced AC-3");
      break;
    case media::container_names::MediaContainerName::CONTAINER_FLAC:
      puts("FLAC (Free Lossless Audio Codec)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_FLV:
      puts("FLV (Flash Video)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_GSM:
      puts("GSM (Global System for Mobile Audio)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_H261:
      puts("H.261");
      break;
    case media::container_names::MediaContainerName::CONTAINER_H263:
      puts("H.263");
      break;
    case media::container_names::MediaContainerName::CONTAINER_H264:
      puts("H.264");
      break;
    case media::container_names::MediaContainerName::CONTAINER_HLS:
      puts("HLS (Apple HTTP Live Streaming PlayList)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_IRCAM:
      puts("Berkeley/IRCAM/CARL Sound Format");
      break;
    case media::container_names::MediaContainerName::CONTAINER_MJPEG:
      puts("MJPEG video");
      break;
    case media::container_names::MediaContainerName::CONTAINER_MOV:
      puts("QuickTime / MOV / MPEG4");
      break;
    case media::container_names::MediaContainerName::CONTAINER_MP3:
      puts("MP3 (MPEG audio layer 2/3)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_MPEG2PS:
      puts("MPEG-2 Program Stream");
      break;
    case media::container_names::MediaContainerName::CONTAINER_MPEG2TS:
      puts("MPEG-2 Transport Stream");
      break;
    case media::container_names::MediaContainerName::CONTAINER_MPEG4BS:
      puts("MPEG-4 Bitstream");
      break;
    case media::container_names::MediaContainerName::CONTAINER_OGG:
      puts("Ogg");
      break;
    case media::container_names::MediaContainerName::CONTAINER_RM:
      puts("RM (RealMedia)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_SRT:
      puts("SRT (SubRip subtitle)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_SWF:
      puts("SWF (ShockWave Flash)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_VC1:
      puts("VC-1");
      break;
    case media::container_names::MediaContainerName::CONTAINER_WAV:
      puts("WAV / WAVE (Waveform Audio)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_WEBM:
      puts("Matroska / WebM");
      return true;
      break;
    case media::container_names::MediaContainerName::CONTAINER_WTV:
      puts("WTV (Windows Television)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_DASH:
      puts("DASH (MPEG-DASH)");
      break;
    case media::container_names::MediaContainerName::CONTAINER_SMOOTHSTREAM:
      puts("thStreaming");
      break;
    }
    return false;
}