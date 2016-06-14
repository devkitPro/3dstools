#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "CxiBuilder.h"
#include "YamlReader.h"

#define safe_call(a) do { int rc = a; if(rc != 0) return rc; } while(0)

enum
{
	MODULE_SM = 0x10,
	MODULE_FS = 0x11,
	MODULE_PM = 0x12,
	MODULE_LOADER = 0x13,
	MODULE_PXI = 0x14,
	MODULE_AM = 0x15,
	MODULE_CAMERA = 0x16,
	MODULE_CONFIG = 0x17,
	MODULE_CODEC = 0x18,
	MODULE_DMNT = 0x19,
	MODULE_DSP = 0x1A,
	MODULE_GPIO = 0x1B,
	MODULE_GSP = 0x1C,
	MODULE_HID = 0x1D,
	MODULE_I2C = 0x1E,
	MODULE_MCU = 0x1F,
	MODULE_MIC = 0x20,
	MODULE_PDN = 0x21,
	MODULE_PTM = 0x22,
	MODULE_SPI = 0x23,
	MODULE_AC = 0x24,
	MODULE_CECD = 0x26,
	MODULE_CSND = 0x27,
	MODULE_DLP = 0x28,
	MODULE_HTTP = 0x29,
	MODULE_MP = 0x2A,
	MODULE_NDM = 0x2B,
	MODULE_NIM = 0x2C,
	MODULE_NWM = 0x2D,
	MODULE_SOCKET = 0x2E,
	MODULE_SSL = 0x2F,
	MODULE_PROC9 = 0x30,
	MODULE_PS = 0x31,
	MODULE_FRIENDS = 0x32,
	MODULE_IR = 0x33,
	MODULE_BOSS = 0x34,
	MODULE_NEWS = 0x35,
	MODULE_DEBUGGER = 0x36,
	MODULE_RO = 0x37,
	MODULE_ACT = 0x38,
	MODULE_NFC = 0x40,
	MODULE_MVD = 0x41,
	MODULE_QTM = 0x42,
	MODULE_HAX = 0x99,
};

static inline u64 makeModuleTid(int id, bool n3ds)
{
	return 0x0004013000000000ULL | (n3ds ? 0x20000000 : 0) | (id<<8) | 2;
}

static inline bool beginsWith(const std::string& a, const std::string& b)
{
	return a.compare(0, b.length(), b) == 0;
}

static inline bool endsWith(const std::string& a, const std::string& b)
{
	size_t la = a.length(), lb = b.length();
	return la >= lb && a.compare(la-lb, lb, b) == 0;
}

static inline bool strToBool(const std::string& a)
{
	return a == "true" || a == "True" || a == "1";
}

void CxiBuilder::AddService(const std::string& service)
{
	if (!services.insert(service).second)
		return;
	else if (beginsWith(service, "am:"))
		deps.insert(makeModuleTid(MODULE_AM, false));
	else if (beginsWith(service, "nim:"))
		deps.insert(makeModuleTid(MODULE_NIM, false));
	else if (beginsWith(service, "cfg:"))
		deps.insert(makeModuleTid(MODULE_CONFIG, false));
	else if (service == "ldr:ro")
		deps.insert(makeModuleTid(MODULE_RO, false));
	else if (beginsWith(service, "ndm:"))
		deps.insert(makeModuleTid(MODULE_NDM, false));
	else if (beginsWith(service, "csnd:"))
		deps.insert(makeModuleTid(MODULE_CSND, false));
	else if (beginsWith(service, "cam:"))
		deps.insert(makeModuleTid(MODULE_CAMERA, false)); //???
	else if (beginsWith(service, "y2r:"))
		deps.insert(makeModuleTid(MODULE_CAMERA, true)); //???
	else if (beginsWith(service, "cdc:"))
		deps.insert(makeModuleTid(MODULE_CODEC, false));
	else if (beginsWith(service, "dlp:"))
		deps.insert(makeModuleTid(MODULE_DLP, false));
	else if (beginsWith(service, "dsp::"))
		deps.insert(makeModuleTid(MODULE_DSP, false));
	else if (beginsWith(service, "gsp::"))
		deps.insert(makeModuleTid(MODULE_GSP, false));
	else if (beginsWith(service, "boss:"))
		deps.insert(makeModuleTid(MODULE_BOSS, false));
	else if (beginsWith(service, "cecd:"))
		deps.insert(makeModuleTid(MODULE_CECD, false));
	else if (beginsWith(service, "ir:"))
		deps.insert(makeModuleTid(MODULE_IR, false));
	else if (beginsWith(service, "i2c::"))
		deps.insert(makeModuleTid(MODULE_I2C, false));
	else if (beginsWith(service, "gpio:"))
		deps.insert(makeModuleTid(MODULE_GPIO, false));
	else if (beginsWith(service, "hid:"))
		deps.insert(makeModuleTid(MODULE_HID, false));
	else if (beginsWith(service, "ptm:"))
		deps.insert(makeModuleTid(MODULE_PTM, false));
	else if (beginsWith(service, "nwm::"))
		deps.insert(makeModuleTid(MODULE_NWM, false));
	else if (beginsWith(service, "http:"))
		deps.insert(makeModuleTid(MODULE_HTTP, false));
	else if (beginsWith(service, "ssl:"))
		deps.insert(makeModuleTid(MODULE_SSL, false));
	else if (beginsWith(service, "soc:"))
		deps.insert(makeModuleTid(MODULE_SOCKET, false));
	else if (beginsWith(service, "ac:"))
		deps.insert(makeModuleTid(MODULE_AC, false));
	else if (beginsWith(service, "frd:"))
		deps.insert(makeModuleTid(MODULE_FRIENDS, false));
	else if (beginsWith(service, "news:"))
		deps.insert(makeModuleTid(MODULE_NEWS, false));
	else if (beginsWith(service, "pdn:"))
		deps.insert(makeModuleTid(MODULE_PDN, false));
	else if (beginsWith(service, "SPI::"))
		deps.insert(makeModuleTid(MODULE_SPI, false));
	else if (beginsWith(service, "mcu::"))
		deps.insert(makeModuleTid(MODULE_MCU, false));
	else if (beginsWith(service, "mic:"))
		deps.insert(makeModuleTid(MODULE_MIC, false));
	else if (beginsWith(service, "act:"))
		deps.insert(makeModuleTid(MODULE_ACT, false));
	else if (beginsWith(service, "nfc:"))
		deps.insert(makeModuleTid(MODULE_NFC, false)); //New3DS?
	else if (beginsWith(service, "mvd:"))
		deps.insert(makeModuleTid(MODULE_MVD, true));
	else if (beginsWith(service, "qtm:"))
		deps.insert(makeModuleTid(MODULE_QTM, true));
	else if (beginsWith(service, "hax:") || beginsWith(service, "hb:"))
		deps.insert(makeModuleTid(MODULE_HAX, false));
}

int CxiBuilder::AddFsPermission11(const std::string& permission)
{
	if (0) ((void)0);
#define PERMISSION(_name) \
	else if (permission == #_name) do { exh.accessCtrlInfo.localCaps11.storageInfo.flags |= le_dword(StorageInfo::_name); return 0; } while(0)
	PERMISSION(CategorySystemApp);
	PERMISSION(CategoryHardwareCheck);
	PERMISSION(CategoryFileSystemTool);
	PERMISSION(Debug);
	PERMISSION(CanAccessTwlCardBackup);
	PERMISSION(CanAccessTwlNand);
	PERMISSION(SpotPass);
	PERMISSION(CanAccessSD);
	PERMISSION(Core);
	PERMISSION(CanAccessCtrNandRO);
	PERMISSION(CanAccessCtrNandRW);
	PERMISSION(CanWriteToCtrNandRO);
	PERMISSION(CategorySystemSettings);
	PERMISSION(SystemTransfer);
	PERMISSION(ExportImportIVS);
	PERMISSION(CanAccessWriteOnlySD);
	PERMISSION(SwitchCleanup);
	PERMISSION(SaveDataMove);
	PERMISSION(Shop);
	PERMISSION(Shell);
	PERMISSION(CategoryHomeMenu);
#undef PERMISSION
	else
	{
		fprintf(stderr, "[ERROR] Unknown ARM11 filesystem permission: %s\n", permission.c_str());
		return 1;
	}
}

int CxiBuilder::AddFsPermission9(const std::string& permission)
{
	if (0) ((void)0);
#define PERMISSION(_name) \
	else if (permission == #_name) do { exh.accessCtrlInfo.localCaps9.descriptor |= le_word(Arm9LocalCaps::_name); return 0; } while(0)
	PERMISSION(MountNand);
	PERMISSION(MountNandRO);
	PERMISSION(MountTwln);
	PERMISSION(MountWnand);
	PERMISSION(MountCardSPI);
	PERMISSION(UseSDIF3);
	PERMISSION(CreateSeed);
	PERMISSION(UseCardSPI);
	PERMISSION(SDApplication);
	PERMISSION(MountSdmcWO);
#undef PERMISSION
	else
	{
		fprintf(stderr, "[ERROR] Unknown ARM9 filesystem permission: %s\n", permission.c_str());
		return 1;
	}
}

int CxiBuilder::AddKernelFlag(const std::string& flag)
{
	if (0) ((void)0);
#define FLAG(_name) \
	else if (flag == #_name) do { exhkflags |= Arm11KernelCaps::_name; return 0; } while(0)
	FLAG(AllowDebug);
	FLAG(ForceDebug);
	FLAG(AllowNonAlphanum);
	FLAG(SharedPageWriting);
	FLAG(PrivilegePriority);
	FLAG(AllowMainArgs);
	FLAG(SharedDevMem);
	FLAG(RunnableOnSleep);
	FLAG(SpecialMemory);
	FLAG(CanUseCore2);
#undef FLAG
	else
	{
		fprintf(stderr, "[ERROR] Unknown ARM11 kernel flag: %s\n", flag.c_str());
		return 1;
	}
}

int CxiBuilder::ReadSettings(const char* f)
{
	std::vector<std::string> tmp(1);
	std::string tmpVal;

	YamlReader spec;
	safe_call(spec.LoadFile(f));

	u32 level = spec.level();
	while (spec.GetEvent() && spec.level()==level)
	{
		if (!spec.is_event_scalar())
			continue;
		std::string sectionName = spec.event_string();
		if (sectionName == "General")
		{
			spec.GetEvent();
			u32 level = spec.level();
			while (spec.GetEvent() && spec.level() >= level)
			{
				if (!spec.is_event_scalar())
					continue;
				std::string settingName = spec.event_string();
				safe_call(spec.SaveValue(tmpVal));
				if (settingName == "Version")
					exh.sysCtrlInfo.remasterVersion = le_hword(strtoul(tmpVal.c_str(), NULL, 0));
				else if (settingName == "Platform")
				{
					if (tmpVal == "Old3DS")
					{
						hdr.flags[4] = NcchHeader::Flag4_Old3DS;
						exh.accessCtrlInfo.localCaps11.firmTidLow &= le_word(~0x20000000);
					} else if (tmpVal == "New3DS")
					{
						hdr.flags[4] = NcchHeader::Flag4_New3DS;
						exh.accessCtrlInfo.localCaps11.firmTidLow |= le_word(0x20000000);
					} else
					{
						fprintf(stderr, "[ERROR] Unknown platform: %s\n", tmpVal.c_str());
						return 1;
					}
				}
				else if (settingName == "TitleType")
				{
					exh.sysCtrlInfo.flag &= ~SystemControlInfo::Flag_IsSD;
					exh.accessCtrlInfo.localCaps11.flags &= ~Arm11LocalCaps::MakeFlags(0,3,3);
					exh.accessCtrlInfo.localCaps9.descriptor &= le_word(~Arm9LocalCaps::SDApplication);
					exhkflags &= ~Arm11KernelCaps::MemType_Mask;
					isFirmCxi = false;
					if (tmpVal == "Application")
					{
						exh.sysCtrlInfo.flag |= SystemControlInfo::Flag_IsSD;
						exh.accessCtrlInfo.localCaps11.resourceLimitType = Arm11LocalCaps::Application;
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(0,1,0);
						exh.accessCtrlInfo.localCaps9.descriptor |= le_word(Arm9LocalCaps::SDApplication);
						exhkflags |= Arm11KernelCaps::MemType_Application;
					} else if (tmpVal == "SysApplet")
					{
						exh.accessCtrlInfo.localCaps11.resourceLimitType = Arm11LocalCaps::SysApplet;
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(0,1,0);
						exhkflags |= Arm11KernelCaps::MemType_System;
					} else if (tmpVal == "LibApplet")
					{
						exh.accessCtrlInfo.localCaps11.resourceLimitType = Arm11LocalCaps::LibApplet;
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(0,1,0);
						exhkflags |= Arm11KernelCaps::MemType_System;
					} else if (endsWith(tmpVal, "SysModule"))
					{
						exh.accessCtrlInfo.localCaps11.resourceLimitType = Arm11LocalCaps::SysModule;
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(0,2,1);
						exhkflags |= beginsWith(tmpVal, "Large") ? Arm11KernelCaps::MemType_System : Arm11KernelCaps::MemType_Base;
						if (beginsWith(tmpVal, "Firm"))
							isFirmCxi = true;
					} else
					{
						fprintf(stderr, "[ERROR] Unknown title type: %s\n", tmpVal.c_str());
						return 1;
					}
				}
				else if (settingName == "SystemMode")
				{
					exh.accessCtrlInfo.localCaps11.flags &= ~Arm11LocalCaps::MakeFlags(0xF,0,0);
					if (tmpVal == "64M")
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(Arm11LocalCaps::SystemMode_Normal,0,0);
					else if (tmpVal == "72M")
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(Arm11LocalCaps::SystemMode_Dev3,0,0);
					else if (tmpVal == "80M")
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(Arm11LocalCaps::SystemMode_Dev2,0,0);
					else if (tmpVal == "96M")
						exh.accessCtrlInfo.localCaps11.flags |= Arm11LocalCaps::MakeFlags(Arm11LocalCaps::SystemMode_Dev1,0,0);
					else
					{
						fprintf(stderr, "[ERROR] Unknown system mode: %s\n", tmpVal.c_str());
						return 1;
					}
				}
				else if (settingName == "NewSystemMode")
				{
					if (tmpVal == "Legacy")
						exh.accessCtrlInfo.localCaps11.systemModeExt = Arm11LocalCaps::SystemModeExt_Old3DS;
					else if (tmpVal == "124M")
						exh.accessCtrlInfo.localCaps11.systemModeExt = Arm11LocalCaps::SystemModeExt_New3DS;
					else if (tmpVal == "178M")
						exh.accessCtrlInfo.localCaps11.systemModeExt = Arm11LocalCaps::SystemModeExt_New3DS_Dev1;
					else
					{
						fprintf(stderr, "[ERROR] Unknown New3DS system mode: %s\n", tmpVal.c_str());
						return 1;
					}
				}
				else if (settingName == "NewSpeedup")
				{
					exh.accessCtrlInfo.localCaps11.extFlags = 0;
					if (strToBool(tmpVal))
						exh.accessCtrlInfo.localCaps11.extFlags = Arm11LocalCaps::ExtFlag_EnableFastCPU | Arm11LocalCaps::ExtFlag_EnableL2Cache;
				}
				else if (settingName == "StackSize")
					exh.sysCtrlInfo.stackSize = le_word(strtoul(tmpVal.c_str(), NULL, 0));
				else if (settingName == "HandleTblSize")
					handleTableSize = strtoul(tmpVal.c_str(), NULL, 0);
				else if (settingName == "MainPriority")
					exh.accessCtrlInfo.localCaps11.priority = strtoul(tmpVal.c_str(), NULL, 0);
				else if (settingName == "CompressCode")
				{
					exh.sysCtrlInfo.flag &= ~SystemControlInfo::Flag_IsCompressed;
					if (strToBool(tmpVal))
						exh.sysCtrlInfo.flag |= SystemControlInfo::Flag_IsCompressed;
				}
				else
				{
					fprintf(stderr, "[ERROR] Unknown general setting: %s\n", settingName.c_str());
					return 1;
				}
			}
		}
		else if (sectionName == "Savegame")
		{
			spec.GetEvent();
			u32 level = spec.level();
			while (spec.GetEvent() && spec.level() >= level)
			{
				if (!spec.is_event_scalar())
					continue;
				std::string settingName = spec.event_string();
				if (settingName == "SystemSaves")
				{
					safe_call(spec.SaveValueSequence(tmp));
					for (size_t i = 0; i < tmp.size(); i ++)
						sysSaves.insert(strtoul(tmp[i].c_str(), NULL, 16) & 0xFFFFF);
				}
				else if (settingName == "OtherSaves")
				{
					safe_call(spec.SaveValueSequence(tmp));
					for (size_t i = 0; i < tmp.size(); i ++)
						otherSaves.insert(strtoul(tmp[i].c_str(), NULL, 16) & 0xFFFFF);
				}
				else
				{
					safe_call(spec.SaveValue(tmpVal));
					if (settingName == "SaveSize")
					{
						char* end;
						u32 rawSize = strtoul(tmpVal.c_str(), &end, 0);
						if (*end == 'K' || *end == 'k')
							rawSize *= 1024;
						else if (*end == 'M' || *end == 'm')
							rawSize *= 1024*1024;
						if (rawSize & 0xFFFF)
						{
							fprintf(stderr, "[ERROR] Savedata size must be multiple of 64KB: %s\n", tmpVal.c_str());
							return 1;
						}
						exh.sysCtrlInfo.saveSize = le_dword(rawSize);
					}
					else if (settingName == "ExtDataId")
						exh.accessCtrlInfo.localCaps11.storageInfo.extdataId = le_dword(strtoull(tmpVal.c_str(), NULL, 16));
					else if (settingName == "UseVariation")
					{
						exh.accessCtrlInfo.localCaps11.storageInfo.storageAccessableId &= ~(1ULL<<60);
						if (strToBool(tmpVal))
							exh.accessCtrlInfo.localCaps11.storageInfo.storageAccessableId |= (1ULL<<60);
					}
					else
					{
						fprintf(stderr, "[ERROR] Unknown savegame setting: %s\n", settingName.c_str());
						return 1;
					}
				}
			}
		}
		else if (sectionName == "ServiceAccess")
		{
			specified.services = true;
			safe_call(spec.SaveValueSequence(tmp));
			for (size_t i = 0; i < tmp.size(); i ++)
				AddService(tmp[i]);
		}
		else if (sectionName == "FsPermissions11")
		{
			safe_call(spec.SaveValueSequence(tmp));
			for (size_t i = 0; i < tmp.size(); i ++)
				AddFsPermission11(tmp[i]);
		}
		else if (sectionName == "FsPermissions9")
		{
			safe_call(spec.SaveValueSequence(tmp));
			for (size_t i = 0; i < tmp.size(); i ++)
				AddFsPermission9(tmp[i]);
		}
		else if (sectionName == "KernelFlags")
		{
			safe_call(spec.SaveValueSequence(tmp));
			for (size_t i = 0; i < tmp.size(); i ++)
				AddKernelFlag(tmp[i]);
		}
		else if (sectionName == "Mappings")
		{
			specified.mappings = true;
			safe_call(spec.SaveValueSequence(tmp));
		}
		else if (sectionName == "SvcAccess")
		{
			specified.syscalls = true;
			safe_call(spec.SaveValueSequence(tmp));
		}
		else
		{
			fprintf(stderr, "[ERROR] Unknown settings section: %s\n", sectionName.c_str());
			return 1;
		}
	}

	return 0;
}
