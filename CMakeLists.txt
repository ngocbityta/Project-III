cmake_minimum_required(VERSION 3.15...3.30)
project(SchedulerAssignmentProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ----------------------------
# OR-TOOLS
# ----------------------------
include_directories(${CMAKE_SOURCE_DIR}/ortools)
include_directories(${CMAKE_SOURCE_DIR}/ortools/ortools)
link_directories(${CMAKE_SOURCE_DIR}/ortools/build/lib)

# ----------------------------
# NLOHMANN JSON (tự động tải)
# ----------------------------
include(FetchContent)
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.2
)
FetchContent_MakeAvailable(nlohmann_json)

# ----------------------------
# SOURCE FILES
# ----------------------------
add_executable(main
    main.cpp
    phase1.cpp
    phase2.cpp
    phase3.cpp
)

target_include_directories(main PRIVATE ${CMAKE_SOURCE_DIR})

# ----------------------------
# LINK LIBRARIES
# ----------------------------
target_link_libraries(main
    ortools
    nlohmann_json::nlohmann_json
)
