cmake_minimum_required(VERSION 3.1)
project(depth_seg)
set(CMAKE_CXX_STANDARD 11)

add_library(my_python3.7m SHARED IMPORTED)
set_target_properties(my_python3.7m PROPERTIES
        IMPORTED_LOCATION "/home/user/anaconda3/envs/my_env/lib/libpython3.7m.so"
        INTERFACE_INCLUDE_DIRECTORIES "/home/user/anaconda3/envs/my_env/include/python3.7m/"
        )

find_package(OpenCV 4)
FIND_PACKAGE(Boost COMPONENTS thread system filesystem REQUIRED)

include_directories(${PYTHON_INCLUDE_DIR} ${OpenCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
add_executable(depth_seg depth_segmentaion.cpp)
target_link_libraries(depth_seg my_python3.7m ${OpenCV_LIBRARIES} ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${Boost_SYSTEM_LIBRARY} )
