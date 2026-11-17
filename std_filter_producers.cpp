#include "std_filter_producers.h"
#include "filters.h"
#include <stdexcept>
#include <string>

namespace FilterProducers {

//  AmplFilter

IFilter *amplProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "ampl")
    throw std::logic_error("Wrong fd for ampl filter producer");
  if (fd.params.size() < 1)
    throw std::logic_error("Wrong params number for ampl filter producer");

  try {
    double factor = std::stod(fd.params[0]);
    return new AmplFilter(factor);
  } catch (const std::invalid_argument &) {
    throw std::logic_error("Error converting factor for ampl filter");
  }
}

//  NormalizeFilter

IFilter *normalizeProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "normalize")
    throw std::logic_error("Wrong fd for normalize filter producer");

  // peak необязательный - по умолчанию 1.0
  double peak = 1.0;
  if (fd.params.size() >= 1) {
    try {
      peak = std::stod(fd.params[0]);
    } catch (const std::invalid_argument &) {
      throw std::logic_error("Error converting peak for normalize filter");
    }
  }
  return new NormalizeFilter(peak);
}

//  SilenceFilter

IFilter *silenceProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "silence")
    throw std::logic_error("Wrong fd for silence filter producer");
  if (fd.params.size() < 3)
    throw std::logic_error("Wrong params number for silence filter producer");

  try {
    std::string unit(fd.params[0]);
    double start = std::stod(fd.params[1]);
    double end = std::stod(fd.params[2]);
    return new SilenceFilter(unit, start, end);
  } catch (const std::invalid_argument &e) {
    // ловит и ошибки std::stod, и ошибки конструктора SilenceFilter
    throw std::logic_error(std::string("Invalid silence filter params: ") +
                           e.what());
  }
}

//  TimestretchFilter

IFilter *timestretchProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "timestretch")
    throw std::logic_error("Wrong fd for timestretch filter producer");
  if (fd.params.size() < 1)
    throw std::logic_error(
        "Wrong params number for timestretch filter producer");

  try {
    double factor = std::stod(fd.params[0]);
    return new TimestretchFilter(factor);
  } catch (const std::invalid_argument &) {
    throw std::logic_error("Error converting factor for timestretch filter");
  }
}

//  LowpassFilter

IFilter *lowpassProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "lowpass")
    throw std::logic_error("Wrong fd for lowpass filter producer");
  if (fd.params.size() < 1)
    throw std::logic_error("Wrong params number for lowpass filter producer");

  try {
    int windowSize = std::stoi(fd.params[0]);
    return new LowpassFilter(windowSize);
  } catch (const std::invalid_argument &) {
    throw std::logic_error("Error converting window_size for lowpass filter");
  }
}

//  SinGenFilter

IFilter *sinGenProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "sin")
    throw std::logic_error("Wrong fd for sin gen filter producer");
  if (fd.params.size() < 2)
    throw std::logic_error("Wrong params number for sin gen filter producer");

  try {
    double frequencyHz = std::stod(fd.params[0]);
    double durationMs = std::stod(fd.params[1]);
    return new SinGenFilter(frequencyHz, durationMs);
  } catch (const std::invalid_argument &) {
    throw std::logic_error("Error converting params for sin gen filter");
  }
}

//  AmGenFilter

IFilter *amGenProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "am")
    throw std::logic_error("Wrong fd for am gen filter producer");
  if (fd.params.size() < 5)
    throw std::logic_error("Wrong params number for am gen filter producer");

  try {
    double amplitude = std::stod(fd.params[0]);
    double carrierHz = std::stod(fd.params[1]);
    double modulationHz = std::stod(fd.params[2]);
    double depth = std::stod(fd.params[3]);
    double durationMs = std::stod(fd.params[4]);
    return new AmGenFilter(amplitude, carrierHz, modulationHz, depth,
                           durationMs);
  } catch (const std::invalid_argument &) {
    throw std::logic_error("Error converting params for am gen filter");
  }
}

//  FmGenFilter

IFilter *fmGenProducer(const FilterDescriptor &fd) {
  if (std::string(fd.name) != "fm")
    throw std::logic_error("Wrong fd for fm gen filter producer");
  if (fd.params.size() < 5)
    throw std::logic_error("Wrong params number for fm gen filter producer");

  try {
    double amplitude = std::stod(fd.params[0]);
    double carrierHz = std::stod(fd.params[1]);
    double modulationHz = std::stod(fd.params[2]);
    double deviationHz = std::stod(fd.params[3]);
    double durationMs = std::stod(fd.params[4]);
    return new FmGenFilter(amplitude, carrierHz, modulationHz, deviationHz,
                           durationMs);
  } catch (const std::invalid_argument &) {
    throw std::logic_error("Error converting params for fm gen filter");
  }
}

} // namespace FilterProducers
