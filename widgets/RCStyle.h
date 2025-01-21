#pragma once

#include "IGraphicsConstants.h"
#include "widgets/Color.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

const bool DEFAULT_DRAW_BG = false;

struct WidgetColorSet
{
  WidgetColorSet() {};

  WidgetColorSet(Color::HSLA color)
    : mMainColor(color)
    , mBorderColor(color.Scaled(0.f, -.1f, .2f))
    , mBGColor(color.Scaled(0.f, -.8f, -.6f))
    , mLabelColor(color.Scaled(0.f, -.1f, .5f)) {};

  Color::HSLA mMainColor;
  Color::HSLA mBorderColor;
  Color::HSLA mBGColor;
  Color::HSLA mLabelColor;

  float GetContrast(Color::HSLA a, Color::HSLA b) const { return (a.mL + .05f) / (b.mL + .05f); }
  float GetCoveredContrast() const { return GetContrast(mLabelColor, mMainColor); }
  float GetUncoveredContrast(bool isMain = false) const { return GetContrast(isMain ? mMainColor : mLabelColor, mBGColor); }
  void SetMainColor(Color::HSLA color) { mMainColor = color; }
  void SetBorderColor(Color::HSLA color) { mBorderColor = color; }
  void SetBGColor(Color::HSLA color) { mBGColor = color; }
  void SetLabelColor(Color::HSLA color) { mLabelColor = color; }
  WidgetColorSet GetComplement() const { return WidgetColorSet(mMainColor.Complement()); }
  IColor GetColor() const { return mMainColor.AsIColor(); }
  IColor GetBorderColor() const { return mBorderColor.AsIColor(); }
  IColor GetBGColor() const { return mBGColor.AsIColor(); }
  IColor GetLabelColor() const { return mLabelColor.AsIColor(); }
};

struct WidgetInteractionColors
{
  WidgetInteractionColors() {};

  WidgetInteractionColors(Color::HSLA color, bool isDisabled = false)
    : normalColors(WidgetColorSet(color))
    , hoverColors(WidgetColorSet(color.Adjusted(0, -.05f, .05f)))
    , pressColors(WidgetColorSet(color.Adjusted(0, -.1f, .1f)))
  {
    if (isDisabled)
      return;

    const float add_l = color.mL >= 0.5 ? -.05f : .1f;
    SetDisabledColors(color.Adjusted(-90, -.3f, -add_l));
  };

  void SetColors(Color::HSLA color, bool alsoSetDisabled = false)
  {
    const WidgetInteractionColors newColors = WidgetInteractionColors(color);
    *this = newColors;
  };

  void SetDisabledColors(Color::HSLA color)
  {
    const WidgetInteractionColors disabled = WidgetInteractionColors(color, true);
    disabledColors = disabled.normalColors;
    disabledHoverColors = disabled.hoverColors;
    disabledPressColors = disabled.pressColors;
  };

  WidgetColorSet normalColors;
  WidgetColorSet hoverColors;
  WidgetColorSet pressColors;
  WidgetColorSet disabledColors;
  WidgetColorSet disabledHoverColors;
  WidgetColorSet disabledPressColors;

  WidgetColorSet GetColors(bool isHovered = false, bool isDown = false, bool isDisabled = false) const
  {
    if (isDisabled)
    {
      if (isDown)
        return disabledPressColors;
      if (isHovered)
        return disabledHoverColors;
      return disabledColors;
    }
    if (isDown)
      return pressColors;
    if (isHovered)
      return hoverColors;
    return normalColors;
  };
};

struct WidgetColors
{
  int mColorCount;
  WidgetInteractionColors* Colors{};

  WidgetColors() {};
  WidgetColors(Color::HSLA color, int count = 1, int hueRange = 0)
    : mColorCount{count}
    , Colors{new WidgetInteractionColors[static_cast<std::size_t>(count)]{}}
  {
    int dHue = 0;
    if (count > 1)
      dHue = static_cast<int>(floor(hueRange / (count - 1)));
    for (int i = 0; i < count; i++)
    {
      Colors[i] = WidgetInteractionColors(color.Adjusted(dHue * i));
    }
  };
  WidgetColors(const std::initializer_list<Color::HSLA>& colors)
    : mColorCount(colors.size())
    , Colors{new WidgetInteractionColors[static_cast<std::size_t>(colors.size())]{}}
  {
    int i = 0;
    for (Color::HSLA color : colors)
    {
      Colors[i] = WidgetInteractionColors(color);
      i++;
    }
  };

  WidgetInteractionColors& Get(int index = 0) const { return Colors[index % mColorCount]; };
};

struct RCStyle
{
  bool hideCursor = DEFAULT_HIDE_CURSOR;
  bool showValue = DEFAULT_SHOW_VALUE;
  bool drawFrame = DEFAULT_DRAW_FRAME;
  bool drawBG = DEFAULT_DRAW_BG;
  float roundness = DEFAULT_ROUNDNESS;
  Color::HSLA baseColor = Color::HSLA();
  IText valueText = DEFAULT_VALUE_TEXT.WithVAlign(EVAlign::Middle);
  float frameThickness = 2.f;
  WidgetColors Colors;

  /** Create a new RCStyle to configure common styling for IVControls
   * @param showValue Display value text
   * @param baseColors A HSLA base color for the style
   * @param valueText The IText for the value text style
   * @param hideCursor Should the cursor be hidden e.g. when dragging the control
   * @param drawFrame Should the frame be drawn around the bounds of the control or around the handle, where relevant
   * @param frameThickness The thickness of the controls frame elements */
  RCStyle(const Color::HSLA baseColor = Color::HSLA(),
          float valueTextSize = DEFAULT_TEXT_SIZE,
          const char* fontID = nullptr,
          bool hideCursor = DEFAULT_HIDE_CURSOR,
          bool showValue = DEFAULT_SHOW_VALUE,
          bool drawFrame = DEFAULT_DRAW_FRAME,
          bool drawBG = DEFAULT_DRAW_BG,
          float roundness = DEFAULT_ROUNDNESS,
          float frameThickness = DEFAULT_FRAME_THICKNESS)
    : hideCursor(hideCursor)
    , showValue(showValue)
    , drawFrame(drawFrame)
    , baseColor(baseColor)
    , Colors(baseColor)
    , roundness(roundness)
    , frameThickness(frameThickness)
  {
    valueText = valueText.WithFont(fontID).WithSize(valueTextSize);
  }

  RCStyle WithShowValue(bool show = true) const
  {
    RCStyle newStyle = *this;
    newStyle.showValue = show;
    return newStyle;
  }
  RCStyle WithValueTextSize(float newSize) const
  {
    RCStyle newStyle = *this;
    newStyle.valueText = valueText.WithSize(newSize);
    return newStyle;
  }
  RCStyle WithValueTextFont(const char* fontID) const
  {
    RCStyle newStyle = *this;
    newStyle.valueText = valueText.WithFont(fontID);
    return newStyle;
  }
  RCStyle WithValueTextHAlign(EAlign newAlign) const
  {
    RCStyle newStyle = *this;
    newStyle.valueText = valueText.WithAlign(newAlign);
    return newStyle;
  }
  RCStyle WithValueTextVAlign(EVAlign newAlign) const
  {
    RCStyle newStyle = *this;
    newStyle.valueText = valueText.WithVAlign(newAlign);
    return newStyle;
  }
  RCStyle WithHideCursor(bool hide = true) const
  {
    RCStyle newStyle = *this;
    newStyle.hideCursor = hide;
    return newStyle;
  }
  RCStyle WithColor(Color::HSLA newColor, int count = 1, int hueRange = 0) const
  {
    RCStyle newStyle = *this;
    newStyle.baseColor = newColor;
    newStyle.Colors = WidgetColors(newColor, count, hueRange);
    return newStyle;
  }
  RCStyle WithColors(const std::initializer_list<Color::HSLA>& colors) const
  {
    RCStyle newStyle = *this;
    newStyle.Colors = WidgetColors(colors);
    return newStyle;
  }
  RCStyle WithRoundness(float v) const
  {
    RCStyle newStyle = *this;
    newStyle.roundness = v;
    return newStyle;
  }
  RCStyle WithFrameThickness(float v) const
  {
    RCStyle newStyle = *this;
    newStyle.frameThickness = v;
    return newStyle;
  }
  RCStyle WithDrawFrame(bool v = true) const
  {
    RCStyle newStyle = *this;
    newStyle.drawFrame = v;
    return newStyle;
  }
  RCStyle WithDrawBG(bool v = true) const
  {
    RCStyle newStyle = *this;
    newStyle.drawBG = v;
    return newStyle;
  }
  IText GetText() const { return valueText; }

  WidgetColorSet GetColors(bool isHovered = false, bool isDown = false, bool isDisabled = false, int index = 0) const { return Colors.Get(index).GetColors(isHovered, isDown, isDisabled); };
};

const RCStyle DEFAULT_RCSTYLE = RCStyle();

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
