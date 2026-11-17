#ifndef SOUND_PROCESSOR_CMD_LINE_ARGS_2_PIPELINE_CONVERTER_H
#define SOUND_PROCESSOR_CMD_LINE_ARGS_2_PIPELINE_CONVERTER_H

#include "filter_descriptor.h"
#include "pipeline.h"
#include <cstring>
#include <map>

// компаратор для const char* в std::map (по строкам)
struct CStrCompare {
  bool operator()(const char *a, const char *b) const {
    return std::strcmp(a, b) < 0;
  }
};

class CmdLineArgs2PipelineConverter {
public:
  CmdLineArgs2PipelineConverter();

  // регистрирует продюсер для имени фильтра
  void addFilterProducer(const char *filterName, FilterProducer producer);

  // возвращает продюсер по имени, или nullptr если не найден
  FilterProducer getFilterProducer(const char *filterName) const;

  // создает Pipeline по списку дескрипторов
  Pipeline createPipeline(const std::vector<FilterDescriptor> &descriptors);

private:
  std::map<const char *, FilterProducer, CStrCompare> _producers;
};

#endif // SOUND_PROCESSOR_CMD_LINE_ARGS_2_PIPELINE_CONVERTER_H
