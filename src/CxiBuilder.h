#pragma once
#include <set>
#include <vector>
#include <string>
#include "ncch.h"
#include "smdh.h"
#include "FileClass.h"

class CxiBuilder
{
	NcchHeader hdr;
	ExHeader exh;

	u32 exhkflags;
	u8 kverMajor, kverMinor;
	u32 svcAccess[8];
	std::set<u64> deps;
	std::set<std::string> services;
	std::vector<u32> mapping;
	u32 handleTableSize;
	int saveIdCount;
	std::set<u32> sysSaves;
	std::set<u32> otherSaves;

	struct
	{
		bool services;
		bool syscalls;
		bool mappings;
	} specified;

	bool isFirmCxi;
	ExeFSHeader exefs;
	void* codeData;
	u32 codeSize;
	void* bnrData;
	u32 bnrSize;
	bool icnPresent;
	smdh_file icnData;

	FILE* romfsLevel3;
	u32 romfsLevel3Offset;
	u64 romfsLevel3Size;
	union
	{
		u8* romfsHashes;
		IVFCHeader* ivfc;
	};
	u8* romfsLevels[3];

	void AddService(const std::string& service);
	int AddFsPermission11(const std::string& permission);
	int AddFsPermission9(const std::string& permission);
	int AddKernelFlag(const std::string& flag);

	int CalculateRomFSSize(u64 size[4]);
	int WriteAndHashRomFS(FileClass& f, u64 size[4]);

public:
	CxiBuilder(const char* procName, const char* prodCode, u64 tid, u16 version);
	~CxiBuilder();
	int Read3DSX(const char* f);
	int ReadSettings(const char* f);
	int ReadBanner(const char* f);
	int FinishConfig(void);
	int BuildExeFS(void);
	int Write(const char* f);

	void EnableSysCall(int id)
	{
		if (id >= 0 && id < 0xC0)
			svcAccess[id/24] |= BIT(id%24);
	}
};
