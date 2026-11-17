#include "wav_io.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

void writeChunkId(std::ofstream &file, const std::string &id) {
  file.write(reinterpret_cast<const char *>(id.data()),
             4); // записывает ровно 4 символа в поток (id.data() - указатель на
                 // данные внутри строки)
}

void writeUint32(std::ofstream &file, uint32_t value) {
  uint8_t bytes[4];
  bytes[3] = (value >> 24) & 0xFF;
  bytes[2] = (value >> 16) & 0xFF;
  bytes[1] = (value >> 8) & 0xFF;
  bytes[0] = value & 0xFF;
  file.write(reinterpret_cast<const char *>(bytes),
             4); // для write нужен указатель на константные символы
}

void writeUint16(std::ofstream &file, uint16_t value) {
  uint8_t bytes[2];
  bytes[1] = (value >> 8) & 0xFF;
  bytes[0] = value & 0xFF;
  file.write(reinterpret_cast<const char *>(bytes), 2);
}

bool WavWriter::write(const std::string &filename, const Waveform &waveform) {
  std::ofstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error creating file: " << filename << std::endl;
    return false;
  }

  const size_t sampleCount = waveform.getSize();
  const uint32_t dataSize =
      static_cast<uint32_t>(sampleCount * sizeof(int16_t));
  const uint32_t fmtChunkSize = 16;
  const uint32_t riffPayloadSize = 4 + (8 + fmtChunkSize) + (8 + dataSize);

  // RIFF header
  writeChunkId(file, "RIFF");
  writeUint32(file, riffPayloadSize);
  writeChunkId(file, "WAVE");

  // fmt chunk
  writeChunkId(file, "fmt ");
  writeUint32(file, fmtChunkSize);
  writeUint16(file, 1);             // audioFormat = PCM
  writeUint16(file, 1);             // numChannels = mono
  writeUint32(file, 44100);         // sampleRate
  writeUint32(file, 44100 * 1 * 2); // byteRate
  writeUint16(file, 1 * 2);         // blockAlign
  writeUint16(file, 16);            // bitsPerSample

  // data chunk
  writeChunkId(file, "data");
  writeUint32(file, dataSize);
  // пишем все сэмплы одним блоком через указатель
  if (sampleCount > 0) {
    file.write(reinterpret_cast<const char *>(waveform.data()),
               sampleCount * sizeof(int16_t));
  }

  return true;
}
