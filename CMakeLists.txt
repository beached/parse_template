cmake_minimum_required( VERSION 2.8.12 )

project( parse_template_prj )

find_package( Boost 1.59.0 REQUIRED COMPONENTS system filesystem regex unit_test_framework program_options iostreams )

add_definitions( -DBOOST_ALL_NO_LIB -DBOOST_ALL_DYN_LINK )

find_package( Threads REQUIRED )
find_package( CURL REQUIRED )

if( ${CMAKE_CXX_COMPILER_ID} STREQUAL 'MSVC' )
	add_compile_options( -D_WIN32_WINNT=0x0601 /std:c++latest )
else( )
	if( ${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang" )
		add_compile_options(-std=c++14 -march=native -pthread -Weverything -Wno-c++98-compat -Wno-covered-switch-default -Wno-padded -Wno-exit-time-destructors -Wno-c++98-compat-pedantic -Wno-unused-parameter -Wno-missing-noreturn -Wno-missing-prototypes -Wno-disabled-macro-expansion)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3")
	elseif( ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU" )
		add_compile_options(-std=c++14 -march=native -pthread -Wall -Wno-deprecated-declarations -Wduplicated-cond -Wlogical-op -Wnull-dereference -Wold-style-cast -Wshadow)
		set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")
		set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -g -O3")
	endif( )
endif( )

include_directories( SYSTEM ${Boost_INCLUDE_DIRS} )
include_directories( SYSTEM ${CURL_INCLUDE_DIRS} )
link_directories( ${Boost_LIBRARY_DIRS} )

if( EXISTS "${CMAKE_SOURCE_DIR}/dependent_projects/build" )
	add_custom_target( dependency_stub )
	include_directories( SYSTEM "${CMAKE_SOURCE_DIR}/dependent_projects/build/install/include" )
	link_directories( "${CMAKE_SOURCE_DIR}/dependent_projects/build/install/lib" )
else( )
	set( SHOULD_BUILD_DEPS ON )
	include( "${CMAKE_SOURCE_DIR}/dependent_projects/dependencies.txt" )

	add_library( dependency_stub STATIC "${CMAKE_SOURCE_DIR}/dependent_projects/stub.cpp" )
	add_dependencies( dependency_stub header_libraries_prj date_prj )
endif( )

set( HEADER_FOLDER "include" )
set( SOURCE_FOLDER "src" )
set( TEST_FOLDER "tests" )

include_directories( ${HEADER_FOLDER} )

set( HEADER_FILES
	${HEADER_FOLDER}/daw_parse_template.h
)

set( SOURCE_FILES
	${SOURCE_FOLDER}/daw_parse_template.cpp
)

add_library ( parse_template STATIC ${HEADER_FILES} ${SOURCE_FILES} )
add_dependencies( parse_template dependency_stub )
target_link_libraries( parse_template char_range tz ${CURL_LIBRARIES} )

add_executable( example_parse_template ${HEADER_FILES} ${TEST_FOLDER}/example_parse_template.cpp )
target_link_libraries( example_parse_template tz parse_template ${Boost_LIBRARIES} ${CMAKE_DL_LIBS} ${CMAKE_THREAD_LIBS_INIT} ${COMPILER_SPECIFIC_LIBS} )
add_dependencies( example_parse_template dependency_stub )
add_test( example_parse_template_test example_parse_template )

install( TARGETS parse_template DESTINATION lib )
install( DIRECTORY ${HEADER_FOLDER}/ DESTINATION include/daw )

