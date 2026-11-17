cmake_minimum_required(VERSION 3.20)
project(sound_processor LANGUAGES CXX)

add_library(audio_filters STATIC
  waveform.cpp
  pipeline.cpp
  wav_reader.cpp
  wav_writer.cpp
  filters.cpp
  std_filter_producers.cpp
  args_parser.cpp
  cmd_line_args_2_pipeline_converter.cpp
)
target_compile_features(audio_filters PUBLIC cxx_std_17)
target_include_directories(audio_filters PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

add_executable(sound_processor main.cpp)
target_link_libraries(sound_processor PRIVATE audio_filters)

include(CTest)
if(BUILD_TESTING)
  find_package(Catch2 3 QUIET)
  if(NOT Catch2_FOUND)
    include(FetchContent)
    FetchContent_Declare(Catch2
      GIT_REPOSITORY https://github.com/catchorg/Catch2.git
      GIT_TAG v3.10.0
      GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(Catch2)
  endif()
  add_executable(test_sound_processor test_main.cpp)
  target_link_libraries(test_sound_processor PRIVATE audio_filters Catch2::Catch2WithMain)
  add_test(NAME sound_processor_tests COMMAND test_sound_processor)
endif()
