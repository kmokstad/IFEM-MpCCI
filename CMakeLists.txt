project(IFEM-MpCCI)

cmake_minimum_required(VERSION 3.0)

# Add local modules
list(APPEND CMAKE_MODULE_PATH
            ${PROJECT_SOURCE_DIR}
            ${PROJECT_SOURCE_DIR}/../../cmake/Modules)

# Required packages
if (NOT IFEM_CONFIGURED)
  find_package(IFEM REQUIRED)
  include_directories(${IFEM_INCLUDE_DIRS})
  add_definitions(${IFEM_DEFINITIONS})
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${IFEM_CXX_FLAGS}")
endif(NOT IFEM_CONFIGURED)

find_package(MpCCI REQUIRED)

set(ELASTICITY_DIR ${PROJECT_SOURCE_DIR}/../IFEM-Elasticity)
if(NOT EXISTS ${ELASTICITY_DIR})
  set(ELASTICITY_DIR ${PROJECT_SOURCE_DIR}/../Elasticity)
endif()

set(FINITEDEF_DIR ${ELASTICITY_DIR}/IFEM-FiniteDeformation)
if(NOT EXISTS ${FINITEDEF_DIR})
  set(FINITEDEF_DIR ${ELASTICITY_DIR}/FiniteDeformation)
endif()

include_directories(${IFEM_INCLUDES} ../Common ${PROJECT_SOURCE_DIR}
                    ${ELASTICITY_DIR} ${ELASTICITY_DIR}/Linear ${FINITEDEF_DIR})

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)

# Testing
enable_testing()
include(IFEMTesting)

if(NOT TARGET Elasticity)
  add_subdirectory(${ELASTICITY_DIR} Elasticity)
endif()

if(NOT TARGET FiniteDeformation)
  add_subdirectory(${FINITEDEF_DIR} FiniteDeformation)
endif()

add_library(MpCCICommon MpCCIJob.C
                        MpCCIJob.h
                        MpCCIMeshData.C
                        MpCCIMeshData.h
                        MpCCIMockJob.C
                        MpCCIMockJob.h
                        MpCCIPressureLoad.C
                        MpCCIPressureLoad.h
                        SIMMpCCIStructure.C
                        SIMMpCCIStructure.h
                        MpCCIDataHandler.h)
target_link_libraries(MpCCICommon FiniteDeformation
                                  Elasticity
                                  MpCCI::MpCCI
                                  IFEMAppCommon
                                  ${IFEM_LIBRARIES})

add_executable(IFEM-MpCCI main_MpCCI.C
                          MpCCIArgs.C
                          MpCCIArgs.h
                          SIMSolverMpCCI.h)
add_executable(IFEM-MockMpCCI main_MockMpCCI.C
                          MpCCIArgs.C
                          MpCCIArgs.h
                          SIMSolverMpCCI.h)
add_executable(StructureSolver main_StructureSolver.C
                               MpCCIArgs.C
                               MpCCIArgs.h)

list(APPEND CHECK_SOURCES main_MpCCI.C
                          main_StructureSolver.C
                          MpCCIJob.C
                          MpCCIArgs.C)

target_link_libraries(IFEM-MpCCI MpCCICommon
                                 Elasticity
                                 FiniteDeformation
                                 MpCCI::MpCCI
                                 IFEMAppCommon
                                 ${IFEM_LIBRARIES})

target_link_libraries(IFEM-MockMpCCI MpCCICommon
                                     Elasticity
                                     FiniteDeformation
                                     MpCCI::MpCCI
                                     IFEMAppCommon
                                     ${IFEM_LIBRARIES})

target_link_libraries(StructureSolver MpCCICommon
                                      Elasticity
                                      FiniteDeformation
                                      MpCCI::MpCCI
                                      IFEMAppCommon
                                      ${IFEM_LIBRARIES})

# Installation
install(TARGETS IFEM-MpCCI DESTINATION bin)

IFEM_add_test_app(${PROJECT_SOURCE_DIR}/Test/*.C
                  ${PROJECT_SOURCE_DIR}/Test
                  MpCCI 0
                  MpCCICommon Elasticity FiniteDeformation IFEMAppCommon ${IFEM_LIBRARIES})

if(IFEM_COMMON_APP_BUILD)
  set(TEST_APPS ${TEST_APPS} PARENT_SCOPE)
else()
  add_check_target()
endif()
