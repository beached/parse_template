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
