cmake_minimum_required(VERSION 3.24) # 3.24+ is best for FetchContent
project(Game LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

include(FetchContent)

set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build libraries as static libraries" FORCE)

FetchContent_Declare(
  SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
  GIT_TAG main
)
FetchContent_Declare(
  freetype
  GIT_REPOSITORY https://gitlab.freedesktop.org/freetype/freetype.git
  GIT_TAG master
)
FetchContent_MakeAvailable(SDL3 freetype)

set(SDL3_SHARED OFF CACHE BOOL "" FORCE)
set(SDL3_STATIC ON CACHE BOOL "" FORCE)

if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()
set(FREETYPE_INCLUDE_DIRS "${FreeType_SOURCE_DIR}/include" CACHE PATH "" FORCE)
set(FREETYPE_LIBRARY FreeType CACHE STRING "" FORCE)

set(SDL3_IMAGE_AVIF OFF CACHE BOOL "" FORCE)
set(SDL3_IMAGE_AVIF_LIBAOM OFF CACHE BOOL "" FORCE)
set(SDL3_IMAGE_AVIF_LIBDAV1D OFF CACHE BOOL "" FORCE)
set(SDL3_IMAGE_JXL OFF CACHE BOOL "" FORCE)
set(SDL3_IMAGE_TIF OFF CACHE BOOL "" FORCE)
set(SDL3_IMAGE_WEBP OFF CACHE BOOL "" FORCE)
set(SDL3_IMAGE_PNG ON CACHE BOOL "" FORCE)
set(SDL3_IMAGE_JPG ON CACHE BOOL "" FORCE)

set(SDLIMAGE_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL3IMAGE_SHARED OFF CACHE BOOL "" FORCE)
set(SDL3IMAGE_STATIC ON CACHE BOOL "" FORCE)

set(SDLMIXER_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL3MIXER_SHARED OFF CACHE BOOL "" FORCE)
set(SDL3MIXER_STATIC ON CACHE BOOL "" FORCE)
set(DECODER_OGGVORBIS OFF CACHE BOOL "" FORCE)
set(DECODER_FLAC OFF CACHE BOOL "" FORCE)
set(DECODER_MP3 ON CACHE BOOL "" FORCE)
set(DECODER_MIDI OFF CACHE BOOL "" FORCE)
set(DECODER_OPUS OFF CACHE BOOL "" FORCE)
set(DECODER_WAV ON CACHE BOOL "" FORCE)

set(SDLTTF_VENDORED OFF CACHE BOOL "" FORCE)
set(SDL3TTF_SHARED OFF CACHE BOOL "" FORCE)
set(SDL3TTF_STATIC ON CACHE BOOL "" FORCE)



FetchContent_Declare(
  SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image.git
  GIT_TAG main
  GIT_SUBMODULES ""
)
FetchContent_Declare(
  SDL3_mixer
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
  GIT_TAG main
  GIT_SUBMODULES ""
)
FetchContent_Declare(
  SDL3_ttf
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_ttf.git
  GIT_TAG main
  GIT_SUBMODULES ""
)

FetchContent_MakeAvailable(SDL3_image SDL3_mixer SDL3_ttf)

set(SOURCES
    "src/Board.cpp"
    "src/BoardView.cpp"
    "src/Game.cpp"
    "src/main.cpp"
    "src/Menu.cpp"
    "src/Piece.cpp"
    "src/Utils.cpp"
)

add_executable(Game ${SOURCES})

target_include_directories(Game PRIVATE "include")

target_link_libraries(Game PRIVATE 
    SDL3::SDL3-static
    SDL3_image::SDL3_image-static
    SDL3_ttf::SDL3_ttf-static
    SDL3_mixer::SDL3_mixer-static
    $<$<PLATFORM_ID:Windows>:
      KERNEL32.lib
      USER32.lib
      GDI32.lib
      WINMM.lib
      IMM32.lib
      OLE32.lib
      OLEAUT32.lib
      VERSION.lib
      ADVAPI32.lib
      SETUPAPI.lib
      SHELL32.lib
      USP10.lib
      RPCRT4.lib
    >
)

set_target_properties(Game PROPERTIES WIN32_EXECUTABLE TRUE)

add_custom_command(TARGET Game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:Game>/assets"
    COMMENT "Copying assets to build directory..."
)