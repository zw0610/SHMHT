cmake_minimum_required(VERSION 3.5)
project(shmht CXX C)

add_definitions(-D_GNU_SOURCE)
add_compile_options(-Wall -Wshadow -Werror)

include_directories(${CMAKE_SOURCE_DIR})

if (${ENABLE_DEBUG})
    add_compile_options(-g -O0)
else ()
    add_compile_options(-g -O2)
endif ()

# if (${USE_ORIGINAL})
#     add_definitions(-DUSE_ORIGINAL)
#     # controller related
#     add_library(cuda-control SHARED
#             src/hijack_call.c
#             include/hijack.h
#             include/cuda-subset.h
#             include/cuda-helper.h
#             include/gmem.h
#             include/rnode.h
#             include/resource.h
#             src/resource.cxx
#             src/cuda_originals.cxx
#             src/loader.cxx)

#     target_link_libraries(cuda-c)

# else (NOT ${USE_ORIGINAL})
#     # controller related
#     add_library(cuda-control SHARED
#             src/hijack_call.c
#             include/hijack.h
#             include/cuda-subset.h
#             include/cuda-helper.h
#             include/gmem.h
#             include/rnode.h
#             include/resource.h
#             src/resource.cxx
#             src/cuda_originals.cxx
#             src/loader.cxx)

#     target_link_libraries(cuda-control ${STATIC_C_LIBRARIES})

# endif ()

add_executable(shmht 
                main.c 
                resource_c_wrapper.hpp 
                resource_c_wrapper.cxx 
                resource.hpp
                resource.cxx
                rnode.hpp
                gmem.hpp
                )

target_compile_options(shmht PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++11>)



