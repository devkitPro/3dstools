/*
 * smdhtool.c
 * Copyright (C) 2014 - plutoo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "lodepng/lodepng.h"

#define SMDH_MAGIC "SMDH"

typedef struct {
	u16 short_desc[0x40];
	u16 long_desc[0x80];
	u16 publisher[0x40];
} smdh_title;

typedef struct {
	u8 ratings[0x10];
	u32 region_lockout;
	u8 matchmaker_id[0xC];
	u32 flags;
	u16 eula_ver;
	u16 zero;
	u32 optimal_bannerframe;
	u32 streetpass_id;
} smdh_settings;

typedef struct {
	u32 magic;
	u16 version;
	u16 zero;
	smdh_title titles[16];
	smdh_settings settings;
	u8 zero2[0x8];
} smdh_header;

// stolen shamelessly from 3ds_hb_menu
static const u8 tile_order[] =
{
	0, 1, 8, 9, 2, 3, 10, 11, 16, 17, 24, 25, 18, 19, 26, 27,
	4, 5, 12, 13, 6, 7, 14, 15, 20, 21, 28, 29, 22, 23, 30, 31,
	32, 33, 40, 41, 34, 35, 42, 43, 48, 49, 56, 57, 50, 51, 58, 59,
	36, 37, 44, 45, 38, 39, 46, 47, 52, 53, 60, 61, 54, 55, 62, 63
};


#ifdef WIN32
static inline void fix_mingw_path(char* buf) {
	if (*buf == '/') {
		buf[0] = buf[1];
		buf[1] = ':';
	}
}
#endif

void usage(char* argv[])
{
	fprintf(stderr,
		"USAGE:\n"
		"%s --create <name> <long description> <author> <icon.png> <outfile>\n"
		"\n"
		"FLAGS:\n"
		"    --create : Create SMDH for use with the 3DS Homebrew Channel.\n",
		argv[0]);
	exit(1);
}

void conv_utf8_to_utf16(const char* in, u8* out, size_t max_len)
{
	size_t n = 0;

	do {
		// Currently only single-byte UTF8 characters.
		out[2*n  ] = in[n];
		out[2*n+1] = 0;

		if(n++ >= max_len)
			return;
	}
	while(in[n]);
}

u16 conv_argb_to_rgb565(u8 a, u8 r, u8 g, u8 b)
{
	r = 1.0f*r*a/255.0f;
	g = 1.0f*g*a/255.0f;
	b = 1.0f*b*a/255.0f;

	r = (r >> 3);
	g = (g >> 2);
	b = (b >> 3);

	return (r << 11) | (g << 5) | b;
}

int convert_png_to_icon(FILE* fd, const char* icon)
{
	unsigned char* img;
	unsigned int width, height;
	int rc;

	rc = lodepng_decode32_file(&img, &width, &height, icon);
	if(rc) {
		printf("Png load/decode error.\n");
		return rc;
	}

	if(width != 48 && height != 48) {
		printf("Expected png dimensions 48x48 pixels.\n");
		free(img);
		return 1;
	}

	u16 large_icon[48*48];
	u16 small_icon[24*24];
	unsigned int x, y, xx, yy, k;
	unsigned int n = 0;

	for(y=0; y<48; y+=8) {
		for(x=0; x<48; x+=8) {
			for(k=0; k<8*8; k++) {
				xx = (tile_order[k] & 0x7);
				yy = (tile_order[k] >> 3);

				u8* rgba = &img[4*(48*(y+yy) + (x+xx))];
				u8 r = rgba[0];
				u8 g = rgba[1];
				u8 b = rgba[2];
				u8 a = rgba[3];

				large_icon[n++] = conv_argb_to_rgb565(a, r, g, b);
			}
		}
	}

	for(y=0; y<24; y++) {
		for(x=0; x<24; x++) {
			small_icon[y*24 + x] = large_icon[y*48 + x];
		}
	}

	fwrite(small_icon, sizeof(small_icon), 1, fd);
	fwrite(large_icon, sizeof(large_icon), 1, fd);

	free(img);
	return 0;
}

int create_hb_banner(char* argv[])
{
	smdh_header hdr;
	memset(&hdr, 0, sizeof(hdr));

	memcpy(&hdr.magic, SMDH_MAGIC, 4);
	hdr.version = 0;

	size_t i;
	for(i=0; i<16; i++) {
		conv_utf8_to_utf16(argv[2], (u8*) &hdr.titles[i].short_desc[0], 0x40);
		conv_utf8_to_utf16(argv[3], (u8*) &hdr.titles[i].long_desc [0], 0x80);
		conv_utf8_to_utf16(argv[4], (u8*) &hdr.titles[i].publisher [0], 0x40);
	}


	FILE* fd = fopen(argv[6], "wb");
	if(fd == NULL) {
		perror("fopen");
		return 1;
	}

	fwrite(&hdr, sizeof(hdr), 1, fd);
	if(convert_png_to_icon(fd, argv[5]) != 0) {
		fclose(fd);
		return 1;
	}

	fclose(fd);
	return 0;
}

int main(int argc, char* argv[])
{
	if(argc < 2)
		usage(argv);

	if(strcmp(argv[1], "--create") == 0) {
		if(argc != 7) {
			fprintf(stderr, "Expected 6 args.\n");
			return 1;
		}

#ifdef WIN32
		fix_mingw_path(argv[5]);
#endif

		return create_hb_banner(argv);
	}
	else usage(argv);

	return 0;
}
