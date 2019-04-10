#include "3dsx_loader.h"

typedef struct
{
	void* segPtrs[3]; // code, rodata & data
	u32 segAddrs[3];
	u32 segSizes[3];
} _3DSX_LoadInfo;

typedef struct
{
	// offset and size of smdh
	u32 smdhOffset, smdhSize;
	// offset to filesystem
	u32 fsOffset;
} _3DSX_ExtendedHeader;

static inline u32 TranslateAddr(u32 addr, _3DSX_LoadInfo* d, u32* offsets)
{
	if (addr < offsets[0])
		return d->segAddrs[0] + addr;
	if (addr < offsets[1])
		return d->segAddrs[1] + addr - offsets[0];
	return d->segAddrs[2] + addr - offsets[1];
}

int Load3DSX(Loaded3DSX& out, FILE* f, u32 baseAddr, bool pagePad)
{
	u32 i, j, k, m;

	_3DSX_Header hdr;
	if (fread(&hdr, sizeof(hdr), 1, f) != 1)
		return 2;

	// Endian swap!
#define ESWAP(_field, _type) \
	hdr._field = le_##_type(hdr._field)
	ESWAP(magic, word);
	ESWAP(headerSize, hword);
	ESWAP(relocHdrSize, hword);
	ESWAP(formatVer, word);
	ESWAP(flags, word);
	ESWAP(codeSegSize, word);
	ESWAP(rodataSegSize, word);
	ESWAP(dataSegSize, word);
	ESWAP(bssSize, word);
#undef ESWAP

	if (hdr.magic != _3DSX_MAGIC)
		return 3;

	_3DSX_LoadInfo d;
	d.segSizes[0] = (hdr.codeSegSize+0xFFF) &~ 0xFFF;
	d.segSizes[1] = (hdr.rodataSegSize+0xFFF) &~ 0xFFF;
	d.segSizes[2] = (hdr.dataSegSize+0xFFF) &~ 0xFFF;
	u32 offsets[2] = { d.segSizes[0], d.segSizes[0] + d.segSizes[1] };
	u32 dataLoadSize = (hdr.dataSegSize-hdr.bssSize+0xFFF) &~ 0xFFF;
	u32 nRelocTables = hdr.relocHdrSize/4;
	u32 allocSize;
	if (pagePad)
		allocSize = d.segSizes[0]+d.segSizes[1]+d.segSizes[2]+4*3*nRelocTables;
	else
		allocSize = hdr.codeSegSize+hdr.rodataSegSize+hdr.dataSegSize+4*3*nRelocTables;
	void* allMem = malloc(allocSize);
	if (!allMem)
		return 3;
	d.segAddrs[0] = baseAddr;
	d.segAddrs[1] = d.segAddrs[0] + d.segSizes[0];
	d.segAddrs[2] = d.segAddrs[1] + d.segSizes[1];
	d.segPtrs[0] = (char*)allMem;
	if (pagePad)
	{
		d.segPtrs[1] = (char*)d.segPtrs[0] + d.segSizes[0];
		d.segPtrs[2] = (char*)d.segPtrs[1] + d.segSizes[1];
	} else
	{
		d.segPtrs[1] = (char*)d.segPtrs[0] + hdr.codeSegSize;
		d.segPtrs[2] = (char*)d.segPtrs[1] + hdr.rodataSegSize;
	}

	// Skip header for future compatibility.
	fseek(f, hdr.headerSize, SEEK_SET);

	// Read the relocation headers
	u32* relocs = (u32*)((char*)d.segPtrs[2] + hdr.dataSegSize);

	for (i = 0; i < 3; i ++)
		if (fread(&relocs[i*nRelocTables], nRelocTables*4, 1, f) != 1)
			return 4;

	// Read the segments
	if (fread(d.segPtrs[0], hdr.codeSegSize, 1, f) != 1) return 5;
	if (fread(d.segPtrs[1], hdr.rodataSegSize, 1, f) != 1) return 5;
	if (fread(d.segPtrs[2], hdr.dataSegSize - hdr.bssSize, 1, f) != 1) return 5;

	// BSS clear
	memset((char*)d.segPtrs[2] + hdr.dataSegSize - hdr.bssSize, 0, hdr.bssSize);

	// Relocate the segments
	for (i = 0; i < 3; i ++)
	{
		for (j = 0; j < nRelocTables; j ++)
		{
			u32 nRelocs = le_word(relocs[i*nRelocTables+j]);
			if (j >= 2)
			{
				// We are not using this table - ignore it
				fseek(f, nRelocs*sizeof(_3DSX_Reloc), SEEK_CUR);
				continue;
			}

#define RELOCBUFSIZE 512
			static _3DSX_Reloc relocTbl[RELOCBUFSIZE];

			u32* pos = (u32*)d.segPtrs[i];
			u32* endPos = pos + (d.segSizes[i]/4);

			while (nRelocs)
			{
				u32 toDo = nRelocs > RELOCBUFSIZE ? RELOCBUFSIZE : nRelocs;
				nRelocs -= toDo;

				if (fread(relocTbl, toDo*sizeof(_3DSX_Reloc), 1, f) != 1)
					return 6;

				for (k = 0; k < toDo && pos < endPos; k ++)
				{
					//printf("(t=%d,skip=%u,patch=%u)\n", j, (u32)relocTbl[k].skip, (u32)relocTbl[k].patch);
					pos += le_hword(relocTbl[k].skip);
					u32 num_patches = le_hword(relocTbl[k].patch);
					for (m = 0; m < num_patches && pos < endPos; m ++)
					{
						u32 inAddr = baseAddr + ((char*)pos-(char*)allMem);
						u32 origData = le_word(*pos);
						u32 subType = origData >> (32-4);
						u32 addr = TranslateAddr(origData &~ 0xF0000000, &d, offsets);
						//printf("Patching %08X <-- rel(%08X,%d,%u) (%08X)\n", baseAddr+inAddr, addr, j, subType, le_word(*pos));
						switch (j)
						{
							case 0:
							{
								if (subType != 0)
									return 7;
								*pos = le_word(addr);
								break;
							}
							case 1:
							{
								u32 data = addr - inAddr;
								switch (subType)
								{
									case 0: *pos = le_word(data);            break; // 32-bit signed offset
									case 1: *pos = le_word(data &~ BIT(31)); break; // 31-bit signed offset
									default: return 8;
								}
								break;
							}
						}
						pos++;
					}
				}
			}
		}
	}

	out.code = allMem;
	out.romfsLevel3Offset = 0;
	out.romfsLevel3Size = 0;
	out.codeAddr = d.segAddrs[0];
	out.codeSize = hdr.codeSegSize;
	out.codePages = d.segSizes[0] / 0x1000;
	out.rodataAddr = d.segAddrs[1];
	out.rodataSize = hdr.rodataSegSize;
	out.rodataPages = d.segSizes[1] / 0x1000;
	out.dataAddr = d.segAddrs[2];
	out.dataSize = hdr.dataSegSize-hdr.bssSize;
	out.dataPages = dataLoadSize / 0x1000;
	out.bssSize = hdr.bssSize;

	if (hdr.headerSize >= (sizeof(_3DSX_Header)+sizeof(_3DSX_ExtendedHeader)))
	{
		_3DSX_ExtendedHeader exhdr;
		fseek(f, sizeof(_3DSX_Header), SEEK_SET);
		fread(&exhdr, 1, sizeof(exhdr), f);
#define ESWAP(_field, _type) \
		exhdr._field = le_##_type(exhdr._field)
		ESWAP(smdhOffset, word);
		ESWAP(smdhSize, word);
		ESWAP(fsOffset, word);
#undef ESWAP

		if (exhdr.smdhOffset && out.smdh)
		{
			if (exhdr.smdhSize != sizeof(smdh_file))
				return 9;
			fseek(f, exhdr.smdhOffset, SEEK_SET);
			fread(out.smdh, 1, sizeof(smdh_file), f);
		} else
			out.smdh = NULL;

		if (exhdr.fsOffset)
		{
			out.romfsLevel3Offset = exhdr.fsOffset;
			fseek(f, 0, SEEK_END);
			out.romfsLevel3Size = ftello(f) - exhdr.fsOffset;
		}
	} else
		out.smdh = NULL;

	return 0; // Success.
}
