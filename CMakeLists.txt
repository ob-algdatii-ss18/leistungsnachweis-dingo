#Force out - of - source build
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "Prevented in-tree built. Please create a build directory outside of the source code and call cmake from there")
endif()

#Specify the minimum CMAKE version required
cmake_minimum_required(VERSION 3.9)

#Project Name
project(AlgDatII)

#Helper Function
function(assign_source_group)
    foreach(_source IN ITEMS ${ARGN})
        if (IS_ABSOLUTE "${_source}")
            file(RELATIVE_PATH _source_rel "${CMAKE_CURRENT_SOURCE_DIR}" "${_source}")
        else()
            set(_source_rel "${_source}")
        endif()
        get_filename_component(_source_path "${_source_rel}" PATH)
        string(REPLACE "/" "\\" _source_path_msvc "${_source_path}")
        source_group("${_source_path_msvc}" FILES "${_source}")
    endforeach()
endfunction(assign_source_group)

#Flags
set (CMAKE_CXX_STANDARD 17)

if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)	
	add_definitions(-D_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING)	
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")

	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

endif()

set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

#Third Party Builds
	set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	
	## SDL	
		if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
			set(SDL2_DIR ThirdParty/SDL2)
		endif()
		find_package(SDL2 REQUIRED)
		
	## 3rd Party Includes
		set(THIRD_PARTY_INCLUDES 	ThirdParty ${SDL2_INCLUDE_DIRS})		
		
		set(THIRD_PARTY_LIBS 		${SDL2_LIBRARIES})
		
		set(THIRD_PARTY_SRC_FILES   )
		set(THIRD_PARTY_BINS ${SDL2_BINS})

#Includes
include_directories(src
					${THIRD_PARTY_INCLUDES})

#Source Files
file(GLOB_RECURSE SOURCES
    src/*.h
    src/*.cpp
)

set(ALL_SOURCE_FILES 	${SOURCES}
						${THIRD_PARTY_SRC_FILES})
	


#Executable
add_executable(${PROJECT_NAME} ${ALL_SOURCE_FILES})

# Create Source Groups based on path
assign_source_group(${ALL_SOURCE_FILES})

target_link_libraries(${PROJECT_NAME} ${THIRD_PARTY_LIBS})

# Copy SDL Dll
if (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
foreach(loop_var ${THIRD_PARTY_BINS})
  get_filename_component(filename ${loop_var} NAME)
  add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${loop_var}
                ${CMAKE_CURRENT_BINARY_DIR}/${filename})
endforeach(loop_var)	
endif()

include(gtest.cmake)
enable_testing()
add_executable(tests test/testMain.cpp)
target_link_libraries(tests gtest gtest_main)
add_test(tests tests)