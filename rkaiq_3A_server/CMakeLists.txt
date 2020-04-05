
if(NOT CMAKE_BUILD_TYPE)
    # default to Release build for GCC builds
    set(CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

cmake_minimum_required(VERSION 2.8)

#project name
PROJECT(rkaiq_3A_server)

#set BUILROOT_BUILD_PROJECT
#set(BUILROOT_BUILD_PROJECT true)
#set(ENABLE_ASAN true)


# ----------------------------------------------------------------------------
# ADD SYMBOL TABLE FOR GDB
# ----------------------------------------------------------------------------
# SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -ggdb")
# SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -ggdb")

# ----------------------------------------------------------------------------
# set property to classify library kinds
# ----------------------------------------------------------------------------
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER "CMakeTargets")

#set(CMAKE_CONFIGURATION_TYPES "Debug;Release")

#set ADD_RK_AIQ
set(ADD_RK_AIQ true)

#SET RK_AIQ SOURCE DIR
set(RK_AIQ_SOURCE_DIR ${PROJECT_SOURCE_DIR}/..)
message("rkaiq src path" ${RK_AIQ_SOURCE_DIR})

if(ENABLE_ASAN)
add_compile_options(-fsanitize=address -fno-omit-frame-pointer -g)
SET(LINK_ASAN asan)
endif()

if(ADD_RK_AIQ)
add_compile_options(-std=c++11)
add_compile_options(-DADD_RK_AIQ)
add_compile_options(-O0)
# add_compile_options(-v)
SET(RK_AIQ_LIB_DIR ${RK_AIQ_SOURCE_DIR}/build/linux/all_lib/${CMAKE_BUILD_TYPE})
message("rkaiq lib path" ${RK_AIQ_LIB_DIR})
SET(RK_AIQ_INCLUDE_DIR
${RK_AIQ_SOURCE_DIR}/xcore/
${RK_AIQ_SOURCE_DIR}/xcore/base/
#${RK_AIQ_SOURCE_DIR}/aiq_core/
${RK_AIQ_SOURCE_DIR}/algos/
${RK_AIQ_SOURCE_DIR}/common/
${RK_AIQ_SOURCE_DIR}/common/linux/
#${RK_AIQ_SOURCE_DIR}/hwi/
${RK_AIQ_SOURCE_DIR}/ipc/
${RK_AIQ_SOURCE_DIR}/iq_parser/
${RK_AIQ_SOURCE_DIR}/uAPI/
#${RK_AIQ_SOURCE_DIR}/algos/awb/
#${RK_AIQ_SOURCE_DIR}/algos/af/
#${RK_AIQ_SOURCE_DIR}/algos/aorb/
#${RK_AIQ_SOURCE_DIR}/../core/inc/luma/
#${RK_AIQ_SOURCE_DIR}/../core/inc/stat_3a_ae/
#${RK_AIQ_SOURCE_DIR}/../core/inc/stat_3a_af/
#${RK_AIQ_SOURCE_DIR}/../core/inc/orb/
#${RK_AIQ_SOURCE_DIR}/../core/inc/common/
${RK_AIQ_SOURCE_DIR}/
)
SET(RK_AIQ_LIB rkaiq)
endif()

#head file path
include_directories(
${RK_AIQ_INCLUDE_DIR}
${PROJECT_SOURCE_DIR}/./
)

add_definitions(-D_CRT_SECURE_NO_WARNINGS)

#
link_directories("${LIBRARY_OUTPUT_PATH}" "${RK_AIQ_LIB_DIR}" )
#add executable file
ADD_EXECUTABLE(${PROJECT_NAME} rkaiq_3A_server.cpp)
target_link_libraries(${PROJECT_NAME} ${RK_AIQ_LIB}  pthread)
