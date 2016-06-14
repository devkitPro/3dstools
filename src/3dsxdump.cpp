#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "FileClass.h"
#include "3dsx_loader.h"

int Dump3DSX(FILE* f, u32 baseAddr, FILE* fout)
{
	Loaded3DSX ldr;
	ldr.smdh = NULL;
	int rc = Load3DSX(ldr, f, baseAddr);
	if (rc != 0) return rc;

	// Write the data
	if (fwrite(ldr.code, 0x1000*(ldr.codePages+ldr.rodataPages+ldr.dataPages), 1, fout) != 1)
		return 9;
	free(ldr.code);

	printf("CODE:   %u pages\n", ldr.codePages);
	printf("RODATA: %u pages\n", ldr.rodataPages);
	printf("DATA:   %u pages\n", ldr.dataPages);
	printf("BSS:    %u pages\n", ((ldr.dataSize+ldr.bssSize+0xFFF)>>12)-ldr.dataPages);

	return 0; // Success.
}

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

	FILE* fin = fopen(argv[1], "rb");
	if (!fin) { printf("Cannot open input file!\n"); return 1; }

	FILE* fout = fopen(argv[2], "wb");
	if (!fout) { fclose(fin); printf("Cannot open output file!\n"); return 1; }

	int rc = Dump3DSX(fin, 0x00100000, fout);
	fclose(fin);
	fclose(fout);
	if (rc != 0)
	{
		remove(argv[2]);
		switch (rc)
		{
			case 2: printf("Cannot read header!\n"); break;
			case 3: printf("Invalid header!\n"); break;
			case 4: printf("Cannot read relocation headers!\n"); break;
			case 5: printf("Cannot read segment data!\n"); break;
			case 6: printf("Cannot read relocation table!\n"); break;
			case 7: printf("Cannot write segment data!\n"); break;
		}
	}

	return rc;
}
