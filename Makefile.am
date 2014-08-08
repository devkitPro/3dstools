# Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = 3dsxtool

_common_SOURCES	=	src/types.h src/FileClass.h
3dsxtool_SOURCES	=	src/3dsxtool.cpp src/elf.h $(_common_SOURCES)
3dsxtool_CXXFLAGS	=	-std=gnu++0x

EXTRA_DIST = autogen.sh
