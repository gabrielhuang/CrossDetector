set(prj_name UAV)

#find_package(OpenCV REQUIRED)
#find_package(Freeglut REQUIRED)
#find_package(OpenGL REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} 
	${FREEGLUT_INCLUDE_DIRS} 
	${OPENGL_INCLUDE_DIR}
	${VIDEOMAN_INCLUDE_DIRS}
)
link_directories(${VIDEOMAN_LIB_DIRS})
add_executable(${prj_name} 
	videoman_dev.h
	background_video_flow.h
	main.cpp
)
add_dependencies(${prj_name}
	Utilities
)
target_link_libraries(${prj_name} 
	VideoMan 
	${FREEGLUT_LIBRARIES}  
	${OPENGL_LIBRARIES} 
	${OpenCV_LIBS}
	Ws2_32.lib	
	${boost_thread}
	VideoFlow
)
