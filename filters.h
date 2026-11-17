#ifndef SOUND_PROCESSOR_FILTERS_H
#define SOUND_PROCESSOR_FILTERS_H

#include "ifilter.h"
#include <cstddef>
#include <string>

class Waveform;

// базовый класс для всех фильтров-генераторов
class AbstractGeneratorFilter : public IFilter {
public:
  explicit AbstractGeneratorFilter(double durationMs);
  ~AbstractGeneratorFilter() override = default;
  FilterState apply(Waveform &sound) const override;

protected:
  virtual double computeSampleValue(double timeSec) const = 0;
  double durationMs_;
};

// формула: s'[i] = clamp(s[i] * factor)
// (factor >= 0; при factor > 1 значения ограничиваются в [-32768, 32767])
class AmplFilter : public IFilter {
public:
  explicit AmplFilter(double factor);
  ~AmplFilter() override = default;
  FilterState apply(Waveform &sound) const override;

private:
  double factor_;
};

// формула: scale = peak * 32767 / currentPeak, s'[i] = s[i] * scale
// (currentPeak = max |s[i]|; пустой или нулевой сигнал не меняется)
class NormalizeFilter : public IFilter {
public:
  explicit NormalizeFilter(double peak = 1.0);
  ~NormalizeFilter() override = default;
  FilterState apply(Waveform &sound) const override;

private:
  double peak_;
};

// вставляет silenceCount = endIdx - startIdx нулей между startIdx и endIdx
// (сэмплы после endIdx сдвигаются вправо; за концом сигнала тишина
// дописывается)
class SilenceFilter : public IFilter {
public:
  SilenceFilter(const std::string &unit, double start, double end);
  ~SilenceFilter() override = default;
  FilterState apply(Waveform &sound) const override;

private:
  double startSec_;
  double endSec_;
};

// формула: newSize = round(oldSize * factor)
// для каждого нового i: pos = i / factor, l = floor(pos), frac = pos - l
// s'[i] = s[l] * (1 - frac) + s[l + 1] * frac
// (если l + 1 >= oldSize, то s[l + 1] = s[l])
class TimestretchFilter : public IFilter {
public:
  explicit TimestretchFilter(double factor);
  ~TimestretchFilter() override = default;
  FilterState apply(Waveform &sound) const override;

private:
  double factor_;
};

// формула: half = windowSize / 2; для каждого i суммируем s[idx] по j в [-half,
// half] idx = clamp(i + j, 0, n - 1); s'[i] = sum / windowSize (на краях idx за
// границами заменяются крайними значениями)
class LowpassFilter : public IFilter {
public:
  explicit LowpassFilter(int windowSize);
  ~LowpassFilter() override = default;
  FilterState apply(Waveform &sound) const override;

private:
  int windowSize_;
};

// формула: t = i / sampleRate, s[i] = clamp(32767 * sin(2 * pi * frequency_hz *
// t))
class SinGenFilter : public AbstractGeneratorFilter {
public:
  SinGenFilter(double frequencyHz, double durationMs);
  ~SinGenFilter() override = default;

protected:
  double computeSampleValue(double timeSec) const override;

private:
  double frequencyHz_;
};

// формула:
// envelope = 1 + depth * sin(2 * pi * modulation_hz * t)
// carrier = sin(2 * pi * carrier_hz * t)
// s[i] = clamp(amplitude * 32767 * envelope * carrier)
class AmGenFilter : public AbstractGeneratorFilter {
public:
  AmGenFilter(double amplitude, double carrierHz, double modulationHz,
              double depth, double durationMs);
  ~AmGenFilter() override = default;

protected:
  double computeSampleValue(double timeSec) const override;

private:
  double amplitude_;
  double carrierHz_;
  double modulationHz_;
  double depth_;
};

// формула:
// phase = 2 * pi * carrier_hz * t + (deviation_hz / modulation_hz) * sin(2 * pi
// * modulation_hz * t) s[i] = clamp(amplitude * 32767 * sin(phase))
// (modulation_hz должен быть > 0, иначе деление на ноль)
class FmGenFilter : public AbstractGeneratorFilter {
public:
  FmGenFilter(double amplitude, double carrierHz, double modulationHz,
              double deviationHz, double durationMs);
  ~FmGenFilter() override = default;

protected:
  double computeSampleValue(double timeSec) const override;

private:
  double amplitude_;
  double carrierHz_;
  double modulationHz_;
  double deviationHz_;
};

#endif // SOUND_PROCESSOR_FILTERS_H
