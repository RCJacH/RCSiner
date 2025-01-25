#pragma once

#include "widgets/Color.h"
#include "widgets/RCButtonControlBase.h"
#include "widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A vector "tab" multi switch control. Click tabs to cycle through states. */
class RCButton : public RCButtonControlBase
{
public:
  /** Constructs a vector tab switch control, with an action function (no parameter)
   * @param bounds The control's bounds
   * @param aF An action function to execute when a button is clicked \see IActionFunction
   * @param label The IVControl label CString
   * @param style The styling of this vector control \see IVStyle
   * @param direction The direction of the buttons */
  RCButton(const IRECT& bounds, IActionFunction aF, const char* label = "", const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal);

  /** Constructs a vector tab switch control, linked to a parameter
   * @param bounds The control's bounds
   * @param paramIdx The parameter index to link this control to
   * @param label The IVControl label CString
   * @param style The styling of this vector control \see IVStyle
   * @param direction The direction of the buttons */
  RCButton(const IRECT& bounds, int paramIdx, const char* label = "", const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal);

  virtual ~RCButton() {}
  void Draw(IGraphics& g) override;
  void OnInit() override;

  void SetValueStr(const char* str) { mLabel = str; };

  virtual void DrawWidget(IGraphics& g);
  virtual void DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds);
  virtual void DrawButtonText(IGraphics& g, const IRECT& bounds, WidgetColorSet colorset);
  virtual const char* GetDisplayText();

protected:
  EDirection mDirection;
  RCStyle mStyle;
  const char* mLabel;
};

RCButton::RCButton(const IRECT& bounds, IActionFunction aF, const char* label, const RCStyle& style, EDirection direction)
  : RCButtonControlBase(bounds, aF)
  , mDirection(direction)
  , mStyle(style)
  , mLabel(label)
{
  mText = style.valueText;
  mText.mAlign = mStyle.valueText.mAlign = EAlign::Center;
  mText.mVAlign = mStyle.valueText.mVAlign = EVAlign::Middle;
}

RCButton::RCButton(const IRECT& bounds, int paramIdx, const char* label, const RCStyle& style, EDirection direction)
  : RCButtonControlBase(bounds, paramIdx)
  , mDirection(direction)
  , mStyle(style)
  , mLabel(label)
{
  mText = style.valueText;
  mText.mAlign = mStyle.valueText.mAlign = EAlign::Center;
  mText.mVAlign = mStyle.valueText.mVAlign = EVAlign::Middle;
}

void RCButton::OnInit() { RCButtonControlBase::OnInit(); }
void RCButton::Draw(IGraphics& g) { DrawWidget(g); }


void RCButton::DrawWidget(IGraphics& g)
{
  int valueIdx = 0;
  if (mLabel == nullptr || strcmp(mLabel, "") == 0)
  {
    auto pParam = GetParam();
    double value = pParam->Value();
    double step = pParam->GetStep();
    valueIdx = static_cast<int>(value / step);
  }

  const WidgetColorSet colorset = mStyle.GetColors(mMouseControl.IsHovering(), mMouseControl.IsLDown(), IsDisabled(), valueIdx);
  DrawBG(g, colorset, mRECT);
  DrawButtonText(g, mRECT, colorset);
}

void RCButton::DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds)
{
  const float borderWidth = mStyle.drawFrame ? mStyle.frameThickness : 0.f;
  IColor frameColor = colorset.GetBorderColor();
  IRECT borderBounds = mRECT;
  IRECT valueBounds = borderBounds.GetPadded(-borderWidth);
  if (mStyle.drawBG)
  {
    const IRECT bgBounds = mRECT.GetPadded(-(borderWidth * .5f));
    g.FillRect(colorset.GetColor(), valueBounds, &mBlend);
  }
  else if (!mStyle.drawFrame)
  {
    frameColor = colorset.GetColor();
    borderBounds = borderBounds.GetPadded(-borderWidth * .5f);
  }

  if (borderWidth)
  {
    g.DrawRect(frameColor, borderBounds, &mBlend, borderWidth);
  }
}

void RCButton::DrawButtonText(IGraphics& g, const IRECT& r, WidgetColorSet color)
{
  const char* textStr = GetDisplayText();

  if (CStringHasContents(textStr))
  {
    IColor textColor = mStyle.drawBG ? color.GetLabelColor() : color.GetColor();
    if (mStyle.drawBG && color.GetCoveredContrast() < 1.5f)
      textColor.Contrast(-.5f);
    const IText& text = mStyle.GetText().WithFGColor(textColor);
    g.DrawText(text, textStr, r, &mBlend);
  }
}

const char* RCButton::GetDisplayText()
{
  if (std::strlen(mLabel))
    return mLabel;

  auto pParam = GetParam();
  if (GetParam())
    return pParam->GetDisplayText(pParam->Value());

  return "";
}


class RCSwitchButton : public RCButton
{
public:
  RCSwitchButton(const IRECT& bounds, IActionFunction aF, const char* label = "", const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal);
  RCSwitchButton(const IRECT& bounds, int paramIdx, const char* label = "", const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal);

  virtual ~RCSwitchButton() {}

  virtual void DrawWidget(IGraphics& g) override;
};

RCSwitchButton::RCSwitchButton(const IRECT& bounds, IActionFunction aF, const char* label, const RCStyle& style, EDirection direction)
  : RCButton(bounds, aF, label, style, direction) {};

RCSwitchButton::RCSwitchButton(const IRECT& bounds, int paramIdx, const char* label, const RCStyle& style, EDirection direction)
  : RCButton(bounds, paramIdx, label, style, direction) {};

void RCSwitchButton::DrawWidget(IGraphics& g)
{
  const WidgetColorSet colorset = mStyle.GetColors(mMouseControl.IsHovering(), mMouseControl.IsLDown(), !(GetParam()->Value()));
  DrawBG(g, colorset, mRECT);
  DrawButtonText(g, mRECT, colorset);
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
