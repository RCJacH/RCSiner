#pragma once

#include "IControl.h"
#include "IControls.h"
#include "IGraphics.h"
#include "IGraphicsStructs.h"
#include "Widgets/Color.h"
#include "Widgets/RCStyle.h"
#include "widgets/RCSliderControlBase.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class RCSlider : public RCSliderControlBase
{
public:
  enum DirectionType
  {
    Horizontal,
    HorizontalSplit,
    Vertical,
    VerticalSplit,
  };

  EDirection ToEDirection(DirectionType dir)
  {
    switch (dir)
    {
    case DirectionType::Horizontal:
    case DirectionType::HorizontalSplit:
      return EDirection::Horizontal;
    case DirectionType::Vertical:
    case DirectionType::VerticalSplit:
      return EDirection::Vertical;
    }
    return EDirection::Horizontal;
  };

  RCSlider(const IRECT& bounds,
           int paramIdx = kNoParameter,
           const char* label = "",
           DirectionType dir = DirectionType::Horizontal,
           const RCStyle& style = DEFAULT_RCSTYLE,
           bool valueIsEditable = false,
           double gearing = 1.0);

  RCSlider(const IRECT& bounds,
           IActionFunction aF,
           const char* label = "",
           DirectionType dir = DirectionType::Horizontal,
           const RCStyle& style = DEFAULT_RCSTYLE,
           bool valueIsEditable = false,
           double gearing = 1.0);

  virtual ~RCSlider() {}

  virtual void Draw(IGraphics& g);
  virtual void DrawWidget(IGraphics& g, WidgetColorSet colorset);
  virtual void DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds, float borderWidth);
  virtual void DrawHandle(IGraphics& g, WidgetColorSet colorset, IRECT bounds, EDirection dir, double pct);
  virtual void DrawValueText(IGraphics& g, WidgetColorSet colorset, IRECT bounds, EDirection dir, double pct, bool covered);

  void OnResize() override;
  void SetDirty(bool push, int valIdx = kNoValIdx) override;
  void OnInit() override;

protected:
  DirectionType mDirectionType;
  RCStyle mStyle = DEFAULT_RCSTYLE;
  WDL_String mValueStr;
};

RCSlider::RCSlider(const IRECT& bounds, int paramIdx, const char* label, DirectionType dir, const RCStyle& style, bool valueIsEditable, double gearing)
  : RCSliderControlBase(bounds, paramIdx, ToEDirection(dir), gearing, 2.f)
  , mStyle(style)
  , mDirectionType(dir)
{
  DisablePrompt(!valueIsEditable);
}

RCSlider::RCSlider(const IRECT& bounds, IActionFunction aF, const char* label, DirectionType dir, const RCStyle& style, bool valueIsEditable, double gearing)
  : RCSliderControlBase(bounds, aF, ToEDirection(dir), gearing, 2.f)
  , mStyle(style)
  , mDirectionType(dir)
{
  DisablePrompt(!valueIsEditable);
}

void RCSlider::Draw(IGraphics& g)
{
  auto color = mStyle.GetColors(mMouseControl.IsHovering(), mMouseControl.IsLDown(), IsDisabled());
  DrawWidget(g, color);
}

void RCSlider::DrawWidget(IGraphics& g, WidgetColorSet colorset)
{
  const float borderWidth = mStyle.frameThickness;
  const IRECT contentBounds = mRECT.GetPadded(-borderWidth);
  IRECT valueBounds;
  IRECT handleBounds;
  IRECT textBounds;
  EDirection fracDirection;
  bool covered = false;
  const double pct = GetValue();
  switch (mDirectionType)
  {
  case DirectionType::Horizontal:
  case DirectionType::Vertical:
    fracDirection = (mDirectionType == DirectionType::Horizontal) ? EDirection::Horizontal : EDirection::Vertical;
    valueBounds = contentBounds.FracRect(fracDirection, pct);
    handleBounds = valueBounds.FracRect(fracDirection, 0.f, true);
    covered = pct >= .5;
    break;
  case DirectionType::HorizontalSplit:
  case DirectionType::VerticalSplit:
    fracDirection = (mDirectionType == DirectionType::HorizontalSplit) ? EDirection::Horizontal : EDirection::Vertical;
    valueBounds = contentBounds.FracRect(fracDirection, .5f, pct >= .5).FracRect(fracDirection, abs(.5f - pct) * 2.f, pct < .5);
    handleBounds = valueBounds.FracRect(fracDirection, 0.f, pct >= .5);
    break;
  }

  DrawBG(g, colorset, mRECT, borderWidth);
  g.FillRect(colorset.GetColor(), valueBounds, &mBlend);
  DrawHandle(g, colorset, handleBounds, fracDirection, pct);
  DrawValueText(g, colorset, contentBounds, fracDirection, pct, covered);
}

void RCSlider::DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds, float borderWidth)
{
  IColor frameColor = colorset.GetBorderColor();
  IRECT borderBounds = mRECT;
  if (mStyle.drawBG)
  {
    const IRECT bgBounds = mRECT.GetPadded(-(borderWidth * .5f));
    g.FillRect(colorset.GetBGColor(), bgBounds, &mBlend);
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

void RCSlider::DrawHandle(IGraphics& g, WidgetColorSet colorset, IRECT bounds, EDirection dir, double pct)
{
  if (!mHandleSize || !pct || pct == 1.)
    return;

  const float halfHandleSize = mHandleSize * .5f;
  switch (dir)
  {
  case EDirection::Horizontal:
    bounds = bounds.GetHPadded(halfHandleSize);
    break;
  case EDirection::Vertical:
    bounds = bounds.GetVPadded(halfHandleSize);
    break;
  }
  g.FillRect(colorset.GetBorderColor(), bounds, &mBlend);
}

void RCSlider::DrawValueText(IGraphics& g, WidgetColorSet colorset, IRECT bounds, EDirection dir, double pct, bool covered)
{
  if (!mStyle.showValue)
    return;

  IColor textColor = colorset.GetLabelColor();
  if ((covered ? colorset.GetCoveredContrast() : colorset.GetUncoveredContrast()) < 1.5f)
    textColor.Contrast(-.618);
  const IText& text = mStyle.GetText().WithFGColor(textColor);

  switch (dir)
  {
  case EDirection::Horizontal:
    bounds = bounds.FracRectHorizontal(.5f, pct < .5);
    g.DrawText(text, mValueStr.Get(), bounds, &mBlend);
    return;
  case EDirection::Vertical:
    bounds = bounds.FracRectVertical(.5f, pct < .5);
    break;
  }

  // Vertical
  IRECT textBounds;
  auto valueStr = mValueStr.Get();
  const char* spacePos = strchr(valueStr, ' ');
  if (!spacePos)
  {
    g.DrawText(text, valueStr, bounds, &mBlend);
    return;
  }

  char* temp = new char[strlen(valueStr) + 1];
  strcpy(temp, valueStr);
  const char* value = strtok(temp, " ");
  const char* unit = strtok(nullptr, " ");
  g.MeasureText(text, value, textBounds);
  const IRECT actualBounds = bounds.GetMidVPadded(textBounds.MH());
  g.DrawText(text, value, actualBounds.GetFromBottom(textBounds.H()), &mBlend);
  g.DrawText(text, unit, actualBounds.GetFromTop(textBounds.H()), &mBlend);
}

void RCSlider::OnResize()
{
  mMouseControl.ReleaseAll();
  SetDirty(false);
}

void RCSlider::SetDirty(bool push, int valIdx)
{
  RCSliderControlBase::SetDirty(push);

  const IParam* pParam = GetParam();

  if (pParam)
    pParam->GetDisplayWithLabel(mValueStr);
}

void RCSlider::OnInit()
{
  const IParam* pParam = GetParam();

  if (pParam)
    pParam->GetDisplayWithLabel(mValueStr);
}

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
