#include "RCSiner.h"
#include "IControls.h"
#include "IPlug_include_in_plug_src.h"
#include "SineWaveshaperDisplay.h"
#include "widgets/Color.h"
#include "widgets/RCButton.h"
#include "widgets/RCLabel.h"
#include "widgets/RCPanel.h"
#include "widgets/RCSlider.h"
#include "widgets/RCStyle.h"

RCSiner::RCSiner(const InstanceInfo& info)
  : iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kAlgorithm)->InitEnum("Algorithm", 0, mSineWaveshaper.Algorithms);
  GetParam(kPull)->InitDouble("Pull", 1., .5, 16., .001, "", 0, "", IParam::ShapeExp());
  GetParam(kSqueeze)->InitDouble("Squeeze", 1., .25, 4., .001, "", 0, "", IParam::ShapeExp());
  GetParam(kCurve)->InitDouble("Curve", 1., .25, 4., .001, "", 0, "", IParam::ShapeExp());
  GetParam(kStages)->InitDouble("Stages", 1, 1, 8, .01);
  GetParam(kPreClip)->InitBool("Pre Clip", 0);
  GetParam(kPostClip)->InitBool("Post Clip", 0);
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
    const Color::HSLA colorMain = Color::HSLA(4, .36f, .28f);
    const Color::HSLA colorPluginBG = colorMain.Scaled(0.f, -.5f, -.05f);
    const Color::HSLA colorWaveform = colorMain.Scaled(0.f, -.8f, .8f);
    const Color::HSLA colorWaveformSectionBG = colorMain.Scaled(0.f, -.4f, -.55f);
    const Color::HSLA colorWaveformSectionBorder = colorMain.Scaled(0.f, -.75f, .5f);
    const Color::HSLA colorWaeformBG = colorMain.Scaled(0.f, -.8f, .8f);
    const Color::HSLA colorDryWet = colorMain.Scaled(0.f, .2f, .2f);
    const Color::HSLA colorInput = colorMain.Adjusted(180).Scaled(0.f, -.9f, .7f);
    const Color::HSLA colorOutput = colorMain.Scaled(0.f, .3f, .6f);
    const Color::HSLA colorSelector = colorMain.Scaled(0.f, -.5f, .6f);
    const Color::HSLA colorControls = colorMain.Scaled(0.f, 0.f, .05f);
    const Color::HSLA colorControlsSectionBG = colorMain.Scaled(0.f, -.8f, -.35f);
    const Color::HSLA colorControlsSectionBorder = colorMain.Scaled(0.f, -.8f, .8f);
    const Color::HSLA colorPull = colorControls.Adjusted(-15);
    const Color::HSLA colorSqueeze = colorControls.Adjusted(15);
    const Color::HSLA colorCurve = colorControls.Adjusted(45);
    const Color::HSLA colorStages = colorControls.Adjusted(75);

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
      float s = color.mL < .5f ? .2f : color.mS;
      return GetSectionColor(color).Scaled(0.f, s, color.mL < .5f ? .1f : -.1f);
    };

    // STYLES
    const RCStyle styleMain = DEFAULT_RCSTYLE.WithColor(colorMain).WithValueTextSize(18.f).WithValueTextFont("FiraSans-Regular");
    const RCStyle stylePanelBG = styleMain.WithRoundness(2.f).WithDrawFrame(false).WithDrawBG(true);
    const RCStyle styleText = styleMain.WithValueTextSize(16.f).WithValueTextFont("FiraSans-Regular").WithDrawFrame(false);
    const RCStyle styleHeaderText = styleText.WithValueTextFont("FiraSans-SemiBold");
    const RCStyle styleController = styleMain.WithDrawFrame().WithDrawBG().WithFrameThickness(1.f);

    auto AddPanelBG = [&](const IRECT bounds, const Color::HSLA color) { pGraphics->AttachControl(new RCPanelBackground(bounds, stylePanelBG.WithColor(color.Scaled(0.f, -.25f, -.35f)))); };

    pGraphics->AttachPanelBackground(colorPluginBG.AsIColor());

    // General Layout
    const IRECT rectContent = pGraphics->GetBounds();
    const float sizeMargin = 8.f;
    const float sizeMarginModule = 4.f;
    const float sizeBorderModule = 2.f;
    const float sizePaddingModule = 4.f;
    const float gapModule = 4.f;
    const float widthWaveformControl = 36.f;
    const IRECT rectContentInMargin = rectContent.GetOffset(sizeMargin, 0.f, -sizeMargin, -sizeMargin);
    IRECT rectCurrent = rectContentInMargin;
    const IRECT rectHeader = rectCurrent.ReduceFromTop(36.f);
    rectCurrent.ReduceFromTop(gapModule);
    const float widthWaveform = rectContentInMargin.W() - (widthWaveformControl + sizePaddingModule + sizePaddingModule) * 2.f;
    const float heightWaveform = widthWaveform;
    const float heightWaveformSelector = 32.f;
    const float heightWaveformSection = heightWaveform + heightWaveformSelector + sizePaddingModule * 3;
    const float heightWaveformClip = 22.f;
    const float heightControlsLabel = 22.f;

    const IRECT rectWaveform = rectCurrent.ReduceFromTop(heightWaveformSection);
    rectCurrent.ReduceFromTop(gapModule + sizeBorderModule * 2.f);
    const IRECT rectControls = rectCurrent;

    // Header Section
    const IBitmap titleBitmap = pGraphics->LoadBitmap(PNGTITLE_FN);
    WDL_String version_str;
    GetPluginVersionStr(version_str);
    const char* cString = version_str.Get();
    const IRECT rectHeaderContent = rectHeader.GetVPadded(-4.f);
    const IRECT rectHeaderTitle = rectHeaderContent.GetTranslated(1.f, 0.f).GetFromLeft(titleBitmap.W()).GetMidVPadded(titleBitmap.H() * .5f);
    const IRECT rectHeaderVersion = rectHeaderTitle.GetTranslated(rectHeaderTitle.W(), 0.f).GetFromLeft(48.f).GetReducedFromTop(10.f).GetTranslated(4.f, 0.f);
    const IRECT rectHeaderDryWet = rectHeaderContent.GetReducedFromTop(3.f);
    const IRECT rectHeaderDryWetSlider = rectHeaderDryWet.GetFromRight(128.f);
    const IRECT rectHeaderDryWetLabel = rectHeaderDryWet.GetReducedFromRight(rectHeaderDryWetSlider.W() + 4.f);

    const Color::HSLA colorHeader = colorPluginBG;

    const RCStyle styleHeader = styleMain.WithColor(colorHeader).WithDrawFrame(false).WithValueTextSize(16.f).WithValueTextFont("FiraSans-SemiBold");
    const RCStyle styleVersion = styleHeader.WithColor(GetSectionTitleLabelColor(colorHeader));
    const RCStyle styleDryWetHeader = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorControls));
    const RCStyle styleDryWet = styleController.WithColor(GetSectionWidgetColor(colorControls));

    pGraphics->AttachControl(new IBButtonControl(rectHeaderTitle, titleBitmap, [](IControl* pCaller) {}));
    pGraphics->AttachControl(new RCLabel(rectHeaderVersion, cString, EDirection::Horizontal, styleVersion, 0.0f));

    pGraphics->AttachControl(new RCLabel(rectHeaderDryWetLabel, "Dry/Wet", EDirection::Horizontal, styleDryWetHeader, 0.0f, RCLabel::End));
    pGraphics->AttachControl(new RCSlider(rectHeaderDryWetSlider, kWetness, "", RCSlider::Horizontal, styleDryWet));

    // Waveform Section
    const IRECT rectWaveformInPadding = rectWaveform.GetPadded(-sizePaddingModule);
    const IRECT rectWaveformMid = rectWaveformInPadding.GetMidHPadded(widthWaveform * .5f);
    const IRECT rectWaveformDisplay = rectWaveformMid.GetFromTop(heightWaveform);
    const IRECT rectWaveformSelector = rectWaveformMid.GetFromBottom(heightWaveformSelector);
    IRECT rectWaveformLeft = rectWaveformInPadding.GetFromLeft(widthWaveformControl);
    IRECT rectWaveformRight = rectWaveformInPadding.GetFromRight(widthWaveformControl);
    const IRECT rectWaveformInClip = rectWaveformLeft.ReduceFromBottom(heightWaveformClip);
    const IRECT rectWaveformOutClip = rectWaveformRight.ReduceFromBottom(heightWaveformClip);
    rectWaveformLeft.ReduceFromBottom(sizePaddingModule);
    rectWaveformRight.ReduceFromBottom(sizePaddingModule);
    const IRECT rectWaveformInLabel = rectWaveformLeft.ReduceFromTop(16.f);
    const IRECT rectWaveformOutLabel = rectWaveformRight.ReduceFromTop(16.f);
    rectWaveformLeft.ReduceFromTop(sizePaddingModule);
    rectWaveformRight.ReduceFromTop(sizePaddingModule);
    const IRECT rectWaveformInSlider = rectWaveformLeft;
    const IRECT rectWaveformOutSlider = rectWaveformRight;

    const RCStyle styleInput = styleController.WithColor(GetSectionWidgetColor(colorInput));
    const RCStyle styleInputLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorWaveformSectionBG)).WithValueTextSize(14.f);
    const RCStyle styleOutput = styleController.WithColor(GetSectionWidgetColor(colorOutput));
    const RCStyle styleOutputLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorWaveformSectionBG)).WithValueTextSize(14.f);
    const RCStyle styleClip = styleController.WithColor(Color::HSLA(4, .8f, .6f)).WithValueTextFont("FiraSans-SemiBold").WithValueTextSize(12.f);
    const RCStyle styleDisplay = styleController.WithColor(GetSectionWidgetColor(colorWaveform)).WithDrawFrame();
    const RCStyle styleSelector = styleController.WithColor(GetSectionWidgetColor(colorSelector));
    styleClip.Colors.Get().SetDisabledColors(colorPluginBG);

    AddPanelBG(rectWaveform.GetPadded(sizeBorderModule), colorWaveformSectionBorder);
    AddPanelBG(rectWaveform, colorWaveformSectionBG);

    pGraphics->AttachControl(new RCSwitchButton(rectWaveformInClip, kPreClip, "CLIP", styleClip));
    pGraphics->AttachControl(new RCLabel(rectWaveformInLabel, "IN", EDirection::Horizontal, styleInputLabel, 0.f));
    pGraphics->AttachControl(new RCSlider(rectWaveformInSlider, kInputGain, "", RCSlider::Vertical, styleInput));
    pGraphics->AttachControl(new RCSwitchButton(rectWaveformOutClip, kPostClip, "CLIP", styleClip));
    pGraphics->AttachControl(new RCLabel(rectWaveformOutLabel, "OUT", EDirection::Horizontal, styleOutputLabel, 0.f));
    pGraphics->AttachControl(new RCSlider(rectWaveformOutSlider, kOutputGain, "", RCSlider::Vertical, styleOutput));
    pGraphics->AttachControl(new RCButton(rectWaveformSelector, kAlgorithm, "", styleSelector));
    pGraphics->AttachControl(new SineWaveshaperDisplay(rectWaveformDisplay, mSineWaveshaper, styleDisplay), kCtrlSineWaveshaperDisplay);

    // Control Section
    IRECT rectControlInPadding = rectControls.GetOffset(sizePaddingModule, 0.f, -sizePaddingModule, -sizePaddingModule);
    const float heightControlsSlider = (rectControlInPadding.H() - heightControlsLabel * 4.f) / 4.f;
    const IRECT rectControlsPullLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsPullSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsSqueezeLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsSqueezeSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsCurveLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsCurveSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsStagesLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsStagesSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);

    const RCStyle stylePull = styleController.WithColor(GetSectionWidgetColor(colorPull));
    const RCStyle stylePullLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorPull)).WithValueTextSize(14.f);
    const RCStyle styleSqueeze = styleController.WithColor(GetSectionWidgetColor(colorSqueeze));
    const RCStyle styleSqueezeLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorSqueeze)).WithValueTextSize(14.f);
    const RCStyle styleCurve = styleController.WithColor(GetSectionWidgetColor(colorCurve));
    const RCStyle styleCurveLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorCurve)).WithValueTextSize(14.f);
    const RCStyle styleStages = styleController.WithColor(GetSectionWidgetColor(colorStages));
    const RCStyle styleStagesLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorStages)).WithValueTextSize(14.f);

    AddPanelBG(rectControls.GetPadded(sizeBorderModule), colorControlsSectionBorder);
    AddPanelBG(rectControls, colorControlsSectionBG);

    pGraphics->AttachControl(new RCLabel(rectControlsPullLabel, "Pull (A)", EDirection::Horizontal, stylePullLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsPullSlider, kPull, "", RCSlider::Horizontal, stylePull));
    pGraphics->AttachControl(new RCLabel(rectControlsSqueezeLabel, "Squeeze (B)", EDirection::Horizontal, styleSqueezeLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsSqueezeSlider, kSqueeze, "", RCSlider::Horizontal, styleSqueeze));
    pGraphics->AttachControl(new RCLabel(rectControlsCurveLabel, "Curve (C)", EDirection::Horizontal, styleCurveLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsCurveSlider, kCurve, "", RCSlider::Horizontal, styleCurve));
    pGraphics->AttachControl(new RCLabel(rectControlsStagesLabel, "Stages", EDirection::Horizontal, styleStagesLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsStagesSlider, kStages, "", RCSlider::Horizontal, styleStages));
  };
#endif
}

#if IPLUG_DSP
void RCSiner::OnParamChange(int idx)
{
  auto value = GetParam(idx)->Value();
  switch (idx)
  {
  case kAlgorithm:
    mSineWaveshaper.SetAlgorithm(value);
    if (const auto ui = GetUI())
      if (const auto ctrl = ui->GetControlWithTag(kCtrlSineWaveshaperDisplay))
        ctrl->SetDirty(false);
    break;
  case kPull:
    mSineWaveshaper.SetPull(value);
    if (const auto ui = GetUI())
      if (const auto ctrl = ui->GetControlWithTag(kCtrlSineWaveshaperDisplay))
        ctrl->SetDirty(false);
    break;
  case kSqueeze:
    mSineWaveshaper.SetSqueeze(value);
    if (const auto ui = GetUI())
      if (const auto ctrl = ui->GetControlWithTag(kCtrlSineWaveshaperDisplay))
        ctrl->SetDirty(false);
    break;
  case kCurve:
    mSineWaveshaper.SetCurve(value);
    if (const auto ui = GetUI())
      if (const auto ctrl = ui->GetControlWithTag(kCtrlSineWaveshaperDisplay))
        ctrl->SetDirty(false);
    break;
  case kStages:
    mSineWaveshaper.SetStages(value);
    if (const auto ui = GetUI())
      if (const auto ctrl = ui->GetControlWithTag(kCtrlSineWaveshaperDisplay))
        ctrl->SetDirty(false);
    break;
  case kPreClip:
    mSineWaveshaper.SetPreClip(value);
    break;
  case kPostClip:
    mSineWaveshaper.SetPostClip(value);
    break;
  }
}

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
