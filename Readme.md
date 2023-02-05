# Template Tags

A library that accepts a UTF-8 text template and generates text(e.g. html)

--------------

| Tag                                            | Description                                                                                                                                                                                                                                                                                                                          |
|------------------------------------------------|--------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| <%call args="callback_name,callback_args..."%> | insert text from callback                                                                                                                                                                                                                                                                                                            |
| <%timestamp args="fmt,tz"%>                    | See [date_formatting.md](date_formatting.md) for fmt, timezone name from IANA database e.g. America/NewYork. The default fmt is "%Y-%m-%dT%T%z", the ISO9601 timestamp format, and tz="" the system's timezone.  See the [wiki](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones) article for listing of time zone names |
| <%date args="tz"%>                             | insert current date with timezone tz. The default timezone is the current system's.  Same as timestamp with fmt="%Y-%m-%d" and tz                                                                                                                                                                                                    |
| <%time args="tz"%>                             | insert current time with timezone tz.  The default timezone is the current system's. Same as timestamp with fmt="%T" and tz                                                                                                                                                                                                          |

### Note:

Do not put spaces between comma separates currently

# Code Usage

The constructor for parse_template takes any container that is string like(e.g. std::string, string_view..). This allows for things like memory mapped files.

``` C++
std::string str = ...;
auto tmp = parse_template{ str };
```

Adding callbacks requires one to specify how the arguments are to be parsed. Often this is just specifying the types of the arguments, but tag types can be used as any type specified must have a ``` std::string parse_to_value( daw::string_view, type_tag ) ``` method

``` C++
tmp.add_callback<int,int,bool>( "callback_name", []( int a, int b, bool c ) {
	if( c ) {
		a += b;
	}
	return a;
} );
```

The previous example adds a callback that takes the arguments of int, int, and bool and returns an int. The result must be a string or have a ``` to_string ``` overload. To use the example in a template you would call it like the following:

``` html
<%call args="callback_name,5,5,true"%><br>
<%call args="callback_name,5,5,false"%>
```

One can output to any Writable type, see [daw-read-write](https://github.com/beached/daw_read_write), such as strings, streams, and FILE *.

```cpp
tmp.to_string( std::cout );
```

If the text output was html, the output would be

```
10
5
```

## Callbacks

Callbacks are specified with the parameters the template will use and needs to be invokable with them, but one can optionally add a `daw::io::WriteProxy &` param and/or a `void * state` param. These allow for directly writing to the output without a string intermediary and passing state to each call. Stateful callbacks also work.

`daw::escaped_string` on the parameter lists says that the strring in the document will be escaped and needs to be unescaped prior to sending to the function.

For example

```cpp
tmp.add_callback<daw::escaped_string, daw::escaped_string>( []( std::string a, std::string b, daw::io::WriteProxy & writer ) {
    writer.write( a, ":", b );
)
```

The previous function takes 2 string parameters and uses the writer to write them to the callers output.