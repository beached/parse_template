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

#include "daw/daw_parse_template.h"

#include <daw/daw_string_view.h>
#include <daw/io/daw_write_proxy.h>

namespace daw::parse_template_impl {
	std::string &to_string( std::string &str ) noexcept {
		return str;
	}

	std::string to_string( std::string &&str ) noexcept {
		return std::move( str );
	}

	void parse_template_impl::doc_parts::operator( )( daw::io::WriteProxy &writer,
	                                                  void *state ) const {
		m_to_string( writer, state );
	}

	std::string trim_quotes( daw::string_view str ) {
		if( str.size( ) >= 2 and str.front( ) == '"' and str.back( ) == '"' ) {
			str.remove_prefix( );
			str.remove_suffix( );
		}
		return static_cast<std::string>( str );
	}
} // namespace daw::parse_template_impl

namespace daw {
	std::string parse_to_value( daw::string_view str, daw::tag_t<escaped_string> ) {
		static constexpr auto unescape = []( char c ) {
			switch( c ) {
			case 'a':
				return '\a';
			case 'b':
				return '\b';
			case 'f':
				return '\f';
			case 'n':
				return '\n';
			case 'r':
				return '\r';
			case 't':
				return '\t';
			case 'v':
				return '\v';
			default:
				return c;
			}
		};

		auto result = std::string( );
		result.reserve( str.size( ) );
		while( not str.empty( ) ) {
			result.append( static_cast<std::string_view>( str.pop_front_until( '\\', nodiscard ) ) );
			if( str.starts_with( '\\' ) ) {
				str.remove_prefix( );
				if( str.empty( ) ) {
					throw std::runtime_error( "Invalid escape sequence" );
				}
				result += unescape( str.pop_front( ) );
			}
		}
		return parse_template_impl::trim_quotes( result );
	}
} // namespace daw
