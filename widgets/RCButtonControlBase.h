#pragma once

#include "widgets/Color.h"
#include "widgets/RCControl.h"
#include "widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A base class for buttons/momentary switches - cannot be linked to parameters.
 * The default action function triggers the default click function, which returns mValue to 0. after DEFAULT_ANIMATION_DURATION */
class RCButtonControlBase : public RCControl
{
public:
  RCButtonControlBase(const IRECT& bounds, IActionFunction aF);
  RCButtonControlBase(const IRECT& bounds, int kParameter = kNoParameter);

  virtual ~RCButtonControlBase() {}
  void MouseLClickAction(const IMouseMod& mod) override;
  void CreateContextMenu(IPopupMenu& contextMenu) override;
  bool IsReset(const IMouseMod& mod) const;
};

RCButtonControlBase::RCButtonControlBase(const IRECT& bounds, IActionFunction aF)
  : RCControl(bounds, kNoParameter, aF)
{
  mDblAsSingleClick = true;
}

RCButtonControlBase::RCButtonControlBase(const IRECT& bounds, int kParameter)
  : RCControl(bounds, kParameter, nullptr)
{
  mDblAsSingleClick = true;
}

void RCButtonControlBase::MouseLClickAction(const IMouseMod& mod)
{
  if (IsReset(mod))
    return SetValueToDefault(GetParamIdx());
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

void RCButtonControlBase::CreateContextMenu(IPopupMenu& contextMenu) { mMouseControl.ReleaseL(); }

bool RCButtonControlBase::IsReset(const IMouseMod& mod) const
{
#ifdef PROTOOLS
  return mod.A;
#else
  return mod.C;
#endif
}

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
