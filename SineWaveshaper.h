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
    kBendBeforeSin = 0,
    kBendAfterSin,
    kSinXPlusBendBeforeSin,
    kSinXPlusBendAfterSin,
    kSinXPlusXBendBeforeSin,
    kSinXPlusXBendAfterSin,
    kSinXPIPlusBendBeforeSin,
    kSinXPIPlusBendAfterSin,
    kSinXPlusXBoundBendBeforeSin,
    kSinXPlusXBoundBendAfterSin,
    kSinXPowEBendBeforeSin,
    kSinXPowEBendAfterSin
  };

  void SetAlgorithm(int algorithm) { mAlgorithm = static_cast<EAlgorithms>(algorithm); }
  void SetPull(double pull)
  {
    mPull = pull;
    mPullMultiplier = std::exp(std::log(.5) + pull * (std::log(16.) - std::log(.5)));
  }
  void SetBend(double bend)
  {
    mBend = bend;
    mBendMultiplier = std::exp(std::log(.25) + bend * (std::log(4) - std::log(.25)));
  }
  void SetClip(int clipflag)
  {
    mPreClip = clipflag & 1;
    mPostClip = clipflag & 2;
  }
  iplug::sample ProcessSample(iplug::sample sample)
  {
    const auto signMul = sign(sample);
    auto uInput = std::abs(sample);
    if (mPreClip)
      uInput = std::min(uInput, 1.);
    switch (mAlgorithm)
    {
    case kBendBeforeSin:
      uInput = BendBeforeSin(uInput);
      break;
    case kBendAfterSin:
      uInput = BendAfterSin(uInput);
      break;
    case kSinXPlusBendBeforeSin:
      uInput = SinXPlusBendBeforeSin(uInput);
      break;
    case kSinXPlusBendAfterSin:
      uInput = SinXPlusBendAfterSin(uInput);
      break;
    case kSinXPlusXBendBeforeSin:
      uInput = SinXPlusXBendBeforeSin(uInput);
      break;
    case kSinXPlusXBendAfterSin:
      uInput = SinXPlusXBendAfterSin(uInput);
      break;
    case kSinXPIPlusBendBeforeSin:
      uInput = SinXPIPlusBendBeforeSin(uInput);
      break;
    case kSinXPIPlusBendAfterSin:
      uInput = SinXPIPlusBendAfterSin(uInput);
      break;
    case kSinXPlusXBoundBendBeforeSin:
      uInput = SinXPlusXBoundBendBeforeSin(uInput);
      break;
    case kSinXPlusXBoundBendAfterSin:
      uInput = SinXPlusXBoundBendAfterSin(uInput);
      break;
    case kSinXPowEBendBeforeSin:
      uInput = SinXPowEBendBeforeSin(uInput);
      break;
    case kSinXPowEBendAfterSin:
      uInput = SinXPowEBendAfterSin(uInput);
      break;
    }
    if (mPostClip)
      uInput = clip(uInput);
    return uInput * signMul;
  };

private:
  std::function<iplug::sample> mAlgorithmCallable;
  EAlgorithms mAlgorithm;
  double mPull;
  double mPullMultiplier;
  double mBend;
  double mBendMultiplier;
  bool mPreClip;
  bool mPostClip;

private:
  iplug::sample BendBeforeSin(iplug::sample x) { return std::sin(mPullMultiplier * std::pow(x, mBendMultiplier) * iplug::PI); }
  iplug::sample BendAfterSin(iplug::sample x)
  {
    const auto s = std::sin(mPullMultiplier * x * iplug::PI);
    if (!s)
      return s;
    return sign(s) * std::pow(std::abs(s), mBendMultiplier);
  }
  iplug::sample SinXPlusBendBeforeSin(iplug::sample x) { return .5 * (std::sin(x) - BendBeforeSin(x)); }
  iplug::sample SinXPlusBendAfterSin(iplug::sample x) { return .5 * (std::sin(x) - BendAfterSin(x)); }
  iplug::sample SinXPlusXBendBeforeSin(iplug::sample x) { return .5 * (x - BendBeforeSin(x)); }
  iplug::sample SinXPlusXBendAfterSin(iplug::sample x) { return .5 * (x - BendAfterSin(x)); }
  iplug::sample SinXPIPlusBendBeforeSin(iplug::sample x) { return .5 * (std::sin(x * iplug::PI) - BendBeforeSin(x)); }
  iplug::sample SinXPIPlusBendAfterSin(iplug::sample x) { return .5 * (std::sin(x * iplug::PI) - BendAfterSin(x)); }
  iplug::sample SinXPlusXBoundBendBeforeSin(iplug::sample x) { return (1. - x) * BendBeforeSin(x) + x; }
  iplug::sample SinXPlusXBoundBendAfterSin(iplug::sample x) { return (1. - x) * BendAfterSin(x) + x; }
  iplug::sample SinXPowEBendBeforeSin(iplug::sample x) { return std::sin(mPullMultiplier * std::pow(std::pow(x, mBendMultiplier), e) * iplug::PI); }
  iplug::sample SinXPowEBendAfterSin(iplug::sample x)
  {
    const auto s = std::sin(mPullMultiplier * std::pow(x * iplug::PI, e));
    if (!s)
      return s;
    return sign(s) * std::pow(std::abs(s), mBendMultiplier);
  }
};
