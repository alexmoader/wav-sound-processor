#ifndef SOUND_PROCESSOR_WAVEFORM_H
#define SOUND_PROCESSOR_WAVEFORM_H

#include <cstddef> // size_t
#include <cstdint> // int16_t
#include <vector>  // vector

// надо расписать все функции, чтобы при наведении было понятно, что они значат

class Waveform {
private:
  size_t sampleRate_;            // частота в Гц
  std::vector<int16_t> samples_; // вектор для хранения отсчетов
public:
  Waveform(size_t sampleRate = 44100); // по умолчанию 44100 Гц
  ~Waveform();

  size_t getSize() const; // возвращает количество отсчетов
  bool isEmpty() const; // возвращает true, если отсчетов нет - для генераторов
  double getDuration()
      const; // возвращает длительность в мс (double для дробных значений)
  double getDurationSec() const; // возвращает длительность в секундах (double)
  size_t getSampleRate() const;  // возвращает частоту дискретизации

  int16_t valueAt(size_t index) const; // возвращает значение отсчета по индексу
  void setValueAt(size_t index,
                  int value); // устанавливает значение отсчета по индексу с
                              // автоматическим насыщением
  void setValueAt(
      size_t index,
      double value); // overload для дробной математики фильтров (round + clamp)

  void resize(size_t newSize); // изменяет количество отсчетов

  void clear(); // очищает все отсчеты

  int16_t *data();             // указатель на массив сэмплов
  const int16_t *data() const; // указатель на массив сэмплов (для чтения)
  size_t
  byteSize() const; // размер данных в байтах (getSize() * sizeof(int16_t))

  size_t msToSampleIndex(
      double ms) const; // преобразует миллисекунды (double) в индекс отсчета
  size_t secToSampleIndex(
      double sec) const; // преобразует секунды (double) в индекс отсчета
  double sampleIndexToMs(
      size_t index) const; // преобразует индекс отсчета в миллисекунды (double)
  double sampleIndexToSec(
      size_t index) const; // преобразует индекс отсчета в секунды (double)
};

#endif // SOUND_PROCESSOR_WAVEFORM_H
