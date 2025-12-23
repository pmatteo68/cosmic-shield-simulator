# ------------------------------
# FILE: CMakeLists.txt
# ------------------------------

cmake_minimum_required(VERSION 3.5 FATAL_ERROR)
project(CosmicShieldSimulation)

find_package(Geant4 REQUIRED ui_all vis_all)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

include(${Geant4_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR}/include)

add_executable(CosmicShieldSimulation CosmicShieldSimulation.cc ${sources} ${headers})
target_link_libraries(CosmicShieldSimulation ${Geant4_LIBRARIES})

install(TARGETS CosmicShieldSimulation DESTINATION bin)

# Optional: copy macro files
# removed in 3.1.0 as now I point them with an env var
#file(GLOB macros ${PROJECT_SOURCE_DIR}/macros/*.mac)
#file(COPY ${macros} DESTINATION ${PROJECT_BINARY_DIR})

# Include local dirs
include_directories(${PROJECT_SOURCE_DIR}/include)
# Include JSON support
#include(FetchContent)
#FetchContent_Declare(
#  json
#  GIT_REPOSITORY https://github.com/nlohmann/json.git
#  GIT_TAG v3.11.2
#)
#FetchContent_MakeAvailable(json)
#target_link_libraries(CosmicShieldSimulation PRIVATE nlohmann_json::nlohmann_json)

#new in 1.0.1 - copy config - removed in 2.1.0 as now I have a env variable
#to point the config
#file(COPY ${CMAKE_SOURCE_DIR}/config DESTINATION ${CMAKE_BINARY_DIR})

#install harness (note: cmake -S . -B build after any change in the CMAKE file)
#to add in this file:
# install(TARGETS CosmicShieldSimulation DESTINATION bin)
# install(DIRECTORY include/ DESTINATION include)
# install(DIRECTORY config/ DESTINATION config)
# install(PROGRAMS your_script.sh DESTINATION .)
#then:
# command: cmake --install build --prefix /desired/path
# (default /usr/local if you don't specify --prefix)

