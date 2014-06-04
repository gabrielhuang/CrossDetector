set(prj_name UAV)

set(FREEGLUT_INCLUDE_DIRS E:/ExternalLibs/VideoMan1.1/3rdparty/freeglut/include)
set(FREEGLUT_LIBRARIES E:/ExternalLibs/VideoMan1.1/3rdparty/freeglut/freeglut.lib)

#find_package(OpenCV REQUIRED)
#find_package(Freeglut REQUIRED)
#find_package(OpenGL REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS} ${FREEGLUT_INCLUDE_DIRS} ${OPENGL_INCLUDE_DIR} )
add_executable(${prj_name} client.cpp main.cpp)
target_link_libraries(${prj_name} 
	VideoMan 
	${FREEGLUT_LIBRARIES}  
	${OPENGL_LIBRARIES} 
	${OpenCV_LIBS}
	Ws2_32.lib	
	${boost_thread}
	VideoFlow
)
set_target_properties(${prj_name} PROPERTIES FOLDER "examples")
add_dependencies(${prj_name} 
	VideoMan 
)

