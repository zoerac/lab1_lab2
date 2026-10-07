cmake_minimum_required (VERSION 3.14)
project (GRE)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH})

include(FetchContent)
include(ExternalProject)

find_package(OpenMP)
#find_package(MKL REQUIRED)
#find_package(JeMalloc REQUIRED)
#find_package(TBB REQUIRED)
find_library(TbbLib tbb)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(ENV_INCLUDE_DIRS "../env/lib/include")
set(ENV_LIBRARIES "../env/lib/lib")
set(MKL_INCLUDE_DIRECTORY "../env/intel/mkl/include")
set(MKL_LINK_DIRECTORY "../env/intel/mkl/lib/intel64")
include_directories(${MKL_INCLUDE_DIRECTORY})
link_directories(${MKL_LINK_DIRECTORY})
#include_directories(${MKL_INCLUDE_DIRS})
#include_directories(${TBB_INCLUDE_DIRS})
include_directories(${ENV_INCLUDE_DIRS})
link_directories(${ENV_LIBRARIES})
#include_directories("/home/zzr/lib/include/")
include_directories(SYSTEM src/competitor/hot/src/libs/hot/commons/include)
include_directories(SYSTEM src/competitor/hot/src/libs/hot/rowex/include)
include_directories(SYSTEM src/competitor/hot/src/libs/hot/single-threaded/include)
include_directories(SYSTEM src/competitor/hot/src/libs/idx/content-helpers/include)

#add_compile_options(-fmax-errors=3 -faligned-new -march=native -O3)
#set(CMAKE_CXX_FLAGS "-fmax-errors=3 -faligned-new -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "-fmax-errors=3 -march=native -g -DAEX_DEBUG -O0 -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "-fmax-errors=3 -faligned-new -march=native -O3")

# add_subdirectory(src/competitor/wormhole)
add_subdirectory(src/competitor/wormhole_u64)
add_subdirectory(src/competitor/masstree)

# microbench
add_executable(microbench ${CMAKE_CURRENT_SOURCE_DIR}/src/benchmark/microbench.cpp)
target_link_libraries(microbench
	PRIVATE
	# WORMHOLE
	WORMHOLEU64
	MASSTREE
)
target_link_libraries(microbench PUBLIC OpenMP::OpenMP_CXX  jemalloc mkl_rt tbbmalloc tbb)

