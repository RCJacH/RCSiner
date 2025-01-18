#pragma once

#include "IControl.h"
#include "widgets/RCStyle.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

class RCLabel : public ITextControl
{
public:
  enum Position
  {
    Start,
    Center,
    End,
  };

  RCLabel(const IRECT& bounds,
          const char* label,
          EDirection dir = EDirection::Horizontal,
          const RCStyle& style = DEFAULT_RCSTYLE.WithDrawFrame(false),
          float gap = 0.f,
          Position position = Position::Center);
  void Draw(IGraphics& g) override;
  void DrawText(IGraphics& g, WDL_String str);

protected:
  EDirection mDirection;
  RCStyle mStyle;
  Position mPosition;
  float mGap;
};

RCLabel::RCLabel(const IRECT& bounds, const char* label, EDirection dir, const RCStyle& style, float gap, Position position)
  : ITextControl(bounds, label)
  , mDirection(dir)
  , mStyle(style)
  , mGap(gap)
  , mPosition(position)
{
  mText = style.valueText;
}

void RCLabel::Draw(IGraphics& g) { DrawText(g, mStr); }

void RCLabel::DrawText(IGraphics& g, WDL_String wdl_string)
{
  auto colors = mStyle.GetColors();
  const auto str = wdl_string.Get();
  const int length = wdl_string.GetLength();

  if (length)
  {
    if (mStyle.drawFrame)
      g.DrawRect(colors.GetBorderColor(), mRECT, &mBlend, mStyle.frameThickness);

    const IText& text = mText.WithFGColor(colors.GetColor());
    IRECT textBounds;
    IRECT remainingBounds = mRECT;
    IRECT charBounds;
    g.MeasureText(text, str, textBounds);
    const float width = textBounds.W() + (length - 1) * mGap;
    const float height = (textBounds.H() + mGap) * length - mGap;
    switch (mDirection)
    {
    case EDirection::Horizontal:
      switch (mPosition)
      {
      case Position::Start:
        remainingBounds = remainingBounds.GetFromLeft(width);
        break;
      case Position::Center:
        remainingBounds.MidHPad(width * .5f);
        break;
      case Position::End:
        remainingBounds = remainingBounds.GetFromRight(width);
        break;
      }
      if (!mGap)
      {
        g.DrawText(text, str, remainingBounds, &mBlend);
        return;
      }
      for (int i = 0; str[i] != '\0'; i++)
      {
        // Create a new const char* for each character
        const char singleChar[2] = {str[i], '\0'}; // Null-terminated string with 1 character
        const char* singleCharPtr = singleChar;
        g.MeasureText(text, singleCharPtr, charBounds);
        g.DrawText(text, singleCharPtr, remainingBounds.ReduceFromLeft(charBounds.W()), &mBlend);
        remainingBounds.ReduceFromLeft(charBounds.W() + mGap);
      }
      break;
    case EDirection::Vertical:
      switch (mPosition)
      {
      case Position::Start:
        remainingBounds = remainingBounds.GetFromTop(height);
        break;
      case Position::Center:
        remainingBounds.MidVPad(height * .5f);
        break;
      case Position::End:
        remainingBounds = remainingBounds.GetFromBottom(height);
        break;
      }
      for (int i = 0; str[i] != '\0'; i++)
      {
        // Create a new const char* for each character
        const char singleChar[2] = {str[i], '\0'}; // Null-terminated string with 1 character
        const char* singleCharPtr = singleChar;
        g.MeasureText(text, singleCharPtr, charBounds);
        charBounds = remainingBounds.GetFromTop(charBounds.H());
        g.DrawText(text, singleCharPtr, charBounds, &mBlend);
        remainingBounds.ReduceFromTop(charBounds.H() + mGap);
      }
      break;
    }
  }
}

class RCValueLabel : public RCLabel
{
public:
  RCValueLabel(const IRECT& bounds,
               int paramIdx = kNoParameter,
               const char* fmtStr = "%0.0f",
               EDirection dir = EDirection::Horizontal,
               const RCStyle& style = DEFAULT_RCSTYLE.WithDrawFrame(false),
               float gap = 0.f,
               Position position = Position::Center)
    : RCLabel(bounds, "", dir, style, gap, position)
    , mFmtStr(fmtStr)
  {
    SetParamIdx(paramIdx, 0);
  }

  void OnAttached() override { SetStrFmt(32, mFmtStr.Get(), mRealValue); }

  void OnInit() override
  {
    const IParam* pParam = GetParam();

    if (pParam)
      pParam->GetDisplayWithLabel(mValueStr);
  };

  void SetValueFromDelegate(double value, int valIdx = 0) override
  {
    if (GetParam())
    {
      mRealValue = GetParam()->FromNormalized(value);
      OnValueChanged(true);
    }
    IControl::SetValueFromDelegate(value, valIdx);
  }

  void OnValueChanged(bool preventAction = false)
  {
    SetStrFmt(32, mFmtStr.Get(), mRealValue);
    SetDirty(false);
  }

private:
  WDL_String mFmtStr;
  WDL_String mValueStr;
  double mRealValue = 0.;
};


END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
