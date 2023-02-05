cmake_policy( SET CMP0065 NEW )

set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

if( CMAKE_BUILD_TYPE EQUAL "Debug" )
    add_compile_definitions( DEBUG=1 )
endif()

add_executable( example_parse_template example_parse_template.cpp )
target_link_libraries( example_parse_template PRIVATE daw::daw-parse-template )
add_compile_options( -fsanitize=address,undefined )
add_link_options( -fsanitize=address,undefined )
add_test( example_parse_template_test example_parse_template )
