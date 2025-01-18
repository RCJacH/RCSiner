#pragma once

#include "IControl.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

struct MouseButtonStatus
{
  enum Status
  {
    Released = 1,
    Pressing = 2,
    Dragging = 6,
    Clicked = 9,
  };

  MouseButtonStatus() {};

  float press_x = -1.;
  float press_y = -1.;
  double d_pos = 0.;
  Status status = Status::Released;

  bool IsReleased() { return status == Status::Released; };
  bool IsUp() { return status & Status::Released; };
  bool IsPressing() { return status == Status::Pressing; };
  bool IsDown() { return status & Status::Pressing; };
  bool IsDragging() { return status == Status::Dragging; };
  bool IsClicked() { return status == Status::Clicked; };

  void Release()
  {
    status = Status::Released;
    press_x = -1.;
    press_y = -1.;
    d_pos = 0.;
  };
  void SetStatus(float x, float y, bool isHovering, bool isOn)
  {
    switch (_CheckPressing(isHovering, isOn))
    {
    case 0:
      Release();
      break;
    case 1:
      status = Status::Pressing;
      press_x = x;
      press_y = y;
      d_pos = 0.;
      break;
    case 2:
      if (isHovering)
        status = Status::Clicked;
      else
        Release();
      break;
    case 3:
      if (x != press_x || y != press_y)
        status = Status::Dragging;
      break;
    };
  };

  int _CheckPressing(bool isHovering, bool isOn)
  {
    switch (status)
    {
    case Status::Released:
      if (isHovering && isOn)
        return 1;
      break;
    case Status::Pressing:
      if (!isOn)
        return 2;
      else
        return 3;
    }
    return 0;
  };
};

struct MouseControl
{
  MouseControl() {};

  float cur_x = -1.;
  float cur_y = -1.;
  bool isHovering = false;
  bool allowRMB = false;
  MouseButtonStatus lmb = MouseButtonStatus();
  MouseButtonStatus rmb = MouseButtonStatus();

  bool IsHovering() { return isHovering; };
  bool IsLUp() { return lmb.IsUp(); };
  bool IsRUp() { return rmb.IsUp(); };
  bool IsAnyPressing() { return IsLPressing() || IsRPressing(); };
  bool IsLPressing() { return lmb.IsPressing(); };
  bool IsRPressing() { return rmb.IsPressing(); };
  bool IsLDown() { return lmb.IsDown(); };
  bool IsRDown() { return rmb.IsDown(); };
  bool IsLDragging() { return lmb.IsDragging(); };
  bool IsRDragging() { return rmb.IsDragging(); };
  bool IsLClicked() { return lmb.IsClicked(); };
  bool IsRClicked() { return rmb.IsClicked(); };

  void ReleaseAll()
  {
    ReleaseL();
    ReleaseR();
  };
  void ReleaseL() { lmb.Release(); };
  void ReleaseR() { rmb.Release(); };
  void SetHovering(bool state = true)
  {
    isHovering = false;
    if (state && lmb.IsUp() && rmb.IsUp())
      isHovering = true;
  };

  void SetPressing(const IMouseMod& mod)
  {
    lmb.SetStatus(cur_x, cur_y, isHovering, mod.L);
    rmb.SetStatus(cur_x, cur_y, isHovering, mod.R);
  };

  void SetLDragging() { lmb.status = MouseButtonStatus::Status::Dragging; }
  void SetRDragging() { rmb.status = MouseButtonStatus::Status::Dragging; }

  void UpdatePosition(float x, float y)
  {
    cur_x = x;
    cur_y = y;
  };
};

class RCControl : public IControl
{
public:
  RCControl(const IRECT& bounds, int paramIdx = kNoParameter, IActionFunction aF = nullptr)
    : IControl(bounds, paramIdx, aF) {};
  RCControl(const IRECT& bounds, IActionFunction aF)
    : IControl(bounds, aF) {};

  virtual ~RCControl() {};
  void OnMouseOver(float x, float y, const IMouseMod& mod) override;
  void OnMouseOut() override;
  void OnMouseDown(float x, float y, const IMouseMod& mod) override;
  void OnMouseUp(float x, float y, const IMouseMod& mod) override;
  void OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod) override;

  void GetValueFromMousePosition(double (&values)[2]);

  virtual void MouseOverAction(const IMouseMod& mod) {};
  virtual void MouseOutAction() {};
  virtual void MouseDownAction(const IMouseMod& mod) {};
  virtual void MouseUpAction(const IMouseMod& mod) {};
  virtual void MouseHoverAction(const IMouseMod& mod) {};
  virtual void MouseLPressAction(const IMouseMod& mod) {};
  virtual void MouseLClickAction(const IMouseMod& mod) {};
  virtual void MouseLReleaseAction() {};
  virtual void MouseRPressAction(const IMouseMod& mod) {};
  virtual void MouseRClickAction(const IMouseMod& mod) {};
  virtual void MouseRReleaseAction() {};
  virtual void MouseDragAction(const IMouseMod& mod) {};
  virtual void MouseLDragAction(float dX, float dY, const IMouseMod& mod) {};
  virtual void MouseRDragAction(float dX, float dY, const IMouseMod& mod) {};

protected:
  MouseControl mMouseControl = MouseControl();
};

void RCControl::OnMouseOver(float x, float y, const IMouseMod& mod)
{
  mMouseControl.UpdatePosition(x, y);
  bool prev = mMouseIsOver;
  mMouseIsOver = true;
  MouseOverAction(mod);
  if (prev == false)
  {
    mMouseControl.SetHovering(true);
    MouseHoverAction(mod);
    SetDirty(false);
  }
}

void RCControl::OnMouseOut()
{
  bool prev = mMouseIsOver;
  mMouseIsOver = false;
  if (prev == true)
  {
    mMouseControl.SetHovering(false);
    MouseOutAction();
    SetDirty(false);
  }
}

void RCControl::OnMouseDown(float x, float y, const IMouseMod& mod)
{
  mMouseControl.UpdatePosition(x, y);
  MouseDownAction(mod);
  mMouseControl.SetPressing(mod);
  if (mMouseControl.IsLPressing())
    MouseLPressAction(mod);
  if (mMouseControl.IsRPressing())
    MouseRPressAction(mod);
  SetDirty(true);
}

void RCControl::OnMouseUp(float x, float y, const IMouseMod& mod)
{
  mMouseControl.UpdatePosition(x, y);
  MouseUpAction(mod);
  mMouseControl.SetPressing(mod);
  if (mMouseControl.IsLClicked())
  {
    MouseLClickAction(mod);
    mMouseControl.ReleaseL();
  }
  if (mMouseControl.IsRClicked())
  {
    MouseRClickAction(mod);
    mMouseControl.ReleaseR();
  }
  if (mMouseControl.IsLDragging())
  {
    mMouseControl.ReleaseL();
  }
  if (mMouseControl.IsRDragging())
  {
    mMouseControl.ReleaseR();
  }
  SetDirty(true);
  mMouseControl.SetHovering(mRECT.Contains(x, y));
}

void RCControl::OnMouseDrag(float x, float y, float dX, float dY, const IMouseMod& mod)
{
  mMouseControl.UpdatePosition(x, y);
  MouseDragAction(mod);
  if (mMouseControl.IsLPressing() || mMouseControl.IsLDragging())
  {
    MouseLDragAction(dX, dY, mod);
    mMouseControl.SetLDragging();
  }
  if (mMouseControl.IsRPressing() || mMouseControl.IsRDragging())
  {
    MouseRDragAction(dX, dY, mod);
    mMouseControl.SetRDragging();
  }
  SetDirty(true);
}

void RCControl::GetValueFromMousePosition(double (&values)[2])
{
  values[0] = static_cast<double>((mMouseControl.cur_x - mRECT.L) / static_cast<double>(mRECT.W()));
  values[1] = static_cast<double>((mRECT.B - mMouseControl.cur_y) / static_cast<double>(mRECT.H()));
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
