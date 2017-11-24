// The MIT License (MIT)
//
// Copyright (c) 2014-2017 Darrell Wright
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <date/date.h>

#include <date/tz.h>
#include <daw/daw_string_view.h>

#include "daw_parse_template.h"

namespace daw {
	void parse_template::process_text( daw::string_view str ) {
		struct raw_text_func {
			daw::string_view str;

			std::string operator( )( ) const noexcept {
				return str.to_string( );
			}
		};
		m_doc_builder.emplace_back( raw_text_func{str} );
	}

	void parse_template::process_template( daw::string_view template_str ) {

		auto cur_pos = template_str.find( "<%" );
		process_text( template_str.substr( 0, cur_pos ) );
		template_str.remove_prefix( cur_pos );
		template_str.remove_prefix( 2 );
		while( !template_str.empty( ) ) {
			cur_pos = template_str.find( "%>" );

			daw::exception::daw_throw_on_false( 0 < cur_pos && cur_pos != template_str.npos, "Unexpected empty tag" );
			parse_tag( template_str.substr( 0, cur_pos ) );
			template_str.remove_prefix( cur_pos );
			template_str.remove_prefix( 2 );

			cur_pos = template_str.find( "<%" );
			process_text( template_str.substr( 0, cur_pos ) );
			template_str.remove_prefix( cur_pos );
			template_str.remove_prefix( 2 );
		}
	}

	void parse_template::parse_tag( daw::string_view tag ) {
		using namespace daw::string_view_literals;
		while( !tag.empty( ) && daw::parser::is_unicode_whitespace( tag.front( ) ) ) {
			tag.remove_prefix( );
		}
		daw::exception::daw_throw_on_true( tag.empty( ), "Empty tag found" );
		if( tag.starts_with( "call" ) ) {
			tag.remove_prefix( "call"_sv.size( ) );
			return process_call_tag( tag );
		}
		if( tag.starts_with( "date" ) ) {
			tag.remove_prefix( "date"_sv.size( ) );
			return process_date_tag( tag );
		}
		if( tag.starts_with( "time" ) ) {
			if( tag.starts_with( "timestamp" ) ) {
				tag.remove_prefix( "timestamp"_sv.size( ) );
				return process_timestamp_tag( tag );
			}
			tag.remove_prefix( ( "time"_sv.size( ) ) );
			return process_time_tag( tag );
		}
		daw::exception::daw_throw( "Unknown tag type" );
	}

	namespace {
		constexpr size_t find_quote( daw::string_view str ) noexcept {
			bool in_slash = false;
			for( size_t n = 0; n < str.size( ); ++n ) {
				if( str[n] == '\\' ) {
					in_slash = true;
					continue;
				} else {
					if( str[n] == '"' ) {
						if( !in_slash ) {
							return n;
						}
					}
					in_slash = false;
				}
			}
			return str.npos;
		}

		constexpr daw::string_view find_args( daw::string_view tag ) {
			using namespace daw::string_view_literals;
			tag.remove_prefix( tag.find( R"(args=")" ) );
			tag.remove_prefix( R"(args=")"_sv.size( ) );
			if( tag.empty( ) ) {
				return tag;
			}
			auto end_quote_pos = find_quote( tag );
			daw::exception::daw_throw_on_true( end_quote_pos == tag.npos, "Could not find end of call args" );
			tag.resize( end_quote_pos );
			return tag;
		}
	} // namespace
	void parse_template::process_call_tag( daw::string_view tag ) {
		using namespace daw::string_view_literals;
		tag = find_args( tag );
		daw::exception::daw_throw_on_true( tag.empty( ), "Could not find start of call args" );
		auto callable_name = tag.substr( 0, tag.find( "," ) );
		daw::exception::daw_throw_on_true( callable_name.empty( ), "Invalid call name, cannot be empty" );
		tag.remove_prefix( callable_name.size( ) + 1 );

		m_callbacks[callable_name.to_string( )] = nullptr;

		m_doc_builder.emplace_back( [callable_name, tag, this]( ) {
			auto cb = m_callbacks[callable_name.to_string( )];
			daw::exception::daw_throw_on_false( cb, "Attempt to call an undefined function" );
			return cb( tag );
		} );
	}

	void parse_template::process_date_tag( daw::string_view ) {
		m_doc_builder.emplace_back( []( ) {
			using namespace date;
			using namespace std::chrono;
			std::stringstream ss{};
			auto const current_time = date::make_zoned( date::current_zone( ), std::chrono::system_clock::now( ) );
			ss << date::format( "%Y-%m-%d", current_time );
			return ss.str( );
		} );
	}

	void parse_template::process_time_tag( daw::string_view ) {
		m_doc_builder.emplace_back( []( ) {
			using namespace date;
			using namespace std::chrono;
			std::stringstream ss{};
			auto const current_time =
			  date::make_zoned( date::current_zone( ), floor<seconds>( std::chrono::system_clock::now( ) ) );
			ss << date::format( "%T", current_time );
			return ss.str( );
		} );
	}

	void parse_template::process_timestamp_tag( daw::string_view str ) {
		str = find_args( str );
		m_doc_builder.emplace_back( [str]( ) {
			daw::string_view tag = str;
			char const default_ts_fmt[] = "%Y-%m-%dT%T";
			char const default_tz_fmt[] = "Z";
			daw::string_view ts_fmt = default_ts_fmt;
			daw::string_view tz_fmt = default_tz_fmt;
			if( !tag.empty( ) ) {
				auto comma_pos = tag.find( "," );
				ts_fmt = tag.substr( 0, comma_pos );
				tag.remove_prefix( comma_pos );
				tag.remove_prefix( 1 );
				if( !tag.empty( ) ) {
					tz_fmt = tag;
				} else {
					tz_fmt = daw::string_view{};
				}
			}
			using namespace date;
			using namespace std::chrono;
			std::stringstream ss{};
			std::string const fmt_str = ts_fmt.to_string( ) + tz_fmt.to_string( );
			auto const current_time =
			  date::make_zoned( date::current_zone( ), floor<seconds>( std::chrono::system_clock::now( ) ) );
			ss << date::format( fmt_str.c_str( ), current_time );
			return ss.str( );
		} );
	}

	std::string parse_template::to_string( ) {
		std::stringstream ss{};
		to_string( ss );
		return ss.str( );
	}

	std::string &impl::to_string( std::string &str ) noexcept {
		return str;
	}

	std::string impl::to_string( std::string &&str ) noexcept {
		return std::move( str );
	}

	std::string impl::doc_parts::operator( )( ) const {
		return m_to_string( );
	}

	namespace {
		constexpr char unescape( char c ) noexcept {
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
		}

		std::string trim_quotes( daw::string_view str ) {
			if( str.size( ) >= 2 && str.front( ) == '"' && str.back( ) == '"' ) {
				str.remove_prefix( );
				str.remove_suffix( );
			}
			return str.to_string( );
		}
	} // namespace

	std::string parse_to_value( daw::string_view str, escaped_string ) {
		std::string result{};
		bool in_escape = false;
		while( !str.empty( ) ) {
			if( in_escape ) {
				in_escape = false;
				result += unescape( str.front( ) );
			} else if( str.front( ) == '\\' ) {
				in_escape = true;
			} else {
				result += str.front( );
			}
			str.remove_prefix( );
		}
		return trim_quotes( result );
	}
} // namespace daw
