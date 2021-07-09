/**********************************************************************

  Sneedacity: A Digital Audio Editor

  Steve Harris
  Markus Meyer

**********************************************************************/

#ifndef __SNEEDACITY_DITHER_H__
#define __SNEEDACITY_DITHER_H__

#include "sneedacity/Types.h" // for samplePtr

template< typename Enum > class EnumSetting;


/// These ditherers are currently available:
enum DitherType : unsigned {
   none = 0, rectangle = 1, triangle = 2, shaped = 3 };

class Dither
{
public:
    static DitherType FastDitherChoice();
    static DitherType BestDitherChoice();

    static SNEEDACITY_DLL_API EnumSetting< DitherType > FastSetting, BestSetting;

    /// Default constructor
    Dither();

    /// Reset state of the dither.
    void Reset();

    /// Apply the actual dithering. Expects the source sample in the
    /// 'source' variable, the destination sample in the 'dest' variable,
    /// and hints to the formats of the samples. Even if the sample formats
    /// are the same, samples are clipped, if necessary.
    void Apply(DitherType ditherType,
               constSamplePtr source, sampleFormat sourceFormat,
               samplePtr dest, sampleFormat destFormat,
               unsigned int len,
               unsigned int sourceStride = 1,
               unsigned int destStride = 1);

private:
    // Dither methods
    float NoDither(float sample);
    float RectangleDither(float sample);
    float TriangleDither(float sample);
    float ShapedDither(float sample);

    // Dither constants
    static const int BUF_SIZE; /* = 8 */
    static const int BUF_MASK; /* = 7 */
    static const float SHAPED_BS[];

    // Dither state
    int mPhase;
    float mTriangleState;
    float mBuffer[8 /* = BUF_SIZE */];
};

#endif /* __SNEEDACITY_DITHER_H__ */
