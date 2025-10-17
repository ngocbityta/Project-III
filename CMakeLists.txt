cmake_minimum_required(VERSION 3.22)
project(TeacherScheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ------------------------------------------------
# Find dependencies
# ------------------------------------------------
find_package(ortools REQUIRED)  
find_package(nlohmann_json REQUIRED)
find_package(Drogon REQUIRED)

# ------------------------------------------------
# Source files
# ------------------------------------------------
add_executable(teacher_scheduler
    src/main.cpp
    src/controller/TeacherSchedulerController.cpp
    src/scheduler/phase1.cpp
    src/scheduler/phase2.cpp
    src/scheduler/phase3.cpp
)

# ------------------------------------------------
# Include + link (Homebrew cài or-tools và json)
# ------------------------------------------------
target_include_directories(teacher_scheduler PRIVATE
    /usr/local/include
)

target_link_directories(teacher_scheduler PRIVATE
    /usr/local/lib
)

target_link_libraries(teacher_scheduler PRIVATE
    ortools::ortools
    nlohmann_json::nlohmann_json
)

target_link_libraries(teacher_scheduler PRIVATE
    ortools::ortools
    nlohmann_json::nlohmann_json
    Drogon::Drogon
)

# ------------------------------------------------
# Output
# ------------------------------------------------
set_target_properties(teacher_scheduler PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/build/bin
    VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

