#include <catch2/catch_test_macros.hpp>

#include "args_parser.h"
#include "filters.h"
#include "pipeline.h"
#include "waveform.h"

#include <cstring>
#include <vector>

// ТЕСТЫ ПАРСЕРА

TEST_CASE("ArgsParser: различные сценарии разбора", "[args_parser]") {
  ArgsParser parser;

  SECTION("только имя программы – справка") {
    char *argv[] = {const_cast<char *>("sound_processor")};
    int argc = 1;
    REQUIRE(parser.parse(argc, argv) == ArgsParser::Result::noArgs);
  }

  SECTION("корректные -i и -o") {
    char *argv[] = {const_cast<char *>("sound_processor"),
                    const_cast<char *>("-i"), const_cast<char *>("in.wav"),
                    const_cast<char *>("-o"), const_cast<char *>("out.wav")};
    int argc = 5;
    REQUIRE(parser.parse(argc, argv) == ArgsParser::Result::ok);
    REQUIRE(std::strcmp(parser.getInFileName(), "in.wav") == 0);
    REQUIRE(std::strcmp(parser.getOutFileName(), "out.wav") == 0);
  }

  SECTION("неизвестный флаг") {
    char *argv[] = {const_cast<char *>("sound_processor"),
                    const_cast<char *>("-x")};
    int argc = 2;
    REQUIRE(parser.parse(argc, argv) == ArgsParser::Result::badArgs);
  }

  SECTION("флаг -i без значения") {
    char *argv[] = {const_cast<char *>("sound_processor"),
                    const_cast<char *>("-i")};
    int argc = 2;
    REQUIRE(parser.parse(argc, argv) == ArgsParser::Result::badArgs);
  }

  SECTION("повторный -i запрещён") {
    char *argv[] = {const_cast<char *>("sound_processor"),
                    const_cast<char *>("-i"), const_cast<char *>("first.wav"),
                    const_cast<char *>("-i"), const_cast<char *>("second.wav")};
    int argc = 5;
    REQUIRE(parser.parse(argc, argv) == ArgsParser::Result::badArgs);
  }
}

// ТЕСТЫ ФИЛЬТРОВ

TEST_CASE("Фильтры: проверка преобразований", "[filters]") {
  constexpr size_t kSampleRate = 44100;

  SECTION("AmplFilter с factor 1.5") {
    Waveform wf(kSampleRate);
    wf.resize(4);
    wf.setValueAt(0, 200);
    wf.setValueAt(1, -400);
    wf.setValueAt(2, 600);
    wf.setValueAt(3, -800);

    AmplFilter filter(1.5);
    REQUIRE(filter.apply(wf) == FilterState::kOk);

    REQUIRE(wf.valueAt(0) == 300);
    REQUIRE(wf.valueAt(1) == -600);
    REQUIRE(wf.valueAt(2) == 900);
    REQUIRE(wf.valueAt(3) == -1200);
  }

  SECTION("NormalizeFilter до пика 0.25") {
    Waveform wf(kSampleRate);
    wf.resize(4);
    wf.setValueAt(0, 100);
    wf.setValueAt(1, -200); // пик 200
    wf.setValueAt(2, 50);
    wf.setValueAt(3, 150);

    NormalizeFilter filter(0.25); // scale = 0.25 * 32767 / 200 = 40.96
    REQUIRE(filter.apply(wf) == FilterState::kOk);

    // после нормализации максимум должен быть около 8192
    REQUIRE(wf.valueAt(1) == -8192);
  }

  SECTION("SilenceFilter в секундах") {
    Waveform wf(kSampleRate);
    wf.resize(44100); // 1 секунда
    for (size_t i = 0; i < wf.getSize(); ++i)
      wf.setValueAt(i, 500);

    SilenceFilter filter("sec", 0.1,
                         0.2); // вставляем 0.1 с тишины (4410 сэмплов)
    REQUIRE(filter.apply(wf) == FilterState::kOk);

    REQUIRE(wf.getSize() == 48510); // 44100 + 4410
    REQUIRE(wf.valueAt(4409) == 500);
    REQUIRE(wf.valueAt(4410) == 0);
    REQUIRE(wf.valueAt(8819) == 0);
    REQUIRE(wf.valueAt(8820) == 500);
  }

  SECTION("TimestretchFilter сжатие в 2 раза") {
    Waveform wf(kSampleRate);
    wf.resize(4);
    wf.setValueAt(0, 0);
    wf.setValueAt(1, 1000);
    wf.setValueAt(2, 2000);
    wf.setValueAt(3, 3000);

    TimestretchFilter filter(0.5);
    REQUIRE(filter.apply(wf) == FilterState::kOk);

    REQUIRE(wf.getSize() == 2);
    REQUIRE(wf.valueAt(0) == 500);
    REQUIRE(wf.valueAt(1) == 2500);
  }

  SECTION("LowpassFilter с окном 5") {
    Waveform wf(kSampleRate);
    wf.resize(5);
    wf.setValueAt(0, 0);
    wf.setValueAt(1, 0);
    wf.setValueAt(2, 1000);
    wf.setValueAt(3, 0);
    wf.setValueAt(4, 0);

    LowpassFilter filter(5);
    REQUIRE(filter.apply(wf) == FilterState::kOk);

    REQUIRE(wf.valueAt(0) == 200);
    REQUIRE(wf.valueAt(1) == 200);
    REQUIRE(wf.valueAt(2) == 200);
    REQUIRE(wf.valueAt(3) == 200);
    REQUIRE(wf.valueAt(4) == 200);
  }
}

// ТЕСТ ПАЙПЛАЙНА

TEST_CASE("Pipeline: последовательное применение", "[pipeline]") {
  Waveform wf(44100);
  wf.resize(3);
  wf.setValueAt(0, 10);
  wf.setValueAt(1, -20);
  wf.setValueAt(2, 30);

  Pipeline p;
  p.addFilter(new AmplFilter(2.0)); // 20, -40, 60
  p.addFilter(new AmplFilter(0.5)); // 10, -20, 30

  REQUIRE(p.apply(wf) == FilterState::kOk);
  REQUIRE(wf.valueAt(0) == 10);
  REQUIRE(wf.valueAt(1) == -20);
  REQUIRE(wf.valueAt(2) == 30);
}