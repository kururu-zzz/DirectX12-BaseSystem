#ifndef SAFEEVENTHANDLE___
#define SAFEEVENTHANDLE___

#include <Windows.h>

/**
*@brief Scoped Event Handle
*/
class SafeEventHandle
{
private:
	HANDLE handle;
public:
	HANDLE* get(){ return &handle; }
	SafeEventHandle()
	{
		handle = CreateEventEx(nullptr, FALSE, FALSE, EVENT_ALL_ACCESS);
		if (handle == nullptr)
		{
			HRESULT_FROM_WIN32(GetLastError());
		}
	}
	~SafeEventHandle()
	{
		CloseHandle(handle);
	}
};

#endif