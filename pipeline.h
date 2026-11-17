#ifndef SOUND_PROCESSOR_PIPELINE_H_
#define SOUND_PROCESSOR_PIPELINE_H_

#include "ifilter.h"
#include <vector>

class Waveform;

class Pipeline {
public:
  Pipeline() = default;

  // запрет копирования
  Pipeline(const Pipeline &) = delete;
  Pipeline &operator=(const Pipeline &) = delete;

  // перемещение разрешено
  Pipeline(Pipeline &&) = default;
  Pipeline &operator=(Pipeline &&) = default;

  // деструктор освобождает все фильтры
  ~Pipeline();

  void addFilter(IFilter *filter);

  FilterState apply(Waveform &sound) const;

  size_t size() const;
  IFilter *operator[](size_t index) const;

private:
  std::vector<IFilter *> _filters;
};

#endif // SOUND_PROCESSOR_PIPELINE_H_