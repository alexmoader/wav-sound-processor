#include "args_parser.h"
#include "cmd_line_args_2_pipeline_converter.h"
#include "filter_descriptor.h"
#include "pipeline.h"
#include "std_filter_producers.h"
#include "wav_io.h"
#include "waveform.h"

#include <exception>
#include <iostream>

namespace {

constexpr int kSampleRate = 44100;

void printHelp(const char *progName) {
  std::cout
      << "Sound Processor - converts WAV files through a chain of filters\n"
      << "\n"
      << "Usage: " << progName << " [options]\n"
      << "\n"
      << "Options:\n"
      << "  -i <file>      input WAV file\n"
      << "  -o <file>      output WAV file\n"
      << "  -f <filter>    apply filter (can be repeated)\n"
      << "\n"
      << "Filters:\n"
      << "  ampl <factor>\n"
      << "  normalize [peak]                (peak in [0,1], default 1.0)\n"
      << "  silence <unit> <start> <end>   (unit = sec or ms)\n"
      << "  timestretch <factor>            (factor > 0)\n"
      << "  lowpass <window_size>           (odd, >= 1)\n"
      << "  generator sin <freq_hz> <dur_ms>\n"
      << "  generator am <amp> <car_hz> <mod_hz> <depth> <dur_ms>\n"
      << "  generator fm <amp> <car_hz> <mod_hz> <dev_hz> <dur_ms>\n"
      << "\n"
      << "Examples:\n"
      << "  " << progName << " -i in.wav -o out.wav -f ampl 0.8\n"
      << "  " << progName << " -o out.wav -f generator sin 440 1000\n"
      << "  " << progName
      << " -i in.wav -o out.wav -f silence sec 0.2 0.4 -f ampl 1.5\n";
}

void registerAllProducers(CmdLineArgs2PipelineConverter &converter) {
  converter.addFilterProducer("ampl", FilterProducers::amplProducer);
  converter.addFilterProducer("normalize", FilterProducers::normalizeProducer);
  converter.addFilterProducer("silence", FilterProducers::silenceProducer);
  converter.addFilterProducer("timestretch",
                              FilterProducers::timestretchProducer);
  converter.addFilterProducer("lowpass", FilterProducers::lowpassProducer);
  converter.addFilterProducer("sin", FilterProducers::sinGenProducer);
  converter.addFilterProducer("am", FilterProducers::amGenProducer);
  converter.addFilterProducer("fm", FilterProducers::fmGenProducer);
}

} // namespace

int main(int argc, char *argv[]) {
  try {
    // шаг 1: разбираем аргументы
    ArgsParser parser;
    ArgsParser::Result parseRes = parser.parse(argc, argv);

    if (parseRes == ArgsParser::Result::noArgs) {
      printHelp(argv[0]);
      return 0;
    }

    if (parseRes == ArgsParser::Result::badArgs) {
      std::cerr << "Error: invalid command line arguments\n";
      printHelp(argv[0]);
      return 1;
    }

    // шаг 2: создаём pipeline через конвертер
    CmdLineArgs2PipelineConverter converter;
    registerAllProducers(converter);
    Pipeline pipeline = converter.createPipeline(parser.getFilterDescriptors());

    // шаг 3: читаем входной WAV (если был -i)
    Waveform sound(kSampleRate);
    const char *inFile = parser.getInFileName();
    if (inFile != nullptr) {
      WavReader reader;
      if (!reader.read(inFile, sound)) {
        std::cerr << "Error: failed to read input file: " << inFile << "\n";
        return 1;
      }
    }

    // шаг 4: применяем pipeline
    pipeline.apply(sound);

    // шаг 5: записываем выходной WAV (если был -o)
    const char *outFile = parser.getOutFileName();
    if (outFile != nullptr) {
      WavWriter writer;
      if (!writer.write(outFile, sound)) {
        std::cerr << "Error: failed to write output file: " << outFile << "\n";
        return 1;
      }
    }

    return 0;
  } catch (const std::exception &e) {
    std::cerr << "An exception handled: " << e.what() << "\n";
    return -1;
  } catch (...) {
    std::cerr << "An unknown exception\n";
    return -2;
  }
}