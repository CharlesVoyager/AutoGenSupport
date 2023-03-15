// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#ifdef _WINDOWS

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#define NOMINMAX
#include <windows.h>

#elif __APPLE__

#define __FUNCTION__

#endif

#include "Helper/MyTools.h"
#ifndef M_PI
#define M_PI (3.14159265358979323846)	
#endif	//C#: 3.1415926535897931;

// reference additional headers your program requires here