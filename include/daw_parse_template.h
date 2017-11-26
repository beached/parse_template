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

#pragma once

#include <unordered_map>
#include <vector>

#include <daw/daw_container_algorithm.h>
#include <daw/daw_output_stream_iterator.h>
#include <daw/daw_parse_to.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>

namespace daw {
	struct escaped_string {};
	std::string parse_to_value( daw::string_view str, escaped_string );

	namespace impl {

		std::string &to_string( std::string &str ) noexcept;

		std::string to_string( std::string &&str ) noexcept;

		template<typename ToStringFunc>
		std::function<std::string( )> make_to_string_func( ToStringFunc func ) {
			return [func = std::move( func )]( ) {
				using daw::impl::to_string;
				using std::to_string;
				return to_string( func( ) );
			};
		}

		class doc_parts {
			std::function<std::string( )> m_to_string;

		public:
			doc_parts( ) = delete;
			~doc_parts( ) = default;
			doc_parts( doc_parts const & ) = default;
			doc_parts( doc_parts && ) noexcept = default;
			doc_parts &operator=( doc_parts const & ) = default;
			doc_parts &operator=( doc_parts && ) noexcept = default;

			template<typename ToStringFunc>
			doc_parts( ToStringFunc to_string_func )
			  : m_to_string{make_to_string_func( std::move( to_string_func ) )} {}

			std::string operator( )( ) const;
		};
	} // namespace impl

	class parse_template {
		std::vector<impl::doc_parts> m_doc_builder;
		std::unordered_map<std::string, std::function<std::string( daw::string_view )>> m_callbacks;

		void process_template( daw::string_view template_str );
		void parse_tag( daw::string_view tag );
		void process_call_tag( daw::string_view tag );
		void process_date_tag( daw::string_view tag );
		void process_time_tag( daw::string_view tag );
		void process_text( string_view str );

	public:
		parse_template( ) = default;
		~parse_template( ) = default;
		parse_template( parse_template const & ) = default;
		parse_template( parse_template && ) noexcept = default;
		parse_template &operator=( parse_template const & ) = default;
		parse_template &operator=( parse_template && ) noexcept = default;

		template<typename StringRange, std::enable_if_t<(daw::traits::is_container_like_v<StringRange> &&
		                                                 daw::traits::is_value_size_equal_v<StringRange, 1>),
		                                                std::nullptr_t> = nullptr>
		parse_template( StringRange const &template_string )
		  : m_doc_builder{}
		  , m_callbacks{} {

			process_template( daw::make_string_view_it( std::cbegin( template_string ), std::cend( template_string ) ) );
		}

		template<typename Stream>
		void to_string( Stream &strm ) {
			daw::container::transform( m_doc_builder, daw::make_output_stream_iterator( strm ),
			                           []( auto const &d ) { return d( ); } );
		}

		std::string to_string( );

		template<typename... ArgTypes, typename Callback>
		void add_callback( daw::string_view name, Callback callback ) {
			m_callbacks[name.to_string( )] = [callback = std::move( callback )]( daw::string_view str ) {
				using daw::impl::to_string;
				using std::to_string;
				auto result = daw::apply_string2<ArgTypes...>( callback, str, "," );
				return to_string( std::move( result ) );
			};
		}

		void process_timestamp_tag( string_view tag );
	}; // class parse_template
} // namespace daw
