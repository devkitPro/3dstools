# Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = 3dsxtool 3dsxdump smdhtool

_common_SOURCES	=	src/types.h src/FileClass.h
_lodepng_SOURCES	=	src/lodepng/lodepng.cpp src/lodepng/lodepng.h
3dsxtool_SOURCES	=	src/3dsxtool.cpp src/elf.h $(_common_SOURCES)
3dsxtool_CXXFLAGS	=
3dsxdump_SOURCES	=	src/3dsxdump.cpp src/3dsx.h $(_common_SOURCES)
3dsxdump_CXXFLAGS	=
smdhtool_SOURCES	=	src/smdhtool.cpp $(_lodepng_SOURCES) $(_common_SOURCES)
smdhtool_CXXFLAGS	=

EXTRA_DIST = autogen.sh
