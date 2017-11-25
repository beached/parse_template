# Template Tags

A library that accepts a UTF-8 text template and generates text(e.g. html)

--------------
|Tag|Description|
|---|-----------|
|<%call args="callback_name,callback_args..."%>|insert text from callback|
|<%timestamp args="fmt,tz"%>|See [date_formatting.md](date_formatting.md)  for fmt, timezone name from IANA database e.g. America/NewYork. defaults to ISO9601 timestamp format and the system's timezone.  See the [wiki](https://en.wikipedia.org/wiki/List_of_tz_database_time_zones) article for listing of time zone names|
|<%date%>|insert current date with system timezone. Same as timestamp with fmt="%Y-%m-%d" and tz=""|
|<%time%>|insert current time. Same as timestamp with fmt="%T" and tz=""|

### Note:
Do not put spaces between comma separates currently
