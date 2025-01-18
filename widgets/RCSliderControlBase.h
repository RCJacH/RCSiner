#pragma once

#include "widgets/RCControl.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A base class for slider/fader controls, to handle mouse action and Sender. */
class RCSliderControlBase : public RCControl
{
public:
  RCSliderControlBase(const IRECT& bounds, int paramIdx = kNoParameter, EDirection dir = EDirection::Vertical, double gearing = DEFAULT_GEARING, float handleSize = 0.f);
  RCSliderControlBase(const IRECT& bounds, IActionFunction aF = nullptr, EDirection dir = EDirection::Vertical, double gearing = DEFAULT_GEARING, float handleSize = 0.f);

  virtual ~RCSliderControlBase() {}

  void OnResize() override;
  void OnMouseDblClick(float x, float y, const IMouseMod& mod) override;
  // void MouseOverAction(const IMouseMod& mod) override;
  // void MouseOutAction() override;
  // void MouseDownAction(const IMouseMod& mod) override;
  void MouseUpAction(const IMouseMod& mod) override;
  // void MouseHoverAction(const IMouseMod& mod) override;
  void MouseLPressAction(const IMouseMod& mod) override;
  void MouseLClickAction(const IMouseMod& mod) override;
  // void MouseLReleaseAction() override;
  // void MouseRPressAction(const IMouseMod& mod) override;
  // void MouseRClickAction(const IMouseMod& mod) override;
  // void MouseRReleaseAction() override;
  // void MouseDragAction(const IMouseMod& mod) override;
  void MouseLDragAction(float dX, float dY, const IMouseMod& mod) override;
  // void MouseRDragAction(float dX, float dY, const IMouseMod& mod) override;
  void OnMouseWheel(float x, float y, const IMouseMod& mod, float d) override;
  void CreateContextMenu(IPopupMenu& contextMenu) override;

  void SetGearing(double gearing) { mGearing = gearing; }
  bool IsFineControl(const IMouseMod& mod, bool wheel) const;
  bool IsReset(const IMouseMod& mod) const;
  bool IsAssign(const IMouseMod& mod) const;

protected:
  bool mHideCursorOnDrag = true;
  EDirection mDirection;
  float mHandleSize;
  double mGearing;
  double mMouseLDownValue;
  double mMouseRDownValue;
};

RCSliderControlBase::RCSliderControlBase(const IRECT& bounds, int paramIdx, EDirection dir, double gearing, float handleSize)
  : RCControl(bounds, paramIdx)
  , mDirection(dir)
  , mHandleSize(handleSize)
  , mGearing(gearing)
{
}

RCSliderControlBase::RCSliderControlBase(const IRECT& bounds, IActionFunction aF, EDirection dir, double gearing, float handleSize)
  : RCControl(bounds, aF)
  , mDirection(dir)
  , mHandleSize(handleSize)
  , mGearing(gearing)
{
}

void RCSliderControlBase::OnResize()
{
  SetTargetRECT(mRECT);
  SetDirty(false);
}

void RCSliderControlBase::MouseUpAction(const IMouseMod& mod)
{
  if (mHideCursorOnDrag)
    GetUI()->HideMouseCursor(false);
}

void RCSliderControlBase::MouseLPressAction(const IMouseMod& mod) { mMouseLDownValue = GetValue(); }

void RCSliderControlBase::MouseLClickAction(const IMouseMod& mod)
{
  if (IsReset(mod))
    SetValueToDefault(GetValIdxForPos(mMouseControl.cur_x, mMouseControl.cur_y));
  if (IsAssign(mod))
  {
    const IParam* pParam = GetParam();
    double values[2];
    GetValueFromMousePosition(values);
    double v = values[mDirection == EDirection::Vertical];
    if (pParam && pParam->GetStepped() && pParam->GetStep() > 0)
      v = pParam->ConstrainNormalized(v);

    SetValue(v);
  }
}

void RCSliderControlBase::OnMouseDblClick(float x, float y, const IMouseMod& mod) { PromptUserInput(GetValIdxForPos(x, y)); }

void RCSliderControlBase::MouseLDragAction(float dX, float dY, const IMouseMod& mod)
{
  const IParam* pParam = GetParam();
  const float x = mMouseControl.cur_x;
  const float y = mMouseControl.cur_y;

  double gearing = IsFineControl(mod, false) ? mGearing * 10.0 : mGearing;

  if (mDirection == EDirection::Vertical)
    mMouseControl.lmb.d_pos -= static_cast<double>(dY / static_cast<double>(mRECT.H()) / gearing);
  else
    mMouseControl.lmb.d_pos += static_cast<double>(dX / static_cast<double>(mRECT.W()) / gearing);

  double v = Clip(mMouseControl.lmb.d_pos + mMouseLDownValue, 0., 1.);

  if (pParam && pParam->GetStepped() && pParam->GetStep() > 0)
    v = pParam->ConstrainNormalized(v);

  SetValue(v);
}

void RCSliderControlBase::OnMouseWheel(float x, float y, const IMouseMod& mod, float d)
{
  const double gearing = IsFineControl(mod, true) ? 0.001 : 0.01;
  double newValue = 0.0;
  const double oldValue = GetValue();
  const IParam* pParam = GetParam();

  if (pParam && pParam->GetStepped() && pParam->GetStep() > 0)
  {
    if (d != 0.f)
    {
      const double step = pParam->GetStep();
      double v = pParam->FromNormalized(oldValue);
      v += d > 0 ? step : -step;
      newValue = pParam->ToNormalized(v);
    }
    else
    {
      newValue = oldValue;
    }
  }
  else
  {
    newValue = oldValue + gearing * d;
  }

  SetValue(newValue);
  SetDirty();
}

void RCSliderControlBase::CreateContextMenu(IPopupMenu& contextMenu) { mMouseControl.ReleaseL(); }

bool RCSliderControlBase::IsFineControl(const IMouseMod& mod, bool wheel) const
{
#ifdef PROTOOLS
  #ifdef OS_WIN
  return mod.C;
  #else
  return wheel ? mod.C : mod.R;
  #endif
#else
  return mod.S;
#endif
}

bool RCSliderControlBase::IsReset(const IMouseMod& mod) const
{
#ifdef PROTOOLS
  return mod.A;
#else
  return mod.C;
#endif
}

bool RCSliderControlBase::IsAssign(const IMouseMod& mod) const
{
#ifdef PROTOOLS
  return mod & mod.L;
#else
  return mod.A;
#endif
}

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
