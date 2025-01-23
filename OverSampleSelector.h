#pragma once

#include "widgets/Color.h"
#include "widgets/RCButtonControlBase.h"
#include "widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A vector "tab" multi switch control. Click tabs to cycle through states. */
class OverSampleSelector : public RCControl
{
public:
  OverSampleSelector(const IRECT& bounds, int paramIdxToggle, int paramIdxOnline, int paramIdxOffline, const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal);

  virtual ~OverSampleSelector() {}
  void Draw(IGraphics& g) override;

  virtual void DrawWidget(IGraphics& g);
  virtual void DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds);
  virtual void DrawButtonText(IGraphics& g, const IRECT& bounds, WidgetColorSet colorset);
  virtual const char* GetDisplayText();

  void MouseLClickAction(const IMouseMod& mod) override;
  void CreateContextMenu(IPopupMenu& contextMenu) override;
  void OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx) override;

protected:
  EDirection mDirection;
  RCStyle mStyle;
  const char* mLabel;
  IPopupMenu mMenu{"OverSample Options"};
  const int activateIdx = 0;
  const int onlineIdx = 1;
  const int offlineIdx = 2;

  void populateMenuItems(IPopupMenu& menu, int idx)
  {
    const IParam* pParam = GetParam(idx);
    auto cur_v = pParam->Value();
    double v = pParam->GetMin();
    auto step = pParam->GetStep();
    int i = 0;
    while (v <= pParam->GetMax())
    {
      auto text = pParam->GetDisplayText(v);
      menu.AddItem(text);
      menu.CheckItem(i, v == cur_v);
      v += step;
      i++;
    }
  }
};

OverSampleSelector::OverSampleSelector(const IRECT& bounds, int paramIdxToggle, int paramIdxOnline, int paramIdxOffline, const RCStyle& style, EDirection direction)
  : RCControl(bounds, {paramIdxToggle, paramIdxOnline, paramIdxOffline})
  , mDirection(direction)
  , mStyle(style)
{
  mDblAsSingleClick = true;
  mText = style.valueText;
  mText.mAlign = mStyle.valueText.mAlign = EAlign::Center;
  mText.mVAlign = mStyle.valueText.mVAlign = EVAlign::Middle;
}

void OverSampleSelector::Draw(IGraphics& g) { DrawWidget(g); }


void OverSampleSelector::DrawWidget(IGraphics& g)
{
  const WidgetColorSet colorset = mStyle.GetColors(mMouseControl.IsHovering(), mMouseControl.IsLDown(), IsDisabled() || !(GetParam(activateIdx)->Value()));
  DrawBG(g, colorset, mRECT);
  DrawButtonText(g, mRECT, colorset);
}

void OverSampleSelector::DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds)
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

void OverSampleSelector::DrawButtonText(IGraphics& g, const IRECT& r, WidgetColorSet color)
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

const char* OverSampleSelector::GetDisplayText()
{
  auto pParamOnline = GetParam(onlineIdx);
  auto pParamOffline = GetParam(offlineIdx);
  if (!pParamOnline || !pParamOffline)
    return "";

  if (!(GetParam(activateIdx)->Value()))
    return pParamOnline->GetDisplayText(0);

  auto v = pParamOnline->Value();
  auto vOffline = pParamOffline->Value();
  const char* str1 = pParamOnline->GetDisplayText(v);
  if (!vOffline)
    return str1;

  const char* str2 = pParamOffline->GetDisplayText(vOffline);
  if (strcmp(str1, str2) == 0)
    return str1;

  char* result = new char[10];
  sprintf(result, "%s:%s", str1, str2);
  return result;
}

void OverSampleSelector::MouseLClickAction(const IMouseMod& mod)
{
  const auto is_on = !(GetParam(activateIdx)->Value());
  SetValue(is_on, activateIdx);
  if (is_on && !(GetParam(onlineIdx)->Value()) && !(GetParam(offlineIdx)->Value()))
    SetValue(GetParam(onlineIdx)->ToNormalized(1.), onlineIdx);
}


void OverSampleSelector::CreateContextMenu(IPopupMenu& contextMenu)
{
  mMouseControl.ReleaseL();
  mMenu.Clear(true);
  populateMenuItems(mMenu, onlineIdx);
  mMenu.AddSeparator();
  auto OfflinePopupMenu = new IPopupMenu("Offline");
  populateMenuItems(*OfflinePopupMenu, offlineIdx);
  auto* pOfflineMenu = mMenu.AddItem("Offline", OfflinePopupMenu)->GetSubmenu();
  GetUI()->CreatePopupMenu(*this, mMenu, mMouseControl.cur_x, mMouseControl.cur_y);
}

void OverSampleSelector::OnPopupMenuSelection(IPopupMenu* pSelectedMenu, int valIdx)
{
  mMouseControl.ReleaseL();

  if (!pSelectedMenu)
    return;

  const char* title = pSelectedMenu->GetRootTitle();

  if (strcmp(title, "Offline") == 0)
  {
    SetValue(GetParam(offlineIdx)->ToNormalized(pSelectedMenu->GetChosenItemIdx()), offlineIdx);
    SetDirty(true);
  }
  else if (strcmp(title, "OverSample Options") == 0)
  {
    SetValue(GetParam(onlineIdx)->ToNormalized(pSelectedMenu->GetChosenItemIdx()), onlineIdx);
    SetDirty(true);
  }
}

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
