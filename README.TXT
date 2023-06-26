si_read.exe v1.6.489

v 1.6 - dynamicaly & monolitic linked build (GCC 4.9.2, MinGW)
v 1.5 - dynamicaly & monolitic linked build (GCC 4.8.2, MinGW)
v 1.4 - dynamicaly & monolitic linked build (GCC 4.7.2, MinGW)
v 1.3 - dynamicaly & monolitic linked build (GCC 4.7.0, MinGW)
v 1.2 - dynamicaly & monolitic linked build (GCC 4.6.0, MinGW)
v 1.0 - dynamicaly linked build (MSVS10)

(C) Copyright 2009-2016 Vit Kasal,
(C) Copyright 2011,2016 Richard Patek

Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

WWW:
https://github.com/arnost00/si_communication

change log:

1.6	- modified output format (trim on whole seconds). Was wrong in SIAC (beacon mode) or sprint mode.
1.5	- p-card readout fixed
	- separate input and output ports possible for readout (experimental)
	- support for SIAC in extended and basic protocol
	- support for SI card v10 and v11 in basic protocol
1.4	- improved failed communication recovery algorithm
	- fixed basic protocol
	- minor bugfixes
1.3	- support for SI card v10 and v11 (tested in extended protocol only)
	- recovery from com port errors improved
	- minor bugfixes
1.2	- multiple device support
	- improved memory management
	- minor bugfixes
1.1	- basic protocol support
	- minor bugfixes
1.0	- SI card read out mode (extended protocol only)
	- SI punch mode (extended protocol only)

version 1.2 and earlier doesn't support SI card v10 and v11