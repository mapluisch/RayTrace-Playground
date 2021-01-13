################################################################################
#  CMake options
################################################################################

PROJECT(RayTrace-Playground)

CMAKE_MINIMUM_REQUIRED(VERSION 3.9)

SET(CMAKE_VERBOSE_MAKEFILE ON)


set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang") 
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")


################################################################################
# Project options
################################################################################




################################################################################
#  Project settings
################################################################################

MESSAGE(${PROJECT_NAME} " root directory: " ${PROJECT_SOURCE_DIR})


SET(MY_OUTPUT_DIR "${PROJECT_SOURCE_DIR}/build")

MESSAGE(${PROJECT_NAME} " output directory: " ${MY_OUTPUT_DIR})


SET(LIBRARY_OUTPUT_DIRECTORY ${MY_OUTPUT_DIR})
SET(LIBRARY_OUTPUT_PATH      ${MY_OUTPUT_DIR})
SET(RUNTIME_OUTPUT_DIRECTORY ${MY_OUTPUT_DIR})
SET(ARCHIVE_OUTPUT_DIRECTORY ${MY_OUTPUT_DIR})
SET(EXECUTABLE_OUTPUT_PATH   ${MY_OUTPUT_DIR})


SET(MY_PROJECT_INCLUDE_DIR include)
SET(MY_PROJECT_SOURCE_DIR src)


################################################################################
# Preprocessor configuration
################################################################################

#ADD_DEFINITIONS( -fno-strict-aliasing )


################################################################################
# Sources
################################################################################

# Only need to list the header files which must be processed by MOC.
# Also templates, if header and implementation are in distinct files.
SET(MY_HEADER_FILES

)

SET(MY_SOURCE_FILES

    ${MY_PROJECT_SOURCE_DIR}/RayTracer.cpp
)


################################################################################
# Find Packages CXXOPTS, OpenCV
################################################################################

find_package(cxxopts REQUIRED)
find_package(OpenCV REQUIRED)


################################################################################
# Compiler flags
################################################################################

INCLUDE_DIRECTORIES(
    ${MY_PROJECT_INCLUDE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    "/usr/local/include" "/usr/local/opt/llvm/include"
)

# CMake sets additional flags for debug and release builds by using the 
# correct build types.
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O3 -march=native -fopenmp")


################################################################################
# Compiler flags
################################################################################

# ...MacOSX.sdk/usr/lib linked to fix -lSystem erorr on -fopenmp usage
LINK_DIRECTORIES(

    "/usr/local/lib" 
    "/usr/local/opt/llvm/lib"
    "/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib"

)


################################################################################
# Build targets
################################################################################

ADD_EXECUTABLE(${PROJECT_NAME}
    
    ${MY_SOURCE_FILES}

)



################################################################################
# Link targets
################################################################################

TARGET_LINK_LIBRARIES(${PROJECT_NAME}
    cxxopts::cxxopts    
    ${OpenCV_LIBS}
)   


