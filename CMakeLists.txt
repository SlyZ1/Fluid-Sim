set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
cmake_minimum_required(VERSION 3.30)

# Compile glad
add_library(glad STATIC ${CMAKE_SOURCE_DIR}/src/glad.c)
target_include_directories(glad PUBLIC ${CMAKE_SOURCE_DIR}/include)

# Compile imgui
file(GLOB_RECURSE IMGUI_SOURCES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/include/imgui/*.cpp)
list(FILTER IMGUI_SOURCES EXCLUDE REGEX ".*freetype.*")
add_library(imgui STATIC ${IMGUI_SOURCES})
target_include_directories(imgui PUBLIC ${CMAKE_SOURCE_DIR}/include/imgui)
target_link_libraries(imgui PUBLIC glfw)

# Get libraries directly from their repo
include(FetchContent)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
FetchContent_Declare(nfd GIT_REPOSITORY https://github.com/btzy/nativefiledialog-extended.git GIT_TAG v1.3.0)
FetchContent_Declare(glfw GIT_REPOSITORY https://github.com/glfw/glfw.git GIT_TAG 3.5.1)
FetchContent_MakeAvailable(nfd glfw)

# Load OpenMP
find_package(OpenMP REQUIRED)

# Look for cpp files recursively
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/src/*.cpp")
add_executable(myprogram ${SOURCES})

if(MSVC)
    target_compile_options(myprogram PRIVATE /W4 /EHsc /openmp /Zi)
    target_compile_features(myprogram PRIVATE cxx_std_17)
else()
    target_compile_options(myprogram PRIVATE -Wall -Wextra -fopenmp -g)
    target_compile_features(myprogram PRIVATE cxx_std_17)
endif()

target_link_libraries(myprogram PRIVATE glad imgui glfw nfd OpenMP::OpenMP_CXX)