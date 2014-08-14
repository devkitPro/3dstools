#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <map>
#include <algorithm>
#include "types.h"
#include "elf.h"
#include "FileClass.h"

using std::vector;
using std::map;

#define die(msg) do { fputs(msg "\n\n", stderr); return 1; } while(0)
#define safe_call(a) do { int rc = a; if(rc != 0) return rc; } while(0)

#ifdef WIN32
static inline void FixMinGWPath(char* buf)
{
	if (*buf == '/')
	{
		buf[0] = buf[1];
		buf[1] = ':';
	}
}
#endif

struct RelocEntry
{
	u16 skip, patch;
};

struct SegConv
{
	u32 fileOff, flags, memSize, fileSize, memPos;
};

struct RelocHdr
{
	u32 cAbsolute;
	u32 cRelative;
};

struct SymConv
{
	const char* name;
	u32 addr;
	bool isFunc;

	inline SymConv(const char* n, u32 a, bool i) : name(n), addr(a), isFunc(i) { }
};

class ElfConvert
{
	FileClass fout;
	byte_t* img;
	int platFlags;

	Elf32_Shdr* elfSects;
	int elfSectCount;
	const char* elfSectNames;

	Elf32_Sym* elfSyms;
	int elfSymCount;
	const char* elfSymNames;

	u32 baseAddr, topAddr;

	vector<bool> absRelocMap, relRelocMap;
	vector<RelocEntry> relocData;

	RelocHdr relocHdr[3];

	u8 *codeSeg, *rodataSeg, *dataSeg;
	u32 codeSegSize, rodataSegSize, dataSegSize, bssSize;
	u32 rodataStart, dataStart;

	int ScanSections();

	int ScanRelocSection(u32 vsect, byte_t* sectData, Elf32_Sym* symTab, Elf32_Rel* relTab, int relCount);
	int ScanRelocations();

	void BuildRelocs(vector<bool>& map, int pos, int posEnd, u32& count);

	void SetReloc(u32 address, vector<bool>& map)
	{
		address = (address-baseAddr)/4;
		if (address >= map.size()) return;
		map[address] = true;
	}

public:
	ElfConvert(const char* f, byte_t* i, int x)
		: fout(f, "wb"), img(i), platFlags(x), elfSyms(nullptr)
		, absRelocMap(), relRelocMap()
		, relocData()
		, codeSeg(nullptr), rodataSeg(nullptr), dataSeg(nullptr)
		, codeSegSize(0), rodataSegSize(0), dataSegSize(0), bssSize(0)
	{
	}
	int Convert();
};

int ElfConvert::ScanRelocSection(u32 vsect, byte_t* sectData, Elf32_Sym* symTab, Elf32_Rel* relTab, int relCount)
{
	for (int i = 0; i < relCount; i ++)
	{
		auto rel = relTab + i;
		u32 relInfo = le_word(rel->r_info);
		int relType = ELF32_R_TYPE(relInfo);
		auto relSym = symTab + ELF32_R_SYM(relInfo);

		u32 relSymAddr = le_word(relSym->st_value);
		u32 relSrcAddr = le_word(rel->r_offset);
		auto& relSrc = *(u32*)(sectData + relSrcAddr - vsect);

		switch (relType)
		{
			// Notes:
			// R_ARM_TARGET2 is equivalent to R_ARM_REL32
			// R_ARM_PREL31 is an address-relative signed 31-bit offset

			case R_ARM_ABS32:
			case R_ARM_TARGET1:
			{
				if(relSrcAddr & 3)
					die("Unaligned relocation!");

				// Ignore unbound weak symbols (keep them 0)
				if (ELF32_ST_BIND(le_word(relSym->st_info)) == STB_WEAK && relSymAddr == 0) break;

				// Add relocation
				SetReloc(relSrcAddr, absRelocMap);
				break;
			}

			case R_ARM_REL32:
			case R_ARM_TARGET2:
			case R_ARM_PREL31:
			{
				if(relSrcAddr & 3)
					die("Unaligned relocation!");

				int relocOff = (int)relSrc - ((int)relSymAddr - (int)relSrcAddr);

				relSymAddr += relocOff;
				if (relSymAddr >= topAddr)
					die("Relocation to invalid address!");

				if (
					((relSymAddr < rodataStart) && !(relSrcAddr < rodataStart)) ||
					((relSymAddr >= rodataStart && relSymAddr < dataStart) && !(relSrcAddr >= rodataStart && relSrcAddr < dataStart)) ||
					((relSymAddr >= dataStart   && relSymAddr < topAddr)   && !(relSrcAddr >= dataStart   && relSrcAddr < topAddr))
					)
				{
#ifdef DEBUG
					printf("{CrossRelReloc} srcAddr=%08X target=%08X relocOff=%d\n", relSrcAddr, relSymAddr, relocOff);
#endif
					relSrc = relSymAddr; // Convert to absolute address
					SetReloc(relSrcAddr, relRelocMap); // Add relocation
				}

				break;
			}
		}
	}
	return 0;
}

int ElfConvert::ScanRelocations()
{
	for (int i = 0; i < elfSectCount; i ++)
	{
		auto sect = elfSects + i;
		auto sectType = le_word(sect->sh_type);
		if (sectType == SHT_RELA)
			die("Unsupported relocation section");
		else if (sectType != SHT_REL)
			continue;

		auto targetSect = elfSects + le_word(sect->sh_info);
		u32 vsect = le_word(targetSect->sh_addr);
		auto sectData = img + le_word(targetSect->sh_offset);

		auto symTab = (Elf32_Sym*)(img + le_word(elfSects[le_word(sect->sh_link)].sh_offset));
		auto relTab = (Elf32_Rel*)(img + le_word(sect->sh_offset));
		int relCount = (int)(le_word(sect->sh_size) / le_word(sect->sh_entsize));

		safe_call(ScanRelocSection(vsect, sectData, symTab, relTab, relCount));
	}

	// Scan for interworking thunks that need to be relocated
	for (int i = 0; i < elfSymCount; i ++)
	{
		auto sym = elfSyms + i;
		auto symName = (const char*)(elfSymNames + le_word(sym->st_name));
		if (!*symName) continue;
		if (symName[0] != '_' && symName[1] != '_') continue;
		if (strncmp(symName+strlen(symName)-9, "_from_arm", 9) != 0) continue;
		SetReloc(le_word(sym->st_value), absRelocMap);
	}

	// Build relocs
	BuildRelocs(absRelocMap, baseAddr/4, rodataStart/4, relocHdr[0].cAbsolute);
	BuildRelocs(relRelocMap, baseAddr/4, rodataStart/4, relocHdr[0].cRelative);
	BuildRelocs(absRelocMap, rodataStart/4, dataStart/4, relocHdr[1].cAbsolute);
	BuildRelocs(relRelocMap, rodataStart/4, dataStart/4, relocHdr[1].cRelative);
	BuildRelocs(absRelocMap, dataStart/4, topAddr/4, relocHdr[2].cAbsolute);
	BuildRelocs(relRelocMap, dataStart/4, topAddr/4, relocHdr[2].cRelative);

	return 0;
}

void ElfConvert::BuildRelocs(vector<bool>& map, int pos, int posEnd, u32& count)
{
	size_t curs = relocData.size();
	for (int i = pos; i < posEnd;)
	{
		RelocEntry reloc;
		u32 rs = 0, rp = 0;
		while ((i < posEnd) && !map[i]) i ++, rs ++;
		while ((i < posEnd) && map[i]) i ++, rp ++;

		// Remove empty trailing relocations
		if (i == posEnd && rs && !rp)
			break;

		// Add excess skip relocations
		for (reloc.skip = 0xFFFF, reloc.patch = 0; rs > 0xFFFF; rs -= 0xFFFF)
			relocData.push_back(reloc);

		// Add excess patch relocations
		for (reloc.skip = rs, reloc.patch = 0xFFFF; rp > 0xFFFF; rp -= 0xFFFF)
		{
			relocData.push_back(reloc);
			rs = reloc.skip = 0;
		}

		// Add remaining relocation
		if (rs || rp)
		{
			reloc.skip = rs;
			reloc.patch = rp;
			relocData.push_back(reloc);
		}
	}
	count = relocData.size() - curs;
}

int ElfConvert::ScanSections()
{
	for (int i = 0; i < elfSectCount; i ++)
	{
		auto sect = elfSects + i;
		//auto sectName = elfSectNames + le_word(sect->sh_name);
		switch (le_word(sect->sh_type))
		{
			case SHT_SYMTAB:
				elfSyms = (Elf32_Sym*) (img + le_word(sect->sh_offset));
				elfSymCount = le_word(sect->sh_size) / sizeof(Elf32_Sym);
				elfSymNames = (const char*)(img + le_word(elfSects[le_word(sect->sh_link)].sh_offset));
				break;
		}
	}

	if (!elfSyms)
		die("ELF has no symbol table!");

	return 0;
}

int ElfConvert::Convert()
{
	if (fout.openerror())
		die("Cannot open output file!");

	auto ehdr = (Elf32_Ehdr*) img;
	if(memcmp(ehdr->e_ident, ELF_MAGIC, 4) != 0)
		die("Invalid ELF file!");
	if(le_hword(ehdr->e_type) != ET_EXEC)
		die("ELF file must be executable! (hdr->e_type should be ET_EXEC)");

	elfSects = (Elf32_Shdr*)(img + le_word(ehdr->e_shoff));
	elfSectCount = (int)le_hword(ehdr->e_shnum);
	elfSectNames = (const char*)(img + le_word(elfSects[le_hword(ehdr->e_shstrndx)].sh_offset));

	auto phdr = (Elf32_Phdr*)(img + le_word(ehdr->e_phoff));
	baseAddr = 1, topAddr = 0;
	if (ehdr->e_phnum > 3)
		die("Too many segments!");
	for (int i = 0; i < ehdr->e_phnum; i ++)
	{
		auto cur = phdr + i;
		SegConv s;
		s.fileOff = le_word(cur->p_offset);
		s.flags = le_word(cur->p_flags);
		s.memSize = le_word(cur->p_memsz);
		s.fileSize = le_word(cur->p_filesz);
		s.memPos = le_word(cur->p_vaddr);

		if (!s.memSize) continue;

#ifdef DEBUG
		fprintf(stderr, "PHDR[%d]: fOff(%X) memPos(%08X) memSize(%u) fileSize(%u) flags(%08X)\n",
			i, s.fileOff, s.memPos, s.memSize, s.fileSize, s.flags);
#endif

		if (i == 0) baseAddr = s.memPos;
		else if (s.memPos != topAddr) die("Non-contiguous segments!");

		if (s.memSize & 3) die("The segments is not word-aligned!");
		if (s.flags != 6 && s.memSize != s.fileSize) die("Only the data segment can have a BSS!");
		if (s.fileSize & 3) die("The loadable part of the segment is not word-aligned!");

		switch (s.flags)
		{
			case 5: // code
				if (codeSeg) die("Too many code segments");
				if (rodataSeg || dataSeg) die("Code segment must be the first");
				codeSeg = img + s.fileOff;
				codeSegSize = s.memSize;
				break;
			case 4: // rodata
				if (rodataSeg) die("Too many rodata segments");
				if (dataSeg) die("Data segment must be before the code segment");
				rodataSeg = img + s.fileOff;
				rodataSegSize = s.memSize;
				break;
			case 6: // data+bss
				if (dataSeg) die("Too many data segments");
				dataSeg = img + s.fileOff;
				dataSegSize = s.memSize;
				bssSize = s.memSize - s.fileSize;
				break;
			default:
				die("Invalid segment!");
		}

		topAddr = s.memPos + s.memSize;
	}

	if (baseAddr != 0)
		die("Invalid executable base address!");

	if (le_word(ehdr->e_entry) != baseAddr)
		die("Entrypoint should be zero!");

	rodataStart = baseAddr + codeSegSize;
	dataStart = rodataStart + rodataSegSize;

	// Create relocation bitmap
	absRelocMap.assign((topAddr - baseAddr) / 4, false);
	relRelocMap.assign((topAddr - baseAddr) / 4, false);

	safe_call(ScanSections());
	safe_call(ScanRelocations());

	// Write header
	fout.WriteWord(0x58534433); // '3DSX'
	fout.WriteHword(8*4); // Header size
	fout.WriteHword(sizeof(RelocHdr)); // Relocation header size
	fout.WriteWord(0); // Version
	fout.WriteWord(0); // Flags

	fout.WriteWord(codeSegSize);
	fout.WriteWord(rodataSegSize);
	fout.WriteWord(dataSegSize);
	fout.WriteWord(bssSize);

	// Write relocation headers
	for (int i = 0; i < 3; i ++)
		fout.WriteRaw(relocHdr+i, sizeof(RelocHdr));

	// Write segments
	if (codeSeg)   fout.WriteRaw(codeSeg,   codeSegSize);
	if (rodataSeg) fout.WriteRaw(rodataSeg, rodataSegSize);
	if (dataSeg)   fout.WriteRaw(dataSeg,   dataSegSize-bssSize);

	// Write relocations
	for (auto& reloc : relocData)
	{
#ifdef DEBUG
		fprintf(stderr, "RELOC {skip: %d, patch: %d}\n", (int)reloc.skip, (int)reloc.patch);
#endif
		fout.WriteHword(reloc.skip);
		fout.WriteHword(reloc.patch);
	}

	return 0;
}

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		fprintf(stderr, "Usage:\n\t%s [inputFile] [outputFile]\n", argv[0]);
		return 1;
	}

#ifdef WIN32
	FixMinGWPath(argv[1]);
	FixMinGWPath(argv[2]);
#endif

	auto elf_file = fopen(argv[1], "rb");
	if (!elf_file) die("Cannot open input file!");

	fseek(elf_file, 0, SEEK_END);
	size_t elfSize = ftell(elf_file);
	rewind(elf_file);

	auto b = (byte_t*) malloc(elfSize);
	if (!b) { fclose(elf_file); die("Cannot allocate memory!"); }

	fread(b, 1, elfSize, elf_file);
	fclose(elf_file);

	int rc = 0;
	{
		ElfConvert cnv(argv[2], b, 0);
		rc = cnv.Convert();
	}
	free(b);

	if (rc != 0)
		remove(argv[2]);

	return rc;
}
