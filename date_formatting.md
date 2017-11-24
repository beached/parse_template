# Date Formatting

|Flag|Description|
|----|-----------|
|%a|The locale's full or abbreviated case-insensitive weekday name.|
|%A|Equivalent to %a.|
|%b|The locale's full or abbreviated case-insensitive month name.|
|%B|Equivalent to %b.|
|%c|The locale's date and time representation. The modified command %Ec interprets the locale's alternate date and time representation.|
|%C|The century as a decimal number. The modified command %NC where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required. The modified commands %EC and %OC interpret the locale's alternative representation of the century.|
|%d|The day of the month as a decimal number. The modified command %Nd where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required. The modified command %EC interprets the locale's alternative representation of the day of the month.|
|%D|Equivalent to %m/%d/%y.|
|%e|Equivalent to %d and can be modified like %d.|
|%F|Equivalent to %Y-%m-%d. If modified with a width, the width is applied to only %Y.|
|%g|The last two decimal digits of the ISO week-based year. The modified command %Ng where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required.|
|%G|The ISO week-based year as a decimal number. The modified command %NG where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 4. Leading zeroes are permitted but not required.|
|%h|Equivalent to %b.|
|%H|The hour (24-hour clock) as a decimal number. The modified command %NH where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required. The modified command %OH interprets the locale's alternative representation.|
|%I|The hour (12-hour clock) as a decimal number. The modified command %NI where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required.|
|%j|The day of the year as a decimal number. Jan 1 is 1. The modified command %Nj where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 3. Leading zeroes are permitted but not required.|
|%m|The month as a decimal number. Jan is 1. The modified command %Nm where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required. The modified command %Om interprets the locale's alternative representation.|
|%M|The minutes as a decimal number. The modified command %NM where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required. The modified command %OM interprets the locale's alternative representation.|
|%n|Matches one white space character. [Note: %n, %t and a space, can be combined to match a wide range of white-space patterns. For example "%n " matches one or more white space charcters, and "%n%t%t" matches one to three white space characters. — end note]|
|%p|The locale's equivalent of the AM/PM designations associated with a 12-hour clock. The command %I must precede %p in the format string.|
|%r|The locale's 12-hour clock time.|
|%R|Equivalent to %H:%M.|
|%S|The seconds as a decimal number. The modified command %NS where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2 if the input time has a precision convertible to seconds. Otherwise the default width is determined by the decimal precision of the input and the field is interpreted as a long double in a fixed format. If encountered, the locale determines the decimal point character. Leading zeroes are permitted but not required. The modified command  %OS interprets the locale's alternative representation.|
|%t|Matches zero or one white space characters.|
|%T|Equivalent to %H:%M:%S.|
|%u|The ISO weekday as a decimal number (1-7), where Monday is 1. The modified command %Nu where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 1. Leading zeroes are permitted but not required. The modified command %Ou interprets the locale's alternative representation.|
|%U|The week number of the year as a decimal number. The first Sunday of the year is the first day of week 01. Days of the same year prior to that are in week 00. The modified command %NU where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required.|
|%V|The ISO week-based week number as a decimal number. The modified command %NV where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required.|
|%w|The weekday as a decimal number (0-6), where Sunday is 0. The modified command %Nw where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 1. Leading zeroes are permitted but not required. The modified command %Ou interprets the locale's alternative representation.|
|%W|The week number of the year as a decimal number. The first Monday of the year is the first day of week 01. Days of the same year prior to that are in week 00. The modified command %NW where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required.|
|%x|The locale's date representation. The modified command %Ex produces the locale's alternate date representation.|
|%X|The locale's time representation. The modified command %Ex produces the locale's alternate time representation.|
|%y|The last two decimal digits of the year. If the century is not otherwise specified (e.g. with %C), values in the range [69 - 99] are presumed to refer to the years [1969 - 1999], and values in the range [00 - 68] are presumed to refer to the years [2000 - 2068]. The modified command %Ny where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 2. Leading zeroes are permitted but not required. The modified commands %Ey and %Oy interpret the locale's alternative representation.|
|%Y|The year as a decimal number. The modified command %NY where N is a positive decimal integer specifies the maximum number of characters to read. If not specified, the default is 4. Leading zeroes are permitted but not required. The modified command %EY interprets the locale's alternative representation.|
|%z|The offset from UTC in the ISO 8601 format. For example -0430 refers to 4 hours 30 minutes behind UTC. The modified commands %Ez and %Ez parse a : between the hours and minutes and leading zeroes on the hour field are optional: -4:30.|
|%Z|The time zone abbreviation or name. A single word is parsed. This word can only contain characters from the basic source character set ([lex.charset] in the C++ standard) that are alphanumeric, or one of '_', '/', '-' or '+'.|
|%%|A % character is extracted.|
