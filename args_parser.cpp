#include "args_parser.h"
#include <string>

ArgsParser::ArgsParser() : _inFileName(nullptr), _outFileName(nullptr) {}

ArgsParser::Result ArgsParser::parse(int argc, char *argv[]) {
  // сбрасываем состояние
  _inFileName = nullptr;
  _outFileName = nullptr;
  _filterDescriptors.clear();

  if (argc <= 1) {
    return Result::noArgs; // только имя программы
  }

  for (int i = 1; i < argc; ++i) {
    std::string flag = argv[i];

    if (flag == "-i") {
      if (_inFileName != nullptr)
        return Result::badArgs; // уже был -i
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        return Result::badArgs;
      }
      _inFileName = argv[++i];
    } else if (flag == "-o") {
      if (_outFileName != nullptr)
        return Result::badArgs; // уже был -o
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        return Result::badArgs;
      }
      _outFileName = argv[++i];
    } else if (flag == "-f") {
      // первый токен после -f: либо имя фильтра, либо "generator"
      if (i + 1 >= argc || argv[i + 1][0] == '-') {
        return Result::badArgs; // нет имени фильтра
      }

      FilterDescriptor fd;
      std::string first = argv[++i];

      if (first == "generator") {
        // следующий токен - имя подтипа генератора (sin / am / fm)
        if (i + 1 >= argc || argv[i + 1][0] == '-') {
          return Result::badArgs;
        }
        fd.name = argv[++i];
      } else {
        fd.name = argv[i];
      }

      // собираем параметры - все токены до следующего флага
      while (i + 1 < argc && argv[i + 1][0] != '-') {
        fd.params.push_back(argv[++i]);
      }

      _filterDescriptors.push_back(fd);
    } else {
      return Result::badArgs; // неизвестный флаг
    }
  }

  return Result::ok;
}

const char *ArgsParser::getInFileName() const { return _inFileName; }

const char *ArgsParser::getOutFileName() const { return _outFileName; }

const std::vector<FilterDescriptor> &ArgsParser::getFilterDescriptors() const {
  return _filterDescriptors;
}
