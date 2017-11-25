# Template Tags

A library that accepts a UTF-8 text template and generates text(e.g. html)

--------------
|Tag|Description|
|---|-----------|
|<%call args="callback_name,callback_args..."%>|insert text from callback|
|<%timestamp args="fmt,tz"%>|See [date_formatting.md](date_formatting.md) for fmt, timezone name from IANA database e.g. America/NewYork. The default fmt is "%Y-%m-%dT%T%z", the ISO9601 timestamp format, and tz="" the system's timezone.  See the [wiki](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones) article for listing of time zone names|
|<%date args="tz"%>|insert current date with timezone tz. The default timezone is the current system's.  Same as timestamp with fmt="%Y-%m-%d" and tz|
|<%time args="tz"%>|insert current time with timezone tz.  The default timezone is the current system's. Same as timestamp with fmt="%T" and tz|

### Note:
Do not put spaces between comma separates currently


# Code Usage

The constructor for parse_template takes any container that is string like(e.g. std::string, string_view..).  This allows for things like memory mapped files.

``` C++
string_view str = ...;
auto template = parse_template{ str };
```

Adding callbacks requires one to specify how the arguments are to be parsed.  Often this is just specifying the types of the arguments, but tag types can be used as any type specified must have a ``` std::string parse_to_value( daw::string_view, type_tag ) ``` method

``` C++
template.add_callback<int,int,bool>( "callback_name", []( int a, int b, bool c ) {
	if( c ) {
		a += b;
	}
	return a;
} );
```

The previous example adds a callback that takes the arguments of int, int, and bool and returns an int.  The result must be a string or have a ``` to_string ``` overload. To use the example in a template you would call it like the following:

``` html
<%call args="callback_name,5,5,true"%><br>
<%call args="callback_name,5,5,false"%>
```

If the text output was html, the output would be

```
10
5
```
