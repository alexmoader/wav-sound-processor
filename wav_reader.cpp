#include "wav_io.h"
#include <fstream>
#include <iostream>
#include <stdexcept>

// важный момент - Little Endian

// ┌─ RIFF header (12 байт)        ──────────────┐
// │ "RIFF" | размер | "WAVE"                    |
// ├─ fmt header (16 байт)         ──────────────┤
// │ "fmt " | 16 | 1 (PCM) | 1 (моно) | 44100    │
// │ byte_rate | block_align | 16 (bits)         │
// ├─ data header (8 байт)         ──────────────┤
// │ "data" | 88200 (44100 * 2 байта на sample)  │
// ├─ Звуковые данные (88200 байт) ──────────────|
// │ int16_t[0] int16_t[1] int16_t[2] ...        │
// └─────────────────────────────────────────────┘

uint16_t readUint16(std::ifstream &stream) // чтение 16-битного целого числа
                                           // little-endian из БИНАРНОГО ФАЙЛА
{
  uint8_t bytes[2];
  // stream.read(buffer, size) - ожидает char*
  stream.read(reinterpret_cast<char *>(bytes), 2);
  return (bytes[1] << 8) | bytes[0];
}

uint32_t readUint32(std::ifstream &stream) // чтение 32-битного целого числа
                                           // little-endian из БИНАРНОГО ФАЙЛА
{
  uint8_t bytes[4];

  // stream.read(buffer, size) - ожидает char*
  stream.read(reinterpret_cast<char *>(bytes), 4);
  uint32_t result =
      (bytes[3] << 24) | (bytes[2] << 16) | (bytes[1] << 8) | bytes[0];
  return result;
}

std::string readChunkId(std::ifstream &file) {
  char buffer[4];
  file.read(buffer, 4);
  return std::string(buffer, 4); // создаём строку из 4 символов
}

bool WavReader::read(const std::string &filename, Waveform &waveform) {

  // открываем файл в бинарке
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    std::cerr << "Error opening file: " << filename << std::endl;
    return false;
  }

  //-----------читаем RIFF header (12 байт) -----------
  std::string riffId = readChunkId(file);
  if (riffId != "RIFF") {
    std::cerr << "Invalid WAV file (not RIFF): " << filename << std::endl;
    return false;
  }

  // читаем размер файла
  uint32_t fileSize = readUint32(file);
  if (fileSize == 0) {
    std::cerr << "Invalid WAV file (empty file): " << filename << std::endl;
    return false;
  }

  // читаем Wave
  std::string waveId = readChunkId(file);
  if (waveId != "WAVE") {
    std::cerr << "Invalid WAV file (not WAVE): " << filename << std::endl;
    return false;
  }

  //----------reading fmt header-----------
  std::string fmtId = readChunkId(file);
  if (fmtId != "fmt ") {
    std::cerr << "Invalid WAV file (not fmt): " << filename << std::endl;
    return false;
  }

  uint32_t fmtSize = readUint32(file);
  if (fmtSize != 16) {
    std::cerr << "Invalid WAV file (unsupported fmt size): " << filename
              << std::endl;
    return false;
  }

  uint16_t audioFormat = readUint16(file);
  if (audioFormat != 1) {
    std::cerr << "Invalid WAV file (not PCM): " << filename << std::endl;
    return false;
  }

  uint16_t numChannels = readUint16(file);
  uint32_t sampleRate = readUint32(file);
  uint32_t byteRate = readUint32(file);
  uint16_t blockAlign = readUint16(file);
  uint16_t bitsPerSample = readUint16(file);

  // проверяем, что формат поддерживается
  if (numChannels != 1 || sampleRate != 44100 || bitsPerSample != 16) {
    std::cerr << "Invalid WAV file (unsupported format): " << filename
              << std::endl;
    return false;
  }

  // проверяем согласованность byteRate и blockAlign
  if (byteRate != sampleRate * numChannels * bitsPerSample / 8 ||
      blockAlign != numChannels * bitsPerSample / 8) {
    std::cerr << "Invalid WAV file (inconsistent fmt fields): " << filename
              << std::endl;
    return false;
  }

  //----------reading data header----------
  std::string dataId = readChunkId(file);
  if (dataId != "data") {
    std::cerr << "Invalid WAV file (not data): " << filename << std::endl;
    return false;
  }

  uint32_t dataSize = readUint32(file);
  size_t numSamples = dataSize / sizeof(int16_t);

  waveform.resize(numSamples);

  // читаем все сэмплы одним блоком через сырой указатель
  if (numSamples > 0) {
    file.read(reinterpret_cast<char *>(waveform.data()),
              numSamples * sizeof(int16_t));
  }

  return true;
}
