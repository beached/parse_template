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
	namespace {
		constexpr size_t find_quote( daw::string_view str ) noexcept {
			bool in_slash = false;
			for( size_t n = 0; n < str.size( ); ++n ) {
				if( str[n] == '\\' ) {
					in_slash = true;
					continue;
				} else {
					if( str[n] == '"' ) {
						if( not in_slash ) {
							return n;
						}
					}
					in_slash = false;
				}
			}
			return daw::string_view::npos;
		}
	} // namespace

	daw::string_view find_args( daw::string_view tag ) {
		using namespace daw::string_view_literals;
		tag.remove_prefix_until( R"(args=")" );
		if( tag.empty( ) ) {
			return tag;
		}
		auto end_quote_pos = find_quote( tag );
		if( end_quote_pos == daw::string_view::npos ) {
			throw std::runtime_error( "Could not find end of call args" );
		}
		tag.resize( end_quote_pos );
		return tag;
	}

	std::vector<daw::string_view> find_split_args( daw::string_view tag ) {
		tag = find_args( tag );
		std::vector<daw::string_view> results{ };
		bool in_quote = false;
		bool is_escaped = false;
		size_t last_pos = 0;
		for( size_t n = 0; n < tag.size( ); ++n ) {
			switch( tag[n] ) {
			case '"':
				if( not is_escaped ) {
					throw std::runtime_error( "Unexpected unescaped double-quote" );
				}
				in_quote = not in_quote;
				continue;
			case '\\':
				is_escaped = not is_escaped;
				continue;
			case ',':
				if( not in_quote ) {
					results.emplace_back( &tag[last_pos], n - last_pos );
					last_pos = n + 1;
				}
				continue;
			}
		}
		if( last_pos < tag.size( ) ) {
			results.emplace_back( &tag[last_pos], tag.size( ) - last_pos );
		}
		return results;
	}

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
