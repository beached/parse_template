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

#pragma once

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

#include <daw/daw_container_algorithm.h>
#include <daw/daw_parse_to.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>
#include <daw/iterator/daw_output_stream_iterator.h>

namespace daw {
	struct escaped_string {};
	std::string parse_to_value( daw::string_view str, daw::tag<escaped_string> );

	namespace impl {

		std::string &to_string( std::string &str ) noexcept;

		std::string to_string( std::string &&str ) noexcept;

		template<typename ToStringFunc>
		std::function<std::string( )> make_to_string_func( ToStringFunc func ) {
			static_assert(
			  std::is_invocable_v<ToStringFunc>,
			  "ToStringFunc must be callable without arguments func( )" );
			return [func = std::move( func )]( ) {
				using daw::impl::to_string;
				using std::to_string;
				return to_string( func( ) );
			};
		}

		class doc_parts {
			std::function<std::string( )> m_to_string;

		public:
			template<typename ToStringFunc>
			doc_parts( ToStringFunc to_string_func )
			  : m_to_string( make_to_string_func( std::move( to_string_func ) ) ) {}

			std::string operator( )( ) const;
		};

		template<typename Container>
		using detect_is_range =
		  decltype( std::distance( std::cbegin( std::declval<Container>( ) ),
		                           std::cend( std::declval<Container>( ) ) ) );

		template<typename Container>
		constexpr bool is_range_v = daw::is_detected_v<detect_is_range, Container>;

		template<typename StringRange, std::enable_if_t<is_range_v<StringRange>,
		                                                std::nullptr_t> = nullptr>
		constexpr size_t value_size_test( ) noexcept {
			return sizeof( *std::cbegin( std::declval<StringRange>( ) ) );
		}

		template<typename StringRange, std::enable_if_t<!is_range_v<StringRange>,
		                                                std::nullptr_t> = nullptr>
		constexpr size_t value_size_test( ) noexcept {
			return 0;
		}

		template<typename StringRange>
		constexpr size_t value_size_v = value_size_test<StringRange>( );

	} // namespace impl

	class parse_template {
		std::vector<impl::doc_parts> m_doc_builder;
		std::unordered_map<std::string,
		                   std::function<std::string( daw::string_view )>>
		  m_callbacks;

		void process_template( daw::string_view template_str );
		void parse_tag( daw::string_view tag );
		void process_call_tag( daw::string_view tag );
		void process_date_tag( daw::string_view tag );
		void process_time_tag( daw::string_view tag );
		void process_text( string_view str );

	public:
		parse_template( daw::string_view template_string );

		template<typename StringRange,
		         std::enable_if_t<( impl::is_range_v<StringRange> &&
		                            impl::value_size_v<StringRange> == 1 ),
		                          std::nullptr_t> = nullptr>
		parse_template( StringRange const &rng )
		  : parse_template(
		      daw::make_string_view_it( std::cbegin( rng ), std::cend( rng ) ) ) {}

		template<typename Stream>
		void to_string( Stream &strm ) {
			daw::container::transform( m_doc_builder,
			                           daw::make_output_stream_iterator( strm ),
			                           []( auto const &d ) { return d( ); } );
		}

		std::string to_string( );

		template<typename... ArgTypes, typename Callback>
		void add_callback( daw::string_view name, Callback callback ) {
			m_callbacks[name.to_string( )] =
			  [callback = std::move( callback )]( daw::string_view str ) mutable {
				  using daw::impl::to_string;
				  using std::to_string;

				  return to_string(
				    daw::apply_string<ArgTypes...>( callback, str, "," ) );
			  };
		}

		void process_timestamp_tag( string_view tag );
	}; // class parse_template
} // namespace daw
