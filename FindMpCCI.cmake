# - Find MpCCI
#
#   MpCCI_FOUND       - True if MpCCI was found.
#   MpCCI_INCLUDE_DIR - where to find mpcci.h
#   MpCCI_LIBRARIES   - List of libraries when using MpCCI.
#

find_path(MpCCI_INCLUDE_DIR "mpcci.h")
find_library(MpCCI_LIBRARIES 
             NAMES "mpcci-64"
             PATH_SUFFIXES "lnx4_x64")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MpCCI DEFAULT_MSG MpCCI_INCLUDE_DIR MpCCI_LIBRARIES)

if(MpCCI_FOUND)
  add_library(MpCCI::MpCCI UNKNOWN IMPORTED)
  set_target_properties(MpCCI::MpCCI PROPERTIES
                        IMPORTED_LOCATION ${MpCCI_LIBRARIES}
                        INTERFACE_INCLUDE_DIRECTORIES ${MpCCI_INCLUDE_DIR}
                        INTERFACE_LINK_LIBRARIES ${MpCCI_LIBRARIES})
endif()

mark_as_advanced(MpCCI_INCLUDE_DIR MpCCI_LIBRARIES )
