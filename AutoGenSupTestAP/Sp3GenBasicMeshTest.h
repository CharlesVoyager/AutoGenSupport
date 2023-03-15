#pragma once
#include <Windows.h>
#include "Common.h"
#include "../AutoGenSupDLL/Helper/MyTools.h"

typedef unsigned char*(*pSP3_GenBasicMesh)(ModelType supType, int &meshSize);

class Sp3GenBasicMeshTest
{
	pSP3_GenBasicMesh SP3_GenBasicMesh = nullptr;

public:
	Sp3GenBasicMeshTest(HINSTANCE dllHandler)
	{
		SP3_GenBasicMesh = (pSP3_GenBasicMesh)GetProcAddress(dllHandler, "SP3_GenBasicMesh");
	}

	void Run()
	{
		int meshSize = 0;

		unsigned char *cell = SP3_GenBasicMesh(ModelType::BED_CELL, meshSize);
		printf("BED_CELL mesh size: %d\n", meshSize);

		BinFile cellFile("bedCell.stl");
		cellFile.Write(cell, meshSize);

		unsigned char *pCuboid = SP3_GenBasicMesh(ModelType::CUBOID_CELL, meshSize);
		printf("CUBOID_CELL mesh size: %d\n", meshSize);

		BinFile cuboidFile("cuboid.stl");
		cuboidFile.Write(pCuboid, meshSize);

		unsigned char *pTab = SP3_GenBasicMesh(ModelType::TAB, meshSize);
		printf("TAB mesh size: %d\n", meshSize);
		BinFile tabFile("tab.stl");
		tabFile.Write(pTab, meshSize);
	}
};
