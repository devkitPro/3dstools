#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CxiBuilder.h"

#define safe_call(a) do { int rc = a; if(rc != 0) return rc; } while(0)

int CxiBuilder::CalculateRomFSSize(u64 size[4])
{
	// Calculate level sizes
	size[3] = romfsLevel3Size;
	size[2] = IVFCHeader::BlockAlign(size[3])/IVFCHeader::kBlockSize * Crypto::kSha256HashLen;
	size[1] = IVFCHeader::BlockAlign(size[2])/IVFCHeader::kBlockSize * Crypto::kSha256HashLen;
	size[0] = IVFCHeader::BlockAlign(size[1])/IVFCHeader::kBlockSize * Crypto::kSha256HashLen;

	// Calculate RomFS region size & update NCCH header
	u64 totalSize = IVFCHeader::BlockAlign(sizeof(*ivfc)+4+size[0]);
	for (int i = 1; i < 4; i ++)
		totalSize += IVFCHeader::BlockAlign(size[i]);
	hdr.romfsSize = le_word(totalSize / NcchHeader::kMediaUnitSize);
	hdr.romfsHashedSize = le_word(NcchHeader::MediaUnitAlign(sizeof(*ivfc)+4+size[0]) / NcchHeader::kMediaUnitSize);

	// Allocate mem for all RomFS levels except for 3
	romfsHashes = (u8*)calloc(totalSize-IVFCHeader::BlockAlign(size[3]),1);
	if (!romfsHashes)
	{
		fprintf(stderr, "Out of memory\n");
		return 1;
	}

	// Prepare level pointers
	romfsLevels[0] = romfsHashes + sizeof(*ivfc)+4;
	romfsLevels[1] = romfsHashes + IVFCHeader::BlockAlign(sizeof(*ivfc)+4+size[0]);
	romfsLevels[2] = romfsLevels[1] + IVFCHeader::BlockAlign(size[1]);

	// Create IVFC header
	ivfc->init();
	ivfc->masterHashSize = le_word(size[0]);
	ivfc->levels[0].size = size[1];
	ivfc->levels[1].size = size[2];
	ivfc->levels[2].size = size[3];
	u64 offset2 = IVFCHeader::BlockAlign(size[1]);
	u64 offset3 = IVFCHeader::BlockAlign(offset2 + size[2]);
	ivfc->levels[1].offset = offset2;
	ivfc->levels[2].offset = offset3;

	return 0;
}

static void HashLevel(u8* inptr, u8* hashptr, u64 hashsize)
{
	while (hashsize)
	{
		Crypto::Sha256(inptr, IVFCHeader::kBlockSize, hashptr);
		inptr += IVFCHeader::kBlockSize;
		hashptr += Crypto::kSha256HashLen;
		hashsize -= hashsize > IVFCHeader::kBlockSize ? IVFCHeader::kBlockSize : hashsize;
	}
}

int CxiBuilder::WriteAndHashRomFS(FileClass& f, u64 size[4])
{
	int pos = f.Tell();

	// Write placeholder header + level 0
	f.WriteRaw(romfsHashes, romfsLevels[1]-romfsHashes);

	static u8 readBuf[IVFCHeader::kBlockSize];
	u8* hashptr = romfsLevels[2];

	// Copy and hash level 3
	u64 remSize = romfsLevel3Size;
	fseek(romfsLevel3, romfsLevel3Offset, SEEK_SET);
	while (remSize)
	{
		u32 readSize = remSize > sizeof(readBuf) ? sizeof(readBuf) : remSize;
		if (fread(readBuf, readSize, 1, romfsLevel3) != 1)
		{
			fprintf(stderr, "Read error\n");
			return 1;
		}

		if (readSize < sizeof(readBuf))
			memset(readBuf + readSize, 0, sizeof(readBuf)-readSize);

		f.WriteRaw(readBuf, sizeof(readBuf));
		Crypto::Sha256(readBuf, sizeof(readBuf), hashptr);
		hashptr += Crypto::kSha256HashLen;
		remSize -= readSize;
	}

	// Hash level 2 into level 1 into level 0
	HashLevel(romfsLevels[2], romfsLevels[1], size[2]);
	HashLevel(romfsLevels[1], romfsLevels[0], size[1]);

	// Hash level 0
	Crypto::Sha256(romfsHashes, le_word(hdr.romfsHashedSize) * NcchHeader::kMediaUnitSize, hdr.romfsHash);

	// Write out level 1 and 2
	f.WriteRaw(romfsLevels[1], romfsLevels[2] + IVFCHeader::BlockAlign(size[2]) - romfsLevels[1]);

	// Write out final header + level 0
	int temp = f.Tell();
	f.Seek(pos, SEEK_SET);
	f.WriteRaw(romfsHashes, romfsLevels[1]-romfsHashes);
	f.Seek(temp, SEEK_SET);

	return 0;
}
