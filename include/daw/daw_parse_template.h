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

#pragma once

#include "parse_template_impl/daw_function_traits.h"

#include <daw/daw_container_algorithm.h>
#include <daw/daw_move.h>
#include <daw/daw_parse_to.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>
#include <daw/io/daw_write_proxy.h>
#include <daw/iterator/daw_output_stream_iterator.h>

#include <string>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace daw {
	struct escaped_string {
		explicit escaped_string( ) = default;
	};
	std::string parse_to_value( daw::string_view str, daw::tag_t<escaped_string> );

	namespace parse_template_impl {
		template<typename T>
		using detect_sv_conv = decltype( daw::basic_string_view( std::data( std::declval<T &>( ) ),
		                                                         std::size( std::declval<T &>( ) ) ) );

		template<typename T>
		using StringViewConvertible =
		  std::enable_if_t<daw::is_detected_v<detect_sv_conv, T>, std::nullptr_t>;
	} // namespace parse_template_impl
	namespace impl {
		template<typename T>
		struct actual_type {
			using type = T;
		};

		template<>
		struct actual_type<escaped_string> {
			using type = std::string;
		};

		template<typename T>
		using actual_type_t = typename actual_type<T>::type;

		std::string &to_string( std::string &str ) noexcept;

		std::string to_string( std::string &&str ) noexcept;

		template<typename ToStringFunc>
		std::function<void( daw::io::WriteProxy & )> make_to_string_func( ToStringFunc func ) {
			static_assert( std::is_invocable_v<ToStringFunc, daw::io::WriteProxy &>,
			               "ToStringFunc must be callable with a WriteProxy argument func( writer )" );
			return [func = std::move( func )]( daw::io::WriteProxy &writer ) { func( writer ); };
		}

		class doc_parts {
			std::function<void( daw::io::WriteProxy & )> m_to_string;

		public:
			template<typename ToStringFunc>
			doc_parts( ToStringFunc to_string_func )
			  : m_to_string( make_to_string_func( std::move( to_string_func ) ) ) {}

			void operator( )( daw::io::WriteProxy & ) const;
		};

		template<typename Container>
		using detect_is_range = decltype( std::distance( std::cbegin( std::declval<Container>( ) ),
		                                                 std::cend( std::declval<Container>( ) ) ) );

		template<typename Container>
		constexpr bool is_range_v = daw::is_detected_v<detect_is_range, Container>;

		template<typename StringRange,
		         std::enable_if_t<is_range_v<StringRange>, std::nullptr_t> = nullptr>
		constexpr size_t value_size_test( ) noexcept {
			return sizeof( *std::cbegin( std::declval<StringRange>( ) ) );
		}

		template<typename StringRange,
		         std::enable_if_t<!is_range_v<StringRange>, std::nullptr_t> = nullptr>
		constexpr size_t value_size_test( ) noexcept {
			return 0;
		}

		template<typename StringRange>
		constexpr size_t value_size_v = value_size_test<StringRange>( );

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
		explicit parse_template( daw::string_view template_string );

		template<typename Writable>
		void to_string( Writable &&wr ) {
			return write_to( daw::io::WriteProxy( wr ) );
		}

		std::string to_string( );
		void write_to( daw::io::WriteProxy &writable );

		inline void write_to( daw::io::WriteProxy &&writable ) {
			write_to( writable );
		}

		template<typename... ArgTypes, typename Callback>
		void add_callback( daw::string_view name, Callback callback ) {
			m_callbacks.insert_or_assign(
			  static_cast<std::string>( name ),
			  [callback = std::move( callback )]( daw::string_view str ) mutable {
				  if constexpr( std::is_same_v<std::string, std::invoke_result<Callback, ArgTypes...>> ) {
					  return daw::apply_string<ArgTypes...>( callback, str, "," );
				  } else {
					  using daw::impl::to_string;
					  using std::to_string;
					  return to_string( daw::apply_string<ArgTypes...>( callback, str, "," ) );
				  }
			  } );
		}

		void process_timestamp_tag( string_view tag );
	}; // class parse_template
} // namespace daw
