#pragma once

#include "Widgets/Color.h"
#include "Widgets/RCStyle.h"
#include "widgets/RCSliderControlBase.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class RCDragBox : public RCSliderControlBase
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

  RCDragBox(const IRECT& bounds,
            int paramIdx = kNoParameter,
            const char* label = "",
            DirectionType dir = DirectionType::Horizontal,
            const RCStyle& style = DEFAULT_RCSTYLE,
            float handleSize = 2.f,
            float gapSize = 0.f,
            bool displayUnit = true,
            bool valueIsEditable = false,
            double gearing = 1.0)
    : RCSliderControlBase(bounds, paramIdx, ToEDirection(dir), gearing, handleSize)
    , mStyle(style)
    , mDirectionType(dir)
    , mDisplayUnit(displayUnit)
    , mGapSize(gapSize)
  {
    DisablePrompt(!valueIsEditable);
  };

  RCDragBox(const IRECT& bounds,
            IActionFunction aF,
            const char* label = "",
            DirectionType dir = DirectionType::Horizontal,
            const RCStyle& style = DEFAULT_RCSTYLE,
            float handleSize = 2.f,
            float gapSize = 0.f,
            bool displayUnit = true,
            bool valueIsEditable = false,
            double gearing = 1.0)
    : RCSliderControlBase(bounds, aF, ToEDirection(dir), gearing, handleSize)
    , mStyle(style)
    , mDirectionType(dir)
    , mDisplayUnit(displayUnit)
    , mGapSize(gapSize)
  {
    DisablePrompt(!valueIsEditable);
  };

  virtual ~RCDragBox() {}

  virtual void Draw(IGraphics& g);
  virtual void DrawWidget(IGraphics& g, WidgetColorSet colorset);
  virtual void DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds, float borderWidth);
  virtual void DrawValueText(IGraphics& g, WidgetColorSet colorset, IRECT bounds, EDirection dir, double pct);

  void MouseLClickAction(const IMouseMod& mod) override
  {
    if (!mIsStepped)
      return RCSliderControlBase::MouseLClickAction(mod);

    const IParam* pParam = GetParam();
    if (!pParam)
      return;
    double value = pParam->Value();
    value += pParam->GetStep();
    double max = pParam->GetMax();
    if (value > max)
      value = fmod(value + pParam->GetStep(), 1.);
    SetValue(value / max);
  }

  void OnResize() override;
  void SetDirty(bool push, int valIdx = kNoValIdx) override;
  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override;
  void OnInit() override;

protected:
  DirectionType mDirectionType;
  RCStyle mStyle = DEFAULT_RCSTYLE;
  WDL_String mValueStr;
  bool mDisplayUnit;
  float mGapSize;
  bool mIsStepped = false;
  IPopupMenu mMenu{"Options"};
};


void RCDragBox::Draw(IGraphics& g)
{
  auto color = mStyle.GetColors(mMouseControl.IsHovering(), mMouseControl.IsLDown(), IsDisabled());
  DrawWidget(g, color);
}

void RCDragBox::DrawWidget(IGraphics& g, WidgetColorSet colorset)
{
  const float borderWidth = mStyle.frameThickness;
  const IRECT contentBounds = mRECT.GetPadded(-borderWidth);
  IRECT valueBounds;
  IRECT handleBounds;
  EDirection fracDirection;
  bool covered = false;
  const double pct = GetValue();
  float margin = mHandleSize * .5f;
  switch (mDirectionType)
  {
  case DirectionType::Horizontal:
    fracDirection = EDirection::Horizontal;
    handleBounds = contentBounds.GetFromBottom(mHandleSize).GetTranslated(0.f, -margin).FracRectHorizontal(pct);
    valueBounds = contentBounds.GetPadded(-mHandleSize).GetReducedFromBottom(handleBounds.H() + mGapSize);
    break;
  case DirectionType::Vertical:
    fracDirection = EDirection::Vertical;
    handleBounds = contentBounds.GetFromRight(mHandleSize).GetTranslated(-margin, 0.f).FracRectVertical(pct);
    valueBounds = contentBounds.GetPadded(-mHandleSize).GetReducedFromRight(handleBounds.W() + mGapSize);
    break;
  case DirectionType::HorizontalSplit:
    fracDirection = EDirection::Horizontal;
    handleBounds = contentBounds.GetFromBottom(mHandleSize).GetTranslated(0.f, -margin).FracRectHorizontal(std::abs(pct - 0.5), pct >= .5);
    valueBounds = contentBounds.GetPadded(-mHandleSize).GetReducedFromBottom(handleBounds.H() + mGapSize);
    break;
  case DirectionType::VerticalSplit:
    fracDirection = EDirection::Vertical;
    handleBounds = contentBounds.GetFromRight(mHandleSize).GetTranslated(-margin, 0.f).FracRectVertical(0.f, pct >= .5);
    valueBounds = contentBounds.GetPadded(-mHandleSize).GetReducedFromRight(handleBounds.W() + mGapSize);
    break;
  }

  DrawBG(g, colorset, mRECT, borderWidth);
  g.FillRect(colorset.GetColor(), handleBounds, &mBlend);
  DrawValueText(g, colorset, valueBounds, fracDirection, pct);
}

void RCDragBox::DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds, float borderWidth)
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

void RCDragBox::DrawValueText(IGraphics& g, WidgetColorSet colorset, IRECT bounds, EDirection dir, double pct)
{
  if (!mStyle.showValue)
    return;

  IColor textColor = colorset.GetLabelColor();
  const IText& text = mStyle.GetText().WithFGColor(textColor);
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

  if (!mDisplayUnit)
  {
    g.DrawText(text, value, bounds, &mBlend);
    return;
  }
  else if (dir == EDirection::Horizontal)
  {
    g.DrawText(text, valueStr, bounds, &mBlend);
    return;
  }

  // Vertical
  IRECT textBounds;
  g.MeasureText(text, value, textBounds);
  const IRECT actualBounds = bounds.GetMidVPadded(textBounds.MH());
  g.DrawText(text, value, actualBounds.GetFromBottom(textBounds.H()), &mBlend);
  g.DrawText(text, unit, actualBounds.GetFromTop(textBounds.H()), &mBlend);
}

void RCDragBox::OnResize()
{
  mMouseControl.ReleaseAll();
  SetDirty(false);
}

void RCDragBox::SetDirty(bool push, int valIdx)
{
  RCSliderControlBase::SetDirty(push);

  const IParam* pParam = GetParam();

  if (pParam)
    pParam->GetDisplayWithLabel(mValueStr);
}

void RCDragBox::CreateContextMenu(IPopupMenu& contextMenu)
{
  mMouseControl.ReleaseL();
  if (!mIsStepped)
    return;

  mMenu.Clear(true);

  const IParam* pParam = GetParam();
  auto cur_v = pParam->Value();
  double v = pParam->GetMin();
  auto step = pParam->GetStep();
  int i = 0;
  while (v <= pParam->GetMax())
  {
    auto text = pParam->GetDisplayText(v);
    mMenu.AddItem(text);
    mMenu.CheckItem(i, v == cur_v);
    v += step;
    i++;
  }

  GetUI()->CreatePopupMenu(*this, mMenu, mMouseControl.cur_x, mMouseControl.cur_y);
}

void RCDragBox::OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx)
{
  if (pSelectedMenu)
  {
    const IParam* pParam = GetParam();
    SetValue(pParam->ToNormalized(pSelectedMenu->GetChosenItemIdx()));
    SetDirty(true);
  }
}
void RCDragBox::OnInit()
{
  const IParam* pParam = GetParam();

  if (pParam)
  {
    pParam->GetDisplayWithLabel(mValueStr);
    mIsStepped = pParam->Type() == IParam::kTypeEnum;
  }
}

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
