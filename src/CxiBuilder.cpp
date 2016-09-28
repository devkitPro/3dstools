#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "CxiBuilder.h"
#include "3dsx_loader.h"

#define safe_call(a) do { int rc = a; if(rc != 0) return rc; } while(0)

static const struct Crypto::sRsa2048Key s_dummyRsaKey =
{
	{ 0xAB, 0x7C, 0x3D, 0x15, 0xDF, 0xA1, 0xB0, 0x06, 0x7C, 0xC1, 0x47, 0xAA, 0x53, 0xD8, 0x86, 0x75, 0x42, 0x99, 0xE0, 0x18, 0x66, 0x03, 0x39, 0xD9, 0x79, 0xDA, 0x0A, 0x49, 0x2B, 0x64, 0x91, 0x45, 0x64, 0x90, 0x3D, 0x5F, 0x56, 0x0D, 0xD6, 0xD0, 0x37, 0xBF, 0x81, 0x1E, 0x92, 0xA8, 0xA5, 0x55, 0x09, 0xD9, 0xAE, 0x82, 0x43, 0x16, 0xD3, 0x68, 0x88, 0xBC, 0x4D, 0xCB, 0xC9, 0x2B, 0x0B, 0x47, 0xBD, 0xF8, 0xD9, 0x1A, 0x30, 0x80, 0x85, 0xA8, 0x30, 0x19, 0x77, 0x2E, 0xE9, 0x9F, 0x2D, 0xCA, 0xFC, 0x91, 0x82, 0xC8, 0x7F, 0xDA, 0xFE, 0xFA, 0xA9, 0x44, 0x87, 0x3E, 0xFF, 0x83, 0xA9, 0x4D, 0x80, 0xEC, 0xD5, 0xCB, 0x3E, 0xC8, 0xE8, 0xFF, 0x36, 0xF0, 0xF0, 0xD7, 0x84, 0x82, 0xE2, 0x09, 0x1A, 0x11, 0x76, 0xDF, 0x7A, 0x9B, 0x1C, 0x25, 0xB0, 0x6D, 0xE9, 0x8B, 0x54, 0x52, 0x55, 0x8F, 0x7F, 0x6F, 0xBF, 0xAF, 0xB8, 0xDD, 0xD4, 0xD4, 0xA1, 0x56, 0x8D, 0xF9, 0xF9, 0x98, 0x0E, 0x71, 0x93, 0xED, 0xB8, 0x99, 0xD3, 0xFA, 0x63, 0xF5, 0x6E, 0xAF, 0x9D, 0x49, 0xEA, 0xD7, 0xF7, 0xD9, 0x79, 0x7E, 0x51, 0x71, 0xE3, 0x4B, 0xEB, 0xA7, 0xCB, 0xD9, 0x5E, 0x89, 0x2B, 0x69, 0xBA, 0xEF, 0x98, 0x94, 0xA5, 0x74, 0x96, 0xAF, 0x4F, 0x9A, 0xDB, 0x93, 0x51, 0xE1, 0x99, 0x78, 0xCD, 0xEB, 0x15, 0xE1, 0x31, 0x32, 0xAC, 0x35, 0x9B, 0xD0, 0x4A, 0xDC, 0x87, 0x38, 0x5E, 0xA6, 0x42, 0x4A, 0xD2, 0x05, 0x51, 0x4F, 0x53, 0x9B, 0x8B, 0x3B, 0xE3, 0x03, 0xB9, 0x34, 0xFB, 0x56, 0xCC, 0x6E, 0x7B, 0x56, 0xEA, 0x38, 0x11, 0x44, 0xEE, 0xB0, 0x7B, 0x89, 0x35, 0x0B, 0x0F, 0x17, 0x2F, 0x5D, 0x4B, 0x30, 0x56, 0xF2, 0x06, 0x63, 0x4D, 0x85, 0x86, 0xB7, 0xFE, 0x85, 0xD4, 0xDF, 0xDE, 0xAF },
	{ 0x1E, 0x4A, 0xE7, 0x1B, 0x8B, 0x12, 0xAB, 0xDE, 0xA9, 0x81, 0x17, 0x20, 0xCE, 0x88, 0xEC, 0x4F, 0xA0, 0x81, 0x40, 0x25, 0xEF, 0x37, 0x58, 0xAB, 0xC3, 0x2B, 0xB2, 0x2F, 0x74, 0xBB, 0xE2, 0x31, 0xA8, 0xEF, 0x15, 0xF8, 0x56, 0x62, 0x41, 0x75, 0x2C, 0xB3, 0xE6, 0xA2, 0x38, 0xF4, 0x13, 0xA8, 0xAF, 0x01, 0xC6, 0x22, 0xFA, 0xA8, 0xF8, 0x95, 0x79, 0xBA, 0x11, 0xE0, 0x12, 0xDC, 0x48, 0xB4, 0xD6, 0xA9, 0x33, 0xE8, 0xBD, 0x72, 0xA6, 0xA9, 0xAC, 0x3D, 0x83, 0x61, 0x45, 0x21, 0xBA, 0x5C, 0x26, 0x3B, 0xAA, 0x27, 0xB2, 0xF6, 0x43, 0x9E, 0x91, 0xF2, 0x2A, 0x16, 0x05, 0xDB, 0x03, 0x38, 0x4E, 0xB3, 0x07, 0x9D, 0x4C, 0xAC, 0xFF, 0x03, 0xBE, 0x77, 0xD7, 0x83, 0xAA, 0xC3, 0xD8, 0x1C, 0x15, 0x7F, 0xCA, 0x48, 0xF6, 0x06, 0x9A, 0x75, 0x49, 0xF2, 0x50, 0x94, 0x2D, 0x44, 0x12, 0x1A, 0xEA, 0x04, 0x01, 0x41, 0xD9, 0x87, 0xF1, 0xC3, 0xDB, 0xBE, 0xD8, 0x69, 0xC1, 0x7C, 0x27, 0xF8, 0x52, 0x80, 0x7A, 0xD9, 0x53, 0x67, 0x93, 0x4D, 0x89, 0x56, 0x55, 0xB6, 0x3E, 0x60, 0x42, 0x05, 0x88, 0xDF, 0xCB, 0x17, 0x9D, 0x92, 0xAF, 0x4B, 0xB2, 0x30, 0xFD, 0xE6, 0x7D, 0x5E, 0x80, 0x5F, 0xFE, 0x0F, 0x62, 0x99, 0x40, 0x99, 0x1B, 0xF0, 0xE2, 0xAD, 0x6B, 0xDD, 0x4D, 0x64, 0xDF, 0x6D, 0x04, 0x62, 0xA9, 0xC0, 0xFD, 0x41, 0x0F, 0x84, 0xBB, 0x85, 0xB0, 0x10, 0xE0, 0x6F, 0xD2, 0xFF, 0x31, 0x5A, 0x0F, 0x47, 0xE4, 0xB5, 0x54, 0x95, 0x34, 0xC6, 0xEB, 0xF8, 0xE5, 0x15, 0x79, 0x56, 0xC2, 0x83, 0xF4, 0xEC, 0x18, 0xF4, 0x82, 0x00, 0x57, 0xB0, 0xF9, 0x64, 0x4A, 0x8B, 0xE7, 0x22, 0x36, 0x4F, 0xA9, 0x59, 0xD3, 0x5B, 0x01, 0x71, 0x5E, 0xE7, 0xFE, 0x1F, 0x4C, 0x18, 0x01, 0x61 }
};

CxiBuilder::CxiBuilder(const char* procName, const char* prodCode, u64 tid, u16 version)
	: exhkflags(Arm11KernelCaps::MemType_Application), kverMajor(2), kverMinor(44), deps(), services(), mapping(), handleTableSize(0x200), saveIdCount(0), sysSaves(), otherSaves()
	, isFirmCxi(false), exefs(), codeData(NULL), codeSize(0), bnrData(NULL), bnrSize(0), icnPresent(false), romfsLevel3(NULL), romfsLevel3Offset(0), romfsLevel3Size(0), romfsHashes(NULL)
{
	memset(&hdr, 0, sizeof(hdr));
	hdr.magic = le_word(NcchHeader::kMagic);
	hdr.partitionId = le_dword(tid);
	strncpy(hdr.makerCode, "00", 2);
	hdr.version = le_hword(2);
	hdr.programId = le_dword(tid);
	strncpy(hdr.productCode, prodCode, sizeof(hdr.productCode));
	hdr.flags[4] = NcchHeader::Flag4_Old3DS;
	hdr.flags[5] = NcchHeader::Flag5_Executable;
	hdr.flags[7] = NcchHeader::Flag7_NoCrypto; // | NcchHeader::Flag7_FixedCryptoKey;

	memset(&exh, 0, sizeof(hdr));
	strncpy(exh.sysCtrlInfo.processName, procName, sizeof(exh.sysCtrlInfo.processName));
	exh.sysCtrlInfo.flag = SystemControlInfo::Flag_IsCompressed | SystemControlInfo::Flag_IsSD;
	exh.sysCtrlInfo.remasterVersion = le_hword(version);
	exh.sysCtrlInfo.stackSize = le_word(0x4000);
	exh.sysCtrlInfo.jumpId = le_dword(tid);
	exh.accessCtrlInfo.localCaps11.programId = le_dword(tid);
	exh.accessCtrlInfo.localCaps11.firmTidLow = le_word(0x00000002);
	exh.accessCtrlInfo.localCaps11.systemModeExt = Arm11LocalCaps::SystemModeExt_New3DS;
	exh.accessCtrlInfo.localCaps11.flags = Arm11LocalCaps::MakeFlags(Arm11LocalCaps::SystemMode_Normal, 0x1, 0);
	exh.accessCtrlInfo.localCaps11.priority = 0x30;
	exh.accessCtrlInfo.localCaps11.storageInfo.flags = le_dword(StorageInfo::CanAccessSD);
	exh.accessCtrlInfo.localCaps11.resourceLimitType = Arm11LocalCaps::Application;
	memset(exh.accessCtrlInfo.kernelCaps11.caps, 0xFF, sizeof(exh.accessCtrlInfo.kernelCaps11.caps));
	exh.accessCtrlInfo.localCaps9.descriptor = le_dword(Arm9LocalCaps::SDApplication);
	exh.accessCtrlInfo.localCaps9.version = 2;

	memset(&specified, 0, sizeof(specified));
	memset(svcAccess, 0, sizeof(svcAccess));
	memset(&icnData, 0, sizeof(icnData));
}

CxiBuilder::~CxiBuilder()
{
	if (codeData) free(codeData);
	if (bnrData) free(bnrData);
	if (romfsLevel3) fclose(romfsLevel3);
	if (romfsHashes) free(romfsHashes);
}

int CxiBuilder::Read3DSX(const char* f)
{
	u32 baseAddr = 0x00100000;
	if (exhkflags & Arm11KernelCaps::SpecialMemory)
		baseAddr = 0x14000000;

	FileClass fin(f, "rb");
	if (fin.openerror())
	{
		fprintf(stderr, "Cannot open input file: %s\n", f);
		return 1;
	}

	Loaded3DSX ldr;
	ldr.smdh = &icnData;
	int rc = Load3DSX(ldr, fin.get_ptr(), baseAddr, !isFirmCxi);
	if (rc != 0)
	{
		fprintf(stderr, "Error #%d while reading input file: %s\n", rc, f);
		return rc;
	}

	codeData = ldr.code;
	if (!isFirmCxi)
		codeSize = (ldr.codePages+ldr.rodataPages+ldr.dataPages)*0x1000;
	else
		codeSize = ldr.codeSize + ldr.rodataSize + ldr.dataSize;
	if (ldr.romfsLevel3Offset && ldr.romfsLevel3Size)
	{
		romfsLevel3 = fin.Detach();
		romfsLevel3Offset = ldr.romfsLevel3Offset;
		romfsLevel3Size = ldr.romfsLevel3Size;
		hdr.flags[5] |= NcchHeader::Flag5_Data;
	} else
	{
		hdr.flags[7] |= NcchHeader::Flag7_NoMountRomFS;
		exh.accessCtrlInfo.localCaps11.storageInfo.flags |= le_dword(StorageInfo::NoRomFS);
	}

	exh.sysCtrlInfo.text.address     = le_word(ldr.codeAddr);
	exh.sysCtrlInfo.text.nPages      = le_word(ldr.codePages);
	exh.sysCtrlInfo.text.sizeBytes   = le_word(ldr.codeSize);
	exh.sysCtrlInfo.rodata.address   = le_word(ldr.rodataAddr);
	exh.sysCtrlInfo.rodata.nPages    = le_word(ldr.rodataPages);
	exh.sysCtrlInfo.rodata.sizeBytes = le_word(ldr.rodataSize);
	exh.sysCtrlInfo.data.address     = le_word(ldr.dataAddr);
	exh.sysCtrlInfo.data.nPages      = le_word(ldr.dataPages);
	exh.sysCtrlInfo.data.sizeBytes   = le_word(ldr.dataSize);
	exh.sysCtrlInfo.bssSize          = le_word(ldr.bssSize);

	if (ldr.smdh) icnPresent = true;

	// Unb0rk broken SMDHs generated by old versions of smdhtool
	if (ldr.smdh && !icnData.hdr.settings.flags)
	{
		smdh_settings& settings            = icnData.hdr.settings;
		settings.flags                     = le_word(SMDH_VISIBLE | SMDH_RECORD_USAGE | SMDH_REGION_RATING_USED);
		settings.region_lockout            = le_word(REGION_ALL);
		settings.ratings[RATING_CERO]      = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_ESRB]      = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_USK]       = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_PEGI_GEN]  = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_PEGI_PRT]  = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_PEGI_BBFC] = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_COB]       = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_GRB]       = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
		settings.ratings[RATING_CGSRR]     = RATING_FLAG_ENABLED | RATING_FLAG_NO_RESTRICTION;
	}

	// Disable savegame backup if there's no savegame
	if (ldr.smdh && !exh.sysCtrlInfo.saveSize)
		icnData.hdr.settings.flags |= le_word(SMDH_DISABLE_SAVE_DATA_BACKUP);

	return 0;
}

int CxiBuilder::ReadBanner(const char* f)
{
	FileClass fin(f, "rb");
	if (fin.openerror())
	{
		fprintf(stderr, "Cannot open banner file: %s\n", f);
		return 1;
	}
	fin.Seek(0, SEEK_END);
	bnrSize = fin.Tell();
	fin.Seek(0, SEEK_SET);
	bnrData = malloc(bnrSize);
	if (!bnrData)
	{
		fprintf(stderr, "Out of memory\n");
		return 2;
	}
	fin.ReadRaw(bnrData, bnrSize);
	return 0;
}

int CxiBuilder::FinishConfig(void)
{
	int cur;
	if (!specified.services && !isFirmCxi)
	{
		static const char* const services[] = { "APT:U", "ac:u", "am:net", "boss:U", "cam:u", "cecd:u",
			"cfg:nor", "cfg:u", "csnd:SND", "dsp::DSP", "frd:u", "fs:USER", "gsp::Gpu", "hid:USER",
			"http:C", "ir:rst", "ir:u", "ir:USER", "mic:u", "ndm:u", "news:u", "nwm::UDS", "ptm:u",
			"pxi:dev", "soc:U", "ssl:C", "y2r:u", NULL };
		for (int i = 0; services[i]; i ++)
			AddService(services[i]);
	}

	if (!specified.syscalls)
	{
		static const u8 syscalls[] =
		{
			0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0F, 0x11,
			0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,
			0x21, 0x22, 0x23, 0x24, 0x25, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x32, 0x33, 0x34,
			0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3C, 0x3D, 0x3E, 0x47, 0x48, 0x49, 0x4A, 0x4F, 0x50,
			0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66,
			0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x70, 0x71, 0x72, 0x73, 0x75, 0x76, 0x77, 0x78,
			0x79, 0x7B, 0x7C, 0x7D, 0xFF
		};
		for (int i = 0; syscalls[i] != 0xFF; i ++)
			EnableSysCall(syscalls[i]);
	}

	if (!specified.mappings)
	{
		mapping.push_back(Arm11KernelCaps::MakeMapping(0x1FF50000, true, false));
		mapping.push_back(Arm11KernelCaps::MakeMapping(0x1FF58000, true, false));
		mapping.push_back(Arm11KernelCaps::MakeMapping(0x1FF70000, true, false));
		mapping.push_back(Arm11KernelCaps::MakeMapping(0x1FF78000, true, false));
		mapping.push_back(Arm11KernelCaps::MakeMapping(0x1F000000, true, true));
		mapping.push_back(Arm11KernelCaps::MakeMapping(0x1F600000, true, true));
	}

	// Write dependencies
	if (deps.size() > 48)
	{
		fprintf(stderr, "Too many dependencies: %u\n", deps.size());
		return 1;
	}
	cur = 0;
	for (std::set<u64>::iterator it = deps.begin(); it != deps.end(); ++it)
		exh.sysCtrlInfo.dependencies[cur++] = le_dword(*it);

	// Write services
	if (services.size() > 34)
	{
		fprintf(stderr, "Too many services: %u\n", services.size());
		return 1;
	}
	cur = 0;
	for (std::set<std::string>::iterator it = services.begin(); it != services.end(); ++it)
		strncpy(exh.accessCtrlInfo.localCaps11.allowedServices[cur++], it->c_str(), 8);

	// Write kernel caps
	cur = 0;
	u32* caps = exh.accessCtrlInfo.kernelCaps11.caps;
	caps[cur++] = Arm11KernelCaps::MakeKernelVer(kverMajor, kverMinor);
	caps[cur++] = Arm11KernelCaps::MakeKernelFlags(exhkflags);
	caps[cur++] = Arm11KernelCaps::MakeHandleTableSize(handleTableSize);
	for (int i = 0; i < 8; i ++)
		if (svcAccess[i])
			caps[cur++] = Arm11KernelCaps::MakeSvcList(i, svcAccess[i]);
	for (size_t i = 0; i < mapping.size(); i ++)
		caps[cur++] = mapping[i];

	// Write system saves
	if (sysSaves.size() > 2)
	{
		fprintf(stderr, "Too many system saves: %u\n", sysSaves.size());
		return 1;
	}
	cur = 0;
	for (std::set<u32>::iterator it = sysSaves.begin(); it != sysSaves.end(); ++it)
		exh.accessCtrlInfo.localCaps11.storageInfo.systemSaveIds[cur++] = *it;

	// Write other saves
	if (otherSaves.size() > 6 || (exh.accessCtrlInfo.localCaps11.storageInfo.extdataId && otherSaves.size() > 3))
	{
		fprintf(stderr, "Too many other saves: %u\n", otherSaves.size());
		return 1;
	}
	cur = 0;
	for (std::set<u32>::iterator it = otherSaves.begin(); it != otherSaves.end(); ++it)
	{
		if (cur < 3)
			exh.accessCtrlInfo.localCaps11.storageInfo.storageAccessableId |= le_dword((u64)*it << (20*cur));
		else
		{
			exh.accessCtrlInfo.localCaps11.storageInfo.flags |= le_dword(StorageInfo::ExtendedSaveDataAccess);
			exh.accessCtrlInfo.localCaps11.storageInfo.extdataId |= le_dword((u64)*it << (20*(cur-3)));
		}
	}

	// Create second access control info
	memcpy(&exh.accessCtrlInfo2, &exh.accessCtrlInfo, sizeof(exh.accessCtrlInfo));
	exh.accessCtrlInfo2.localCaps11.flags &= ~3;
	exh.accessCtrlInfo2.localCaps11.flags |= BIT(exh.accessCtrlInfo.localCaps11.flags&3);
	exh.accessCtrlInfo2.localCaps11.priority = 0;

	// Hash and sign ExHeader
	u8 hash[Crypto::kSha256HashLen];
	memcpy(exh.ncchPubKey, s_dummyRsaKey.modulus, Crypto::kRsa2048Size);
	Crypto::Sha256(exh.ncchPubKey, sizeof(exh.ncchPubKey)+sizeof(exh.accessCtrlInfo2), hash);
	safe_call(Crypto::SignRsa2048Sha256(s_dummyRsaKey.modulus, s_dummyRsaKey.priv_exponent, hash, exh.accessDescSig));
	Crypto::Sha256((const u8*)&exh, sizeof(ExHeaderProper), hdr.exheaderHash);
	hdr.exheaderSizeBytes = le_word(sizeof(ExHeaderProper));

	return 0;
}

static void AlignToMediaUnits(FileClass& f)
{
	static const u8 zeros[NcchHeader::kMediaUnitSize] = {0};
	u32 pos = f.Tell();
	u32 rem = pos % sizeof(zeros);
	if (!rem) return;
	f.WriteRaw(zeros, sizeof(zeros)-rem);
}

int CxiBuilder::Write(const char* f)
{
	FileClass fout(f, "wb");
	if (fout.openerror())
	{
		fprintf(stderr, "Cannot open output file: %s\n", f);
		return 1;
	}

	// Calculate offsets
	u64 romfsSizes[4];
	u32 layout = (sizeof(NcchHeader)+sizeof(ExHeader)) / NcchHeader::kMediaUnitSize;
	hdr.exefsOffset = le_word(layout);
	if (exh.accessCtrlInfo.localCaps11.resourceLimitType == Arm11LocalCaps::Application)
	{
		memcpy(hdr.logoHash, exefs.fileHashes[ExeFSHeader::kMaxFiles-1-1], Crypto::kSha256HashLen);
		hdr.logoOffset = le_word(layout + (sizeof(ExeFSHeader) + le_word(exefs.files[1].offset)) / NcchHeader::kMediaUnitSize);
		hdr.logoSize = le_word(le_word(exefs.files[1].size) / NcchHeader::kMediaUnitSize);
	}
	layout += le_word(hdr.exefsSize);
	if (romfsLevel3)
	{
		hdr.romfsOffset = le_word(layout);
		safe_call(CalculateRomFSSize(romfsSizes));
		layout += le_word(hdr.romfsSize);
	}
	hdr.contentSize = le_word(layout);

	// Write placeholder NCCH header
	fout.WriteRaw(&hdr, sizeof(hdr));

	// Write ExHeader
	fout.WriteRaw(&exh, sizeof(exh));

	// Write ExeFS
	fout.WriteRaw(&exefs, sizeof(exefs));
	fout.WriteRaw(codeData, codeSize);
	AlignToMediaUnits(fout);
	if (exh.accessCtrlInfo.localCaps11.resourceLimitType == Arm11LocalCaps::Application)
		fout.WriteRaw(ExeFSHeader::s_homebrewLogo, sizeof(ExeFSHeader::s_homebrewLogo));
	if (icnPresent)
	{
		fout.WriteRaw(&icnData, sizeof(icnData));
		AlignToMediaUnits(fout);
	}
	if (bnrData)
	{
		fout.WriteRaw(bnrData, bnrSize);
		AlignToMediaUnits(fout);
	}

	// Write RomFS
	if (romfsLevel3)
		safe_call(WriteAndHashRomFS(fout, romfsSizes));

	// Sign and write final NCCH header
	u8 hash[Crypto::kSha256HashLen];
	Crypto::Sha256((const u8*)&hdr.magic, sizeof(hdr)-offsetof(NcchHeader,magic), hash);
	safe_call(Crypto::SignRsa2048Sha256(s_dummyRsaKey.modulus, s_dummyRsaKey.priv_exponent, hash, hdr.signature));
	fout.Seek(0, SEEK_SET);
	fout.WriteRaw(&hdr, sizeof(hdr));

	return 0;
}
