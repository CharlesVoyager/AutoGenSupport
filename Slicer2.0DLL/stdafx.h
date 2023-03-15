// stdafx.h : 可在此標頭檔中包含標準的系統 Include 檔，
// 或是經常使用卻很少變更的
// 專案專用 Include 檔案
//

#pragma once

#ifdef _WINDOWS

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows Headers
// Windows Header Files
#include <windows.h>

#elif __APPLE__

typedef int BOOL;
#define FALSE 0
typedef const wchar_t *LPCWSTR;
typedef wchar_t TCHAR;

#endif



