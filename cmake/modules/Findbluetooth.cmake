# - Try to find bluetooth
# Once done this will define
# bluetooth_FOUND - System has bluetooth
# bluetooth_INCLUDE_DIRS - The bluetooth include directories
# bluetooth_LIBRARIES - The libraries needed to use bcm2835

find_path ( bluetooth_INCLUDE_DIR bluetooth/bluetooth.h )
find_library ( bluetooth_LIBRARY NAMES bluetooth )

set ( bluetooth_LIBRARIES ${bluetooth_LIBRARY} )
set ( bluetooth_INCLUDE_DIRS ${bluetooth_INCLUDE_DIR} )

include ( FindPackageHandleStandardArgs )
# handle the QUIETLY and REQUIRED arguments and set bluetooth_FOUND to TRUE
# if all listed variables are TRUE
find_package_handle_standard_args ( bluetooth DEFAULT_MSG bluetooth_LIBRARY bluetooth_INCLUDE_DIR )
