#pragma once

#include "IControl.h"

BEGIN_IPLUG_NAMESPACE
BEGIN_IGRAPHICS_NAMESPACE

float scale(float v, float s)
{
  if (s > 0.f)
    return v + (1.f - v) * s;
  else if (s < 0.f)
    return v * abs(s + 1.f);
  else
    return v;
}

namespace Color
{
struct HSLA
{
  HSLA(const int h = 0, const float s = .5f, const float l = .5f, const float a = 1.f)
  {
    mH = (h % 360);
    mS = Clip(s, 0.f, 1.f);
    mL = Clip(l, 0.f, 1.f);
    mA = Clip(a, 0.f, 1.f);
  }
  int mH;
  float mS;
  float mL;
  float mA;

  HSLA HSLA::WithHue(int hue) const
  {
    HSLA newHSLA = *this;
    newHSLA.mH = hue;
    return newHSLA;
  }

  HSLA HSLA::WithSaturation(float saturation) const
  {
    HSLA newHSLA = *this;
    newHSLA.mS = saturation;
    return newHSLA;
  }

  HSLA HSLA::WithLightness(float lightness) const
  {
    HSLA newHSLA = *this;
    newHSLA.mL = lightness;
    return newHSLA;
  }

  HSLA HSLA::WithAlpha(float alpha) const
  {
    HSLA newHSLA = *this;
    newHSLA.mA = alpha;
    return newHSLA;
  }

  HSLA HSLA::Adjusted(int add_h = 0, float add_s = 0.f, float add_l = 0.f, float add_a = 0.f) const
  {
    int h = (mH + add_h + 360) % 360;
    float s = std::max(0.f, std::min(1.f, mS + add_s));
    float l = std::max(0.f, std::min(1.f, mL + add_l));
    float a = std::max(0.f, std::min(1.f, mA + add_a));
    return HSLA(h, s, l, a);
  }

  HSLA HSLA::Scaled(float scale_h = 0.f, float scale_s = 0.f, float scale_l = 0.f, float scale_a = 0.f) const
  {
    int h = mH + int(360.f * (scale_h / 360.f + 1.f));
    float s = mS;
    if (s > 0.f)
      s = scale(s, scale_s);
    float l = scale(mL, scale_l);
    float a = scale(mA, scale_a);
    return HSLA(h, s, l, a);
  }

  HSLA HSLA::LinearInterpolate(HSLA other, float h_t = 0.f, float s_t = 0.f, float l_t = 0.f, float a_t = 0.f) const
  {
    IColor thisRGB = AsIColor();
    IColor otherRGB = other.AsIColor();
    float h;
    float _;
    IColor::LinearInterpolateBetween(thisRGB, otherRGB, h_t).GetHSLA(h, _, _, _);
    return HSLA(int(h * 360), Lerp(mS, other.mS, s_t), Lerp(mL, other.mL, l_t), Lerp(mA, other.mA, a_t));
  }

  HSLA HSLA::Complement() const
  {
    int h = mH + 180;
    return HSLA(h, mS, mL, mA);
  }

  HSLA HSLA::Contrast(float amount = 1.f) const
  {
    float l = Lerp(mL, 1.f - mL, amount);
    return HSLA(mH, mS, l, mA);
  }

  IColor HSLA::AsIColor() const { return IColor::FromHSLA(mH / 360.f, mS, mL, mA); }
};
} // namespace Color

END_IGRAPHICS_NAMESPACE
END_IPLUG_NAMESPACE
