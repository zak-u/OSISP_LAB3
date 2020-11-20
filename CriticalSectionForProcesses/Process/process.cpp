#include <iostream>
#include <windows.h>
#include <memoryapi.h>


#define CRITICAL_SECTION_NAME L"CRITICAL_SECTION"
#define SHARED_MEMORY_NAME L"SHARED_MEMORY"

int arr[3];
LPCTSTR OpenAndViewFileMap(HANDLE* mapping, size_t size, LPCWSTR name);

int main() {

	size_t sizeOfSection = sizeof(CRITICAL_SECTION);
	size_t sizeOfSharedMemory = sizeof(int) * 3;

	LPCTSTR  SharedMemory, Cs;
	HANDLE mappingForCrSection, mappingForMemory;
	CRITICAL_SECTION* CrSection;

	Cs = OpenAndViewFileMap(&mappingForCrSection, sizeOfSection, CRITICAL_SECTION_NAME);
	if (Cs == NULL)
	{
		return -1;
	}
	CrSection = (CRITICAL_SECTION*)Cs;

	SharedMemory = OpenAndViewFileMap(&mappingForMemory, sizeOfSharedMemory, SHARED_MEMORY_NAME);
	if (SharedMemory == NULL)
	{
		return -1;
	}

	int procId = GetCurrentProcessId();
	while (!TryEnterCriticalSection(CrSection))
	{
		Sleep(100);
	}
	std::cout << "Process " << procId << ": Array Befor ";
	for (int i = 0; i < 3; i++)
	{
		std::cout << ((int*)SharedMemory)[i] << "-";
	}
	std::cout << std::endl;
	std::cout << "Process " << procId << ": Array After ";
	for (int i = 0; i < 3; i++)
	{
		((int*)SharedMemory)[i] += rand() % 5;
		std::cout << ((int*)SharedMemory)[i] << "-";
	}
	std::cout << std::endl;
	std::cout << std::endl;

	LeaveCriticalSection(CrSection);

	UnmapViewOfFile(Cs);
	UnmapViewOfFile(SharedMemory);

	CloseHandle(mappingForCrSection);
	CloseHandle(mappingForMemory);
	return 0;
}
LPCTSTR OpenAndViewFileMap(HANDLE* mapping, size_t size, LPCWSTR name)
{
	*mapping = OpenFileMapping(FILE_MAP_ALL_ACCESS, TRUE, name);
	if (*mapping == NULL) {
		std::cout << "Cannot open shared memory critical section" << std::endl;
		return NULL;
	}

	LPCTSTR file_view = (LPCTSTR)MapViewOfFile(*mapping, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if (file_view == NULL) {
		CloseHandle(*mapping);
		std::cout << "Cannot map view critical section" << std::endl;
		return NULL;
	}

	return file_view;
}