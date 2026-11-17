#ifndef SOUND_PROCESSOR_IFILTER_H_
#define SOUND_PROCESSOR_IFILTER_H_

class Waveform;

enum class FilterState // использование enum class для работы с зоной видимости
                       // kOk, kError
{
  kOk,    // константа успеха
  kError, // константа провала
};

class IFilter {
public:
  virtual ~IFilter() = default; // деструктор обязан быть виртуальным, чтобы
                                // освобождать не только память IFilter,
  // но и память класса - наследника корректно

  // применяет фильтр к звуковому фрагменту, возвращает состояние
  virtual FilterState apply(Waveform &sound)
      const = 0; // "= 0 " - делает функцию чисто виртуальной, так как у нас
                 // IFilter - абстрактный класс
};

#endif // SOUND_PROCESSOR_IFILTER_H_