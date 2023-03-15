#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <limits>       // std::numeric_limits
#include "Sp3ApiTestBase.h"
#include "../AutoGenSupDLL/Helper/MyTools.h"
#include "IniFile.h"

using namespace std;
using namespace MyTools;

Sp3ApiTestBase::Sp3ApiTestBase(string _testIniFile, HINSTANCE _dllHandler)
{
	testIniFile = _testIniFile;
	dllHandler = _dllHandler;

	IniFile MyIni(testIniFile);
	testName = MyIni.ReadStr("TestName", "Main");

	SP3_Create = (pSP3_Create)GetProcAddress(dllHandler, "SP3_Create");

	SP3_SetModelAndTrans = (pSP3_SetModelAndTrans)GetProcAddress(dllHandler, "SP3_SetModelAndTrans");
	SP3_AddSupportPointAuto = (pSP3_AddSupportPointAuto)GetProcAddress(dllHandler, "SP3_AddSupportPointAuto");
	SP3_AddSupportPointManual = (pSP3_AddSupportPointManual)GetProcAddress(dllHandler, "SP3_AddSupportPointManual");
	SP3_AddSupportSymbolCone = (pSP3_AddSupportSymbolCone)GetProcAddress(dllHandler, "SP3_AddSupportSymbolCone");
	SP3_GenSupportDataWOMesh = (pSP3_GenSupportDataWOMesh)GetProcAddress(dllHandler, "SP3_GenSupportDataWOMesh");
	SP3_AddSupportDataWOMesh = (pSP3_AddSupportDataWOMesh)GetProcAddress(dllHandler, "SP3_AddSupportDataWOMesh");
	SP3_GetSupportDataWOMesh = (pSP3_GetSupportDataWOMesh)GetProcAddress(dllHandler, "SP3_GetSupportDataWOMesh");

	SUP_GetSupportDataWOMesh = (pSUP_GetSupportDataWOMesh)GetProcAddress(dllHandler, "SUP_GetSupportDataWOMesh");

	SP3_GenSupportDataMesh = (pSP3_GenSupportDataMesh)GetProcAddress(dllHandler, "SP3_GenSupportDataMesh");
	
	SP3_GetSupportPointsCount = (pSP3_GetSupportPointsCount)GetProcAddress(dllHandler, "SP3_GetSupportPointsCount");
	SP3_GetSupportPoint = (pSP3_GetSupportPoint)GetProcAddress(dllHandler, "SP3_GetSupportPoint");

	SP3_Release = (pSP3_Release)GetProcAddress(dllHandler, "SP3_Release");
}

