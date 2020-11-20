#include <iostream>
#include <windows.h>
#include <memoryapi.h>
#include <string>
#include <time.h>

#define CRITICAL_SECTION_NAME L"CRITICAL_SECTION"
#define SHARED_MEMORY_NAME L"SHARED_MEMORY"

int arr[3] = { 1, 2, 3 };
int constexpr maxProcesses = 5;//create const on compilation stage

typedef struct
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
} PROCESS_INFO;

LPCTSTR CreateAndViewFileMap(HANDLE* mapping, size_t size, LPCWSTR name);
std::wstring ExePath();

int main() {

	size_t sizeOfSection = sizeof(CRITICAL_SECTION);
	size_t sizeOfSharedMemory = sizeof(int) * 3;

	LPCTSTR  SharedMemory, Cs;
	HANDLE mappingForCrSection, mappingForMemory;
	CRITICAL_SECTION* CrSection;

	Cs = CreateAndViewFileMap(&mappingForCrSection, sizeOfSection, CRITICAL_SECTION_NAME);
	if (Cs == NULL)
	{
		return -1;
	}
	CrSection = (CRITICAL_SECTION*)Cs;

	SharedMemory = CreateAndViewFileMap(&mappingForMemory, sizeOfSharedMemory, SHARED_MEMORY_NAME);
	if (SharedMemory == NULL)
	{
		return -1;
	}

	InitializeCriticalSection(CrSection);
	std::cout << "Section is ready" << std::endl;

	std::cout << "Original array : " << std::endl;
	std::cout << arr[0] << " - " << arr[1] << " - " << arr[2] << std::endl;

	CopyMemory((PVOID)SharedMemory, arr, sizeof(int) * 3);
	;
	while (!TryEnterCriticalSection(CrSection))
	{
		Sleep(100);
	}

	std::cout << "Process main:  Array Befor :";
	for (int i = 0; i < 3; i++)
	{
		std::cout << ((int*)SharedMemory)[i] << "-";
	}
	std::cout << std::endl;
	std::cout << "Process main:  Array After :";
	for (int i = 0; i < 3; i++)
	{
		((int*)SharedMemory)[i] = rand() % 100;;
		std::cout << ((int*)SharedMemory)[i] << "-";
	}
	std::cout << std::endl;
	std::cout << std::endl;

	LeaveCriticalSection(CrSection);

	PROCESS_INFO pInfos[maxProcesses];
	for (int i = 0; i < maxProcesses; i++)
	{
		ZeroMemory(&pInfos[i], sizeof(PROCESS_INFO));
		pInfos[i].si.cb = sizeof(STARTUPINFO);
		if (!CreateProcess(
			std::wstring(ExePath() + L"\\Process.exe").c_str(),
			NULL,
			0,
			0,
			TRUE,
			0,
			0,
			NULL,
			&pInfos[i].si,
			&pInfos[i].pi))
		{
			std::cout << "Failed to create process Error:" << GetLastError() << std::endl;
		}

	}

	for (int i = 0; i < maxProcesses; i++)
	{
		DWORD res = WaitForSingleObject(pInfos[i].pi.hProcess, INFINITE);
		CloseHandle(pInfos[i].pi.hProcess);
		CloseHandle(pInfos[i].pi.hThread);
	}

	DeleteCriticalSection(CrSection);
	UnmapViewOfFile(Cs);
	UnmapViewOfFile(SharedMemory);

	CloseHandle(mappingForCrSection);
	CloseHandle(mappingForMemory);
	return 0;
}
std::wstring ExePath()
{
	TCHAR buffer[MAX_PATH] = { 0 };
	GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring::size_type pos = std::wstring(buffer).find_last_of(L"\\/");
	return std::wstring(buffer).substr(0, pos);
}

LPCTSTR CreateAndViewFileMap(HANDLE* mapping, size_t size, LPCWSTR name)
{
	*mapping = CreateFileMapping(
		INVALID_HANDLE_VALUE,//set the size, in operation system
		NULL,// cant be inherited
		PAGE_READWRITE,
		0,
		size,
		name);
	if (*mapping == NULL) {
		std::cout << "Failed to create mapping\n";
		return NULL;
	}

	LPCTSTR file_view = (LPCTSTR)MapViewOfFile(
		*mapping,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		size);
	if (file_view == NULL) {
		CloseHandle(*mapping);
		std::cout << "Failed to View Of File\n";
		return NULL;
	}

	return file_view;
}
