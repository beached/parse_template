cmake_minimum_required(VERSION 3.20)

project(daw-parse-template
        VERSION "0.1.0"
        DESCRIPTION "Generate strings from templates"
        HOMEPAGE_URL "https://github.com/beached/parse_template"
        LANGUAGES C CXX
        )

set(CMAKE_CXX_STANDARD 17 CACHE STRING "The C++ standard whose features are requested.")
option(DAW_ENABLE_TESTING "Build unit tests and examples" OFF)
option(DAW_USE_PACKAGE_MANAGEMENT "Do not use FetchContent and assume dependencies are installed" OFF)
option(DAW_SKIP_DEPS "Do not install deps, let parent" OFF)

if (NOT DAW_USE_PACKAGE_MANAGEMENT AND NOT DAW_SKIP_DEPS)
    add_subdirectory(extern)
elseif (DAW_USE_PACKAGE_MANAGEMENT)
    find_package(daw-header-libraries REQUIRED)
endif ()

find_package(date CONFIG REQUIRED)

include_directories(include)

add_library(${PROJECT_NAME} src/daw/daw_parse_template.cpp)
target_link_libraries(${PROJECT_NAME} PUBLIC daw::daw-header-libraries date::date date::date-tz)
add_library(daw::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

install(TARGETS ${PROJECT_NAME} DESTINATION lib)
install(DIRECTORY include DESTINATION include)

if (DAW_ENABLE_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif ()