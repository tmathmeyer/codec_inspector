
#ifndef VP9_STRUCT_LOGGER_H_
#define VP9_STRUCT_LOGGER_H_

#include "media/gpu/vp9_decoder.h"
#include "media/gpu/vp9_picture.h"
#include "base/bind.h"

class VP9PictureSerializer : public media::VP9Picture {
 public:
  VP9PictureSerializer();

  media::Vp9SegmentationParams segParams;
  media::Vp9LoopFilterParams filterParams;
  base::Value reffrVec;

 protected:
  ~VP9PictureSerializer() override;
};


class VP9StructLogger : public media::VP9Decoder::VP9Accelerator {
 public:
  VP9StructLogger(base::RepeatingCallback<void(base::Value)> cb);
  ~VP9StructLogger() override;

  scoped_refptr<media::VP9Picture> CreateVP9Picture() override;

  bool SubmitDecode(scoped_refptr<media::VP9Picture> picture,
                    const media::Vp9SegmentationParams& segmentation_params,
                    const media::Vp9LoopFilterParams& loop_filter_params,
                    const media::Vp9ReferenceFrameVector& reference_frames,
                    base::OnceClosure on_finished_cb) override;

  bool OutputPicture(scoped_refptr<media::VP9Picture> picture) override;

  bool IsFrameContextRequired() const override;

  bool GetFrameContext(scoped_refptr<media::VP9Picture> picture,
                       media::Vp9FrameContext* frame_context) override;

 private:
  base::RepeatingCallback<void(base::Value)> OnOutput;
};

#endif