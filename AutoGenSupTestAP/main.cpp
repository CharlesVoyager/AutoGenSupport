// AutoGenSupTestAP.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "pch.h"
#include <iostream>
#include <vector>
#include <limits>       // std::numeric_limits
#include "../AutoGenSupDLL/Helper/MyTools.h"
#include "Sp3ApiTest.h"
#include "IniFile.h"
#include "Sp3GenBasicMeshTest.h"

using namespace std;
using namespace MyTools;

int main()
{
    std::cout << "=== C++ Support Module (AutoGenSupDLL.dll) Test Program ===\n";

	HINSTANCE dllHandler = LoadLibrary(L"AutoGenSupDLL.dll");

	if (dllHandler == nullptr)
	{
		printf("Error: Unable to find AutoGenSupDLL.dll (C++).\n");
		printf("Press a key to exit.\n");
		int c = getchar();
		return 0;
	}

	string iniFilename = "AutoGenSupTestAP.ini";
	std::cout << "Load the ini file: " << iniFilename.c_str() << std::endl;

	IniFile MyIni(iniFilename);

	if (MyIni.IniFileExists() == false)
	{
		printf("Error: Unable to find AutoGenSupTestAP.ini file.\n");
		printf("Press a key to exit.\n");
		int c = getchar();
		return 0;
	}

	Sp3ApiTest test(iniFilename, dllHandler);
	//Sp3GenBasicMeshTest test(dllHandler);

	for (int i = 0; i < 1; i++)
	{
		test.Run();

		printf("================= Test %d Complete =================\n", i+1);
	}

	bool fFreeResult = FreeLibrary(dllHandler);
	printf("Press a key to exit.\n");
	int c = getchar();
	return 0;
}