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
    kSinX = 0,
    kSinXPlusX,
    kSinXPlusSinX,
    kSinXPlusSinXPI,
    kSinXPlusXBound,
    kSinXPowEuler
  };

#if defined VST3_API || defined VST3C_API
  static constexpr std::initializer_list<const char*> Algorithms = {
    "sin(A*PI*x^B)^C", "(x - sin(A*PI*x^B)^C) / 2", "(sin(x) - sin(A*PI*x^B*PI*)^C) / 2", "(sin(PI*x) - sin(A*PI*x^B)^C) / 2", "(1-x)(sin(A*PI*x^B)^C) + x", "sin(A*PI*x^B^e)^C"};
#else
  static constexpr std::initializer_list<const char*> Algorithms = {
    "sin(Aπx^B)^C", "(x - sin(Aπx^B)^C) / 2", "(sin(x) - sin(Aπx^Bπ)^C) / 2", "(sin(πx) - sin(Aπx^B)^C) / 2", "(1-x)(sin(Aπx^B)^C) + x", "sin(Aπx^B^e)^C"};
#endif

  void SetAlgorithm(int algorithm) { mAlgorithm = static_cast<EAlgorithms>(algorithm); }
  void SetPull(double pull) { mPull = pull; }
  void SetSqueeze(double squeeze) { mSqueeze = squeeze; }
  void SetCurve(double curve) { mCurve = curve; }
  void SetStages(double stages)
  {
    double baseStages;
    mStagePct = std::modf(stages, &baseStages);
    mBaseStages = static_cast<int>(baseStages);
    mOverStages = static_cast<int>(baseStages + std::ceil(mStagePct));
  }
  void SetPreClip(bool clip) { mPreClip = clip; }
  void SetPostClip(bool clip) { mPostClip = clip; }
  iplug::sample ProcessSample(iplug::sample sample)
  {
    auto signMul = sign(sample);
    auto uInput = std::abs(sample);
    if (mPreClip)
      uInput = std::min(uInput, 1.);

    auto post = uInput;
    for (int i = 1; i <= mOverStages; i++)
    {
      post = ApplyAlgoritm(uInput);
      signMul *= sign(post);
      post = abs(post);
      if (i == mBaseStages && mStagePct > 0.)
      {
        uInput = iplug::Lerp(uInput, post, mStagePct);
      }
      else
      {
        uInput = post;
      }
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
  int mBaseStages;
  int mOverStages;
  double mStagePct;
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
  iplug::sample ApplyAlgoritm(iplug::sample x)
  {
    switch (mAlgorithm)
    {
    case kSinX:
      return SinX(x);
    case kSinXPlusX:
      return SinXPlusX(x);
    case kSinXPlusSinX:
      return SinXPlusSinX(x);
    case kSinXPlusSinXPI:
      return SinXPlusSinXPI(x);
    case kSinXPlusXBound:
      return SinXPlusXBound(x);
    case kSinXPowEuler:
      return SinXPowEuler(x);
    }
  }
};
