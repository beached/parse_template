# Template Tags

A library that accepts a UTF-8 text template and generates text(e.g. html)

--------------
|Tag|Description|
|---|-----------|
|<%call args="callback_name,args..."%>|insert text from callback|
|<%date%>|insert current date|
|<%time%>|insert current time|
|<%timestamp args="fmt,tz"%>|See [date_formatting.md](date_formatting.md)  for fmt, timezone name from IANA database e.g. America/NewYork. defaults to ISO9601 timestamp format and the system's timezone|
