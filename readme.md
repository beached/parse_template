# Template Tags

A library that accepts a UTF-8 text template and generates text(e.g. html)

--------------
|Tag|Description|
|---|-----------|
|<%call args="callback_name,args..."%>|insert text from callback|
|<%date%>|insert current date|
|<%time%>|insert current time|
|<%timestamp args="fmt,tz"%>|See [date_formatting.md](date_formatting.md) |
|<%repeat args="callback_name,args..." prefix="" suffix=""%>|insert text from callback that generates an iterable list of items.|
