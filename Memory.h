#pragma once
#include <Windows.h>
#include <vector>
#include <string> 
#include <iostream>
#include <TlHelp32.h>
#include <tchar.h>
#include <winternl.h>

extern std::string messger;
extern bool copypst;
extern bool text_animation_hide0;

class Memory
{

public:


	const char* GetEmulatorRunning() {
		if (GetPid("HD-Player.exe") != 0)
			return "HD-Player.exe";

		else if (GetPid("MEmuHeadless.exe") != 0)
			return "MEmuHeadless.exe";

		else if (GetPid("LdVBoxHeadless.exe") != 0)
			return "LdVBoxHeadless.exe";

		else if (GetPid("AndroidProcess.exe") != 0)
			return "AndroidProcess.exe";

		else if (GetPid("Nox.exe") != 0)
			return "Nox.exe";
	}

	void ReWrite(DWORD_PTR dwStartRange, DWORD_PTR dwEndRange, BYTE* Search, BYTE* Replace)
	{
		if (!AttackProcess(GetEmulatorRunning())) {
			messger = "An unexpected error occurred";
			copypst = false;
			text_animation_hide0 = true;
		}

		bool Status = ReplacePattern(dwStartRange, dwEndRange, Search, Replace, true);
		if (Status) {
			messger = "Enabled!";
			copypst = true;
			text_animation_hide0 = true;
		}
		else {
			messger = "Failed to Enable!";
			copypst = false;
			text_animation_hide0 = true;
		}

		CloseHandle(ProcessHandle);
	}

	void deWrite(DWORD_PTR dwStartRange, DWORD_PTR dwEndRange, BYTE* Search, BYTE* Replace)
	{
		if (!AttackProcess(GetEmulatorRunning())) {
			messger = "An unexpected error occurred";
			copypst = false;
			text_animation_hide0 = true;
		}

		bool Status = ReplacePattern(dwStartRange, dwEndRange, Search, Replace, true);
		if (Status) {
			messger = "Disabled!";
			copypst = true;
			text_animation_hide0 = true;
		}
		else {
			messger = "Failed to Disable!";
			copypst = false;
			text_animation_hide0 = true;
		}

		CloseHandle(ProcessHandle);
	}

	DWORD ProcessId = 0;
	HANDLE ProcessHandle;

	typedef struct _MEMORY_REGION {
		DWORD_PTR dwBaseAddr;
		DWORD_PTR dwMemorySize;
	}MEMORY_REGION;

	int GetPid(const char* procname) {

		if (procname == NULL)
			return 0;
		DWORD pid = 0;
		DWORD threadCount = 0;

		HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		PROCESSENTRY32 pe;

		pe.dwSize = sizeof(PROCESSENTRY32);
		Process32First(hSnap, &pe);
		while (Process32Next(hSnap, &pe)) {
			if (_tcsicmp(pe.szExeFile, procname) == 0) {
				if ((int)pe.cntThreads > threadCount) {
					threadCount = pe.cntThreads;

					pid = pe.th32ProcessID;

				}
			}
		}
		return pid;
	}
	BOOL AttackProcess(const char* procname) {
		DWORD ProcId = GetPid(procname);
		if (ProcId == 0)
			return false;

		ProcessId = ProcId;
		ProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, ProcessId);
		return ProcessHandle != nullptr;
	}

	bool ChangeProtection(ULONG Address, size_t size, DWORD NewProtect, DWORD& OldProtect)
	{
		return VirtualProtectEx(ProcessHandle, (LPVOID)Address, size, NewProtect, &OldProtect);;
	}

	bool ReplacePattern(DWORD_PTR dwStartRange, DWORD_PTR dwEndRange, BYTE* SearchAob, BYTE* ReplaceAob, bool ForceWrite = false)
	{
		int RepByteSize = _msize(ReplaceAob);
		if (RepByteSize <= 0) return false;
		std::vector<DWORD_PTR> foundedAddress;
		FindPattern(dwStartRange, dwEndRange, SearchAob, foundedAddress);
		if (foundedAddress.empty())
			return false;

		DWORD OldProtect;
		for (int i = 0; i < foundedAddress.size(); i++)
		{
			ChangeProtection(foundedAddress[i], RepByteSize, PAGE_EXECUTE_READWRITE, OldProtect);
			WriteProcessMemory(ProcessHandle, (LPVOID)foundedAddress[i], ReplaceAob, RepByteSize, 0);
		}

		return true;
	}


	bool FindPattern(DWORD_PTR StartRange, DWORD_PTR EndRange, BYTE* SearchBytes, std::vector<DWORD_PTR>& AddressRet)
	{

		BYTE* pCurrMemoryData = NULL;
		MEMORY_BASIC_INFORMATION	mbi;
		std::vector<MEMORY_REGION> m_vMemoryRegion;
		mbi.RegionSize = 0x1000;
		DWORD_PTR dwAddress = StartRange;
		DWORD_PTR nSearchSize = _msize(SearchBytes);


		while (VirtualQueryEx(ProcessHandle, (LPCVOID)dwAddress, &mbi, sizeof(mbi)) && (dwAddress < EndRange) && ((dwAddress + mbi.RegionSize) > dwAddress))
		{

			if ((mbi.State == MEM_COMMIT) && ((mbi.Protect & PAGE_GUARD) == 0) && (mbi.Protect != PAGE_NOACCESS) && ((mbi.AllocationProtect & PAGE_NOCACHE) != PAGE_NOCACHE))
			{

				MEMORY_REGION mData = { 0 };
				mData.dwBaseAddr = (DWORD_PTR)mbi.BaseAddress;
				mData.dwMemorySize = mbi.RegionSize;
				m_vMemoryRegion.push_back(mData);

			}
			dwAddress = (DWORD_PTR)mbi.BaseAddress + mbi.RegionSize;

		}

		std::vector<MEMORY_REGION>::iterator it;
		for (it = m_vMemoryRegion.begin(); it != m_vMemoryRegion.end(); it++)
		{
			MEMORY_REGION mData = *it;


			DWORD_PTR dwNumberOfBytesRead = 0;
			pCurrMemoryData = new BYTE[mData.dwMemorySize];
			ZeroMemory(pCurrMemoryData, mData.dwMemorySize);
			ReadProcessMemory(ProcessHandle, (LPCVOID)mData.dwBaseAddr, pCurrMemoryData, mData.dwMemorySize, &dwNumberOfBytesRead);
			if ((int)dwNumberOfBytesRead <= 0)
			{
				delete[] pCurrMemoryData;
				continue;
			}
			DWORD_PTR dwOffset = 0;
			int iOffset = Memfind(pCurrMemoryData, dwNumberOfBytesRead, SearchBytes, nSearchSize);
			while (iOffset != -1)
			{
				dwOffset += iOffset;
				AddressRet.push_back(dwOffset + mData.dwBaseAddr);
				dwOffset += nSearchSize;
				iOffset = Memfind(pCurrMemoryData + dwOffset, dwNumberOfBytesRead - dwOffset - nSearchSize, SearchBytes, nSearchSize);
			}

			if (pCurrMemoryData != NULL)
			{
				delete[] pCurrMemoryData;
				pCurrMemoryData = NULL;
			}

		}
		return TRUE;
	}

	int Memfind(BYTE* buffer, DWORD_PTR dwBufferSize, BYTE* bstr, DWORD_PTR dwStrLen) {
		if (dwBufferSize < 0) {
			return -1;
		}
		DWORD_PTR  i, j;
		for (i = 0; i < dwBufferSize; i++) {
			for (j = 0; j < dwStrLen; j++) {
				if (buffer[i + j] != bstr[j] && bstr[j] != '?')
					break;

			}
			if (j == dwStrLen)
				return i;
		}
		return -1;
	}










	// Security

	void AntiBlacklist() {
		ReWrite(
			0x0L, 0x00007fffffffffff,
			new BYTE[]{ 0x0A, 0x00, 0xA0, 0xE3, 0x96, 0x00, 0x81, 0xE0, 0x00, 0x00, 0x51, 0xE3, 0x02, 0x01, 0x00, 0x1A },
			new BYTE[]{ 0x00, 0xF0, 0x20, 0xE3 }
		);

		ReWrite(
			0x0L, 0x00007fffffffffff,
			new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00 },
			new BYTE[]{ 0x01, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1 }
		);

		ReWrite(
			0x0L, 0x00007fffffffffff,
			new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00 },
			new BYTE[]{ 0x01, 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1 }
		);

		ReWrite(
			0x0L, 0x00007fffffffffff,
			new BYTE[]{ 0x0A, 0x00, 0xA0, 0xE3, '?', '?', '?', '?', '?', '?', '?', '?', 0x02 },
			new BYTE[]{ 0x00, 0xF0, 0x20, 0xE3 }
		);

		ReWrite(
			0x0L, 0x00007fffffffffff,
			new BYTE[]{ 0x49, 0x44, 0x48, 0x48, 0x42, 0x47, 0x42, 0x4E, 0x48, 0x4D, 0x44 },
			new BYTE[]{ 0x50, 0x4B, 0x45, 0x4A, 0x42, 0x4C, 0x4E, 0x42, 0x41, 0x48, 0x48 }
		);
	}

	// Aim's

	void AimbotHead(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x69, 0x00, 0x70, 0x00, 0x73, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0x09, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64 },
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0x09, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64 },
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x69, 0x00, 0x70, 0x00, 0x73 }
			);
		}
	}

	void AimbotNeck(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x69, 0x00, 0x70, 0x00, 0x73, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0x09, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64 },
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x4E, 0x00, 0x65, 0x00, 0x63, 0x00, 0x6B }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x4E, 0x00, 0x65, 0x00, 0x63, 0x00, 0x6B, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0x09, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64 },
				new BYTE[]{ 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x69, 0x00, 0x70, 0x00, 0x73 }
			);
		}
	}

	void AimbotTrick(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x4C, 0x7B, 0x5A, 0xBD, 0x0A, 0x57, 0x66, 0xBB, 0x1E, 0x21, 0x48, 0xBA, 0x2A, 0xC2, 0xCF, 0x3B, 0x96, 0xFB, 0x28, 0x3D, 0xE8, 0xB1, 0x17, 0xBD, 0xE3, 0x99, 0x7F, 0x3F, 0x04, 0x00, 0x80, 0x3F, 0x01, 0x00, 0x80, 0x3F, 0xFC, 0xFF, 0x7F, 0x3F },
				new BYTE[]{ 0xD1, 0x0A, 0xC0, 0xBE, 0x16, 0xDC, 0x98, 0xBD, 0xBB, 0x82, 0x97, 0xB4, 0x00, 0x00, 0x00, 0x00, 0xBF, 0xB2, 0x2F, 0x3F, 0x43, 0x32, 0x73, 0x36, 0x66, 0x03, 0x7B, 0x35, 0x72, 0x1C, 0xC7, 0x3F, 0x72, 0x1C, 0xC7, 0x3F, 0x72, 0x1C, 0xC7, 0x3F }
			);

			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x10, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x4C, 0x00, 0x65, 0x00, 0x66, 0x00, 0x74, 0x00, 0x5F, 0x00, 0x57, 0x00, 0x65, 0x00, 0x61, 0x00, 0x70, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x00 },
				new BYTE[]{ 0x10, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);

			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x23, 0xAA, 0xA6, 0xB8, 0x46, 0x0A, 0xCD, 0x70, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x23, 0xAA, 0xA6, 0xB8, 0xB2, 0xF7, 0x1F, 0xA4, 0xFF, 0xFF, 0xFF, 0xFF }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xD1, 0x0A, 0xC0, 0xBE, 0x16, 0xDC, 0x98, 0xBD, 0xBB, 0x82, 0x97, 0xB4, 0x00, 0x00, 0x00, 0x00, 0xBF, 0xB2, 0x2F, 0x3F, 0x43, 0x32, 0x73, 0x36, 0x66, 0x03, 0x7B, 0x35, 0x72, 0x1C, 0xC7, 0x3F, 0x72, 0x1C, 0xC7, 0x3F, 0x72, 0x1C, 0xC7, 0x3F },
				new BYTE[]{ 0x4C, 0x7B, 0x5A, 0xBD, 0x0A, 0x57, 0x66, 0xBB, 0x1E, 0x21, 0x48, 0xBA, 0x2A, 0xC2, 0xCF, 0x3B, 0x96, 0xFB, 0x28, 0x3D, 0xE8, 0xB1, 0x17, 0xBD, 0xE3, 0x99, 0x7F, 0x3F, 0x04, 0x00, 0x80, 0x3F, 0x01, 0x00, 0x80, 0x3F, 0xFC, 0xFF, 0x7F, 0x3F }
			);

			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x10, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x48, 0x00, 0x65, 0x00, 0x61, 0x00, 0x64, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x10, 0x00, 0x00, 0x00, 0x62, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x4C, 0x00, 0x65, 0x00, 0x66, 0x00, 0x74, 0x00, 0x5F, 0x00, 0x57, 0x00, 0x65, 0x00, 0x61, 0x00, 0x70, 0x00, 0x6F, 0x00, 0x6E, 0x00, 0x00 }
			);

			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x23, 0xAA, 0xA6, 0xB8, 0xB2, 0xF7, 0x1F, 0xA4, 0xFF, 0xFF, 0xFF, 0xFF },
				new BYTE[]{ 0x23, 0xAA, 0xA6, 0xB8, 0x46, 0x0A, 0xCD, 0x70, 0x00, 0x00, 0x00, 0x00 }
			);
		}
	}

	void AimbotScope(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xA0, 0x42, 0x00, 0x00, 0xC0, 0x3F, 0x33, 0x33, 0x13, 0x40, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x80, 0x3F },
				new BYTE[]{ 0xA0, 0x42, 0x00, 0x00, 0xC0, 0x3F, 0xE0, 0xB1, 0xFF, 0xFF, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x80, 0x3F }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xA0, 0x42, 0x00, 0x00, 0xC0, 0x3F, 0x33, 0x33, 0x13, 0x40, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x80, 0x3F },
				new BYTE[]{ 0xA0, 0x42, 0x00, 0x00, 0xC0, 0x3F, 0xE0, 0xB1, 0xFF, 0xFF, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x80, 0x3F }
			);
		}
	}

	void AimbotSniper(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x8F, 0xC2, 0xB5, 0x3F, 0x01, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00 }
			);
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', 0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0x99, 0x19, 0x3F, 0x9A, 0x99, 0x19, 0x3F, 0x08, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00 }
			);
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x8F, 0xC2, 0xB5, 0x3F, 0x01, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x01 }
			);
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', 0xA5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9A, 0x99, 0x19, 0x3F, 0x9A, 0x99, 0x19, 0x3F, 0x08, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x01 }
			);
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', 0x5F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x01 }
			);
		}
	}

	void AimfovLegit(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x47, 0x01, 0x3F, 0xAE, 0x47, 0xE1, 0x3E, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xAE, 0xC7, 0x3E, 0xF6, 0x28, 0x1C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0xFF, 0xFF, 0xFF, 0xAE, 0x47, 0xE1, 0x3E, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xFF, 0xFF, 0xFF, 0xF6, 0x28, 0x1C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x00, 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xFF, 0xFF, 0xFF, 0xAE, 0x47, 0xE1, 0x3E, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xFF, 0xFF, 0xFF, 0xF6, 0x28, 0x1C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x47, 0x01, 0x3F, 0xAE, 0x47, 0xE1, 0x3E, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xAE, 0xC7, 0x3E, 0xF6, 0x28, 0x1C, 0x3F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x3F, 0x00, 0x00, 0x00, 0x00 }
			);
		}
	}

	void AimfovFull(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xAE, 0x47, 0x01, 0x3F, 0xA4, 0x70, 0xFD, 0x3E, 0xAE, 0x47, 0x01, 0x3F, 0xAE, 0x47, 0xE1, 0x3E, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xAE, 0xC7, 0x3E },
				new BYTE[]{ 0x80, 0x7B, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x7B, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xAE, 0xC7, 0x3E }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x80, 0x7B, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x80, 0x7B, 0xE1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xAE, 0xC7, 0x3E },
				new BYTE[]{ 0xAE, 0x47, 0x01, 0x3F, 0xA4, 0x70, 0xFD, 0x3E, 0xAE, 0x47, 0x01, 0x3F, 0xAE, 0x47, 0xE1, 0x3E, 0x29, 0x5C, 0x0F, 0x3F, 0x14, 0xAE, 0xC7, 0x3E }
			);
		}
	}

	// Visuals

	void ESPName(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x70, 0x00, 0x65, 0x00, 0x72, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6B, 0x00, 0x61, 0x00, 0x69, 0x00, 0x70, 0x00, 0x61, 0x00, 0x77, 0x00, 0x6E, 0x00, 0x2F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6B, 0x00, 0x61, 0x00, 0x69, 0x00, 0x70, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x79, 0x00, 0x6E, 0x00, 0x70, 0x00, 0x63, 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6B, 0x00, 0x61, 0x00, 0x69, 0x00, 0x70, 0x00, 0x61, 0x00, 0x77, 0x00, 0x6E, 0x00, 0x2F, 0x00, 0x6E, 0x00, 0x65, 0x00, 0x74, 0x00, 0x77, 0x00, 0x6F, 0x00, 0x72, 0x00, 0x6B, 0x00, 0x61, 0x00, 0x69, 0x00, 0x70, 0x00, 0x61, 0x00, 0x72, 0x00, 0x74, 0x00, 0x79, 0x00, 0x6E, 0x00, 0x70, 0x00, 0x63, 0x00 },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x70, 0x00, 0x65, 0x00, 0x72, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);
		}
	}

	void ESPArrow(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x70, 0x00, 0x65, 0x00, 0x72, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x65, 0x00, 0x66, 0x00, 0x66, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x73, 0x00, 0x2F, 0x00, 0x66, 0x00, 0x66, 0x00, 0x5F, 0x00, 0x66, 0x00, 0x78, 0x00, 0x5F, 0x00, 0x67, 0x00, 0x75, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x61, 0x00, 0x72, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x65, 0x00, 0x66, 0x00, 0x66, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x73, 0x00, 0x2F, 0x00, 0x66, 0x00, 0x66, 0x00, 0x5F, 0x00, 0x66, 0x00, 0x78, 0x00, 0x5F, 0x00, 0x67, 0x00, 0x75, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x61, 0x00, 0x72, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x77, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x70, 0x00, 0x65, 0x00, 0x72, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);
		}
	}

	void ESPLaser(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x70, 0x00, 0x65, 0x00, 0x72, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x65, 0x00, 0x66, 0x00, 0x66, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x73, 0x00, 0x2F, 0x00, 0x76, 0x00, 0x66, 0x00, 0x78, 0x00, 0x5F, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x26, 0x00, 0x00, 0x00, 0x65, 0x00, 0x66, 0x00, 0x66, 0x00, 0x65, 0x00, 0x63, 0x00, 0x74, 0x00, 0x73, 0x00, 0x2F, 0x00, 0x76, 0x00, 0x66, 0x00, 0x78, 0x00, 0x5F, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x5F, 0x00, 0x6C, 0x00, 0x61, 0x00, 0x73, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x00, 0x21, 0x00, 0x00, 0x00, 0x69, 0x00, 0x6E, 0x00, 0x67, 0x00, 0x61, 0x00, 0x6D, 0x00, 0x65, 0x00, 0x2F, 0x00, 0x63, 0x00, 0x61, 0x00, 0x70, 0x00, 0x73, 0x00, 0x75, 0x00, 0x6C, 0x00, 0x65, 0x00, 0x68, 0x00, 0x75, 0x00, 0x6D, 0x00, 0x61, 0x00, 0x6E, 0x00, 0x73, 0x00, 0x6E, 0x00, 0x69, 0x00, 0x70, 0x00, 0x65, 0x00, 0x72, 0x00, 0x63, 0x00, 0x6F, 0x00, 0x6C, 0x00, 0x6C, 0x00, 0x69, 0x00, 0x64, 0x00, 0x65, 0x00, 0x72, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);
		}
	}

	void AntenaHandF(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x80, 0x3F, 0x21, 0x13, 0x17, 0xBE, 0xDE, 0xA0, 0x76, 0xBF },
				new BYTE[]{ 0xEC, 0x11, 0x8C, 0x43, 0x21, 0x13, 0x17, 0xBE, 0xDE, 0xA0, 0x76, 0xBF }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xEC, 0x11, 0x8C, 0x43, 0x21, 0x13, 0x17, 0xBE, 0xDE, 0xA0, 0x76, 0xBF },
				new BYTE[]{ 0x00, 0x00, 0x80, 0x3F, 0x21, 0x13, 0x17, 0xBE, 0xDE, 0xA0, 0x76, 0xBF }
			);
		}
	}

	void AntenaHandM(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x80, 0x3F, 0xCF, 0xF7, 0xAD, 0x34 },
				new BYTE[]{ 0x33, 0x33, 0x34, 0x43, 0xCF, 0xF7, 0xAD, 0x34 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x33, 0x33, 0x34, 0x43, 0xCF, 0xF7, 0xAD, 0x34 },
				new BYTE[]{ 0x00, 0x00, 0x80, 0x3F, 0xCF, 0xF7, 0xAD, 0x34 }
			);
		}
	}

	void FakeDamage(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0xB0, 0x40, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x3F, 0x00, 0x00, 0x80, 0x3F },
				new BYTE[]{ 0x00, 0xC7, 0x0A, 0x5F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x3F, 0x00, 0x00, 0x80, 0x3F }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0xC7, 0x0A, 0x5F, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x3F, 0x00, 0x00, 0x80, 0x3F },
				new BYTE[]{ 0x00, 0x00, 0xB0, 0x40, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x3F, 0x00, 0x00, 0x80, 0x3F }
			);
		}
	}

	void IncreaseVision(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xDB, 0x0F, 0x49, 0x40, 0x10, 0x2A, 0x00, 0xEE },
				new BYTE[]{ 0x00, 0x00, 0xA0, 0x40, 0x10, 0x2A, 0x00, 0xEE }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0xA0, 0x40, 0x10, 0x2A, 0x00, 0xEE },
				new BYTE[]{ 0xDB, 0x0F, 0x49, 0x40, 0x10, 0x2A, 0x00, 0xEE }
			);
		}
	}

	void OnlyRed(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x40, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x3F },
				new BYTE[]{ 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB8, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },
				new BYTE[]{ 0x96, 0x00, 0x00, 0x00, 0x00, 0x00, 0xB0, 0x40, 0x00, 0x00, 0x80, 0x3F, 0x00, 0x00, 0x40, 0x3F }
			);
		}
	}

	// Weapon
	void NoRecoil(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x0A, 0x81, 0xEE, 0x10, 0x0A, 0x10, 0xEE, 0x10, 0x8C, 0xBD, 0xE8, 0x00, 0x00, 0x7A, 0x44, 0xF0, 0x48, 0x2D, 0xE9, 0x10, 0xB0, 0x8D, 0xE2, 0x02, 0x8B, 0x2D, 0xED },
				new BYTE[]{ 0x00, 0x0A, 0x81, 0xEE, 0x10, 0x0A, 0x10, 0xEE, 0x10, 0x8C, 0xBD, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x48, 0x2D, 0xE9, 0x10, 0xB0, 0x8D, 0xE2, 0x02, 0x8B, 0x2D, 0xED }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x0A, 0x81, 0xEE, 0x10, 0x0A, 0x10, 0xEE, 0x10, 0x8C, 0xBD, 0xE8, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x48, 0x2D, 0xE9, 0x10, 0xB0, 0x8D, 0xE2, 0x02, 0x8B, 0x2D, 0xED },
				new BYTE[]{ 0x00, 0x0A, 0x81, 0xEE, 0x10, 0x0A, 0x10, 0xEE, 0x10, 0x8C, 0xBD, 0xE8, 0x00, 0x00, 0x7A, 0x44, 0xF0, 0x48, 0x2D, 0xE9, 0x10, 0xB0, 0x8D, 0xE2, 0x02, 0x8B, 0x2D, 0xED }
			);
		}
	}

	void FastWitchSniper(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ '?' , '?', '?',   0x3F, 0x00, 0x00, 0x80, 0x3E, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F },
				new BYTE[]{ 0xEC, 0x51, 0xB8, 0x3D, 0x8F, 0xC2, 0xF5, 0x3C, 0x00, 0x00, 0x00, 0x00 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xEC, 0x51, 0xB8, 0x3D, 0x8F, 0xC2, 0xF5, 0x3C, 0x00, 0x00, 0x00, 0x00, '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x3F },
				new BYTE[]{ 0x00, 0x00, 0x00, 0x3D, 0x8F, 0xC2, 0xF5, 0x3C, 0x00, 0x00, 0x00, 0x00 }
			);
		}
	}

	void Precision(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x70, 0x41, 0x00, 0x00, 0x0C, 0x42, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0xA0, 0x41 },
				new BYTE[]{ 0x00, 0x00, 0x71, 0x41, 0x00, 0x00, 0x0F, 0x38, 0x00, 0x00, 0x72, 0x41, 0x00, 0x00, 0x47, 0x45 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x71, 0x41, 0x00, 0x00, 0x0F, 0x38, 0x00, 0x00, 0x72, 0x41, 0x00, 0x00, 0x47, 0x45 },
				new BYTE[]{ 0x00, 0x00, 0x70, 0x41, 0x00, 0x00, 0x0C, 0x42, 0x00, 0x00, 0x20, 0x41, 0x00, 0x00, 0xA0, 0x41 }
			);
		}
	}

	void Aimlock(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00 },
				new BYTE[]{ 0x70, 0x4C, 0x2D, 0xE9, 0x10, 0xB0, 0x8D, 0xE1 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x70, 0x4C, 0x2D, 0xE9, 0x10, 0xB0, 0x8D, 0xE1 },
				new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00 }
			);
		}
	}
	
	// Misc's

	void BypassEmulador() {
		ReWrite(
			0x0, 0xFFFFFFFF,
			new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00 },
			new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00 }
		);
	}

	void BypassEmulador280() {
		ReWrite(
			0x0, 0xFFFFFFFF,
			new BYTE[]{ 0x7F, 0x45, 0x4C, 0x46, 0x01, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x03 },
			new BYTE[]{ 0x01 , 0x00, 0xA0, 0xE3, 0x1E, 0xFF, 0x2F, 0xE1 }
		);
	}

	void AlokFix64bits(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xA9, 0xE0, 0x03, 0x13, 0xAA, 0xF4, 0x4F, 0xC2, 0xA8, 0x40, 0x00, 0x1F, 0xD6, 0xF5, 0x0F, 0x1D, 0xF8, 0xF4, 0x4F, 0x01, 0xA9, 0xFD, 0x7B, 0x02, 0xA9, 0xFD, 0x83, 0x00, 0x91 },
				new BYTE[]{ 0xA9, 0xE0, 0x03, 0x13, 0xAA, 0xF4, 0x4F, 0xC2, 0xA8, 0x40, 0x00, 0x1F, 0xD6, 0xC0, 0x03, 0x5F, 0xD6, 0xF4, 0x4F, 0x01, 0xA9, 0xFD, 0x7B, 0x02, 0xA9, 0xFD, 0x83, 0x00, 0x91 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xA9, 0xE0, 0x03, 0x13, 0xAA, 0xF4, 0x4F, 0xC2, 0xA8, 0x40, 0x00, 0x1F, 0xD6, 0xC0, 0x03, 0x5F, 0xD6, 0xF4, 0x4F, 0x01, 0xA9, 0xFD, 0x7B, 0x02, 0xA9, 0xFD, 0x83, 0x00, 0x91 },
				new BYTE[]{ 0xA9, 0xE0, 0x03, 0x13, 0xAA, 0xF4, 0x4F, 0xC2, 0xA8, 0x40, 0x00, 0x1F, 0xD6, 0xF5, 0x0F, 0x1D, 0xF8, 0xF4, 0x4F, 0x01, 0xA9, 0xFD, 0x7B, 0x02, 0xA9, 0xFD, 0x83, 0x00, 0x91 }
			);
		}
	}


	void Fly(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x9A, 0x99, 0x99, 0x3E, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x34, 0x42, 0xCD, 0xCC, 0xCC },
				new BYTE[]{ 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x34, 0x42, 0xCD, 0xCC, 0xCC }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x80, 0x40, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x34, 0x42, 0xCD, 0xCC, 0xCC },
				new BYTE[]{ 0x9A, 0x99, 0x99, 0x3E, 0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x34, 0x42, 0xCD, 0xCC, 0xCC }
			);
		}
	}

	void FastMedkit(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x40, 0x00, 0x00, 0x80, 0x40 },
				new BYTE[]{ 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x40, 0x00, 0x00, 0x40, 0x40 }
			);
		}
		else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x40, 0x00, 0x00, 0x40, 0x40 },
				new BYTE[]{ 0x4B, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x40, 0x00, 0x00, 0x80, 0x40 }
			);
		}
	}

	void MagicBullets(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0xAC, 0xC5, 0x27, 0x37, 0x30, 0x48, 0x2D, 0xE9, 0x01, 0x40, 0xA0, 0xE1, 0x20, 0x10, 0x9F, 0xE5 },
				new BYTE[]{ 0x00, 0x00, 0x80, 0x3F, 0x30, 0x48, 0x2D, 0xE9, 0x01, 0x40, 0xA0, 0xE1, 0x20, 0x10, 0x9F, 0xE5 }
			);
		} else {
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{ 0x00, 0x00, 0x80, 0x3F, 0x30, 0x48, 0x2D, 0xE9, 0x01, 0x40, 0xA0, 0xE1, 0x20, 0x10, 0x9F, 0xE5 },
				new BYTE[]{ 0xAC, 0xC5, 0x27, 0x37, 0x30, 0x48, 0x2D, 0xE9, 0x01, 0x40, 0xA0, 0xE1, 0x20, 0x10, 0x9F, 0xE5 }
			);
		}
	}
	/*
	void AimbotHead(bool style) {
		if (style) {
			ReWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{  },
				new BYTE[]{  }
			);
		}
		else
			deWrite(
				0x0, 0xFFFFFFFF,
				new BYTE[]{  },
				new BYTE[]{  }
			);
		}
	}
	*/
};