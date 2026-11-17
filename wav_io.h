#ifndef SOUND_PROCESSOR_WAV_IO_H
#define SOUND_PROCESSOR_WAV_IO_H

#include "waveform.h"
#include <cstdint>
#include <fstream>
#include <string>

// будем пользоваться вспомогательными функциями для работы с данными

std::string readChunkId(std::ifstream &file);
uint32_t readUint32(std::ifstream &file);
uint16_t readUint16(std::ifstream &file);

void writeChunkId(std::ofstream &file, const std::string &id);
void writeUint32(std::ofstream &file, uint32_t value);
void writeUint16(std::ofstream &file, uint16_t value);

class WavReader {
public:
  bool read(const std::string &filename, Waveform &waveform);
  // возвращает true, если чтение прошло успешно
};

class WavWriter {
public:
  bool write(const std::string &filename, const Waveform &waveform);
  // возвращает true, если запись прошла успешно
};

#endif // SOUND_PROCESSOR_WAV_IO_H
