#pragma once

#include "BlockOversampler.h"
#include "IPlug_include_in_plug_hdr.h"
#include "ISender.h"
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
  kOverSampleOnline,
  kOverSampleOffline,
  kNumParams
};

enum ECtrlTags
{
  kCtrlTagOutputMeter = 1000, // To avoid debugging other controls being affected
  kCtrlSineWaveshaperDisplay,
  kNumCtrlTags
};

using namespace iplug;
using namespace igraphics;

class RCSiner final : public Plugin
{
public:
  RCSiner(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void OnIdle() override;
  void OnParamChange(int idx) override;
  void OnReset() override;
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;

  IPeakAvgSender<2> mOutputPeakSender;
#endif

private:
  SineWaveshaper mSineWaveshaper = SineWaveshaper();
  BlockOverSampler<sample> mOversampler = BlockOverSampler(EFactor::kNone, 2, 2, GetBlockSize());
  BlockOverSampler<sample> mOversamplerOffline = BlockOverSampler(EFactor::kNone, 2, 2, GetBlockSize());
  bool mPendingUpdateOversampler = false;
  bool mPendingUpdateOfflineOversampler = false;
};
