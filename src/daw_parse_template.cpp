// The MIT License (MIT)
//
// Copyright (c) 2014-2018 Darrell Wright
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

#include <date/date.h>

#include <date/tz.h>
#include <daw/daw_move.h>
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

		process_text( template_str.pop_front( "<%" ) );
		while( !template_str.empty( ) ) {
			auto item = template_str.pop_front( "%>" );
			daw::exception::Assert( !template_str.empty( ), "Unexpected empty tag" );

			parse_tag( item );
			process_text( template_str.pop_front( "<%" ) );
		}
	}

	namespace {
		constexpr void remove_leading_whitespace( daw::string_view &sv ) noexcept {
			size_t count = 0;
			while( !sv.empty( ) && daw::parser::is_unicode_whitespace( sv[count] ) ) {
				++count;
			}
			sv.remove_prefix( count );
		}
	} // namespace
	void parse_template::parse_tag( daw::string_view tag ) {
		using namespace daw::string_view_literals;

		remove_leading_whitespace( tag );

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
		daw::exception::daw_throw( "Unknown or empty tag type" );
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
			tag.pop_front( R"(args=")" );
			if( tag.empty( ) ) {
				return tag;
			}
			auto end_quote_pos = find_quote( tag );
			daw::exception::daw_throw_on_true( end_quote_pos == tag.npos,
			                                   "Could not find end of call args" );
			tag.resize( end_quote_pos );
			return tag;
		}

		std::vector<daw::string_view> find_split_args( daw::string_view tag ) {
			tag = find_args( tag );
			std::vector<daw::string_view> results{};
			bool in_quote = false;
			bool is_escaped = false;
			size_t last_pos = 0;
			for( size_t n = 0; n < tag.size( ); ++n ) {
				switch( tag[n] ) {
				case '"':
					daw::exception::daw_throw_on_true(
					  !is_escaped, "Unexpected unescaped double-quote" );
					in_quote = !in_quote;
					continue;
				case '\\':
					is_escaped = !is_escaped;
					continue;
				case ',':
					if( !in_quote ) {
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
	} // namespace
	void parse_template::process_call_tag( daw::string_view tag ) {
		using namespace daw::string_view_literals;
		tag = find_args( tag );
		daw::exception::daw_throw_on_true( tag.empty( ),
		                                   "Could not find start of call args" );
		auto callable_name = tag.pop_front( "," );
		daw::exception::daw_throw_on_true( callable_name.empty( ),
		                                   "Invalid call name, cannot be empty" );

		m_callbacks[callable_name.to_string( )] = nullptr;

		m_doc_builder.emplace_back( [callable_name, tag, this]( ) {
			auto cb = m_callbacks[callable_name.to_string( )];
			daw::exception::daw_throw_on_false(
			  cb, "Attempt to call an undefined function" );
			return cb( tag );
		} );
	}

	void parse_template::process_date_tag( daw::string_view str ) {
		auto args = find_split_args( str );
		daw::exception::daw_throw_on_false( args.size( ) <= 1,
		                                    "Unexpected argument count" );
		auto const tz = [&]( ) {
			if( args.empty( ) || args[0].empty( ) ) {
				return date::current_zone( );
			}
			return date::locate_zone( args[0].to_string( ).c_str( ) );
		}( );

		m_doc_builder.emplace_back( [tz]( ) {
			using namespace date;
			using namespace std::chrono;
			std::stringstream ss{};
			auto const current_time =
			  date::make_zoned( tz, std::chrono::system_clock::now( ) );
			ss << date::format( "%Y-%m-%d", current_time );
			return ss.str( );
		} );
	}

	void parse_template::process_time_tag( daw::string_view str ) {
		auto args = find_split_args( str );
		daw::exception::daw_throw_on_false( args.size( ) <= 1,
		                                    "Unexpected argument count" );
		auto const tz = [&]( ) {
			if( args.empty( ) || args[0].empty( ) ) {
				return date::current_zone( );
			}
			return date::locate_zone( args[0].to_string( ).c_str( ) );
		}( );
		m_doc_builder.emplace_back( [tz]( ) {
			using namespace date;
			using namespace std::chrono;
			std::stringstream ss{};
			auto const current_time = date::make_zoned(
			  tz, floor<seconds>( std::chrono::system_clock::now( ) ) );
			ss << date::format( "%T", current_time );
			return ss.str( );
		} );
	}

	void parse_template::process_timestamp_tag( daw::string_view str ) {
		static char const default_ts_fmt[] = "%Y-%m-%dT%T%z";
		auto args = find_split_args( str );
		daw::exception::daw_throw_on_false( args.size( ) <= 2,
		                                    "Unexpected argument count" );

		std::string const ts_fmt =
		  [&]( ) {
			  if( args.empty( ) || args[0].empty( ) ) {
				  return daw::string_view{default_ts_fmt};
			  }
			  return args[0];
		  }( )
		    .to_string( );

		auto const tz = [&]( ) {
			if( args.size( ) < 2 || args[1].empty( ) ) {
				return date::current_zone( );
			}
			return date::locate_zone( args[1].to_string( ).c_str( ) );
		}( );

		m_doc_builder.emplace_back( [ts_fmt, tz]( ) {
			using namespace date;
			using namespace std::chrono;

			std::stringstream ss{};

			auto current_time =
			  make_zoned( tz, floor<seconds>( std::chrono::system_clock::now( ) ) );
			ss << format( ts_fmt, current_time );

			return ss.str( );
		} );
	}

	std::string parse_template::to_string( ) {
		std::stringstream ss{};
		to_string( ss );
		return ss.str( );
	}

	parse_template::parse_template( daw::string_view template_string )
	  : m_doc_builder( )
	  , m_callbacks( ) {

		process_template( template_string );
	}

	std::string &impl::to_string( std::string &str ) noexcept {
		return str;
	}

	std::string impl::to_string( std::string &&str ) noexcept {
		return daw::move( str );
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

	std::string parse_to_value( daw::string_view str, daw::tag_t<escaped_string> ) {
		auto result = std::string( );
		bool in_escape = false;

		while( !str.empty( ) ) {
			auto const item = str.pop_front( );
			if( in_escape ) {
				in_escape = false;
				result += daw::unescape( item );
			} else if( item == '\\' ) {
				in_escape = true;
			} else {
				result += item;
			}
		}
		return daw::trim_quotes( result );
	}

} // namespace daw
