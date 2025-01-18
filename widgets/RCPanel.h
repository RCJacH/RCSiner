#pragma once

#include "IControl.h"
#include "IControls.h"
#include "IGraphics.h"
#include "IGraphicsStructs.h"
#include "Widgets/Color.h"
#include "Widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A panel control which can be styled with emboss etc. */
class RCPanelBackground : public IContainerBase
{
public:
  RCPanelBackground(const IRECT& bounds, const RCStyle& style = DEFAULT_RCSTYLE.WithColor(Color::HSLA(0, 0.f, 0.f, 0.f)))
    : IContainerBase(bounds)
    , mStyle(style)
  {
  }

  void Draw(IGraphics& g) override
  {
    if (mStyle.drawBG)
      g.FillRoundRect(mStyle.GetColors().GetColor(), mRECT, mStyle.roundness /*, &blend*/);

    if (mStyle.drawFrame)
      g.DrawRoundRect(mStyle.GetColors().GetBorderColor(), mRECT, mStyle.roundness, &mBlend, mStyle.frameThickness);
  }

  void OnResize() override
  {
    SetTargetRECT(mRECT);
    SetDirty(false);
  }

private:
  RCStyle mStyle;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
