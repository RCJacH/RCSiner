#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "Oversampler.h"
#include "SineWaveshaper.h"

const int kNumPresets = 1;

enum EParams
{
  kAlgorithm = 0,
  kPull,
  kSqueeze,
  kCurve,
  kStages,
  kPreClip,
  kPostClip,
  kInputGain,
  kOutputGain,
  kWetness,
  kOverSample,
  kNumParams
};

enum ECtrlTags
{
  kCtrlSineWaveshaperDisplay = 1000, // To avoid debugging other controls being affected
  kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class RCSiner final : public Plugin
{
public:
  RCSiner(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnParamChange(int idx) override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif

private:
  SineWaveshaper mSineWaveshaper = SineWaveshaper();
  iplug::OverSampler<iplug::sample> mOversampler = iplug::OverSampler(EFactor::kNone, true, 2, 2);
};
