# Makefile.am -- Process this file with automake to produce Makefile.in
bin_PROGRAMS = 3dsxtool 3dsxdump smdhtool cxitool

_common_SOURCES	=	src/types.h src/FileClass.h
_3dsxloader_SOURCES	=	src/3dsx_loader.cpp src/3dsx_loader.h
_lodepng_SOURCES	=	src/lodepng/lodepng.cpp src/lodepng/lodepng.h
_crypto_SOURCES     =	src/crypto.cpp src/crypto.h src/polarssl/aes.c src/polarssl/rsa.c src/polarssl/sha1.c src/polarssl/sha2.c src/polarssl/base64.c src/polarssl/bignum.c src/polarssl/aes.h src/polarssl/rsa.h src/polarssl/sha1.h src/polarssl/sha2.h src/polarssl/base64.h src/polarssl/bignum.h src/polarssl/bn_mul.h src/polarssl/config.h
_libyaml_SOURCES	=	src/YamlReader.cpp src/YamlReader.h src/libyaml/api.c src/libyaml/dumper.c src/libyaml/emitter.c src/libyaml/loader.c src/libyaml/parser.c src/libyaml/reader.c src/libyaml/scanner.c src/libyaml/writer.c src/libyaml/yaml_private.h src/libyaml/yaml.h
_blz_SOURCES	=	src/blz.c src/blz.h
3dsxtool_SOURCES	=	src/3dsxtool.cpp src/romfs.cpp src/elf.h src/romfs.h $(_common_SOURCES)
3dsxtool_CXXFLAGS	=	-Wall -D__USE_MINGW_ANSI_STDIO
3dsxdump_SOURCES	=	src/3dsxdump.cpp src/3dsx.h $(_3dsxloader_SOURCES) $(_common_SOURCES)
3dsxdump_CXXFLAGS	=	-Wall -D__USE_MINGW_ANSI_STDIO
smdhtool_SOURCES	=	src/smdhtool.cpp src/smdh.h $(_lodepng_SOURCES) $(_common_SOURCES)
smdhtool_CXXFLAGS	=	-Wall -D__USE_MINGW_ANSI_STDIO
cxitool_SOURCES	=	src/cxitool.cpp src/ncch.h src/CxiBuilder.h src/CxiBuilder.cpp src/CxiSettings.cpp src/CxiExeFS.cpp src/CxiRomFS.cpp src/smdh.h $(_blz_SOURCES) $(_3dsxloader_SOURCES) $(_crypto_SOURCES) $(_libyaml_SOURCES) $(_common_SOURCES)
cxitool_CXXFLAGS	=	-Wall -D__USE_MINGW_ANSI_STDIO

EXTRA_DIST = autogen.sh
