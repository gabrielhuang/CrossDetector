set(prj_name WebcamTest)

add_executable(
	${prj_name}
	main.cpp
)

target_link_libraries(
	${prj_name}
	VideoFlow	
)	