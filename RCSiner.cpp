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
    pGraphics->LoadFont("FiraSans-Regular", FIRASANS_REGULAR_FN);
    pGraphics->LoadFont("FiraSans-SemiBold", FIRASANS_SEMIBOLD_FN);
    pGraphics->EnableMouseOver(true);

    // COLORS
    const Color::HSLA colorMain = Color::HSLA(4, .32f, .30f);
    const Color::HSLA colorPluginBG = colorMain.Adjusted(0.f, -.15f, -.05f);
    const Color::HSLA colorWaveformSectionBG = colorMain.Scaled(0.f, -.4f, -.7f);
    const Color::HSLA colorInput = colorPluginBG;
    const Color::HSLA colorOutput = colorPluginBG;
    const Color::HSLA colorControls = colorPluginBG;
    const Color::HSLA colorPull = colorPluginBG;
    const Color::HSLA colorSqueeze = colorPluginBG;
    const Color::HSLA colorCurve = colorPluginBG;

    auto GetSectionColor = [&](Color::HSLA color) { return color.Scaled(0.f, 0.f, .2f); };
    auto GetSectionBGColor = [&](Color::HSLA color) { return color.Scaled(0.f, -.8f, -.6f); };
    auto GetSectionTitleLabelColor = [&](const Color::HSLA color) {
      float invert = color.mL < .5f ? 1 : -1;
      return color.Scaled(0, -.8f, invert * .6f);
    };
    auto GetSectionLabelColor = [&](const Color::HSLA color) {
      float invert = color.mL < .5f ? 1 : -1;
      return color.Scaled(0, -.8f, invert * .8f);
    };
    auto GetSectionWidgetColor = [&](const Color::HSLA color) {
      float s = color.mL < .5f ? .2f : .5f;
      return GetSectionColor(color).Scaled(0.f, s, color.mL < .5f ? .1f : -.1f);
    };

    // STYLES
    const RCStyle styleMain = DEFAULT_RCSTYLE.WithColor(colorMain).WithValueTextSize(18.f).WithValueTextFont("FiraSans-Regular");
    const RCStyle stylePanelBG = styleMain.WithRoundness(4.f).WithDrawFrame(false).WithDrawBG(true);
    const RCStyle styleText = styleMain.WithValueTextSize(16.f).WithValueTextFont("FiraSans-Regular").WithDrawFrame(false);
    const RCStyle styleHeaderText = styleText.WithValueTextFont("FiraSans-SemiBold");
    const RCStyle styleController = styleMain.WithDrawFrame().WithDrawBG().WithFrameThickness(1.f);

    auto AddPanelBG = [&](const IRECT bounds, const Color::HSLA color) { pGraphics->AttachControl(new RCPanelBackground(bounds, stylePanelBG.WithColor(color.Scaled(0.f, -.25f, -.35f)))); };

    pGraphics->AttachPanelBackground(colorPluginBG.AsIColor());

    // General Layout
    const IRECT rectContent = pGraphics->GetBounds();
    const float sizeMargin = 8.f;
    const IRECT rectContentInMargin = rectContent.GetPadded(-sizeMargin);
    IRECT rectCurrent = rectContentInMargin;
    const IRECT rectHeader = rectCurrent.ReduceFromTop(36.f + sizeMargin);
    const float sizeMarginWaveform = 4.f;
    const float gapWaveform = 4.f;
    const float widthWaveformControl = 40.f;
    const float widthWaveform = rectContentInMargin.W() - (widthWaveformControl + sizeMarginWaveform + gapWaveform) * 2.f;
    const float heightWaveform = widthWaveform;
    const float heightWaveformSelector = 32.f;
    const float heightWaveformSection = heightWaveform + sizeMarginWaveform * 2.f + heightWaveformSelector;
    const float heightWaveformClip = 24.f;
    const float heightControlsLabel = 22.f;

    const IRECT rectWaveform = rectCurrent.ReduceFromTop(heightWaveformSection);
    const IRECT rectControls = rectCurrent;

    // Header Section
    const IBitmap titleBitmap = pGraphics->LoadBitmap(PNGTITLE_FN);
    WDL_String version_str;
    GetPluginVersionStr(version_str);
    const char* cString = version_str.Get();
    const IRECT rectHeaderContent = rectHeader.GetOffset(4.f, 4.f, -4.f, -8.f);
    const IRECT rectHeaderTitle = rectHeaderContent.GetFromLeft(titleBitmap.W()).GetMidVPadded(titleBitmap.H() * .5f);
    const IRECT rectHeaderVersion = rectHeaderTitle.GetTranslated(rectHeaderTitle.W(), 0.f).GetFromLeft(48.f).GetReducedFromTop(10.f).GetTranslated(4.f, 0.f);
    const IRECT rectHeaderDryWetSlider = rectHeaderContent.GetFromRight(128.f);
    const IRECT rectHeaderDryWet = rectHeaderContent.GetReducedFromRight(rectHeaderDryWetSlider.W() + 4.f);

    const Color::HSLA colorHeader = colorPluginBG;

    const RCStyle styleHeader = styleMain.WithColor(colorHeader).WithDrawFrame(false).WithValueTextSize(16.f).WithValueTextFont("FiraSans-SemiBold");
    const RCStyle styleVersion = styleHeader.WithColor(GetSectionTitleLabelColor(colorHeader));
    const RCStyle styleDryWetHeader = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorControls));
    const RCStyle styleDryWet = styleController.WithColor(GetSectionWidgetColor(colorControls));

    pGraphics->AttachControl(new IBButtonControl(rectHeaderTitle, titleBitmap, [](IControl* pCaller) {}));
    pGraphics->AttachControl(new RCLabel(rectHeaderVersion, cString, EDirection::Horizontal, styleVersion, 0.0f));

    pGraphics->AttachControl(new RCLabel(rectHeaderDryWet, "Dry/Wet", EDirection::Horizontal, styleDryWetHeader, 0.0f, RCLabel::End));
    pGraphics->AttachControl(new RCSlider(rectHeaderDryWetSlider, kWetness, "", RCSlider::Horizontal, styleDryWet));

    // Waveform Section
    const IRECT rectWaveformInPadding = rectWaveform.GetPadded(-sizeMarginWaveform);
    const IRECT rectWaveformMid = rectWaveformInPadding.GetMidHPadded(widthWaveform * .5f);
    const IRECT rectWaveformLeft = rectWaveformInPadding.GetFromLeft(widthWaveformControl);
    const IRECT rectWaveformRight = rectWaveformInPadding.GetFromRight(widthWaveformControl);
    const IRECT rectWaveformDisplay = rectWaveformMid.GetFromTop(heightWaveform);
    const IRECT rectWaveformSelector = rectWaveformMid.GetFromBottom(heightWaveformSelector);
    const IRECT rectWaveformInClip = rectWaveformLeft.GetFromTop(heightWaveformClip);
    const IRECT rectWaveformOutClip = rectWaveformRight.GetFromTop(heightWaveformClip);
    const IRECT rectWaveformInLabel = rectWaveformLeft.GetFromBottom(24.f);
    const IRECT rectWaveformOutLabel = rectWaveformRight.GetFromBottom(24.f);
    const IRECT rectWaveformInSlider = rectWaveformLeft.GetReducedFromTop(rectWaveformInClip.H() + 4.f).GetReducedFromBottom(rectWaveformInLabel.H() + 2.f);
    const IRECT rectWaveformOutSlider = rectWaveformRight.GetReducedFromTop(rectWaveformOutClip.H() + 4.f).GetReducedFromBottom(rectWaveformOutLabel.H() + 2.f);

    const RCStyle styleInput = styleController.WithColor(GetSectionWidgetColor(colorInput));
    const RCStyle styleInputLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorInput)).WithValueTextSize(16.f);
    const RCStyle styleOutput = styleController.WithColor(GetSectionWidgetColor(colorOutput));
    const RCStyle styleOutputLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorOutput)).WithValueTextSize(16.f);
    const RCStyle styleClip = styleController.WithColor(Color::HSLA(4, .8f, .6f)).WithValueTextFont("FiraSans-SemiBold").WithValueTextSize(12.f);

    AddPanelBG(rectWaveform, colorWaveformSectionBG);

    pGraphics->AttachControl(new RCButton(rectWaveformInClip, [](IControl* pCaller) {}, "CLIP", styleClip));
    pGraphics->AttachControl(new RCLabel(rectWaveformInLabel, "IN", EDirection::Horizontal, styleInputLabel, 0.f));
    pGraphics->AttachControl(new RCSlider(rectWaveformInSlider, kInputGain, "", RCSlider::Vertical, styleInput));
    pGraphics->AttachControl(new RCButton(rectWaveformOutClip, [](IControl* pCaller) {}, "CLIP", styleClip));
    pGraphics->AttachControl(new RCLabel(rectWaveformOutLabel, "OUT", EDirection::Horizontal, styleOutputLabel, 0.f));
    pGraphics->AttachControl(new RCSlider(rectWaveformOutSlider, kOutputGain, "", RCSlider::Vertical, styleOutput));

    // Control Section
    IRECT rectControlInPadding = rectControls;
    const float heightControlsSlider = (rectControlInPadding.H() - heightControlsLabel * 3.f) / 3.f;
    const IRECT rectControlsPullLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsPullSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsSqueezeLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsSqueezeSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsCurveLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsCurveSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);

    const RCStyle stylePull = styleController.WithColor(GetSectionWidgetColor(colorPull));
    const RCStyle stylePullLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorPull)).WithValueTextSize(14.f);
    const RCStyle styleSqueeze = styleController.WithColor(GetSectionWidgetColor(colorSqueeze));
    const RCStyle styleSqueezeLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorSqueeze)).WithValueTextSize(14.f);
    const RCStyle styleCurve = styleController.WithColor(GetSectionWidgetColor(colorCurve));
    const RCStyle styleCurveLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorCurve)).WithValueTextSize(14.f);

    pGraphics->AttachControl(new RCLabel(rectControlsPullLabel, "Pull", EDirection::Horizontal, stylePullLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsPullSlider, kPull, "", RCSlider::Horizontal, stylePull));
    pGraphics->AttachControl(new RCLabel(rectControlsSqueezeLabel, "Squeeze", EDirection::Horizontal, styleSqueezeLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsSqueezeSlider, kSqueeze, "", RCSlider::HorizontalSplit, styleSqueeze));
    pGraphics->AttachControl(new RCLabel(rectControlsCurveLabel, "Curve", EDirection::Horizontal, styleCurveLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsCurveSlider, kCurve, "", RCSlider::HorizontalSplit, styleCurve));
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
