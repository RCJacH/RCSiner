#pragma once

#include "IControl.h"
#include "IControls.h"
#include "IGraphics.h"
#include "IGraphicsStructs.h"
#include "SineWaveshaper.h"
#include "widgets/Color.h"
#include "widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** A panel control which can be styled with emboss etc. */
class SineWaveshaperDisplay : public IControl
{
public:
  SineWaveshaperDisplay(const IRECT& bounds, SineWaveshaper& waveshaper, const RCStyle& style = DEFAULT_RCSTYLE, float gridThickness = 1.f)
    : IControl(bounds)
    , mWaveshaper(waveshaper)
    , mStyle(style)
    , mGridThickness(gridThickness)
  {
  }

  void Draw(IGraphics& g) override
  {
    auto colorset = mStyle.GetColors();
    DrawBG(g, colorset);
    DrawData(g, colorset);
  }

  void DrawBG(IGraphics& g, WidgetColorSet colorset)
  {
    auto frameColor = colorset.GetBorderColor();

    auto borderWidth = mStyle.frameThickness;
    auto borderBounds = mRECT;
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

    if (mGridThickness)
    {
      g.DrawHorizontalLine(frameColor.WithOpacity(.75f), mRECT, .25f, &mBlend, mGridThickness);
      g.DrawHorizontalLine(frameColor.WithOpacity(.75f), mRECT, .5f, &mBlend, mGridThickness);
      g.DrawHorizontalLine(frameColor.WithOpacity(.75f), mRECT, .75f, &mBlend, mGridThickness);
      g.DrawVerticalLine(frameColor.WithOpacity(.75f), mRECT, .25f, &mBlend, mGridThickness);
      g.DrawVerticalLine(frameColor.WithOpacity(.75f), mRECT, .5f, &mBlend, mGridThickness);
      g.DrawVerticalLine(frameColor.WithOpacity(.75f), mRECT, .75f, &mBlend, mGridThickness);
    }
  }

  void DrawData(IGraphics& g, WidgetColorSet colorset)
  {
    const auto bounds = mRECT.GetPadded(-mStyle.frameThickness);
    const auto w = std::ceil(bounds.W());
    const auto h = std::ceil(bounds.H());
    mData.resize(w + 1);

    for (int i = 0; i <= w; i++)
    {
      const auto x = i / w * 2.f - 1.f;
      const auto y = mWaveshaper.ProcessSample(x);
      mData[i] = static_cast<float>(y);
    }

    float xPos = bounds.L;
    float yPos;
    bool init = true;
    bool clipY = false;
    g.PathClear();
    g.PathMoveTo(xPos, bounds.MH());
    for (auto& data : mData)
    {
      if (std::abs(data) > 1.)
      {
        clipY = true;
        xPos += 1.f;
        continue;
      }
      if (clipY)
      {
        clipY = false;
        g.PathMoveTo(xPos, yPos);
      }
      yPos = bounds.MH() - (bounds.H() * data * .5f);
      if (init)
      {
        g.PathMoveTo(xPos, yPos);
        init = false;
      }
      else
        g.PathLineTo(xPos, yPos);
      xPos += 1.f;
    }
    g.PathStroke(colorset.GetColor(), 1.f, IStrokeOptions(), &mBlend);

    xPos = bounds.L;
    g.PathClear();
    g.PathMoveTo(xPos, bounds.MH());
    for (auto& data : mData)
    {
      auto yPos = bounds.MH() - (bounds.H() * data * .5f);
      bounds.Constrain(xPos, yPos);
      g.PathLineTo(xPos, yPos);
      xPos += 1.f;
    }
    g.PathLineTo(xPos, bounds.MH());
    g.PathFill(colorset.GetColor().WithOpacity(.382f), IFillOptions(true), &mBlend);
  }

  void OnResize() override
  {
    SetTargetRECT(mRECT);
    SetDirty(false);
  }

private:
  SineWaveshaper& mWaveshaper;
  RCStyle mStyle;
  float mGridThickness;
  std::vector<float> mData;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
