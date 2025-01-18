#include "RCSiner.h"
#include "IControls.h"
#include "IPlug_include_in_plug_src.h"
#include "widgets/Color.h"
#include "widgets/RCButton.h"
#include "widgets/RCLabel.h"
#include "widgets/RCPanel.h"
#include "widgets/RCSlider.h"
#include "widgets/RCStyle.h"

RCSiner::RCSiner(const InstanceInfo& info)
  : iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kAlgorithm)->InitEnum("Algorithm", 0, {});
  GetParam(kPull)->InitDouble("Pull", .2, 0., 1., .001);
  GetParam(kSqueeze)->InitDouble("Squeeze", .5, 0., 1., .001);
  GetParam(kCurve)->InitDouble("Curve", .5, 0., 1., .001);
  GetParam(kInputGain)->InitDouble("Input Gain", 0., -24., 24., .1, "dB");
  GetParam(kOutputGain)->InitDouble("Output Gain", 0., -96., 24., .1, "dB");
  GetParam(kWetness)->InitDouble("Wetness", 100., 0., 100., .1, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() { return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT)); };

  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("FiraSans-Regular", FIRASANS_REGULAR_FN);
    pGraphics->LoadFont("FiraSans-SemiBold", FIRASANS_SEMIBOLD_FN);

    // COLORS
    const Color::HSLA colorMain = Color::HSLA(209, .32f, .30f);

    const IRECT b = pGraphics->GetBounds();
    // pGraphics->AttachControl(new IVKnobControl(b.GetFromTop(200).GetGridCell(0, 0, 2, 3), kAlgorithm));
    // pGraphics->AttachControl(new IVKnobControl(b.GetFromTop(200).GetGridCell(0, 1, 2, 3), kPull));
    // pGraphics->AttachControl(new IVKnobControl(b.GetFromTop(200).GetGridCell(0, 2, 2, 3), kSqueeze));
    // pGraphics->AttachControl(new IVKnobControl(b.GetFromTop(200).GetGridCell(1, 0, 2, 3), kCurve));
    // pGraphics->AttachControl(new IVKnobControl(b.GetFromTop(200).GetGridCell(1, 1, 2, 3), kInputGain));
    // pGraphics->AttachControl(new IVKnobControl(b.GetFromTop(200).GetGridCell(1, 2, 2, 3), kOutputGain));
  };
#endif
}

#if IPLUG_DSP
void RCSiner::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double wetAmp = GetParam(kWetness)->Value() * .01;
  const double dryAmp = 1. - wetAmp;
  const double inGain = GetParam(kInputGain)->Value() * .01;
  const double outGain = GetParam(kOutputGain)->Value() * .01;
  const int nChans = NOutChansConnected();

  for (int s = 0; s < nFrames; s++)
  {
    for (int c = 0; c < nChans; c++)
    {
      outputs[c][s] = inputs[c][s] * dryAmp + mSineWaveshaper.ProcessSample(inputs[c][s] * inGain) * outGain * wetAmp;
    }
  }
}
#endif
