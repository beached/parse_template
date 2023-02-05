// The MIT License (MIT)
//
// Copyright (c) Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and / or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <daw/daw_parse_template.h>
#include <daw/io/daw_write_proxy.h>

#include <date/date.h>
#include <date/tz.h>
#include <daw/daw_memory_mapped_file.h>
#include <daw/daw_string_view.h>

#include <cstdlib>
#include <ctime>
#include <iostream>

int main( int argc, char const **argv ) {
	auto const current_time =
	  date::make_zoned( date::current_zone( ), std::chrono::system_clock::now( ) );
	std::cout << "Starting at: " << current_time << '\n';

	if( argc <= 1 ) {
		std::cerr << "Must supply a template file" << std::endl;
		exit( EXIT_FAILURE );
	}

	auto const template_str = daw::filesystem::memory_mapped_file_t<char>( argv[1] );

	if( not template_str ) {
		std::cerr << "Error opening file: " << argv[1] << std::endl;
		exit( EXIT_FAILURE );
	}

	auto p = daw::parse_template( template_str );

	p.add_callback( "dummy_text_cb", []( ) { return "This is some dummy text"; } );

	p.add_callback<int, int, daw::escaped_string>(
	  "dummy_text_cb2",
	  []( int a, int b, std::string str, daw::io::WriteProxy &writer ) {
		  using std::to_string;
		  using namespace std::literals;
		  auto ret =
		    writer.write( { "From ", to_string( a ), " to ", to_string( b ), " we say ", str } );
		  if( ret.status != daw::io::IOOpStatus::Ok ) {
			  std::terminate( );
		  }
	  } );

	p.add_callback<size_t, daw::escaped_string, daw::escaped_string>(
	  "repeat_test",
	  []( size_t how_many, std::string prefix, std::string suffix ) {
		  auto result = std::string( );

		  for( size_t n = 0; n < how_many; ++n ) {
			  result += prefix + std::to_string( n ) + suffix;
		  }
		  return result;
	  } );

	int x = 5;
	p.add_callback( "stateful_test", [count = 0U]( void *s ) mutable {
		int *v = reinterpret_cast<int *>( s );
		if( v ) {
			++( *v );
		}
		++count;
		return count + ( v ? *v : 0 );
	} );

	p.write_to( std::cout, &x );
	p.write_to( std::cout );

	return EXIT_SUCCESS;
}
