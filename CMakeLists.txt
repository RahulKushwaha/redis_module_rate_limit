cmake_minimum_required(VERSION 3.22)
project(redis_rate_limiter_module)

set(CMAKE_CXX_STANDARD 20)
option(BUILD_SHARED_LIBS "Build using shared libraries" ON)
include(FetchContent)
FetchContent_Declare(
        folly
        GIT_REPOSITORY https://github.com/facebook/folly.git
        GIT_TAG v2024.01.01.00
)

FetchContent_MakeAvailable(folly)

include_directories(${folly_SOURCE_DIR})

find_package(Glog REQUIRED)  # GlogConfig.cmake NOT available; use FindGlog.cmake
include_directories(${GLOG_INCLUDE_DIRS})

add_library(redis_module SHARED RateLimiterModule.cc
        RateLimiter.h
        RateLimiter.cc
        RateLimiterModule.cc)
target_link_libraries(redis_module folly)

add_executable(redis_rate_limiter_module main.cpp
        RateLimiter.h
        RateLimiter.cc)
target_link_libraries(redis_rate_limiter_module folly)