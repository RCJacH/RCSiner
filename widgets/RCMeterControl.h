#pragma once

/**
 * @file
 * @ingroup Controls
 */

#include "IControl.h"
#include "IPlugStructs.h"
#include "ISender.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

/** Vectorial multi-channel capable meter control, linear or log response
 * @ingroup IControls */
template <int MAXNC = 1>
class RCMeterControl : public IVTrackControlBase
{
public:
  enum class EResponse
  {
    Linear,
    Log,
  };

  RCMeterControl(const IRECT& bounds,
                 const IVStyle& style = DEFAULT_STYLE,
                 EDirection dir = EDirection::Vertical,
                 std::initializer_list<const char*> trackNames = {},
                 int totalNSegs = 0,
                 EResponse response = EResponse::Linear,
                 float lowRangeDB = -72.f,
                 float highRangeDB = 12.f,
                 std::initializer_list<int> markers = {0, -6, -12, -24, -48})
    : IVTrackControlBase(bounds, "", style, MAXNC, totalNSegs, dir, trackNames)
    , mResponse(response)
    , mLowRangeDB(lowRangeDB)
    , mHighRangeDB(highRangeDB)
    , mMarkers(markers)
  {
  }

  void SetResponse(EResponse response)
  {
    mResponse = response;
    SetDirty(false);
  }

  void Draw(IGraphics& g) override
  {
    DrawBackground(g, mRECT);
    DrawWidget(g);

    if (mResponse == EResponse::Log)
    {
      DrawMarkers(g);
    }

    if (mStyle.drawFrame)
      g.DrawRect(GetColor(kFR), mWidgetBounds, &mBlend, mStyle.frameThickness);
  }

  void DrawPeak(IGraphics& g, const IRECT& r, int chIdx, bool aboveBaseValue) override { g.FillRect(GetColor(kX1), r, &mBlend); }

  virtual void DrawMarkers(IGraphics& g)
  {
    auto lowPointAbs = std::fabs(mLowRangeDB);
    auto rangeDB = std::fabs(mHighRangeDB - mLowRangeDB);

    for (auto pt : mMarkers)
    {
      auto linearPos = (pt + lowPointAbs) / rangeDB;

      auto r = mWidgetBounds.FracRect(IVTrackControlBase::mDirection, linearPos);

      if (IVTrackControlBase::mDirection == EDirection::Vertical)
      {
        r.B = r.T + 10.f;
        g.DrawLine(GetColor(kHL), r.L, r.T, r.R, r.T);
      }
      else
      {
        r.L = r.R - 10.f;
        g.DrawLine(GetColor(kHL), r.MW(), r.T, r.MW(), r.B);
      }

      if (mStyle.showValue)
      {
        WDL_String str;
        str.SetFormatted(32, "%i dB", pt);
        g.DrawText(mStyle.valueText, str.Get(), r);
      }
    }
  }

  void OnMsgFromDelegate(int msgTag, int dataSize, const void* pData) override
  {
    if (!IsDisabled() && msgTag == ISender<>::kUpdateMessage)
    {
      IByteStream stream(pData, dataSize);

      int pos = 0;
      ISenderData<MAXNC> d;
      pos = stream.Get(&d, pos);

      if (mResponse == EResponse::Log)
      {
        auto lowPointAbs = std::fabs(mLowRangeDB);
        auto rangeDB = std::fabs(mHighRangeDB - mLowRangeDB);
        for (auto c = d.chanOffset; c < (d.chanOffset + d.nChans); c++)
        {
          auto ampValue = AmpToDB(static_cast<double>(d.vals[c]));
          auto linearPos = (ampValue + lowPointAbs) / rangeDB;
          SetValue(Clip(linearPos, 0., 1.), c);
        }
      }
      else
      {
        for (auto c = d.chanOffset; c < (d.chanOffset + d.nChans); c++)
        {
          SetValue(Clip(static_cast<double>(d.vals[c]), 0., 1.), c);
        }
      }

      SetDirty(false);
    }
  }

  void DrawTrackBackground(IGraphics& g, const IRECT& r, int chIdx) override { g.FillRect(GetColor(kBG), r); }

protected:
  float mHighRangeDB;
  float mLowRangeDB;
  EResponse mResponse = EResponse::Linear;
  std::vector<int> mMarkers;
};

/** Vectorial multi-channel capable meter control, with log response, held-peaks and filled-average/rms
 * Requires an IPeakAvgSender
 * @ingroup IControls */
template <int MAXNC = 1>
class RCPeakAvgMeterControl : public RCMeterControl<MAXNC>
{
public:
  RCPeakAvgMeterControl(const IRECT& bounds,
                        const IVStyle& style = DEFAULT_STYLE,
                        EDirection dir = EDirection::Vertical,
                        std::initializer_list<const char*> trackNames = {},
                        int totalNSegs = 0,
                        float lowRangeDB = -60.f,
                        float highRangeDB = 12.f,
                        std::initializer_list<int> markers = {0, -6, -12, -24, -48})
    : RCMeterControl<MAXNC>(bounds, style, dir, trackNames, totalNSegs, RCMeterControl<MAXNC>::EResponse::Log, lowRangeDB, highRangeDB, markers)
  {
  }

  void DrawPeak(IGraphics& g, const IRECT& r, int chIdx, bool aboveBaseValue) override
  {
    IBlend blend = IVTrackControlBase::GetBlend();
    float trackPos = mPeakValues[chIdx];
    float peakSize = IVTrackControlBase::mPeakSize;
    EVColor colorIdx = kX1;

    if (trackPos < 0.0001)
      return;

    if (trackPos > 1.0)
    {
      trackPos = 1.0f;
      peakSize *= 4;
      colorIdx = kX2;
    }

    const auto widgetBounds = IVTrackControlBase::mWidgetBounds;
    const auto dir = IVTrackControlBase::mDirection;
    IRECT peakRect = widgetBounds.FracRect(dir, trackPos);

    if (dir == EDirection::Vertical)
    {
      peakRect = peakRect.GetFromTop(peakSize);
      peakRect.L = r.L;
      peakRect.R = r.R;
    }
    else
    {
      peakRect = peakRect.GetFromRight(peakSize);
      peakRect.T = r.T;
      peakRect.B = r.B;
    }
    g.FillRect(IVTrackControlBase::GetColor(colorIdx), peakRect, &blend);
  }

  void OnMsgFromDelegate(int msgTag, int dataSize, const void* pData) override
  {
    if (!IVTrackControlBase::IsDisabled() && msgTag == ISender<>::kUpdateMessage)
    {
      IByteStream stream(pData, dataSize);

      int pos = 0;
      ISenderData<MAXNC, std::pair<float, float>> d;
      pos = stream.Get(&d, pos);

      const auto lowRangeDB = RCPeakAvgMeterControl::mLowRangeDB;
      const auto highRangeDB = RCPeakAvgMeterControl::mHighRangeDB;

      double lowPointAbs = std::fabs(lowRangeDB);
      double rangeDB = std::fabs(highRangeDB - lowRangeDB);

      for (auto c = d.chanOffset; c < (d.chanOffset + d.nChans); c++)
      {
        double peakValue = AmpToDB(static_cast<double>(std::get<0>(d.vals[c])));
        double avgValue = AmpToDB(static_cast<double>(std::get<1>(d.vals[c])));
        double linearPeakPos = (peakValue + lowPointAbs) / rangeDB;
        double linearAvgPos = (avgValue + lowPointAbs) / rangeDB;

        IVTrackControlBase::SetValue(Clip(linearAvgPos, 0., 1.), c);
        mPeakValues[c] = static_cast<float>(linearPeakPos);
      }

      IVTrackControlBase::SetDirty(false);
    }
  }

protected:
  std::array<float, MAXNC> mPeakValues;
};

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
