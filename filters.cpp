#include "filters.h"
#include "waveform.h"
#include <algorithm> // abs, copy, fill
#include <cmath>     // sin, lround
#include <iostream>
#include <stdexcept>
#include <vector>

static constexpr double kTwoPi = 6.28318530717958647692;

//  AbstractGeneratorFilter

AbstractGeneratorFilter::AbstractGeneratorFilter(double durationMs)
    : durationMs_(durationMs) {
  if (durationMs < 0.0) {
    throw std::invalid_argument(
        "AbstractGeneratorFilter: duration must be >= 0");
  }
}

FilterState AbstractGeneratorFilter::apply(Waveform &sound) const {
  const size_t sampleRate = sound.getSampleRate();
  if (sampleRate == 0) {
    return FilterState::kError;
  }

  size_t numSamples = sound.msToSampleIndex(durationMs_);
  sound.resize(numSamples);

  const double sr = static_cast<double>(sampleRate);
  for (size_t i = 0; i < numSamples; ++i) {
    double t = static_cast<double>(i) / sr;
    double value = computeSampleValue(t);
    sound.setValueAt(i, value); // double overload делает clamp в int16_t
  }
  return FilterState::kOk;
}

//  AmplFilter

AmplFilter::AmplFilter(double factor) : factor_(factor) {
  if (factor < 0.0) {
    throw std::invalid_argument("AmplFilter: factor must be >= 0");
  }
}

FilterState AmplFilter::apply(Waveform &sound) const {
  const size_t n = sound.getSize();
  for (size_t i = 0; i < n; ++i) {
    double value = static_cast<double>(sound.valueAt(i)) * factor_;
    sound.setValueAt(i, value);
  }
  return FilterState::kOk;
}

//  NormalizeFilter

NormalizeFilter::NormalizeFilter(double peak) : peak_(peak) {
  if (peak < 0.0 || peak > 1.0) {
    throw std::invalid_argument("NormalizeFilter: peak must be in [0, 1]");
  }
}

FilterState NormalizeFilter::apply(Waveform &sound) const {
  const size_t n = sound.getSize();
  if (n == 0) {
    return FilterState::kOk;
  }

  // находим максимальный по модулю отсчет
  int currentPeak = 0;
  for (size_t i = 0; i < n; ++i) {
    int v = std::abs(static_cast<int>(sound.valueAt(i)));
    if (v > currentPeak) {
      currentPeak = v;
    }
  }

  if (currentPeak == 0) {
    return FilterState::kOk; // нулевой сигнал оставляем как есть
  }

  double scale = (peak_ * 32767.0) / static_cast<double>(currentPeak);

  for (size_t i = 0; i < n; ++i) {
    double value = static_cast<double>(sound.valueAt(i)) * scale;
    sound.setValueAt(i, value);
  }
  return FilterState::kOk;
}

//  SilenceFilter

SilenceFilter::SilenceFilter(const std::string &unit, double start, double end)
    : startSec_(0.0), endSec_(0.0) {
  if (unit != "sec" && unit != "ms") {
    throw std::invalid_argument(
        "SilenceFilter: unit must be \"sec\" or \"ms\"");
  }
  if (start < 0.0) {
    throw std::invalid_argument("SilenceFilter: start must be >= 0");
  }
  if (end < start) {
    throw std::invalid_argument("SilenceFilter: end must be >= start");
  }

  double startMs = (unit == "sec") ? start * 1000.0 : start;
  double endMs = (unit == "ms") ? end : end * 1000.0;

  startSec_ = startMs / 1000.0;
  endSec_ = endMs / 1000.0;
}

FilterState SilenceFilter::apply(Waveform &sound) const {
  const size_t originalSize = sound.getSize();

  size_t startIdx = sound.secToSampleIndex(startSec_);
  if (startIdx > originalSize) {
    startIdx = originalSize;
  }

  size_t endIdx = sound.secToSampleIndex(endSec_);
  if (endIdx > originalSize) {
    endIdx = originalSize;
  }

  if (endIdx <= startIdx) {
    return FilterState::kOk; // нечего вставлять
  }

  size_t silenceCount = endIdx - startIdx;
  size_t newSize = originalSize + silenceCount;

  sound.resize(
      newSize); // новые элементы инициализируются нулями (в конце вектора)

  // сдвигаем сэмплы после startIdx вправо на silenceCount позиций
  int16_t *d = sound.data();
  std::copy(d + startIdx, d + originalSize, d + startIdx + silenceCount);

  // обнуляем позиции тишины [startIdx, startIdx + silenceCount)
  std::fill(d + startIdx, d + startIdx + silenceCount, 0);

  return FilterState::kOk;
}

//  TimestretchFilter

TimestretchFilter::TimestretchFilter(double factor) : factor_(factor) {
  if (factor <= 0.0) {
    throw std::invalid_argument("TimestretchFilter: factor must be > 0");
  }
}

FilterState TimestretchFilter::apply(Waveform &sound) const {
  const size_t oldSize = sound.getSize();
  if (oldSize == 0) {
    return FilterState::kOk;
  }

  size_t newSize =
      static_cast<size_t>(std::lround(static_cast<double>(oldSize) * factor_));

  std::vector<int16_t> original(oldSize);
  for (size_t i = 0; i < oldSize; ++i)
    original[i] = sound.valueAt(i);

  sound.resize(newSize);

  if (factor_ >= 1.0) {
    // Растяжение: линейная интерполяция (старая логика)
    for (size_t i = 0; i < newSize; ++i) {
      double pos = static_cast<double>(i) / factor_;
      size_t l = static_cast<size_t>(pos);
      double frac = pos - static_cast<double>(l);

      int16_t s_l = (l < oldSize) ? original[l] : original[oldSize - 1];
      int16_t s_lp1 = (l + 1 < oldSize) ? original[l + 1] : s_l;

      double value = static_cast<double>(s_l) * (1.0 - frac) +
                     static_cast<double>(s_lp1) * frac;
      sound.setValueAt(i, value);
    }
  } else {
    // Сжатие: усреднение по блокам
    for (size_t i = 0; i < newSize; ++i) {
      double start = (static_cast<double>(i) * oldSize) / newSize;
      double end = (static_cast<double>(i + 1) * oldSize) / newSize;

      size_t first = static_cast<size_t>(std::floor(start));
      size_t last = static_cast<size_t>(std::ceil(end)) - 1;
      if (last >= oldSize)
        last = oldSize - 1;

      double sum = 0.0;
      double weightSum = 0.0;

      for (size_t j = first; j <= last; ++j) {
        double left = std::max(start, static_cast<double>(j));
        double right = std::min(end, static_cast<double>(j + 1));
        double weight = right - left;
        sum += original[j] * weight;
        weightSum += weight;
      }

      if (weightSum > 0.0)
        sound.setValueAt(i, sum / weightSum);
      else
        sound.setValueAt(i, 0);
    }
  }

  return FilterState::kOk;
}
//  LowpassFilter

LowpassFilter::LowpassFilter(int windowSize) : windowSize_(windowSize) {
  if (windowSize < 1 || windowSize % 2 == 0) {
    throw std::invalid_argument(
        "LowpassFilter: window_size must be odd and >= 1");
  }
}

FilterState LowpassFilter::apply(Waveform &sound) const {
  const size_t n = sound.getSize();
  if (n == 0) {
    return FilterState::kOk;
  }

  int half = windowSize_ / 2;

  std::vector<int16_t> original(n);
  for (size_t i = 0; i < n; ++i) {
    original[i] = sound.valueAt(i);
  }

  const int lastIdx = static_cast<int>(n) - 1;
  for (size_t i = 0; i < n; ++i) {
    double sum = 0.0;
    for (int j = -half; j <= half; ++j) {
      int idx = static_cast<int>(i) + j;
      if (idx < 0) {
        idx = 0;
      }
      if (idx > lastIdx) {
        idx = lastIdx;
      }
      sum += static_cast<double>(original[idx]);
    }
    double avg = sum / static_cast<double>(windowSize_);
    sound.setValueAt(i, avg);
  }

  return FilterState::kOk;
}

//  SinGenFilter

SinGenFilter::SinGenFilter(double frequencyHz, double durationMs)
    : AbstractGeneratorFilter(durationMs), frequencyHz_(frequencyHz) {
  if (frequencyHz < 0.0) {
    throw std::invalid_argument("SinGenFilter: frequency must be >= 0");
  }
}

double SinGenFilter::computeSampleValue(double timeSec) const {
  return 32767.0 * std::sin(kTwoPi * frequencyHz_ * timeSec);
}

//  AmGenFilter

AmGenFilter::AmGenFilter(double amplitude, double carrierHz,
                         double modulationHz, double depth, double durationMs)
    : AbstractGeneratorFilter(durationMs), amplitude_(amplitude),
      carrierHz_(carrierHz), modulationHz_(modulationHz), depth_(depth) {
  if (amplitude < 0.0 || amplitude > 1.0) {
    throw std::invalid_argument("AmGenFilter: amplitude must be in [0, 1]");
  }
  if (carrierHz < 0.0) {
    throw std::invalid_argument("AmGenFilter: carrierHz must be >= 0");
  }
  if (modulationHz < 0.0) {
    throw std::invalid_argument("AmGenFilter: modulationHz must be >= 0");
  }
  if (depth < 0.0 || depth > 1.0) {
    throw std::invalid_argument("AmGenFilter: depth must be in [0, 1]");
  }
}

double AmGenFilter::computeSampleValue(double timeSec) const {
  double envelope = 1.0 + depth_ * std::sin(kTwoPi * modulationHz_ * timeSec);
  double carrier = std::sin(kTwoPi * carrierHz_ * timeSec);
  return amplitude_ * 32767.0 * envelope * carrier;
}

//  FmGenFilter

FmGenFilter::FmGenFilter(double amplitude, double carrierHz,
                         double modulationHz, double deviationHz,
                         double durationMs)
    : AbstractGeneratorFilter(durationMs), amplitude_(amplitude),
      carrierHz_(carrierHz), modulationHz_(modulationHz),
      deviationHz_(deviationHz) {
  if (amplitude < 0.0 || amplitude > 1.0) {
    throw std::invalid_argument("FmGenFilter: amplitude must be in [0, 1]");
  }
  if (carrierHz < 0.0) {
    throw std::invalid_argument("FmGenFilter: carrierHz must be >= 0");
  }
  if (modulationHz <= 0.0) {
    throw std::invalid_argument("FmGenFilter: modulationHz must be > 0");
  }
  if (deviationHz < 0.0) {
    throw std::invalid_argument("FmGenFilter: deviationHz must be >= 0");
  }
}

double FmGenFilter::computeSampleValue(double timeSec) const {
  double phase = kTwoPi * carrierHz_ * timeSec +
                 (deviationHz_ / modulationHz_) *
                     std::sin(kTwoPi * modulationHz_ * timeSec);
  return amplitude_ * 32767.0 * std::sin(phase);
}
