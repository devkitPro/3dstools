#pragma once
#include "types.h"

#define SMDH_MAGIC "SMDH"

enum
{
	LANG_JAPANESE,
	LANG_ENGLISH,
	LANG_GERMAN,
	LANG_ITALIAN,
	LANG_SPANISH,
	LANG_SIMPLIFIED_CHINESE,
	LANG_KOREAN,
	LANG_DUTCH,
	LANG_PORTUGESE,
	LANG_RUSSIAN,
	LANG_TRADITIONAL_CHINESE,
};

enum
{
	REGION_JAPAN     = BIT(0),
	REGION_USA       = BIT(1),
	REGION_EUROPE    = BIT(2),
	REGION_AUSTRALIA = BIT(3),
	REGION_CHINA     = BIT(4),
	REGION_TAIWAN    = BIT(5),
	REGION_KOREA     = BIT(6),
	REGION_ALL       = 0xFFFFFFFF,
};

enum
{
	RATING_CERO      = 0,
	RATING_ESRB      = 1,
	RATING_USK       = 3,
	RATING_PEGI_GEN  = 4,
	RATING_PEGI_PRT  = 6,
	RATING_PEGI_BBFC = 7,
	RATING_COB       = 8,
	RATING_GRB       = 9,
	RATING_CGSRR     = 10,
};

enum
{
	RATING_FLAG_NO_RESTRICTION = BIT(5),
	RATING_FLAG_RATING_PENDING = BIT(6),
	RATING_FLAG_MASK = RATING_FLAG_RATING_PENDING - 1,
	RATING_FLAG_ENABLED = BIT(7),
};

enum
{
	SMDH_VISIBLE                  = BIT(0),
	SMDH_AUTOBOOT                 = BIT(1),
	SMDH_USE_3D_EFFECT            = BIT(2),
	SMDH_REQUIRE_ACCEPT_EULA      = BIT(3),
	SMDH_AUTOSAVE_ON_EXIT         = BIT(4),
	SMDH_USE_EXTENDED_BANNER      = BIT(5),
	SMDH_REGION_RATING_USED       = BIT(6),
	SMDH_USE_SAVE_DATA            = BIT(7),
	SMDH_RECORD_USAGE             = BIT(8),
	SMDH_DISABLE_SAVE_DATA_BACKUP = BIT(10),
};

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

typedef struct {
	smdh_header hdr;
	u16 smallIconData[0x240];
	u16 bigIconData[0x900];
} smdh_file;
