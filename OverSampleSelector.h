#pragma once

#include "widgets/Color.h"
#include "widgets/RCButton.h"
#include "widgets/RCControl.h"
#include "widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class OverSampleButton : public RCButton
{
public:
  OverSampleButton(const IRECT& bounds, IActionFunction aF, const char* label = "", const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal)
    : RCButton(bounds, aF, label, style, direction) {};
  void Draw(IGraphics& g) override;

  virtual void DrawWidget(IGraphics& g);
  virtual void DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds);
  virtual void DrawButtonText(IGraphics& g, const IRECT& bounds, WidgetColorSet colorset);

  void SetLClickAction(IActionFunction action) { mLClickAction = action; };
  void SetRClickAction(IActionFunction action) { mRClickAction = action; };
  bool GetActivated() { return mActivated; };
  void MouseLClickAction(const IMouseMod& mod) override
  {
    RCButton::MouseLClickAction(mod);
    mActivated = !mActivated;
    if (mLClickAction)
      mLClickAction(this);
  };
  void MouseRClickAction(const IMouseMod& mod) override
  {
    RCButton::MouseRClickAction(mod);
    if (mRClickAction)
      mRClickAction(this);
  };
  void CreateContextMenu(IPopupMenu& contextMenu) override
  {
    mMouseControl.ReleaseL();
    if (mRClickAction)
      mRClickAction(this);
  };

private:
  IActionFunction mLClickAction = nullptr;
  IActionFunction mRClickAction = nullptr;
  bool mActivated = false;
};

void OverSampleButton::Draw(IGraphics& g) { DrawWidget(g); }


void OverSampleButton::DrawWidget(IGraphics& g)
{
  const WidgetColorSet colorset = mStyle.GetColors(mMouseControl.IsHovering(), mMouseControl.IsLDown(), IsDisabled() || !mActivated);
  DrawBG(g, colorset, mRECT);
  DrawButtonText(g, mRECT, colorset);
}

void OverSampleButton::DrawBG(IGraphics& g, WidgetColorSet colorset, IRECT bounds)
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

void OverSampleButton::DrawButtonText(IGraphics& g, const IRECT& r, WidgetColorSet color)
{
  if (CStringHasContents(mLabel))
  {
    IColor textColor = mStyle.drawBG ? color.GetLabelColor() : color.GetColor();
    if (mStyle.drawBG && color.GetCoveredContrast() < 1.5f)
      textColor.Contrast(-.5f);
    const IText& text = mStyle.GetText().WithFGColor(textColor);
    g.DrawText(text, mLabel, r, &mBlend);
  }
}

class OverSampleSelector : public IContainerBase
{
public:
  OverSampleSelector(const IRECT& bounds, int paramIdxToggle, int paramIdxOnline, int paramIdxOffline, const RCStyle& style = DEFAULT_RCSTYLE, EDirection direction = EDirection::Horizontal);

  virtual ~OverSampleSelector() { mChildren.Empty(); }
  virtual const char* GetDisplayText();

  void CreateCustomContextMenu(IPopupMenu& contextMenu, const IRECT& bounds);
  void SetValueFromDelegate(double value, int valIdx) override;

protected:
  EDirection mDirection;
  RCStyle mStyle;
  OverSampleButton* mButtonControl = nullptr;
  WDL_PtrList<IControl> mChildren;

  IPopupMenu mMenu{"OverSample Options"};
  const int activateIdx = 0;
  const int onlineIdx = 1;
  const int offlineIdx = 2;

  void populateMenuItems(IPopupMenu& menu, int idx, int startingIdx = 0)
  {
    const IParam* pParam = GetParam(idx);
    auto cur_v = pParam->Value();
    double v = pParam->GetMin();
    auto step = pParam->GetStep();
    int i = startingIdx;
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
  : IContainerBase(bounds, {paramIdxToggle, paramIdxOnline, paramIdxOffline})
  , mDirection(direction)
  , mStyle(style)
{
  mDblAsSingleClick = true;
  mText = style.valueText;
  mText.mAlign = mStyle.valueText.mAlign = EAlign::Center;
  mText.mVAlign = mStyle.valueText.mVAlign = EVAlign::Middle;
  SetAttachFunc([&, style](IContainerBase* pContainer, const IRECT& bounds) {
    AddChildControl(mButtonControl = new OverSampleButton(mRECT, EmptyClickActionFunc, "", mStyle), kNoTag, GetGroup());

    mButtonControl->SetValueStr(GetDisplayText());
    mButtonControl->SetLClickAction([&](IControl* pCaller) {
      const auto is_on = mButtonControl->GetActivated();
      SetValue(is_on, activateIdx);
      if (is_on && !(GetParam(onlineIdx)->Value()) && !(GetParam(offlineIdx)->Value()))
        SetValue(GetParam(onlineIdx)->ToNormalized(1.), onlineIdx);
      SetDirty();
      mButtonControl->SetValueStr(GetDisplayText());
      mButtonControl->SetDirty(false);
    });
    mButtonControl->SetRClickAction([&](IControl* pCaller) { CreateCustomContextMenu(mMenu, mRECT); });
  });

  SetResizeFunc([&](IContainerBase* pContainer, const IRECT& bounds) { mButtonControl->SetTargetAndDrawRECTs(bounds); });
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

void OverSampleSelector::SetValueFromDelegate(double value, int valIdx) { mButtonControl->SetValueStr(GetDisplayText()); }

void OverSampleSelector::CreateCustomContextMenu(IPopupMenu& contextMenu, const IRECT& bounds)
{
  contextMenu.Clear(true);
  populateMenuItems(contextMenu, onlineIdx);
  contextMenu.AddSeparator();
  auto OfflinePopupMenu = new IPopupMenu("Offline");
  populateMenuItems(*OfflinePopupMenu, offlineIdx);
  auto* pOfflineMenu = contextMenu.AddItem("Offline", OfflinePopupMenu)->GetSubmenu();
  auto offlineMenuFunc = [this](IPopupMenu* pMenu) {
    SetValue(GetParam(offlineIdx)->ToNormalized(pMenu->GetChosenItemIdx()), offlineIdx);
    SetDirty(true, offlineIdx);
    mButtonControl->SetValueStr(GetDisplayText());
  };
  auto menufunc = [this](IPopupMenu* pMenu) {
    SetValue(GetParam(onlineIdx)->ToNormalized(pMenu->GetChosenItemIdx()), onlineIdx);
    SetDirty(true, onlineIdx);
    mButtonControl->SetValueStr(GetDisplayText());
  };
  pOfflineMenu->SetFunction(offlineMenuFunc);
  GetUI()->CreatePopupMenu(*this, contextMenu, bounds);
  contextMenu.SetFunction(menufunc);
}

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
