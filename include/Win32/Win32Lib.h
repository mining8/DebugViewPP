// (C) Copyright Gert-Jan de Vos and Jan Wilmans 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Repository at: https://github.com/djeedjay/DebugViewPP/

#pragma once

//#include <single-header/system_error2.hpp>
#include <windows.h>
#include "Utilities.h"
#include <string>
#include <memory>
#include <vector>

#include <AccCtrl.h>
#include <Aclapi.h>

#pragma comment(lib, "Win32Lib.lib")

namespace fusion {
namespace Win32 {

static const int fixedNumberOfSystemPids = 5;

struct LocalAllocDeleter
{
	typedef HLOCAL pointer;

	void operator()(pointer p) const;
};

typedef std::unique_ptr<void, LocalAllocDeleter> HLocal;

struct GlobalAllocDeleter
{
	typedef HGLOBAL pointer;

	void operator()(pointer p) const;
};

typedef std::unique_ptr<void, GlobalAllocDeleter> HGlobal;

struct HandleDeleter
{
	typedef HANDLE pointer;

	void operator()(pointer p) const;
};

typedef std::unique_ptr<void, HandleDeleter> Handle;

struct ChangeNotificationHandleDeleter
{
	typedef HANDLE pointer;

	void operator()(pointer p) const;
};

typedef std::unique_ptr<void, ChangeNotificationHandleDeleter> ChangeNotificationHandle;

template <typename T>
struct GdiObjectDeleter
{
	typedef T pointer;

	void operator()(pointer p) const
	{
		if (p != nullptr)
			DeleteObject(p);
	}
};

template <>
struct GdiObjectDeleter<HICON>
{
	typedef HICON pointer;

	void operator()(pointer p) const
	{
		if (p != nullptr)
			DestroyIcon(p);
	}
};

typedef std::unique_ptr<std::remove_pointer<HGDIOBJ>::type, GdiObjectDeleter<HGDIOBJ>> GdiObject;
typedef std::unique_ptr<std::remove_pointer<HPEN>::type, GdiObjectDeleter<HPEN>> HPen;
typedef std::unique_ptr<std::remove_pointer<HBRUSH>::type, GdiObjectDeleter<HBRUSH>> HBrush;
typedef std::unique_ptr<std::remove_pointer<HFONT>::type, GdiObjectDeleter<HFONT>> HFont;
typedef std::unique_ptr<std::remove_pointer<HBITMAP>::type, GdiObjectDeleter<HBITMAP>> HBitmap;
typedef std::unique_ptr<std::remove_pointer<HRGN>::type, GdiObjectDeleter<HRGN>> HRegion;
typedef std::unique_ptr<std::remove_pointer<HPALETTE>::type, GdiObjectDeleter<HPALETTE>> HPalette;
typedef std::unique_ptr<std::remove_pointer<HICON>::type, GdiObjectDeleter<HICON>> HIcon;

template <typename T>
class GlobalLock
{
public:
	explicit GlobalLock(const HGlobal& hg) :
		m_hg(hg.get()),
		m_ptr(::GlobalLock(m_hg))
	{
	}

	~GlobalLock()
	{
		::GlobalUnlock(m_hg);
	}

	T* Ptr() const
	{
		return static_cast<T*>(m_ptr);
	}

private:
	HGLOBAL m_hg;
	void* m_ptr;
};

class GdiObjectSelection : fusion::noncopyable
{
public:
	GdiObjectSelection(HDC hdc, HGDIOBJ hObject);
	~GdiObjectSelection();

private:
	HDC m_hdc;
	HGDIOBJ m_hObject;
};

class ScopedTextColor : fusion::noncopyable
{
public:
	ScopedTextColor(HDC hdc, COLORREF color);
	~ScopedTextColor();

private:
	HDC m_hdc;
	COLORREF m_color;
};

class ScopedBkColor : fusion::noncopyable
{
public:
	ScopedBkColor(HDC hdc, COLORREF color);
	~ScopedBkColor();

private:
	HDC m_hdc;
	COLORREF m_color;
};

class ScopedTextAlign : fusion::noncopyable
{
public:
	ScopedTextAlign(HDC hdc, UINT align);
	~ScopedTextAlign();

private:
	HDC m_hdc;
	UINT m_align;
};

class Win32Error : public std::system_error
{
public:
	Win32Error(DWORD error, const std::string& what);
};

std::wstring MultiByteToWideChar(std::string_view str);
std::wstring MultiByteToWideChar_std(std::string_view str);
std::string WideCharToMultiByte(std::wstring_view str);

void ThrowWin32Error(DWORD error, const std::string& what);
void ThrowLastError(const std::string& what);
void ThrowLastError(const std::wstring& what);

SYSTEMTIME GetSystemTime();
SYSTEMTIME GetLocalTime();
FILETIME GetSystemTimeAsFileTime();
FILETIME FileTimeToLocalFileTime(const FILETIME& ft);
FILETIME LocalFileTimeToFileTime(const FILETIME& ft);
SYSTEMTIME FileTimeToSystemTime(const FILETIME& ft);
FILETIME SystemTimeToFileTime(const SYSTEMTIME& st);

Handle CreateFileMapping(HANDLE hFile, const SECURITY_ATTRIBUTES* pAttributes, DWORD protect, DWORD maximumSizeHigh, DWORD maximumSizeLow, const wchar_t* pName);

Handle OpenProcess(DWORD dwDesiredAccess, BOOL bInheritHandle, DWORD dwProcessId);

Handle CreateEvent(const SECURITY_ATTRIBUTES* pEventAttributes, bool manualReset, bool initialState, const wchar_t* pName);

void SetEvent(Handle& hEvent);
void SetEvent(HANDLE hEvent);

Handle CreateMutex(const SECURITY_ATTRIBUTES* pMutexAttributes, bool initialOwner, const wchar_t* pName);

#define DACL_MODIFY

#ifdef DACL_MODIFY
void SetSecurityInfo(HANDLE hObject, SE_OBJECT_TYPE ObjectType, SECURITY_INFORMATION SecurityInfo, const PSID psidOwner, const PSID psidGroup, const PACL pDacl, const PACL pSacl);
void DeleteObjectDACL(HANDLE hObject);
void AdjustObjectDACL(HANDLE hObject);
#endif

void SetPrivilege(const wchar_t* privilege, bool enablePrivilege);
void SetPrivilege(HANDLE hToken, const wchar_t* privilege, bool enablePrivilege);

struct WaitResult
{
	explicit WaitResult(bool signaled = false, int index = 0);
	bool signaled;
	int index;
};

void WaitForSingleObject(HANDLE hObject);
bool WaitForSingleObject(HANDLE hObject, DWORD milliSeconds);
void WaitForSingleObject(const Handle& hObject);
bool WaitForSingleObject(const Handle& hObject, DWORD milliSeconds);

WaitResult WaitForMultipleObjects(const HANDLE* begin, const HANDLE* end, bool waitAll, DWORD milliSeconds);

template <typename Coll>
WaitResult WaitForMultipleObjects(const Coll& handles, bool waitAll, DWORD milliSeconds)
{
	return WaitForMultipleObjects(handles.data(), handles.data() + handles.size(), waitAll, milliSeconds);
}

WaitResult WaitForAnyObject(const HANDLE* begin, const HANDLE* end, DWORD milliSeconds);

template <typename Coll>
WaitResult WaitForAnyObject(const Coll& handles, DWORD milliSeconds)
{
	return WaitForMultipleObjects(handles, false, milliSeconds);
}

WaitResult WaitForAllObjects(const HANDLE* begin, const HANDLE* end, DWORD milliSeconds);

template <typename Coll>
WaitResult WaitForAllObjects(const Coll& handles, DWORD milliSeconds)
{
	return WaitForMultipleObjects(handles, true, milliSeconds);
}

bool IsProcessRunning(HANDLE handle);

class MutexLock : fusion::noncopyable
{
public:
	explicit MutexLock(HANDLE hMutex);
	~MutexLock();

	void Release();

private:
	HANDLE m_hMutex;
};

class MappedViewOfFile : fusion::noncopyable
{
public:
	MappedViewOfFile(HANDLE hFileMappingObject, DWORD access, DWORD offsetHigh, DWORD offsetLow, size_t bytesToMap);
	~MappedViewOfFile();

	void* Ptr();
	const void* Ptr() const;

private:
	void* m_ptr;
};

class ScopedCursor : fusion::noncopyable
{
public:
	explicit ScopedCursor(HCURSOR hCursor);
	ScopedCursor(ScopedCursor&& sc);
	~ScopedCursor();

private:
	HCURSOR m_hCursor;
};

DWORD GetParentProcessId();
std::vector<std::wstring> GetCommandLineArguments();

DWORD GetExitCodeProcess(HANDLE hProcess);
DWORD GetExitCodeProcess(const Handle& hProcess);

std::wstring GetWindowText(HWND hWnd);
std::wstring GetDlgItemText(HWND hDlg, int idc);
bool IsGUIThread();

class HFile : fusion::noncopyable
{
public:
	explicit HFile(const std::string& filename);
	~HFile();
	size_t size() const;
	void resize(size_t size) const;

private:
	int m_handle;
};

std::wstring GetSEHcodeDescription(DWORD code);
std::wstring GetHresultMessage(HRESULT hr);
std::wstring GetHresultName(HRESULT hr);
std::wstring GetHresultDescription(HRESULT hr);

class JobObject
{
public:
    JobObject();
    HANDLE get() const;
    void AddProcessById(DWORD processId) const;
    void AddProcessByHandle(HANDLE processHandle) const;

private:
    Handle m_jobHandle;
};

Handle DuplicateHandle(HANDLE handle);

} // namespace Win32

bool operator==(const FILETIME& ft1, const FILETIME& ft2);
bool operator!=(const FILETIME& ft1, const FILETIME& ft2);
bool operator<(const FILETIME& ft1, const FILETIME& ft2);
bool operator>=(const FILETIME& ft1, const FILETIME& ft2);
bool operator>(const FILETIME& ft1, const FILETIME& ft2);
bool operator<=(const FILETIME& ft1, const FILETIME& ft2);

} // namespace fusion
