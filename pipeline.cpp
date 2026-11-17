#include "pipeline.h"

// освобождает все фильтры при уничтожении pipeline
Pipeline::~Pipeline() {
  for (IFilter *filter : _filters) {
    delete filter;
  }
}

void Pipeline::addFilter(IFilter *filter) {
  if (filter == nullptr)
    return; // защита от nullptr
  _filters.push_back(filter);
}

FilterState Pipeline::apply(Waveform &sound) const {
  for (IFilter *filter : _filters) {
    FilterState state = filter->apply(sound);
    if (state == FilterState::kError) {
      return state; // прерываем цепочку при ошибке
    }
  }
  return FilterState::kOk;
}

size_t Pipeline::size() const { return _filters.size(); }

IFilter *Pipeline::operator[](size_t index) const {
  return _filters.at(index); // бросает std::out_of_range при выходе за границы
}
