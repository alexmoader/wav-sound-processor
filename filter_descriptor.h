#ifndef SOUND_PROCESSOR_FILTER_DESCRIPTOR_H
#define SOUND_PROCESSOR_FILTER_DESCRIPTOR_H

#include <vector>

class IFilter;

// дескриптор одного фильтра: имя + список параметров-токенов
// хранит const char* (указатели на токены из argv[]) - без копирования строк
struct FilterDescriptor {
  const char *name;
  std::vector<const char *> params;
};

// тип продюсера фильтра: функция, которая по дескриптору создает IFilter*
// это указатель на функцию (function pointer)
using FilterProducer = IFilter *(*)(const FilterDescriptor &);

#endif // SOUND_PROCESSOR_FILTER_DESCRIPTOR_H
