cmake_minimum_required(VERSION 2.6)

###### Solution Name ######
project(UAV)

message("PROJECT_BINARY_DIR  = ${PROJECT_BINARY_DIR}")
message("PROJECT_SOURCE_DIR  = ${PROJECT_SOURCE_DIR}")
 

# Solution Dirs
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(my_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(my_include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${my_include_dir})

# Boost - Set the following 5 options
set(boost_include_dir "E:/ExternalLibs/boost_1_55_0")
set(boost_lib_dir "E:/ExternalLibs/boost_1_55_0/stage/lib")
set(boost_mt "mt-")
set(boost_dbg "gd-")
set(boost_ver "1_55")
include_directories(${boost_include_dir})
link_directories(${boost_lib_dir})
set(boost_libs
	debug
	libboost_thread-vc100-${boost_mt}${boost_dbg}${boost_ver}.lib
	optimized
	libboost_thread-vc100-${boost_mt}${boost_ver}.lib)

# OpenCV - Set the following 3 options
set(opencv_ver 248)
set(opencv_include_dir "E:/ExternalLibs/opencv/build/include") 
set(opencv_lib_dir "E:/ExternalLibs/opencv/build/x86/vc10/lib")

include_directories(${opencv_include_dir})
link_directories(${opencv_lib_dir})
set(opencv_libs
	debug opencv_calib3d${opencv_ver}d.lib
	debug opencv_contrib${opencv_ver}d.lib
	debug opencv_core${opencv_ver}d.lib
	debug opencv_features2d${opencv_ver}d.lib
	debug opencv_flann${opencv_ver}d.lib
	debug opencv_gpu${opencv_ver}d.lib
	debug opencv_highgui${opencv_ver}d.lib
	debug opencv_imgproc${opencv_ver}d.lib
	debug opencv_legacy${opencv_ver}d.lib
	debug opencv_ml${opencv_ver}d.lib
	debug opencv_nonfree${opencv_ver}d.lib
	debug opencv_objdetect${opencv_ver}d.lib
	debug opencv_photo${opencv_ver}d.lib
	debug opencv_stitching${opencv_ver}d.lib
	debug opencv_ts${opencv_ver}d.lib
	debug opencv_video${opencv_ver}d.lib
	debug opencv_videostab${opencv_ver}d.lib
	optimized opencv_calib3d${opencv_ver}.lib
	optimized opencv_contrib${opencv_ver}.lib
	optimized opencv_core${opencv_ver}.lib
	optimized opencv_features2d${opencv_ver}.lib
	optimized opencv_flann${opencv_ver}.lib
	optimized opencv_gpu${opencv_ver}.lib
	optimized opencv_highgui${opencv_ver}.lib
	optimized opencv_imgproc${opencv_ver}.lib
	optimized opencv_legacy${opencv_ver}.lib
	optimized opencv_ml${opencv_ver}.lib
	optimized opencv_nonfree${opencv_ver}.lib
	optimized opencv_objdetect${opencv_ver}.lib
	optimized opencv_photo${opencv_ver}.lib
	optimized opencv_stitching${opencv_ver}.lib
	optimized opencv_ts${opencv_ver}.lib
	optimized opencv_video${opencv_ver}.lib
	optimized opencv_videostab${opencv_ver}d.lib)
include_directories(${opencv_include_dir})
link_directories(${opencv_link_dir})

add_subdirectory(src)

# # Boost
# include_directories(${boost_include_dir})
# link_directories(${boost_lib_dir})

# # Eigen
# include_directories(${eigen_include_dir})



# file(GLOB_RECURSE prj_include "${my_include_dir}/${prj_name}/*")
# message("Added library ${prj_name}")
# file(GLOB_RECURSE prj_src "${my_src_dir}/${prj_name}/*")
# add_executable(
	# ${prj_name}
	# ${prj_src}
# )
# # target_link_libraries(
	# # ${prj_name}
	# # ${prj_dep}
# # )	

