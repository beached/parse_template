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

#include <daw/daw_arith_traits.h>
#include <daw/daw_container_algorithm.h>
#include <daw/daw_move.h>
#include <daw/daw_parse_to.h>
#include <daw/daw_string_view.h>
#include <daw/daw_traits.h>
#include <daw/io/daw_type_writers.h>
#include <daw/io/daw_write_proxy.h>
#include <daw/iterator/daw_output_stream_iterator.h>

#include <date/date.h>
#include <date/tz.h>
#include <map>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace daw {
	enum class parse_template_error_types {
		none,
		callback_exception,
		empty_tag,
		eof,
		io_error,
		missing_dbl_quote,
		missing_tag,
		parser_exception,
		precondition_violation,
		unexpected_arg_count,
		unexpected_dbl_quote,
		unknown_function,
		unknown_tag,
	};
	struct escaped_string {
		explicit escaped_string( ) = default;
	};

	namespace parse_template_impl {
		std::string parse_to_value( daw::string_view str, daw::tag_t<escaped_string> );
		template<typename T>
		using detect_sv_conv = decltype( daw::basic_string_view( std::data( std::declval<T &>( ) ),
		                                                         std::size( std::declval<T &>( ) ) ) );

		template<typename T>
		using StringViewConvertible =
		  std::enable_if_t<daw::is_detected_v<detect_sv_conv, T>, std::nullptr_t>;

		template<typename ErrorHandler>
		std::vector<daw::string_view> find_split_args( ErrorHandler const &on_error,
		                                               daw::string_view tag ) {
			tag = find_args( on_error, tag );
			std::vector<daw::string_view> results{ };
			bool in_quote = false;
			bool is_escaped = false;
			size_t last_pos = 0;
			for( size_t n = 0; n < tag.size( ); ++n ) {
				switch( tag[n] ) {
				case '"':
					if( not is_escaped ) {
						on_error( parse_template_error_types::unexpected_dbl_quote,
						          tag,
						          "Unexpected unescaped double-quote" );
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
		constexpr auto make_to_string_func( ToStringFunc func ) {
			static_assert( std::is_invocable_v<ToStringFunc, daw::io::WriteProxy &> or
			                 std::is_invocable_v<ToStringFunc, daw::io::WriteProxy &, void *>,
			               "ToStringFunc must be callable with a WriteProxy argument func( writer )" );
			return [func = std::move( func )]( daw::io::WriteProxy &writer, void *state ) {
				if constexpr( std::is_invocable_v<ToStringFunc, daw::io::WriteProxy &> ) {
					func( writer );
				} else {
					func( writer, state );
				}
			};
		}

		class doc_parts {
			std::function<void( daw::io::WriteProxy &, void *state )> m_to_string;

		public:
			template<typename ToStringFunc>
			doc_parts( ToStringFunc to_string_func )
			  : m_to_string( make_to_string_func( std::move( to_string_func ) ) ) {}

			void operator( )( daw::io::WriteProxy &, void *state ) const;
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

		template<typename ErrorHandler, typename Callback, typename... Args>
		constexpr void write_to_output_state( ErrorHandler const &on_error,
		                                      Callback &callback,
		                                      daw::io::WriteProxy &writer,
		                                      void *state,
		                                      Args &&...args ) {
			auto wret = daw::io::IOOpResult{ };
			using result_t = std::invoke_result_t<Callback, Args..., void *>;
			try {
				if constexpr( daw::traits::is_string_view_like_v<result_t> ) {
					wret = writer.write( callback( DAW_FWD( args )..., state ) );
				} else if constexpr( daw::io::type_writer::has_type_writer_v<result_t> ) {
					wret = daw::io::type_writer::type_writer( writer, callback( DAW_FWD( args )..., state ) );
				} else {
					using parse_template_impl::to_string;
					using std::to_string;
					auto cb_res = callback( DAW_FWD( args )..., state );
					wret = writer.write( to_string( cb_res ) );
				}
			} catch( std::exception &ex ) {
				on_error( daw::parse_template_error_types::callback_exception, { }, ex.what( ) );
			} catch( ... ) {
				on_error( daw::parse_template_error_types::callback_exception,
				          { },
				          "Exception while calling callback" );
			}
			if( DAW_UNLIKELY( wret.status != daw::io::IOOpStatus::Ok ) ) {
				if( wret.status == io::IOOpStatus::Eof ) {
					on_error( daw::parse_template_error_types::io_error,
					          { },
					          "End of file while writing. " + std::to_string( wret.count ) +
					            " bytes written" );
				}
			}
		}

		template<typename ErrorHandler, typename Callback, typename... Args>
		constexpr void write_to_output_nostate( ErrorHandler const &on_error,
		                                        Callback &callback,
		                                        daw::io::WriteProxy &writer,
		                                        Args &&...args ) {
			auto wret = daw::io::IOOpResult{ };
			using result_t = std::invoke_result_t<Callback, Args...>;
			try {
				if constexpr( daw::traits::is_string_view_like_v<result_t> ) {
					wret = writer.write( callback( DAW_FWD( args )... ) );
				} else if constexpr( daw::io::type_writer::has_type_writer_v<result_t> ) {
					wret = daw::io::type_writer::type_writer( writer, callback( DAW_FWD( args )... ) );
				} else {
					using parse_template_impl::to_string;
					using std::to_string;
					auto cb_res = callback( DAW_FWD( args )... );
					wret = writer.write( to_string( cb_res ) );
				}
			} catch( std::exception &ex ) {
				on_error( daw::parse_template_error_types::callback_exception, { }, ex.what( ) );
			} catch( ... ) {
				on_error( daw::parse_template_error_types::callback_exception,
				          { },
				          "Exception while calling callback" );
			}
			if( DAW_UNLIKELY( wret.status != daw::io::IOOpStatus::Ok ) ) {
				if( wret.status == io::IOOpStatus::Eof ) {
					on_error( daw::parse_template_error_types::io_error,
					          { },
					          "End of file while writing. " + std::to_string( wret.count ) +
					            " bytes written" );
				}
			}
		}

		template<typename... ArgTypes, typename ErrorHandler, typename Callback>
		DAW_ATTRIB_FLATINLINE constexpr auto make_callback( ErrorHandler const &on_error,
		                                                    Callback &callback,
		                                                    daw::io::WriteProxy &writer,
		                                                    void *state ) {
			if constexpr( std::is_invocable_v<Callback,
			                                  parse_template_impl::actual_type_t<ArgTypes>...,
			                                  daw::io::WriteProxy &,
			                                  void *> ) {
				return [&, state]( parse_template_impl::actual_type_t<ArgTypes>... args ) mutable {
					(void)callback( DAW_FWD( args )..., writer, state );
				};
			} else if constexpr( std::is_invocable_v<Callback,
			                                         parse_template_impl::actual_type_t<ArgTypes>...,
			                                         daw::io::WriteProxy &> ) {
				return [&]( parse_template_impl::actual_type_t<ArgTypes>... args ) mutable {
					(void)callback( DAW_FWD( args )..., writer );
				};
			} else if constexpr( std::is_invocable_v<Callback,
			                                         parse_template_impl::actual_type_t<ArgTypes>...,
			                                         void *> ) {
				return [&, state]( parse_template_impl::actual_type_t<ArgTypes>... args ) mutable {
					write_to_output_state( on_error, callback, writer, state, DAW_FWD( args )... );
				};
			} else {
				static_assert(
				  std::is_invocable_v<Callback, parse_template_impl::actual_type_t<ArgTypes>...>,
				  "Unsupported callback" );
				return [&]( parse_template_impl::actual_type_t<ArgTypes>... args ) mutable {
					write_to_output_nostate( on_error, callback, writer, DAW_FWD( args )... );
				};
			}
		}

		template<typename StateType, typename... ArgTypes, typename ErrorHandler, typename Callback>
		DAW_ATTRIB_FLATINLINE constexpr auto make_stateful_callback( ErrorHandler const &on_error,
		                                                             Callback &&callback ) {
			using state_t = std::remove_reference_t<StateType>;
			if constexpr( std::is_invocable_v<Callback,
			                                  actual_type_t<ArgTypes>...,
			                                  daw::io::WriteProxy &,
			                                  state_t &> ) {
				return [&, callback = DAW_FWD( callback )]( auto &&...args,
				                                            daw::io::WriteProxy &writer,
				                                            void *state ) mutable {
					if( not state ) {
						on_error( parse_template_error_types::unknown_tag,
						          daw::string_view{ },
						          "Stateful function expects state param on write_to/to_string call" );
						std::terminate( );
					}
					return callback( DAW_FWD( args )..., writer, *reinterpret_cast<state_t *>( state ) );
				};
			} else {
				static_assert( std::is_invocable_v<Callback, actual_type_t<ArgTypes>..., state_t &> );
				return [&, callback = DAW_FWD( callback )]( auto &&...args, void *state ) mutable {
					if( not state ) {
						on_error( parse_template_error_types::unknown_tag,
						          daw::string_view{ },
						          "Stateful function expects state param on write_to/to_string call" );
						std::terminate( );
					}
					return callback( DAW_FWD( args )..., *reinterpret_cast<state_t *>( state ) );
				};
			}
		}

		DAW_ATTRIB_INLINE constexpr void remove_leading_whitespace( daw::string_view &sv ) noexcept {
			sv.remove_prefix_while( []( char c ) { return daw::parser::is_unicode_whitespace( c ); } );
		}

		struct default_error_handler_t {
			explicit default_error_handler_t( ) = default;

			[[noreturn]] void operator( )( parse_template_error_types type,
			                               daw::string_view /*data*/,
			                               daw::string_view error_message ) const {
				if( std::uncaught_exceptions( ) > 0 ) {
					throw;
				}
				throw std::runtime_error( static_cast<std::string>( error_message ) );
			}
		};

		template<typename Key, typename T, typename Allocator = std::allocator<std::pair<Key const, T>>>
		using heterogenous_lookup_map_t = std::map<Key, T, std::less<>, Allocator>;

		template<typename ErrorHandler>
		struct raw_text_func {
			daw::string_view str;
			ErrorHandler const *on_error;

			explicit constexpr raw_text_func( ErrorHandler const &error_handler, daw::string_view sv )
			  : str( sv )
			  , on_error( &error_handler ) {}

			template<typename Writer>
			constexpr void operator( )( Writer &writer ) const {
				auto ret = writer.write( str );
				if( ret.status != io::IOOpStatus::Ok ) {
					( *on_error )( parse_template_error_types::io_error, str, "Error writing to output" );
				}
			}
		};
		template<typename ErrorHandler>
		raw_text_func( ErrorHandler const &, daw::string_view ) -> raw_text_func<ErrorHandler>;

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

		template<typename ErrorHandler>
		daw::string_view find_args( ErrorHandler const &on_error, daw::string_view tag ) {
			using namespace daw::string_view_literals;
			tag.remove_prefix_until( R"(args=")" );
			if( tag.empty( ) ) {
				return tag;
			}
			auto end_quote_pos = find_quote( tag );
			if( end_quote_pos == daw::string_view::npos ) {
				on_error( parse_template_error_types::missing_dbl_quote,
				          tag,
				          "Could not find end of call args" );
			}
			tag.resize( end_quote_pos );
			return tag;
		}

		template<typename ErrorHandler>
		class date_tag_func {
			date::time_zone const *tz;
			ErrorHandler const *on_error;

		public:
			explicit constexpr date_tag_func( ErrorHandler const &eh,
			                                  date::time_zone const *timezone ) noexcept
			  : tz( timezone )
			  , on_error( &eh ) {}

			template<typename Writer>
			void operator( )( Writer &writer ) const {
				using namespace date;
				using namespace std::chrono;
				std::stringstream ss{ };
				auto const current_time = date::make_zoned( tz, std::chrono::system_clock::now( ) );
				ss << date::format( "%Y-%m-%d", current_time );
				auto ret = writer.write( ss.str( ) );
				if( ret.status != io::IOOpStatus::Ok ) {
					( *on_error )( parse_template_error_types::io_error,
					               ss.str( ),
					               "Error writing to output" );
				}
			}
		};
		template<typename ErrorHandler>
		date_tag_func( ErrorHandler const &, date::time_zone const * ) -> date_tag_func<ErrorHandler>;

		template<typename ErrorHandler>
		class ErrorWrapper {
			static_assert( std::is_invocable_v<ErrorHandler,
			                                   parse_template_error_types,
			                                   daw::string_view,
			                                   daw::string_view>,
			               "ErrorHandler does not work with required parameters" );

			DAW_NO_UNIQUE_ADDRESS ErrorHandler m_on_error{ };

		public:
			explicit ErrorWrapper( ) = default;
			constexpr ErrorWrapper( ErrorHandler eh )
			  : m_on_error( std::move( eh ) ) {}

			DAW_ATTRIB_NOINLINE DAW_ATTRIB_FLATTEN [[noreturn]] constexpr void
			operator( )( parse_template_error_types type,
			             daw::string_view data,
			             daw::string_view message ) const {
				(void)m_on_error( type, data, message );
				std::terminate( );
			}
		};
	} // namespace parse_template_impl
	  //*****************************************************************

	template<typename ErrorHandler = parse_template_impl::default_error_handler_t>
	class parse_template {
		using callback_map_t = parse_template_impl::heterogenous_lookup_map_t<
		  std::string,
		  std::function<void( daw::string_view, daw::io::WriteProxy &, void * )>>;

		DAW_NO_UNIQUE_ADDRESS parse_template_impl::ErrorWrapper<ErrorHandler> m_on_error{ };
		std::vector<parse_template_impl::doc_parts> m_doc_builder{ };
		callback_map_t m_callbacks{ };

	public:
		explicit parse_template( daw::string_view template_string ) {
			process_template( template_string );
		}

		explicit parse_template( daw::string_view template_string, ErrorHandler on_error )
		  : m_on_error( std::move( on_error ) ) {

			process_template( template_string );
		}

		template<typename Writable>
		void write_to( Writable &wr ) {
			return write_to( daw::io::WriteProxy( wr ) );
		}

		template<typename Writable, typename T>
		void write_to( Writable &wr, T &&state ) {
			return write_to( daw::io::WriteProxy( wr ), state );
		}

		std::string to_string( ) {
			auto result = std::string( );
			write_to( daw::io::WriteProxy( result ) );
			return result;
		}

		template<typename T>
		std::string to_string( T &state ) {
			auto result = std::string( );
			write_to( daw::io::WriteProxy( result ), state );
			return result;
		}

		inline void write_to( daw::io::WriteProxy &&writable ) {
			write_to_impl( writable, nullptr );
		}

		inline void write_to( daw::io::WriteProxy &writable ) {
			write_to_impl( writable, nullptr );
		}

		template<typename T>
		inline void write_to( daw::io::WriteProxy &writable, T &state ) {
			static_assert( not std::is_const_v<T>, "Only mutable state is supported" );
			void *state_ptr = reinterpret_cast<void *>( std::addressof( state ) );
			write_to_impl( writable, state_ptr );
		}

		template<typename T>
		inline void write_to( daw::io::WriteProxy &&writable, T &state ) {
			write_to( writable, state );
		}

		template<typename... Args, typename Callback, typename Splitter>
		constexpr decltype( auto )
		apply_from_string( Callback &cb, daw::string_view sv, Splitter &&sp ) {
			auto parse_value = [&]( daw::string_view &sv, auto Tag ) {
				auto part = sv.pop_front_until( sp );
				using daw::parser::converters::parse_to_value;
				using parse_template_impl::parse_to_value;
				return parse_to_value( part, Tag );
			};
			using tp_t = DAW_TYPEOF( std::forward_as_tuple( parse_value( sv, daw::tag<Args> )... ) );
			// Order matters here, must be left to right as the string_view is state
			return std::apply( cb, tp_t{ parse_value( sv, daw::tag<Args> )... } );
		}

		template<typename... ArgTypes, typename Callback>
		void add_callback( daw::string_view name, Callback &&callback ) {
			m_callbacks.insert_or_assign(
			  static_cast<std::string>( name ),
			  [&, callback = DAW_FWD( callback )]( daw::string_view str,
			                                       daw::io::WriteProxy &writer,
			                                       void *state ) mutable {
				  auto cb =
				    parse_template_impl::make_callback<ArgTypes...>( m_on_error, callback, writer, state );
				  try {
					  apply_from_string<ArgTypes...>( cb, str, ',' );
				  } catch( std::exception const &ex ) {
					  m_on_error( parse_template_error_types::parser_exception, str, ex.what( ) );
				  } catch( ... ) {
					  m_on_error( parse_template_error_types::parser_exception,
					              str,
					              "Exception while parsing" );
				  }
			  } );
		}

		template<typename StateType, typename... ArgTypes, typename Callback>
		void add_stateful_callback( daw::string_view name, Callback &&callback ) {
			static_assert( not std::is_const_v<std::remove_reference_t<StateType>>,
			               "Only mutable state is supported" );
			return add_callback<ArgTypes...>(
			  name,
			  parse_template_impl::make_stateful_callback<StateType, ArgTypes...>(
			    m_on_error,
			    DAW_FWD( callback ) ) );
		}

		void process_timestamp_tag( string_view tag ) {
			static char const default_ts_fmt[] = "%Y-%m-%dT%T%z";
			auto args = parse_template_impl::find_split_args( m_on_error, tag );
			if( args.size( ) > 2 ) {
				m_on_error( parse_template_error_types::unexpected_arg_count,
				            tag,
				            "Unexpected argument count" );
			}

			auto const ts_fmt = static_cast<std::string>( [&]( ) {
				if( args.empty( ) or args[0].empty( ) ) {
					return daw::string_view{ default_ts_fmt };
				}
				return args[0];
			}( ) );

			auto const tz = [&]( ) {
				if( args.size( ) < 2 or args[1].empty( ) ) {
					return date::current_zone( );
				}
				return date::locate_zone( static_cast<std::string_view>( args[1] ) );
			}( );

			m_doc_builder.emplace_back( [&, ts_fmt, tz]( daw::io::WriteProxy &writer ) {
				using namespace date;
				using namespace std::chrono;

				std::stringstream ss{ };

				auto current_time = make_zoned( tz, floor<seconds>( std::chrono::system_clock::now( ) ) );
				ss << format( ts_fmt, current_time );

				auto ret = writer.write( ss.str( ) );

				if( ret.status != io::IOOpStatus::Ok ) {
					m_on_error( parse_template_error_types::io_error, ss.str( ), "Error writing to output" );
				}
			} );
		}

	private:
		void process_template( daw::string_view template_str ) {
			process_text( template_str.pop_front_until( "<%" ) );
			while( not template_str.empty( ) ) {
				auto item = template_str.pop_front_until( "%>" );
				if( template_str.empty( ) ) {
					m_on_error( parse_template_error_types::empty_tag, template_str, "Unexpected empty tag" );
				}
				parse_tag( item );
				process_text( template_str.pop_front_until( "<%" ) );
			}
		}

		void parse_tag( daw::string_view tag ) {
			using namespace daw::string_view_literals;

			parse_template_impl::remove_leading_whitespace( tag );

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
			if( tag.empty( ) ) {
				m_on_error( parse_template_error_types::missing_tag, tag, "Empty tag" );
			}
		}

		void process_call_tag( daw::string_view tag ) {
			using namespace daw::string_view_literals;
			tag = parse_template_impl::find_args( m_on_error, tag );
			if( tag.empty( ) ) {
				m_on_error( parse_template_error_types::missing_tag,
				            tag.data( ),
				            "Could not find start of call args" );
			}
			auto callable_name = tag.pop_front_until( "," );
			if( callable_name.empty( ) ) {
				m_on_error( parse_template_error_types::missing_tag,
				            callable_name.data( ),
				            "Invalid call name, cannot be empty" );
			}

			// This ensures that a lookup will always success when processing but the callback may not be
			// set
			m_callbacks[static_cast<std::string>( callable_name )] = nullptr;

			m_doc_builder.emplace_back(
			  [callable_name, tag, this]( daw::io::WriteProxy &writer, void *state ) {
				  auto &cb = m_callbacks.find( callable_name )->second;
				  if( not cb ) {
					  m_on_error( parse_template_error_types::unknown_function,
					              callable_name.data( ),
					              "Attempt to call an undefined function" );
				  }
				  cb( tag, writer, state );
			  } );
		}

		void process_date_tag( daw::string_view str ) {
			auto args = parse_template_impl::find_split_args( m_on_error, str );
			if( args.size( ) > 1 ) {
				m_on_error( parse_template_error_types::unexpected_arg_count,
				            str,
				            "Unexpected argument count" );
			}
			auto const tz = [&]( ) {
				if( args.empty( ) or args[0].empty( ) ) {
					return date::current_zone( );
				}
				return date::locate_zone( static_cast<std::string>( args[0] ).c_str( ) );
			}( );
			m_doc_builder.emplace_back( parse_template_impl::date_tag_func( m_on_error, tz ) );
		}

		void process_time_tag( daw::string_view str ) {
			auto args = parse_template_impl::find_split_args( m_on_error, str );
			if( args.size( ) > 1 ) {
				m_on_error( parse_template_error_types::unexpected_arg_count,
				            str,
				            "Unexpected argument count" );
			}
			auto const tz = [&]( ) {
				if( args.empty( ) or args[0].empty( ) ) {
					return date::current_zone( );
				}
				return date::locate_zone( static_cast<std::string>( args[0] ).c_str( ) );
			}( );
			m_doc_builder.emplace_back( [&, tz]( daw::io::WriteProxy &writer ) {
				using namespace date;
				using namespace std::chrono;
				std::stringstream ss{ };
				auto const current_time =
				  date::make_zoned( tz, floor<seconds>( std::chrono::system_clock::now( ) ) );
				ss << date::format( "%T", current_time );
				auto ret = writer.write( ss.str( ) );
				if( ret.status != io::IOOpStatus::Ok ) {
					m_on_error( parse_template_error_types::io_error, ss.str( ), "Error writing to output" );
				}
			} );
		}

		void process_text( daw::string_view str ) {
			m_doc_builder.emplace_back( parse_template_impl::raw_text_func( m_on_error, str ) );
		}

		void write_to_impl( daw::io::WriteProxy &writable, void *state ) {
			for( auto const &part : m_doc_builder ) {
				part( writable, state );
			}
		}
	}; // class parse_template

} // namespace daw
