#pragma once
#include <Windows.h>
#include "Sp3ApiTestBase.h"
#include "../AutoGenSupDLL/Helper/MyTools.h"

using namespace std;
using namespace MyTools;

class Sp3ApiTest : public Sp3ApiTestBase
{
private:
	unsigned char* getStlBufferFromStlFile(float trans[16], /*out*/ float uiPosition[3]);
	int getThreshold(int threshold[4]);	// get thredshold parameter for automatically support points

	float getNewBaseZScale();
	bool hasEasyReleaseTab();
	int addSupportPointAuto();
	
	int GenSupportCylinder(PointEditModeCode mode, MeshTypeCode typeCode);
	int GenSupportCone(MeshTypeCode typeCode);
	int GenSupportTree(MeshTypeCode typeCode);

	int AddSupportDataWOMeshTest(PointEditModeCode mode, MeshTypeCode typeCode);

	vector<SupportDataWOMesh> vecSupportDataWOMesh;

	static void Print_Percentage(float progress, bool &isCancelled);

public:
	Sp3ApiTest(string testIniFile, HINSTANCE dllHandler);

	int Run();	// depending ini file. Run Generate cylinder support, cone, or tree...
};
