#include "waveform.h"
#include <algorithm> //clamp
#include <cmath>     // lround

Waveform::Waveform(size_t sampleRate) : sampleRate_(sampleRate) {}

Waveform::~Waveform() = default;

size_t Waveform::getSize() const { return samples_.size(); }

bool Waveform::isEmpty() const { return samples_.empty(); }

double Waveform::getDuration() const {
  if (sampleRate_ == 0)
    return 0.0; // защита от деления на 0
  return (static_cast<double>(samples_.size()) * 1000.0) /
         static_cast<double>(sampleRate_);
}

double Waveform::getDurationSec() const {
  if (sampleRate_ == 0)
    return 0.0; // защита от деления на 0
  return static_cast<double>(samples_.size()) /
         static_cast<double>(sampleRate_);
}

size_t Waveform::getSampleRate() const { return sampleRate_; }

int16_t Waveform::valueAt(size_t index) const {
  if (index >= samples_.size()) {
    return 0;
  }
  return samples_[index];
}

void Waveform::setValueAt(size_t index, int value) {
  if (index >= samples_.size()) {
    return;
  }
  // значение в диапазон int16_t [-32768, 32767]
  int16_t clamped = static_cast<int16_t>(std::clamp(value, -32768, 32767));
  samples_[index] = clamped;
}

void Waveform::setValueAt(size_t index, double value) {
  if (index >= samples_.size()) {
    return;
  }
  // clamp в диапазон int16_t и округление до ближайшего целого
  if (value > 32767.0)
    value = 32767.0;
  if (value < -32768.0)
    value = -32768.0;
  samples_[index] = static_cast<int16_t>(std::lround(value));
}

void Waveform::resize(size_t newSize) {
  samples_.resize(newSize, 0); // новые элементы инициализируются нулями
}

void Waveform::clear() { samples_.clear(); }

int16_t *Waveform::data() { return samples_.data(); }

const int16_t *Waveform::data() const { return samples_.data(); }

size_t Waveform::byteSize() const { return samples_.size() * sizeof(int16_t); }

size_t Waveform::msToSampleIndex(double ms) const {
  if (sampleRate_ == 0)
    return 0; // защита от деления на 0
  return static_cast<size_t>(
      std::lround((ms * static_cast<double>(sampleRate_)) / 1000.0));
}

size_t Waveform::secToSampleIndex(double sec) const {
  if (sampleRate_ == 0)
    return 0;
  return static_cast<size_t>(
      std::lround(sec * static_cast<double>(sampleRate_)));
}

double Waveform::sampleIndexToMs(size_t index) const {
  if (sampleRate_ == 0)
    return 0.0;
  return (static_cast<double>(index) * 1000.0) /
         static_cast<double>(sampleRate_);
}

double Waveform::sampleIndexToSec(size_t index) const {
  if (sampleRate_ == 0)
    return 0.0;
  return static_cast<double>(index) / static_cast<double>(sampleRate_);
}
