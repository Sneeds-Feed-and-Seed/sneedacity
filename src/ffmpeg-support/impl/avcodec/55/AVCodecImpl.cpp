/**********************************************************************

  Sneedacity: A Digital Audio Editor

  AVCodecImpl.cpp

  Dmitry Vedenko

**********************************************************************/

extern "C"
{
#include "../../avutil/52/avconfig.h"
#include "../../ffmpeg-2.3.6-single-header.h"
}

#include <cstring>
#include <numeric>

#include "float_cast.h"

#include "FFmpegFunctions.h"

#include "wrappers/AVCodecContextWrapper.h"
#include "wrappers/AVCodecWrapper.h"
#include "wrappers/AVPacketWrapper.h"

#include "../../FFmpegAPIResolver.h"

namespace avcodec_55
{
#include "../AVCodecContextWrapperImpl.inl"
#include "../AVCodecWrapperImpl.inl"
#include "../AVPacketWrapperImpl.inl"

const bool registered = ([]() {
   FFmpegAPIResolver::Get().AddAVCodecFactories(55, {
      &CreateAVCodecContextWrapper,
      &CreateAVCodecContextWrapperFromCodec,
      &CreateAVCodecWrapper,
      &CreateAVPacketWrapper,
   });

   return true;
})();
}

#include "FFmpegTypes.h"
static_assert(
   CODEC_FLAG_GLOBAL_HEADER == SNEEDACITY_AV_CODEC_FLAG_GLOBAL_HEADER
   && CODEC_CAP_SMALL_LAST_FRAME == SNEEDACITY_AV_CODEC_CAP_SMALL_LAST_FRAME
   && CODEC_FLAG_QSCALE == SNEEDACITY_AV_CODEC_FLAG_QSCALE
,
   "FFmpeg constants don't match"
);

