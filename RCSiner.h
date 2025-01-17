#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kAlgorithm = 0,
  kSync,
  kBend,
  kInputGain,
  kOutputGain,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class RCSiner final : public Plugin
{
public:
  RCSiner(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
