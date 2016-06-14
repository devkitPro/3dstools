#pragma once
#include "types.h"
#include "crypto.h"

struct NcchHeader
{
	u8 signature[Crypto::kRsa2048Size];
	u32 magic;
	u32 contentSize;
	u64 partitionId;
	char makerCode[2];
	u16 version;
	u32 field_0x114;
	u64 programId;
	u8 _reserved1[0x10];
	u8 logoHash[Crypto::kSha256HashLen];
	char productCode[0x10];
	u8 exheaderHash[Crypto::kSha256HashLen];
	u32 exheaderSizeBytes;
	u32 _reserved2;
	u8 flags[8];
	u32 plainOffset, plainSize;
	u32 logoOffset, logoSize;
	u32 exefsOffset, exefsSize, exefsHashedSize, _reserved3;
	u32 romfsOffset, romfsSize, romfsHashedSize, _reserved4;
	u8 exefsHash[Crypto::kSha256HashLen];
	u8 romfsHash[Crypto::kSha256HashLen];

	static const int kMediaUnitSize = 0x200;
	static const int kMagic = 0x4843434E; // 'NCCH'

	static inline u32 MediaUnitAlign(u32 val)
	{
		val += kMediaUnitSize-1;
		return val - (val%kMediaUnitSize);
	}

	enum
	{
		Flag4_Old3DS = 1,
		Flag4_New3DS = 2,
	};

	enum
	{
		Flag5_Data       = BIT(0),
		Flag5_Executable = BIT(1),
		Flag5_SysUpdate  = BIT(2),
		Flag5_Manual     = BIT(3),
		Flag5_Trial      = BIT(4),
	};

	enum
	{
		Flag7_FixedCryptoKey = BIT(0),
		Flag7_NoMountRomFS   = BIT(1),
		Flag7_NoCrypto       = BIT(2),
	};
};

struct CodeSetInfo
{
	u32 address;
	u32 nPages;
	u32 sizeBytes;
};

struct SystemControlInfo
{
	char processName[8];
	u8 _reserved1[5];
	u8 flag;
	u16 remasterVersion;
	CodeSetInfo text;
	u32 stackSize;
	CodeSetInfo rodata;
	u32 _reserved2;
	CodeSetInfo data;
	u32 bssSize;
	u64 dependencies[48];
	u64 saveSize;
	u64 jumpId;
	u8 _reserved3[0x30];

	enum
	{
		Flag_IsCompressed = BIT(0),
		Flag_IsSD         = BIT(1),
	};
};

struct StorageInfo
{
	u64 extdataId;
	u32 systemSaveIds[2];
	u64 storageAccessableId;
	u64 flags;

	enum
	{
		CategorySystemApp      = BIT(0),
		CategoryHardwareCheck  = BIT(1),
		CategoryFileSystemTool = BIT(2),
		Debug                  = BIT(3),
		CanAccessTwlCardBackup = BIT(4),
		CanAccessTwlNand       = BIT(5),
		SpotPass               = BIT(6),
		CanAccessSD            = BIT(7),
		Core                   = BIT(8),
		CanAccessCtrNandRO     = BIT(9),
		CanAccessCtrNandRW     = BIT(10),
		CanWriteToCtrNandRO    = BIT(11),
		CategorySystemSettings = BIT(12),
		SystemTransfer         = BIT(13),
		ExportImportIVS        = BIT(14),
		CanAccessWriteOnlySD   = BIT(15),
		SwitchCleanup          = BIT(16),
		SaveDataMove           = BIT(17),
		Shop                   = BIT(18),
		Shell                  = BIT(19),
		CategoryHomeMenu       = BIT(20),
		NoRomFS                = (1ULL<<56),
		ExtendedSaveDataAccess = (1ULL<<57),
	};
};

struct Arm11LocalCaps
{
	u64 programId;
	u32 firmTidLow;
	u8 extFlags, systemModeExt, flags;
	u8 priority;
	u16 resourceLimits[16];
	StorageInfo storageInfo;
	char allowedServices[34][8];
	u8 _reserved1[0xF];
	u8 resourceLimitType;

	enum
	{
		ExtFlag_EnableL2Cache = BIT(0),
		ExtFlag_EnableFastCPU = BIT(1),
	};

	enum
	{
		SystemMode_Normal = 0,
		SystemMode_Dev1   = 2,
		SystemMode_Dev2   = 3,
		SystemMode_Dev3   = 4,
	};

	enum
	{
		SystemModeExt_Old3DS      = 0,
		SystemModeExt_New3DS      = 1,
		SystemModeExt_New3DS_Dev1 = 2,
		SystemModeExt_New3DS_Dev2 = 3,
	};

	enum
	{
		Application = 0,
		SysApplet   = 1,
		LibApplet   = 2,
		SysModule   = 3,
	};

	static inline u8 MakeFlags(int systemMode, int affinityMask, int idealProcessor)
	{
		return (systemMode<<4) | (affinityMask<<2) | idealProcessor;
	}
};

struct Arm11KernelCaps
{
	u32 caps[28];
	u8 _reserved1[0x10];

	enum
	{
		AllowDebug          = BIT(0),
		ForceDebug          = BIT(1),
		AllowNonAlphanum    = BIT(2),
		SharedPageWriting   = BIT(3),
		PrivilegePriority   = BIT(4),
		AllowMainArgs       = BIT(5),
		SharedDevMem        = BIT(6),
		RunnableOnSleep     = BIT(7),
		MemType_Application = (1<<8),
		MemType_System      = (2<<8),
		MemType_Base        = (3<<8),
		MemType_Mask        = (0xF<<8),
		SpecialMemory       = BIT(12),
		CanUseCore2         = BIT(13),
	};

	enum
	{
		Prefix_IrqList         = 0xE0000000,
		Prefix_SvcList         = 0xF0000000,
		Prefix_KernelVer       = 0xFC000000,
		Prefix_HandleTableSize = 0xFE000000,
		Prefix_KernelFlags     = 0xFF000000,
		Prefix_StaticMap       = 0xFF800000,
		Prefix_IOMap           = 0xFFE00000,
		Prefix_Unused          = 0xFFFFFFFF,
	};

	static inline u32 MakeIrqList(u32 irq)
	{
		return le_word(Prefix_IrqList | irq);
	}

	static inline u32 MakeSvcList(int index, u32 mask)
	{
		return le_word(Prefix_SvcList | (index<<24) | mask);
	}

	static inline u32 MakeKernelVer(int major, int minor)
	{
		return le_word(Prefix_KernelVer | (major<<8) | minor);
	}

	static inline u32 MakeHandleTableSize(u32 size)
	{
		return le_word(Prefix_HandleTableSize | size);
	}

	static inline u32 MakeKernelFlags(u32 flags)
	{
		return le_word(Prefix_KernelFlags | flags);
	}

	static inline u32 MakeMapping(u32 addr, bool isStatic, bool isReadOnly)
	{
		return le_word((isStatic ? Prefix_StaticMap : Prefix_IOMap) | (isReadOnly ? BIT(20) : 0) | (addr>>12));
	}
};

struct Arm9LocalCaps
{
	u32 descriptor;
	u8 _reserved[11];
	u8 version;

	enum
	{
		MountNand     = BIT(0),
		MountNandRO   = BIT(1),
		MountTwln     = BIT(2),
		MountWnand    = BIT(3),
		MountCardSPI  = BIT(4),
		UseSDIF3      = BIT(5),
		CreateSeed    = BIT(6),
		UseCardSPI    = BIT(7),
		SDApplication = BIT(8),
		MountSdmcWO   = BIT(9),
	};
};

struct AccessControlInfo
{
	Arm11LocalCaps  localCaps11;
	Arm11KernelCaps kernelCaps11;
	Arm9LocalCaps   localCaps9;
};

struct ExHeaderProper
{
	SystemControlInfo sysCtrlInfo;
	AccessControlInfo accessCtrlInfo;
};

struct ExHeader : public ExHeaderProper
{
	u8 accessDescSig[Crypto::kRsa2048Size];
	u8 ncchPubKey[Crypto::kRsa2048Size];
	AccessControlInfo accessCtrlInfo2;
};

struct ExeFSEntry
{
	char name[8];
	u32 offset, size;
};

struct ExeFSHeader
{
	static const int kMaxFiles = 10;
	static const u8 s_homebrewLogo[0x2000];

	ExeFSEntry files[kMaxFiles];
	u8 _reserved1[0x20];
	u8 fileHashes[kMaxFiles][Crypto::kSha256HashLen];

	ExeFSHeader() { memset(this, 0, sizeof(*this)); }

	void AddFile(int pos, const char* name, const void* data, u32 size)
	{
		u32 offset = (pos>0) ? NcchHeader::MediaUnitAlign(le_word(files[pos-1].offset) + le_word(files[pos-1].size)) : 0;
		ExeFSEntry& e = files[pos];
		strncpy(e.name, name, sizeof(e.name));
		e.offset = le_word(offset);
		e.size = le_word(size);
		Crypto::Sha256((const u8*)data, size, fileHashes[kMaxFiles-pos-1]);
	}

	u32 TotalSizeMediaUnits()
	{
		u32 size = sizeof(*this);
		for (int i = 0; i < kMaxFiles; i ++)
			size += NcchHeader::MediaUnitAlign(le_word(files[i].size));
		return size / NcchHeader::kMediaUnitSize;
	}
};

struct IVFCHeader
{
	u32 magic;
	u32 type;
	u32 masterHashSize;

	struct misaligned_le_u64
	{
		u32 lo, hi;
		u64 operator =(u64 x)
		{
			lo = le_word(x);
			hi = le_word(x>>32);
			return x;
		}
		operator u64()
		{
			return (u64)le_word(lo) | ((u64)le_word(hi)<<32);
		}
	};

	struct
	{
		misaligned_le_u64 offset;
		misaligned_le_u64 size;
		u32 blockSizeLog2;
		u32 _reserved1;
	} levels[3];

	u32 optionalInfoSize;
	u32 _reserved1;

	static const u32 kMagic = 0x43465649; // IVFC
	static const u32 kTypeRomFS = 0x10000;
	static const u32 kBlockSizeLog2 = 12;
	static const u32 kBlockSize = BIT(kBlockSizeLog2);

	static inline u64 BlockAlign(u64 value)
	{
		return (value+kBlockSize-1) &~ (kBlockSize-1);
	}

	void init()
	{
		memset(this, 0, sizeof(*this));
		magic = le_word(kMagic);
		type = le_word(kTypeRomFS);
		masterHashSize = le_word(Crypto::kSha256HashLen);
		for (int i = 0; i < 3; i ++)
			levels[i].blockSizeLog2 = le_word(kBlockSizeLog2);
		optionalInfoSize = le_word(sizeof(*this));
	}
};
