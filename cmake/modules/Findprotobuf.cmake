 # - Try to find protobuf
# Once done this will define
# protobuf_FOUND - System has protobuf
# protobuf_INCLUDE_DIRS - The protobuf include directories
# protobuf_LIBRARIES - The libraries needed to use bcm2835

find_path ( protobuf_INCLUDE_DIR usr/share/include/google/protobuf)
find_library ( protobuf_LIBRARY NAMES protobuf )

set ( protobuf_LIBRARIES ${protobuf_LIBRARY} )
set ( protobuf_INCLUDE_DIRS ${protobuf_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
# handle the QUIETLY and REQUIRED arguments and set protobuf_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args ( protobuf DEFAULT_MSG protobuf_LIBRARY protobuf_INCLUDE_DIR )
