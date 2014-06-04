set(prj_name VideoFlow)

add_library(
	${prj_name}
	video_flow.cpp
	${my_include_dir}/video_flow.h
)

target_link_libraries(
	${prj_name}
	${opencv_libs}	
)	