#ifndef SOUND_PROCESSOR_ARGS_PARSER_H
#define SOUND_PROCESSOR_ARGS_PARSER_H

#include "filter_descriptor.h"
#include <vector>

class ArgsParser {
public:
  enum class Result {
    ok,      // разбор прошел успешно
    noArgs,  // нет аргументов - показать справку
    badArgs, // некорректная последовательность аргументов
  };

  ArgsParser();

  Result parse(int argc, char *argv[]);

  const char *getInFileName() const;
  const char *getOutFileName() const;
  const std::vector<FilterDescriptor> &getFilterDescriptors() const;

private:
  const char *_inFileName;
  const char *_outFileName;
  std::vector<FilterDescriptor> _filterDescriptors;
};

#endif // SOUND_PROCESSOR_ARGS_PARSER_H