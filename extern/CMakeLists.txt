# Copyright (c) Darrell Wright
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE or copy at "http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: "https://github.com/beached/daw_json_link
#

include( FetchContent )

set( DAW_ENABLE_TESTING OFF )

FetchContent_Declare(
        daw_header_libraries
        GIT_REPOSITORY https://github.com/beached/header_libraries.git
        GIT_TAG v2
)
FetchContent_Declare(
        daw_read_write
        GIT_REPOSITORY https://github.com/beached/daw_read_write.git
        GIT_TAG release
)
FetchContent_MakeAvailable( daw_header_libraries daw_read_write )

