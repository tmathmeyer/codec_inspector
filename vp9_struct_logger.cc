
#include "media/tools/codec_inspector/vp9_struct_logger.h"
#include "media/base/media_serializers_base.h"


VP9PictureSerializer::VP9PictureSerializer() {}
VP9PictureSerializer::~VP9PictureSerializer() {}


#define FIELD_SERIALIZE(NAME, CONSTEXPR) \
  result.SetKey(NAME, MediaSerialize(CONSTEXPR))

template<typename L>
base::Value SerializeOneDimension(L* array, size_t size) {
  base::Value result(base::Value::Type::LIST);
  for (size_t i = 0; i < size; i++) {
    result.Append(media::MediaSerialize(array[i]));
  }
  return result;
}

template<int X, int Y, typename L>
base::Value SerializeTwoDimension(L array[][Y]) {
  base::Value result(base::Value::Type::LIST);
  for (size_t i = 0; i < X; i++) {
    result.Append(SerializeOneDimension<L>(array[i], Y));
  }
  return result;
}

template<int X, int Y, int Z, typename L>
base::Value SerializeThreeDimension(L array[][Y][Z]) {
  base::Value result(base::Value::Type::LIST);
  for (size_t i = 0; i < X; i++) {
    result.Append(SerializeTwoDimension<Y, Z, L>(array[i]));
  }
  return result;
}

// Class (complex)
template <>
struct media::internal::MediaSerializer<media::Vp9SegmentationParams> {
  static base::Value Serialize(const Vp9SegmentationParams& value) {
    base::Value result(base::Value::Type::DICTIONARY);
    // Normal things
    FIELD_SERIALIZE("enabled", value.enabled);
    FIELD_SERIALIZE("update_map", value.update_map);
    FIELD_SERIALIZE("temporal_update", value.temporal_update);
    FIELD_SERIALIZE("update_data", value.update_data);
    FIELD_SERIALIZE("abs_or_delta_update", value.abs_or_delta_update);

    result.SetKey("tree_probs", SerializeOneDimension(
      value.tree_probs, media::Vp9SegmentationParams::kNumTreeProbs));

    result.SetKey("pred_probs", SerializeOneDimension(
      value.pred_probs, media::Vp9SegmentationParams::kNumPredictionProbs));

    result.SetKey("feature_enabled", SerializeTwoDimension<8, 4>(
      value.feature_enabled));

    result.SetKey("feature_data", SerializeTwoDimension<8, 4>(
      value.feature_data));

    result.SetKey("y_dequant", SerializeTwoDimension<8, 2>(
      value.y_dequant));

    result.SetKey("uv_dequant", SerializeTwoDimension<8, 2>(
      value.uv_dequant));

    return result;
  }
};

// Class (complex)
template <>
struct media::internal::MediaSerializer<media::Vp9LoopFilterParams> {
  static base::Value Serialize(const media::Vp9LoopFilterParams& value) {
    base::Value result(base::Value::Type::DICTIONARY);

    FIELD_SERIALIZE("level", value.level);
    FIELD_SERIALIZE("sharpness", value.sharpness);
    FIELD_SERIALIZE("delta_enabled", value.delta_enabled);
    FIELD_SERIALIZE("delta_update", value.delta_update);

    result.SetKey("update_ref_deltas", SerializeOneDimension(
      value.update_ref_deltas, 4)); // VP9_FRAME_MAX

    result.SetKey("ref_deltas", SerializeOneDimension(
      value.ref_deltas, 4)); // VP9_FRAME_MAX

    result.SetKey("update_mode_deltas", SerializeOneDimension(
      value.update_mode_deltas, 2));

    result.SetKey("mode_deltas", SerializeOneDimension(
      value.mode_deltas, 2));

    result.SetKey("lvl", SerializeThreeDimension<8, 4, 2>(
      value.lvl));

    return result;
  }
};

// Class (complex)
template <>
struct media::internal::MediaSerializer<media::Vp9ReferenceFrameVector> {
  static base::Value Serialize(const media::Vp9ReferenceFrameVector& value) {
    base::Value result(base::Value::Type::DICTIONARY);
    FIELD_SERIALIZE("This Should be an array of frames",
                    std::string("But we aren't including it to save space"));
    return result;
  }
};

// Class (complex)
template <>
struct media::internal::MediaSerializer<VP9PictureSerializer> {
  static base::Value Serialize(const VP9PictureSerializer& value) {
    base::Value result(base::Value::Type::DICTIONARY);
    FIELD_SERIALIZE("SegmentationParams", value.segParams);
    FIELD_SERIALIZE("FilterParams", value.filterParams);
    result.SetKey("ReferenceFrameVector", value.reffrVec.Clone());
    return result;
  }
};

#undef FIELD_SERIALIZE





VP9StructLogger::VP9StructLogger(base::RepeatingCallback<void(base::Value)> cb)
    : OnOutput(std::move(cb)) {}

VP9StructLogger::~VP9StructLogger() {}

scoped_refptr<media::VP9Picture> VP9StructLogger::CreateVP9Picture() {
  return base::MakeRefCounted<VP9PictureSerializer>();
}

bool VP9StructLogger::SubmitDecode(scoped_refptr<media::VP9Picture> picture,
                  const media::Vp9SegmentationParams& segmentation_params,
                  const media::Vp9LoopFilterParams& loop_filter_params,
                  const media::Vp9ReferenceFrameVector& reference_frames,
                  base::OnceClosure on_finished_cb) {
  VP9PictureSerializer* local = 
    static_cast<VP9PictureSerializer*>(picture.get());

  local->segParams = segmentation_params;
  local->filterParams = loop_filter_params;
  local->reffrVec = media::MediaSerialize(reference_frames);
  
  if (on_finished_cb)
    std::move(on_finished_cb).Run();
  return true;
}

bool VP9StructLogger::OutputPicture(scoped_refptr<media::VP9Picture> picture) {
  VP9PictureSerializer* local = 
    static_cast<VP9PictureSerializer*>(picture.get());
  OnOutput.Run(media::MediaSerialize(*local));
  return true;
}

bool VP9StructLogger::IsFrameContextRequired() const {
  return false;
}

bool VP9StructLogger::GetFrameContext(scoped_refptr<media::VP9Picture> picture,
                     media::Vp9FrameContext* frame_context) {
  return false;
}

