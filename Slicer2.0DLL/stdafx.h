// stdafx.h : �i�b�����Y�ɤ��]�t�зǪ��t�� Include �ɡA
// �άO�g�`�ϥΫo�ܤ��ܧ�
// �M�ױM�� Include �ɮ�
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



