cmake_minimum_required(VERSION 2.8)
project( csrt_descaled_tracker  )
find_package( OpenCV REQUIRED )
include_directories( ${OpenCV_INCLUDE_DIRS} )
add_executable( csrt_descaled_tracker  main.cpp )
target_link_libraries( csrt_descaled_tracker ${OpenCV_LIBS} )

