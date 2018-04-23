cmake_minimum_required (VERSION 3.5)

project(Lightshafts)

file(GLOB SOURCE_FILES
	${CMAKE_CURRENT_LIST_DIR}/Lightshafts/*.h
	${CMAKE_CURRENT_LIST_DIR}/Lightshafts/*.cpp
)
file(GLOB SHADER_FILES
	${CMAKE_CURRENT_LIST_DIR}/Lightshafts/assets/shaders/*.vert
	${CMAKE_CURRENT_LIST_DIR}/Lightshafts/assets/shaders/*.frag
)
file(GLOB OBJ_FILES
	${CMAKE_CURRENT_LIST_DIR}/Lightshafts/assets/obj/*.obj
)

find_package(OpenGL REQUIRED)
find_package(OpenMP REQUIRED)
if(WIN32)
	include_directories(${CMAKE_CURRENT_LIST_DIR}/Lightshafts/include)
	set(SDL2_LIBRARY
		${CMAKE_CURRENT_LIST_DIR}/Lightshafts/lib/SDL2.lib
		${CMAKE_CURRENT_LIST_DIR}/Lightshafts/lib/SDL2main.lib
		${CMAKE_CURRENT_LIST_DIR}/Lightshafts/lib/SDL2test.lib
	)
	set(GLEW_LIBRARY
		${CMAKE_CURRENT_LIST_DIR}/Lightshafts/lib/glew32.lib
		${CMAKE_CURRENT_LIST_DIR}/Lightshafts/lib/glew32s.lib
	)
	set(PROJECT_LINK_LIBS ${OPENGL_LIBRARY} ${OpenMP_CXX_LIBRARIES} ${GLEW_LIBRARY} ${SDL2_LIBRARY})
	file(GLOB DLL_FILES ${CMAKE_CURRENT_LIST_DIR}/Lightshafts/lib/*.dll)

	file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Debug/)
	file(COPY ${DLL_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Release/)
	file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Debug/assets/shaders/)
	file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Release/assets/shaders/)
	file(COPY ${OBJ_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Debug/assets/obj/)
	file(COPY ${OBJ_FILES} DESTINATION ${CMAKE_BINARY_DIR}/Release/assets/obj/)
elseif(UNIX)
	find_package(SDL2 REQUIRED)
	find_package(GLEW REQUIRED)
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
	set(CMAKE_CXX_FLAGS "-std=c++11")
	include_directories("/usr/include" $(CMAKE_CURRENT_LIST_DIR)/Lightshafts/include)
	set(PROJECT_LINK_LIBS ${OPENGL_LIBRARY} ${OpenMP_CXX_LIBRARIES} ${GLEW_LIBRARIES} ${SDL2_LIBRARIES})
	file(COPY ${SHADER_FILES} DESTINATION ${CMAKE_BINARY_DIR}/assets/shaders/)
    file(COPY ${OBJ_FILES} DESTINATION ${CMAKE_BINARY_DIR}/assets/obj/)
endif()

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${PROJECT_LINK_LIBS})