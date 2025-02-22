#include "RCSiner.h"
#include "IControls.h"
#include "IPlug_include_in_plug_src.h"
#include "OverSampleSelector.h"
#include "SineWaveshaperDisplay.h"
#include "widgets/Color.h"
#include "widgets/RCButton.h"
#include "widgets/RCDragBox.h"
#include "widgets/RCLabel.h"
#include "widgets/RCMeterControl.h"
#include "widgets/RCPanel.h"
#include "widgets/RCSlider.h"
#include "widgets/RCStyle.h"

RCSiner::RCSiner(const InstanceInfo& info)
  : iplug::Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  GetParam(kAlgorithm)->InitEnum("Algorithm", 0, mSineWaveshaper.Algorithms);
  GetParam(kSync)->InitDouble("Sync", 1., .5, 16., .001, "", 0, "", IParam::ShapeExp());
  GetParam(kPull)->InitDouble("Pull", 1., .25, 4., .001, "", 0, "", IParam::ShapeExp());
  GetParam(kDeform)->InitDouble("Deform", 1., .25, 4., .001, "", 0, "", IParam::ShapeExp());
  GetParam(kStages)->InitDouble("Stages", 1, 1, 8, .01);
  GetParam(kPreClip)->InitBool("Pre Clip", 0);
  GetParam(kPostClip)->InitBool("Post Clip", 0);
  GetParam(kInputGain)->InitDouble("Input Gain", 0., -24., 24., .1, "dB");
  GetParam(kOutputGain)->InitDouble("Output Gain", -6., -96., 12., .1, "dB", 0, "", IParam::ShapePowCurve(0.5));
  GetParam(kWetness)->InitDouble("Wetness", 100., 0., 100., .1, "%");
  GetParam(kOverSample)->InitBool("OverSample Switch", 0);
  GetParam(kOverSampleOnline)->InitEnum("OverSample", 0, {"1x", "2x", "4x", "8x", "16x"});
  GetParam(kOverSampleOffline)->InitEnum("OverSample (Render)", 0, {"Same as real-time", "1x", "2x", "4x", "8x", "16x"});

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
    const Color::HSLA colorWaveform = colorMain.Scaled(0.f, -.5f, .8f);
    const Color::HSLA colorWaveformSectionBG = colorMain.Scaled(0.f, -.4f, -.55f);
    const Color::HSLA colorWaveformSectionBorder = colorMain.Scaled(0.f, -.75f, .5f);
    const Color::HSLA colorDryWet = colorMain.Scaled(0.f, .2f, .2f);
    const Color::HSLA colorInput = colorMain.Adjusted(180).Scaled(0.f, -.95f, .7f);
    const Color::HSLA colorOutput = colorMain.Scaled(0.f, .3f, .6f);
    const Color::HSLA colorSelector = colorMain.Scaled(0.f, -.5f, .6f);
    const Color::HSLA colorControls = colorMain.Scaled(0.f, 0.f, .05f);
    const Color::HSLA colorControlsSectionBG = colorMain.Scaled(0.f, -.8f, -.35f);
    const Color::HSLA colorControlsSectionBorder = colorMain.Scaled(0.f, -.8f, .8f);
    const Color::HSLA colorSync = colorControls.Adjusted(-15);
    const Color::HSLA colorPull = colorControls.Adjusted(15);
    const Color::HSLA colorDeform = colorControls.Adjusted(45);
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
    IRECT rectHeaderControls = rectHeaderContent.GetReducedFromLeft(rectHeaderTitle.W() + rectHeaderVersion.W()).GetReducedFromTop(3.f);
    const IRECT rectHeaderVolumeMeter = rectHeaderControls.ReduceFromRight(12.f);
    rectHeaderControls.ReduceFromRight(gapModule);
    const IRECT rectHeaderDryWetSlider = rectHeaderControls.ReduceFromRight(60.f);
    rectHeaderControls.ReduceFromRight(gapModule);
    const IRECT rectHeaderDryWetLabel = rectHeaderControls.ReduceFromRight(28.f).GetReducedFromTop(7.f);
    rectHeaderControls.ReduceFromRight(gapModule);
    const IRECT rectHeaderOverSampleSlider = rectHeaderControls.ReduceFromRight(60.f);
    rectHeaderControls.ReduceFromRight(gapModule);
    const IRECT rectHeaderOverSampleLabel = rectHeaderControls.GetReducedFromTop(7.f);

    const Color::HSLA colorHeader = colorPluginBG;

    const RCStyle styleHeader = styleMain.WithColor(colorHeader).WithDrawFrame(false).WithValueTextSize(16.f).WithValueTextFont("FiraSans-SemiBold");
    const RCStyle styleVersion = styleHeader.WithColor(GetSectionTitleLabelColor(colorHeader));
    const RCStyle styleDryWetHeader = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorControls));
    const RCStyle styleDryWet = styleController.WithColor(GetSectionWidgetColor(colorControls)).WithDrawFrame(false);
    const RCStyle styleOverSample = styleController.WithColor(GetSectionWidgetColor(colorControls)).WithValueTextFont("FiraSans-SemiBold");
    const RCStyle styleMeter = styleController.WithShowValue(false);

    const IVStyle ivstyleVolumeMeter = DEFAULT_STYLE.WithShowLabel(false)
                                         .WithValueText(styleMeter.valueText.WithFGColor(styleMeter.GetColors(false, false).GetLabelColor().WithContrast(-.16f)))
                                         .WithColor(kX1, styleMeter.GetColors().mLabelColor.WithHue(44).AsIColor())
                                         .WithColor(kX2, styleMeter.GetColors().mLabelColor.Scaled(0, .8f).WithHue(0).AsIColor())
                                         .WithColor(kHL, styleMeter.GetColors(false, false, true).GetBorderColor().WithOpacity(.25f))
                                         .WithColor(kFG, styleMeter.GetColors(false, true).GetLabelColor())
                                         .WithColor(kBG, styleMeter.GetColors().mMainColor.Scaled(0, -.5f, -.5f).AsIColor().WithOpacity(.5f))
                                         .WithColor(kFR, styleMeter.GetColors(false, true).GetBorderColor().WithOpacity(.5f));

    pGraphics->AttachControl(new IBButtonControl(rectHeaderTitle, titleBitmap, [](IControl* pCaller) {}));
    pGraphics->AttachControl(new RCLabel(rectHeaderVersion, cString, EDirection::Horizontal, styleVersion, 0.0f));

    pGraphics->AttachControl(new RCDragBox(rectHeaderDryWetSlider, kWetness, "", RCDragBox::Horizontal, styleDryWet));
    pGraphics->AttachControl(new RCLabel(rectHeaderDryWetLabel, "Mix", EDirection::Horizontal, styleDryWetHeader, 0.0f, RCLabel::End));
    pGraphics->AttachControl(new OverSampleSelector(rectHeaderOverSampleSlider, kOverSample, kOverSampleOnline, kOverSampleOffline, styleOverSample));
    pGraphics->AttachControl(new RCLabel(rectHeaderOverSampleLabel, "OS", EDirection::Horizontal, styleDryWetHeader, 0.0f, RCLabel::End));
    pGraphics->AttachControl(new RCPeakAvgMeterControl<2>(rectHeaderVolumeMeter, ivstyleVolumeMeter, EDirection::Vertical, {}, 0, -90.f, 0.f, {}), kCtrlTagOutputMeter);

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
    const RCStyle styleInputLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorWaveformSectionBG)).WithValueTextSize(16.f);
    const RCStyle styleOutput = styleController.WithColor(GetSectionWidgetColor(colorOutput));
    const RCStyle styleOutputLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorWaveformSectionBG)).WithValueTextSize(16.f);
    const RCStyle styleClip = styleController.WithColor(Color::HSLA(4, .8f, .6f)).WithValueTextFont("FiraSans-SemiBold").WithValueTextSize(14.f);
    const RCStyle styleDisplay = styleController.WithColor(GetSectionWidgetColor(colorWaveform)).WithDrawFrame();
    const RCStyle styleSelector = styleController.WithColor(GetSectionWidgetColor(colorSelector));
    auto shuffleSelectorColors = [&]() {
      for (int i = 0; i < 3; i++)
      {
        auto colorset = styleSelector.GetColors(i == 1, i == 2);
        auto mainColor = colorset.mMainColor;
        auto bgColor = colorset.mBGColor;
        colorset.SetBGColor(mainColor);
        colorset.SetMainColor(bgColor);
        switch (i)
        {
        case 0:
          styleSelector.Colors.Get().normalColors = colorset;
          break;
        case 1:
          styleSelector.Colors.Get().hoverColors = colorset;
          break;
        case 2:
          styleSelector.Colors.Get().pressColors = colorset;
          break;
        }
      }
    };
    shuffleSelectorColors();
    styleClip.Colors.Get().SetDisabledColors(colorPluginBG);

    AddPanelBG(rectWaveform.GetPadded(sizeBorderModule), colorWaveformSectionBorder);
    AddPanelBG(rectWaveform, colorWaveformSectionBG);

    pGraphics->AttachControl(new RCSwitchButton(rectWaveformInClip, kPreClip, "CLIP", styleClip));
    pGraphics->AttachControl(new RCLabel(rectWaveformInLabel, "IN", EDirection::Horizontal, styleInputLabel, 0.f));
    pGraphics->AttachControl(new RCSlider(rectWaveformInSlider, kInputGain, "", RCSlider::Vertical, styleInput));
    pGraphics->AttachControl(new RCSwitchButton(rectWaveformOutClip, kPostClip, "CLIP", styleClip));
    pGraphics->AttachControl(new RCLabel(rectWaveformOutLabel, "OUT", EDirection::Horizontal, styleOutputLabel, 0.f));
    pGraphics->AttachControl(new RCSlider(rectWaveformOutSlider, kOutputGain, "", RCSlider::Vertical, styleOutput));
    pGraphics->AttachControl(new RCDragBox(rectWaveformSelector, kAlgorithm, "", RCDragBox::Horizontal, styleSelector));
    // pGraphics->AttachControl(new RCButton(rectWaveformSelector, kAlgorithm, "", styleSelector));
    pGraphics->AttachControl(new SineWaveshaperDisplay(rectWaveformDisplay, mSineWaveshaper, styleDisplay), kCtrlSineWaveshaperDisplay);

    // Control Section
    IRECT rectControlInPadding = rectControls.GetOffset(sizePaddingModule, 0.f, -sizePaddingModule, -sizePaddingModule);
    const float heightControlsSlider = (rectControlInPadding.H() - heightControlsLabel * 4.f) / 4.f;
    const IRECT rectControlsSyncLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsSyncSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsPullLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsPullSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsDeformLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsDeformSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);
    const IRECT rectControlsStagesLabel = rectControlInPadding.ReduceFromTop(heightControlsLabel);
    const IRECT rectControlsStagesSlider = rectControlInPadding.ReduceFromTop(heightControlsSlider);

    const RCStyle styleSync = styleController.WithColor(GetSectionWidgetColor(colorSync));
    const RCStyle styleSyncLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorSync)).WithValueTextSize(14.f);
    const RCStyle stylePull = styleController.WithColor(GetSectionWidgetColor(colorPull));
    const RCStyle stylePullLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorPull)).WithValueTextSize(14.f);
    const RCStyle styleDeform = styleController.WithColor(GetSectionWidgetColor(colorDeform));
    const RCStyle styleDeformLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorDeform)).WithValueTextSize(14.f);
    const RCStyle styleStages = styleController.WithColor(GetSectionWidgetColor(colorStages));
    const RCStyle styleStagesLabel = styleHeaderText.WithColor(GetSectionTitleLabelColor(colorStages)).WithValueTextSize(14.f);

    AddPanelBG(rectControls.GetPadded(sizeBorderModule), colorControlsSectionBorder);
    AddPanelBG(rectControls, colorControlsSectionBG);

    pGraphics->AttachControl(new RCLabel(rectControlsSyncLabel, "Sync (A)", EDirection::Horizontal, styleSyncLabel, 0.f, RCLabel::Position::Center));
    auto sliderSync = new RCSlider(rectControlsSyncSlider, kSync, "", RCSlider::Horizontal, styleSync);
    // sliderSync->SetRoundBy(10.f);
    pGraphics->AttachControl(sliderSync);
    pGraphics->AttachControl(new RCLabel(rectControlsPullLabel, "Pull (B)", EDirection::Horizontal, stylePullLabel, 0.f, RCLabel::Position::Center));
    auto sliderPull = new RCSlider(rectControlsPullSlider, kPull, "", RCSlider::Horizontal, stylePull);
    sliderPull->SetRoundBy(2.f);
    pGraphics->AttachControl(sliderPull);
    pGraphics->AttachControl(new RCLabel(rectControlsDeformLabel, "Deform (C)", EDirection::Horizontal, styleDeformLabel, 0.f, RCLabel::Position::Center));
    auto sliderDeform = new RCSlider(rectControlsDeformSlider, kDeform, "", RCSlider::Horizontal, styleDeform);
    sliderDeform->SetRoundBy(2.f);
    pGraphics->AttachControl(sliderDeform);
    pGraphics->AttachControl(new RCLabel(rectControlsStagesLabel, "Stages", EDirection::Horizontal, styleStagesLabel, 0.f, RCLabel::Position::Center));
    pGraphics->AttachControl(new RCSlider(rectControlsStagesSlider, kStages, "", RCSlider::Horizontal, styleStages));
  };
#endif
}

#if IPLUG_DSP
void RCSiner::OnIdle() { mOutputPeakSender.TransmitData(*this); }
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
  case kSync:
    mSineWaveshaper.SetSync(value);
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
  case kDeform:
    mSineWaveshaper.SetDeform(value);
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
  case kOverSampleOnline:
    mPendingUpdateOversampler = true;
    if (!GetParam(kOverSampleOffline)->Value())
      mPendingUpdateOfflineOversampler = true;
    break;
  case kOverSampleOffline:
    mPendingUpdateOfflineOversampler = true;
    break;
  }
}

void RCSiner::OnReset()
{
  auto blocksize = GetBlockSize();
  mOversampler.SetBlockSize(blocksize);
  mOversamplerOffline.SetBlockSize(blocksize);
  mOutputPeakSender.Reset(GetSampleRate());
}

void RCSiner::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double wetAmp = GetParam(kWetness)->Value() * .01;
  const double dryAmp = 1. - wetAmp;
  const double inGain = iplug::DBToAmp(GetParam(kInputGain)->Value());
  const double outGain = iplug::DBToAmp(GetParam(kOutputGain)->Value());
  const int nChans = NOutChansConnected();
  const auto oversample = GetParam(kOverSample)->Value();

  const auto oversampleOnline = static_cast<EFactor>(GetParam(kOverSampleOnline)->Value());
  const auto oversampleOfflineValue = GetParam(kOverSampleOffline)->Value();
  const auto oversampleOffline = !oversampleOfflineValue ? oversampleOnline : static_cast<EFactor>(oversampleOfflineValue - 1.);

  if (mPendingUpdateOversampler)
  {
    mOversampler.SetOverSampling(oversampleOnline);
    mPendingUpdateOversampler = false;
  }
  if (mPendingUpdateOfflineOversampler)
  {
    mOversamplerOffline.SetOverSampling(oversampleOffline);
    mPendingUpdateOfflineOversampler = false;
  }

  auto processFunc = [&](sample** osinputs, sample** osoutputs, int osnFrames) {
    for (int s = 0; s < osnFrames; s++)
    {
      for (int c = 0; c < nChans; c++)
      {
        const auto spl = osinputs[c][s];
        if (!spl)
          continue; // This skips calculation because SineWaveshaper always return 0 when x is 0
        osoutputs[c][s] = spl * dryAmp + mSineWaveshaper.ProcessSample(spl * inGain) * outGain * wetAmp;
      }
    }
  };

  if (oversample)
  {
    if (GetRenderingOffline())
      mOversamplerOffline.ProcessBlock(inputs, outputs, nFrames, 2, nChans, processFunc);
    else
      mOversampler.ProcessBlock(inputs, outputs, nFrames, 2, nChans, processFunc);
  }
  else
    processFunc(inputs, outputs, nFrames);

  if (GetUI())
    mOutputPeakSender.ProcessBlock(outputs, nFrames, kCtrlTagOutputMeter, 2);
}
#endif
