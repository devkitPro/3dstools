#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "3dsx.h"
#include "smdh.h"

struct Loaded3DSX
{
	void* code;
	smdh_file* smdh;
	u64 romfsLevel3Size;
	u32 romfsLevel3Offset;

	u32 codeAddr;
	u32 codeSize;
	u32 codePages;
	u32 rodataAddr;
	u32 rodataSize;
	u32 rodataPages;
	u32 dataAddr;
	u32 dataSize;
	u32 dataPages;
	u32 bssSize;
};

int Load3DSX(Loaded3DSX& out, FILE* f, u32 baseAddr, bool pagePad = true);
