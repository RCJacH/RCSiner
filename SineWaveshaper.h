#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include <algorithm>
#include <cmath>

const double e = std::exp(1.);
iplug::sample sign(iplug::sample sample) { return signbit(sample) ? -1. : 1.; }
iplug::sample clip(iplug::sample sample) { return sign(sample) * std::min(std::abs(sample), 1.); }

class SineWaveshaper
{
public:
  enum EAlgorithms
  {
    kSinXPlusX = 0,
    kSinXPlusSinX,
    kSinXPlusSinXPI,
    kSinXPlusXBound,
    kSinXPowEuler
  };

  void SetAlgorithm(int algorithm) { mAlgorithm = static_cast<EAlgorithms>(algorithm); }
  void SetPull(double pull) { mPull = pull; }
  void SetSqueeze(double squeeze) { mSqueeze = squeeze; }
  void SetCurve(double curve) { mCurve = curve; }
  void SetPreClip(bool clip) { mPreClip = clip; }
  void SetPostClip(bool clip) { mPostClip = clip; }
  iplug::sample ProcessSample(iplug::sample sample)
  {
    const auto signMul = sign(sample);
    auto uInput = std::abs(sample);
    if (mPreClip)
      uInput = std::min(uInput, 1.);
    switch (mAlgorithm)
    {
    case kSinXPlusX:
      uInput = SinXPlusX(uInput);
      break;
    case kSinXPlusSinX:
      uInput = SinXPlusSinX(uInput);
      break;
    case kSinXPlusSinXPI:
      uInput = SinXPlusSinXPI(uInput);
      break;
    case kSinXPlusXBound:
      uInput = SinXPlusXBound(uInput);
      break;
    case kSinXPowEuler:
      uInput = SinXPowEuler(uInput);
      break;
    }
    if (mPostClip)
      uInput = clip(uInput);
    return uInput * signMul;
  };

private:
  EAlgorithms mAlgorithm;
  double mPull;
  double mSqueeze;
  double mCurve;
  bool mPreClip;
  bool mPostClip;

private:
  iplug::sample SinX(iplug::sample x)
  {
    const auto s = std::sin(mPull * std::pow(x, mSqueeze) * iplug::PI);
    if (!s)
      return s;
    return sign(s) * std::pow(std::abs(s), mCurve);
  }
  iplug::sample SinXPlusX(iplug::sample x) { return .5 * (x - SinX(x)); }
  iplug::sample SinXPlusSinX(iplug::sample x) { return .5 * (std::sin(x) - SinX(x)); }
  iplug::sample SinXPlusSinXPI(iplug::sample x) { return .5 * (std::sin(x * iplug::PI) - SinX(x)); }
  iplug::sample SinXPlusXBound(iplug::sample x) { return (1. - x) * SinX(x) + x; }
  iplug::sample SinXPowEuler(iplug::sample x)
  {
    const auto s = std::sin(mPull * std::pow(std::pow(x, mSqueeze), e) * iplug::PI);
    if (!s)
      return s;
    return sign(s) * std::pow(std::abs(s), mCurve);
  }
};
