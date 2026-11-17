#include "cmd_line_args_2_pipeline_converter.h"
#include <stdexcept>
#include <string>

CmdLineArgs2PipelineConverter::CmdLineArgs2PipelineConverter() {}

void CmdLineArgs2PipelineConverter::addFilterProducer(const char *filterName,
                                                      FilterProducer producer) {
  _producers[filterName] = producer;
}

FilterProducer
CmdLineArgs2PipelineConverter::getFilterProducer(const char *filterName) const {
  auto it = _producers.find(filterName);
  if (it == _producers.end()) {
    return nullptr;
  }
  return it->second;
}

Pipeline CmdLineArgs2PipelineConverter::createPipeline(
    const std::vector<FilterDescriptor> &descriptors) {
  Pipeline pipeline;

  for (const auto &fd : descriptors) {
    FilterProducer producer = getFilterProducer(fd.name);
    if (producer == nullptr) {
      throw std::runtime_error(std::string("Unknown filter: ") + fd.name);
    }

    IFilter *filter =
        producer(fd); // может бросить исключение из самого продюсера
    if (filter == nullptr) {
      throw std::runtime_error(std::string("Failed to create filter: ") +
                               fd.name);
    }

    pipeline.addFilter(filter);
  }

  return pipeline;
}
