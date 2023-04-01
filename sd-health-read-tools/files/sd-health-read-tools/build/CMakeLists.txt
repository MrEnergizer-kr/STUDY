project(sd-health-read-tools)
cmake_minimum_required(VERSION 2.6)

set(SD_HEALTH_READ_ROOT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/..)
set(SRC_PATH "${SD_HEALTH_READ_ROOT_PATH}/src")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Werror -Wno-int-to-pointer-cast")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -Wno-int-to-pointer-cast")

# add target
add_executable (sd_health_read ${SRC_PATH}/sd_health_read.c)
add_library (sd_get_data SHARED ${SRC_PATH}/sd_get_data.c)

#CFLAGS += "-lpthread -lsgutils2 -ludev"
target_link_libraries(sd_get_data pthread sgutils2 )

# Linking Libraries on target
target_link_libraries (sd_health_read sd_get_data udev)

# Reference include directories
include_directories (${SYSROOTINC_PATH} ${SYSROOT_INCLUDEDIR} ${SD_HEALTH_READ_ROOT_PATH}/include )

# libraries directory
link_directories(${SYSROOT_LIBDIR})

# install target and files
install (TARGETS sd_health_read DESTINATION ${CMAKE_INSTALL_BINDIR})
install (TARGETS sd_get_data DESTINATION ${CMAKE_INSTALL_LIBDIR})
install (FILES ${SRC_PATH}/sd_usb.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install (FILES ${SRC_PATH}/sd_usb_win32.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}) 
install (FILES ${SRC_PATH}/sd_get_data.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
