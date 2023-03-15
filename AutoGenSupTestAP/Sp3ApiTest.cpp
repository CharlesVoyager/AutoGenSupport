#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <limits>       // std::numeric_limits
#include <direct.h>
#include "Sp3ApiTest.h"
#include "../AutoGenSupDLL/Helper/MyTools.h"
#include "../AutoGenSupDLL/geom/Matrix4.h"
#include "IniFile.h"

using namespace std;
using namespace MyTools;
using namespace XYZSupport;

void Sp3ApiTest::Print_Percentage(float progress, bool &isCancelled)
{
	printf("Progress: %.2f%%\r", progress);
	//printf("Progress: %.2f%%\n", progress);

	// TEST: 0 ~ 20: Slice, 20 ~ 50: Auto/Manual Symbols, 50 ~ 90: genSupFromList, 90 ~ 95: Bed, 95 ~ 100: GenMesh.

	//if (progress > 30 && progress < 40)
	//	isCancelled = true;

	//if (progress > 60 && progress < 70)
	//	isCancelled = true;

	//if (progress > 90 && progress < 95)
	//	isCancelled = true;

	//if (progress > 95)
	//	isCancelled = true;
}

Sp3ApiTest::Sp3ApiTest(string _testIniFile, HINSTANCE _dllHandler) : Sp3ApiTestBase(_testIniFile, _dllHandler)
{

}

int Sp3ApiTest::Run()
{
	IniFile MyIni(testIniFile);
	string strEditMode = MyIni.ReadStr("EditMode", "Main");
	string strTypeCode = MyIni.ReadStr("TypeCode", "Main");

	MeshTypeCode typeCode;

	if (strTypeCode == "Normal")
		typeCode = MeshTypeCode::Normal;
	else if (strTypeCode == "Mark")
		typeCode = MeshTypeCode::Mark;
	else
		typeCode = MeshTypeCode::Normal;

	if (strEditMode == "Normal")
		return GenSupportCylinder(PointEditModeCode::Normal, typeCode);
	else if(strEditMode == "User")
		return GenSupportCylinder(PointEditModeCode::User, typeCode);
	else if (strEditMode == "Auto")
		return GenSupportCylinder(PointEditModeCode::Auto, typeCode);
	else if (strEditMode == "Cone")
		return GenSupportCone(typeCode);
	else if (strEditMode == "Tree")
		return GenSupportTree(typeCode);
	else
		return GenSupportCylinder(PointEditModeCode::Normal, typeCode);
}

unsigned char* Sp3ApiTest::getStlBufferFromStlFile(float trans[16], /*out*/float uiPosition[3])
{
	bool blRet = false;
	bool innerSupport = false;
	int ret = 0;
	IniFile MyIni(testIniFile);

	string stlFile = MyIni.ReadStr("Model_Filename", testName);
	printf("Model_Filename: %s\n", stlFile.c_str());

	// Read a STL file
	unsigned char *bufferStl = ReadAllBytes(stlFile);
	if (bufferStl == nullptr) return nullptr;

	// Get bounding box center
	BoundingBox bbox = GetModelBoundingBox(bufferStl);
	MyTools::Vector3 bboxCenterOri = bbox.GetCenter();
	
	// The bufferStl is world coordinate. It needs to changed to model coordinate.
	ResetVertexPosToBBox(bufferStl, bboxCenterOri);

	printf("MinPoint of Bounding Box: %f %f %f\n", bbox.minPoint.x, bbox.minPoint.y, bbox.minPoint.z);
	printf("MaxPoint of Bounding Box: %f %f %f\n", bbox.maxPoint.x, bbox.maxPoint.y, bbox.maxPoint.z);
	printf("Center of Bounding Box: %f %f %f\n", bboxCenterOri.x, bboxCenterOri.y, bboxCenterOri.z);

	//float uiPosition[3];		// x, y are center of the model. z is the bottom of the model.
	float modelPosition[3];		// x, y, z are all center of the model
	float modelScale[3];
	float modelRotation[3];

	blRet = MyIni.ReadFloatXYZ("UI_Position", testName, uiPosition); if (blRet == false) return 0;
	blRet = MyIni.ReadFloatXYZ("Model_Scale", testName, modelScale); if (blRet == false) return 0;
	blRet = MyIni.ReadFloatXYZ("Model_Rotation", testName, modelRotation); if (blRet == false) return 0;

	modelPosition[0] = uiPosition[0];
	modelPosition[1] = uiPosition[1];
	modelPosition[2] = uiPosition[2] + bboxCenterOri.z - bbox.minPoint.z;	// Note: Subtracting bbox.minPoint.z is to ensure the model's z poistion is measuring from land.

	printf("UI_Position:	%f %f %f\n", uiPosition[0], uiPosition[1], uiPosition[2]);
	printf("Model_Position: %f %f %f\n", modelPosition[0], modelPosition[1], modelPosition[2]);
	printf("Model_Scale:	%f %f %f\n", modelScale[0], modelScale[1], modelScale[2]);
	printf("Model_Rotation: %f %f %f\n", modelRotation[0], modelRotation[1], modelRotation[2]);

	Matrix4 transM4 = Matrix4::ToTransMatrix(	XYZSupport::Vector3(modelPosition[0], modelPosition[1], modelPosition[2]),
												XYZSupport::Vector3(modelScale[0], modelScale[1], modelScale[2]),
												XYZSupport::Vector3(modelRotation[0], modelRotation[1], modelRotation[2]));
	memcpy(trans, transM4.get(), 16 * sizeof(float));

	printf("====================================\n");
	printf("Transformation Matrix: \n");
	PrintMatrix4(trans);
	printf("====================================\n");

	// Get bounding box after the model applies transformation matrix
	vector<unsigned char> stlBufferWor;
	ModelStlBinaryToWorldStlBinary(bufferStl, trans, stlBufferWor);
	bbox = GetModelBoundingBox(&stlBufferWor[0]);
	printf("Update Bounding Box with applying transformation matrix\n");
	printf("MinPoint of Bounding Box: %f %f %f\n", bbox.minPoint.x, bbox.minPoint.y, bbox.minPoint.z);
	printf("MaxPoint of Bounding Box: %f %f %f\n", bbox.maxPoint.x, bbox.maxPoint.y, bbox.maxPoint.z);

	// the min z of bounding box should be the z of UI position
	modelPosition[2] = modelPosition[2] + (uiPosition[2] - bbox.minPoint.z);

	// Update transformation matrix
	trans[11] = modelPosition[2];
	printf("====================================\n");
	printf("Finalize Tranformation Matrix: \n");
	PrintMatrix4(trans);
	printf("====================================\n");

	return bufferStl;
}
int Sp3ApiTest::getThreshold(int threshold[4]) // get thredshold parameter for automatically support points
{
	IniFile MyIni(testIniFile);
	string density("MED");

	if (density == "MED")
	{
		threshold[0] = 50000;
		threshold[1] = 50000;
		threshold[2] = 50000;
		threshold[3] = 20000;
	}
	else if (density == "LOW")
	{
		threshold[0] = 80000;
		threshold[1] = 80000;
		threshold[2] = 80000;
		threshold[3] = 35000;
	}
	else
	{
		threshold[0] = 20000;
		threshold[1] = 20000;
		threshold[2] = 20000;
		threshold[3] = 5000;
	}

	return 0;
}

// The following code is referenced from SupportMeshsBed.cs.
float Sp3ApiTest::getNewBaseZScale()
{
	IniFile MyIni(testIniFile);
	int nSupportBedStyle = MyIni.ReadInt("SupportBedStyle", "Main");

	if (nSupportBedStyle == 1)
	{
		return 1.2f;//3mm
	}
	else
		return 0;
}

bool Sp3ApiTest::hasEasyReleaseTab()
{
	IniFile MyIni(testIniFile);
	int nSupportBedStyle = MyIni.ReadInt("SupportBedStyle", "Main");

	if (nSupportBedStyle == 1)
	{
		return true;
	}
	else
		return false;
}

int Sp3ApiTest::addSupportPointAuto()
{
	int ret = 0;

	// Generate SupportPointsList and set Symbols
	int threshold[4];
	getThreshold(threshold);

	// Generate Support Points List
	IniFile MyIni(testIniFile);

	double contactSize = MyIni.ReadFloat("UIAutoContactSize", "Main");

	if (SP3_AddSupportPointAuto)
		ret = SP3_AddSupportPointAuto(	L"MED",
										contactSize / 2.0,	// UI: 0.3mm ~ 1.3mm
										threshold,
										L".",	//Temp Folder
										nullptr);

	printf("\n");
	return ret;
}

/* The following code snippet from SupportMeshsCylinder.cs. In order to support bed (honeycomb base), the model must lift at least 3mm (or 6mm) or higher from base.

	------------------------------------------------------------------------------------
		int heightThreshold = 6;
		if ((machineType == PrinterType.NSF)
		|| (machineType == PrinterType.NCASTPRO100XP)
		|| (machineType == PrinterType.NPARTPRO120XP)
		|| (machineType == PrinterType.NDENTPRO100XP)
		|| (machineType == PrinterType.NPARTPRO100XP))
		{
			heightThreshold = 3;
		}
		if (Math.Round(liftVal, 2, MidpointRounding.AwayFromZero) >= heightThreshold)
			supTail = GenADDsupBed(n);
		else
			supTail = GenADDsupTail(n);
	------------------------------------------------------------------------------------
*/
int Sp3ApiTest::GenSupportCylinder(PointEditModeCode mode, MeshTypeCode typeCode)
{
	printf("============================================\n");
	printf(" GenSupportCylinder Test Start  [%s]        \n", testName.c_str());
	printf("============================================\n");
	/*
		Preparation
	*/
	float modelTrans[16];
	float uiPosition[3];
	unsigned char* stlBinModel = getStlBufferFromStlFile(/*out*/modelTrans, uiPosition);
	if (stlBinModel == nullptr) return -1;

	// convert bufferStl (model coordination) and trans to world-coordination for automatically generating support points.
	vector<unsigned char> stlBufferWor;
	ModelStlBinaryToWorldStlBinary(stlBinModel, modelTrans, stlBufferWor);
	/*
		Start calling SP3 APIs.
	*/
	Stopwatch sw;
	sw.Start();

	// Step 1: Create
	int ret = SP3_Create(mode, typeCode, false, getNewBaseZScale(), hasEasyReleaseTab(), Print_Percentage);	// false means without internal support

	// Step 2: Set TopoModel and transformation
	ret = SP3_SetModelAndTrans(stlBinModel, modelTrans);

	// Step 3: Add Symbols to C++ support module.
	if (mode == PointEditModeCode::Normal || mode == PointEditModeCode::Auto)
		ret = addSupportPointAuto();
	else
	{
		//double pos1[3] = { 30.5, 18.5, 20 };
		//double pos2[3] = { uiPosition[0] + 3,  uiPosition[1],  uiPosition[2] };
		
		double pos1[3] = { 29, 17, 10 };
		double pos2[3] = { 35, 23, 10 };

		double dir[3] = { 0, 0, -1 };
		IniFile MyIni(testIniFile);
		double contactSize = MyIni.ReadFloat("UIManualContactSize", "Main");
		ret = SP3_AddSupportPointManual(pos1, dir, contactSize/2.0);
		printf("\n===> Add Symbol: [Pos] %f %f %f\n", pos1[0], pos1[1], pos1[2]);
		//ret = SP3_AddSupportPointManual(pos2, dir, 0.3);
		//printf("===> Add Symbol: [Pos] %f %f %f\n", pos1[0], pos1[1], pos1[2]);
	}

	int symbolsCount = SP3_GetSupportPointsCount();

	if (symbolsCount == 0)
	{	// Error
		printf("\n");
		printf(" Error: No Support Points generated. Check if Slicer2.0DLL.dll exists in the folder where executable file is.\n");
		printf("\n");
		printf("End the program.\n");

		// Step 6: Release.
		ret = SP3_Release();
		free(stlBinModel);
		return -1;
	}
	else
	{	// OK
		printf("Current Support Points Count before generating supports: %d\n", symbolsCount);
	}

	// Step 4: Generate supports.
	int countSupportData = 0;
	ret = SP3_GenSupportDataWOMesh(countSupportData);
	printf("\n===> SupportData Count: %d\n", countSupportData);

	// Step 5: Get the STL binary for each support
	vector<MeshData> vecStlBinary;

	// add model as the first of vecStlBinary
	MeshData stlBinary;
	stlBinary.mesh = stlBinModel;
	memcpy(stlBinary.trans, modelTrans, sizeof(stlBinary.trans));
	vecStlBinary.push_back(stlBinary);

	vecSupportDataWOMesh.clear();
	for (int i = 0; i < countSupportData; i++)
	{
#if 1	// NOTE: "supportDataWOMesh.bufferBoundingBox" will not be available untill meshes are generated!
		float trans[16];

		int stlBufferSize = 0;
		unsigned char *stlBuffer = SP3_GenSupportDataMesh(i, stlBufferSize, trans);

		stlBinary.mesh = stlBuffer;
		memcpy(stlBinary.trans, trans, sizeof(trans));
		vecStlBinary.push_back(stlBinary);
#endif
		// Get SupportDataWOMesh
		SupportDataWOMesh supportDataWOMesh;
		SUP_GetSupportDataWOMesh(i,
									supportDataWOMesh.bufferPosition, supportDataWOMesh.bufferScale, supportDataWOMesh.bufferRotation,
									supportDataWOMesh.supType,															// out
									supportDataWOMesh.bufferOrientation,												// out
									supportDataWOMesh.bufferCurPos, supportDataWOMesh.bufferCurPos2, supportDataWOMesh.bufferTrans,		// out
									supportDataWOMesh.bufferBoundingBox,												// out: min point, max point
									supportDataWOMesh.depthAdjust,														// out
									supportDataWOMesh.length, supportDataWOMesh.radius1, supportDataWOMesh.radius2, supportDataWOMesh.bufferMatrix);	// out
		vecSupportDataWOMesh.push_back(supportDataWOMesh);
		// End of SupportDataWOMesh
	}

	symbolsCount = SP3_GetSupportPointsCount();
	printf("Current Support Points Count after generating supports: %d\n", symbolsCount);

	sw.Stop();
	printf("Elapsed Milli Seconds: %ld\n", sw.ElapsedMilliseconds());

	/* 
		Dump the support result
		NOTE: This dump must before SP3_Release(); otherwise, the buffer in vecStlBinary will be released.
	*/
	_mkdir(".\\Output");

	string resultFilename = ".\\Output\\" + testName + "_SupCylinder.stl";
	MeshDataToStlFile(vecStlBinary, resultFilename);
	printf("%s file has been generated...\n", resultFilename.c_str());
	// <>

	// Step 6: Release.
	ret = SP3_Release();

	printf("\n");

	free(stlBinModel);

#if 0 // Output a STL file with only supports.
	// *********************************************************
	AddSupportDataWOMeshTest(mode, typeCode);
	// *********************************************************
#endif
	
	return 0;
}

int Sp3ApiTest::GenSupportCone(MeshTypeCode typeCode)
{
	pSP3_AddSupportSymbolCone SP3_AddSupportSymbolCone = nullptr;
	SP3_AddSupportSymbolCone = (pSP3_AddSupportSymbolCone)GetProcAddress(dllHandler, "SP3_AddSupportSymbolCone");

	printf("============================================\n");
	printf("           Support Cone Test Start  [MeshTypeCode: %s]    \n", typeCode == MeshTypeCode::Normal ? "Normal" : "Mark");
	printf("============================================\n");
	
	/*
		Start calling SP3 APIs.
	*/
	Stopwatch sw;
	sw.Start();

	// Step 1: create
	int ret = SP3_Create(PointEditModeCode::Cone, typeCode, false, 0, hasEasyReleaseTab(), Print_Percentage);

	// Step 2: Add Symbols to C++ modules and dump the support points
	double depthAdjust = 0.5;
	double pos[3] = { 30, 17, 20, };
	double radius_top = 1;		//UI: 2mm
	double radius_base = 1.5;	//UI: 3mm

	printf("\nPrint Support Symbol paramters: \n");
	printf("===> depthAdjust: %f\n", depthAdjust);
	printf("===> pos: %f %f %f\n", pos[0], pos[1], pos[2]);
	printf("===> radius_top: %f\n", radius_top);
	printf("===> radius_base: %f\n", radius_base);
	ret = SP3_AddSupportSymbolCone(depthAdjust, pos, radius_top, radius_base);

	// Step 3: Generate supports.
	int countSupportData = 0;
	ret = SP3_GenSupportDataWOMesh(countSupportData);
	printf("\n===> SupportData Count: %d\n", countSupportData);

	// Step 4: Get the STL binary for each support
	vector<MeshData> vecStlBinary;

	vecSupportDataWOMesh.clear();
	for (int i = 0; i < countSupportData; i++)
	{
		// Get SupportDataWOMesh
		SupportDataWOMesh supportDataWOMesh;
		SP3_GetSupportDataWOMesh(i,
			supportDataWOMesh.supType,
			supportDataWOMesh.depthAdjust,
			supportDataWOMesh.length, supportDataWOMesh.radius1, supportDataWOMesh.radius2, supportDataWOMesh.bufferMatrix);

		vecSupportDataWOMesh.push_back(supportDataWOMesh);
		// End of SupportDataWOMesh

		float trans[16];

		int stlBufferSize = 0;
		unsigned char *stlBuffer = SP3_GenSupportDataMesh(i, stlBufferSize, trans);

		MeshData meshData;
		meshData.mesh = stlBuffer;
		memcpy(meshData.trans, trans, sizeof(trans));
		vecStlBinary.push_back(meshData);
	}

	sw.Stop();
	printf("Elapsed Milli Seconds: %ld\n", sw.ElapsedMilliseconds());

	/*
		Dump the support result
		NOTE: This dump must before SP3_Release(); otherwise, the buffer in vecStlBinary will be released.
	*/
	string resultFilename = "SupCone.stl";
	if (typeCode == MeshTypeCode::Normal)
		resultFilename = "SupConeNormal.stl";
	else if (typeCode == MeshTypeCode::Mark)
		resultFilename = "SupConeMark.stl";

	MeshDataToStlFile(vecStlBinary, resultFilename);

	// Step 5: Release.
	ret = SP3_Release();

	printf("\n%s file has been generated...\n", resultFilename.c_str());

	// *********************************************************
	AddSupportDataWOMeshTest(PointEditModeCode::Cone, typeCode);
	// *********************************************************
	return 0;
}

int Sp3ApiTest::GenSupportTree(MeshTypeCode typeCode)
{
	pSP3_AddSupportSymbolTree SP3_AddSupportSymbolTree = nullptr;
	SP3_AddSupportSymbolTree = (pSP3_AddSupportSymbolTree)GetProcAddress(dllHandler, "SP3_AddSupportSymbolTree");

	printf("============================================\n");
	printf("           Support Tree Test Start  [MeshTypeCode: %s]    \n", typeCode == MeshTypeCode::Normal ? "Normal":"Mark");
	printf("============================================\n");

	/*
		Start calling SP3 APIs.
	*/
	Stopwatch sw;
	sw.Start();

	// Step 1: create
	int ret = SP3_Create(PointEditModeCode::Tree, typeCode, false, 0, hasEasyReleaseTab(), Print_Percentage);

#if 1
	{// Add Trunk Symbol
		double depthAdjust = 0.5;
		double position_top[3] = { 70, 80, 20 };
		double position_base[3] = { 70, 80, 0 };
		double radius_top = 0.5;
		double radius_base = 1.0;
		int type = 3;

		ret = SP3_AddSupportSymbolTree(depthAdjust, position_top, position_base, radius_top, radius_base, type);
	}

	{// Add Branch Symbol
		double depthAdjust = 0.5;
		double position_top[3] = { 73, 80, 24 };
		double position_base[3] = { 70,80, 20 };
		double radius_top = 0.15;
		double radius_base = 0.3;
		int type = 4;

		ret = SP3_AddSupportSymbolTree(depthAdjust, position_top, position_base, radius_top, radius_base, type);
	}
#endif

	// Step 3: Generate supports.
	int countSupportData = 0;
	ret = SP3_GenSupportDataWOMesh(countSupportData);
	printf("===> SupportData Count: %d\n", countSupportData);

	// Step 4: Get the STL binary for each support
	vector<MeshData> vecStlBinary;

	// add model as the first of vecStlBinary
	MeshData stlBinary;

	vecSupportDataWOMesh.clear();
	for (int i = 0; i < countSupportData; i++)
	{
		// Get SupportDataWOMesh
		SupportDataWOMesh supportDataWOMesh;
		SP3_GetSupportDataWOMesh(i,
			supportDataWOMesh.supType,
			supportDataWOMesh.depthAdjust,
			supportDataWOMesh.length, supportDataWOMesh.radius1, supportDataWOMesh.radius2, supportDataWOMesh.bufferMatrix);

		vecSupportDataWOMesh.push_back(supportDataWOMesh);
		// End of SupportDataWOMesh

		// CharlesTest
		DbgMsg("===> [%d] supType: %d", i, vecSupportDataWOMesh[i].supType);
		DbgMsg("===> [%d] depthAdjust: %f", i, vecSupportDataWOMesh[i].depthAdjust);
		DbgMsg("===> [%d] length: %f", i, vecSupportDataWOMesh[i].length);
		DbgMsg("===> [%d] radius1: %f", i, vecSupportDataWOMesh[i].radius1);
		DbgMsg("===> [%d] radius2: %f", i, vecSupportDataWOMesh[i].radius2);
		printf("SupportData Matrix [%d]: \n", i);
		PrintMatrix4(vecSupportDataWOMesh[i].bufferMatrix);
		//End of tets

		float trans[16];
		int stlBufferSize = 0;

		unsigned char *stlBuffer = SP3_GenSupportDataMesh(i, stlBufferSize, trans);

		stlBinary.mesh = stlBuffer;
		memcpy(stlBinary.trans, trans, sizeof(trans));
		vecStlBinary.push_back(stlBinary);
	}

	sw.Stop();
	printf("Elapsed Milli Seconds: %ld\n", sw.ElapsedMilliseconds());

	/*
		Dump the support result
		NOTE: This dump must before SP3_Release(); otherwise, the buffer in vecStlBinary will be released.
	*/
	string resultFilename = "SupTree.stl";
	if(typeCode == MeshTypeCode::Normal)
		resultFilename = "SupTreeNormal.stl";
	else if (typeCode == MeshTypeCode::Mark)
		resultFilename = "SupTreeMark.stl";

	MeshDataToStlFile(vecStlBinary, resultFilename);

	// Step 5: Release.
	ret = SP3_Release();

	printf("%s file has been generated...\n", resultFilename.c_str());

	// *********************************************************
	AddSupportDataWOMeshTest(PointEditModeCode::Tree, typeCode);
	// *********************************************************

	return 0;
}
int Sp3ApiTest::AddSupportDataWOMeshTest(PointEditModeCode mode, MeshTypeCode typeCode)
{
	printf("============================================\n");
	printf("                Test Start  [AddSupportDataWOMeshTest ===> mode: %d, typeCode: %d]        \n", mode, typeCode);
	printf("============================================\n");

	// Step 1: create
	int ret = SP3_Create(mode, typeCode, true, getNewBaseZScale(), hasEasyReleaseTab(), Print_Percentage);
	
	printf("===> vecSupportDataWOMesh.size(): %zu\n", vecSupportDataWOMesh.size());

	for (int i = 0; i < vecSupportDataWOMesh.size(); i++)
	{
		ret = SP3_AddSupportDataWOMesh(i, 
										vecSupportDataWOMesh[i].supType,
										vecSupportDataWOMesh[i].depthAdjust,
										vecSupportDataWOMesh[i].length, vecSupportDataWOMesh[i].radius1, vecSupportDataWOMesh[i].radius2, vecSupportDataWOMesh[i].bufferMatrix);
	}

	vector<MeshData> vecStlBinary;
	MeshData stlBinary;
	for (int i = 0; i < vecSupportDataWOMesh.size(); i++)
	{
		float trans[16];
		int stlBufferSize = 0;
		unsigned char *stlBuffer = SP3_GenSupportDataMesh(i, stlBufferSize, trans);

		// Charles: TEST
		//printf("GenSupportData() %d trans: \n", i);
		//PrintMatrix4(trans);
		// End of test


		if (stlBuffer != nullptr && stlBufferSize > 0)
		{
			stlBinary.mesh = stlBuffer;
			memcpy(stlBinary.trans, trans, sizeof(trans));
			vecStlBinary.push_back(stlBinary);
		}
	}

	/*
		Dump the support result
		NOTE: This dump must before SP3_Release(); otherwise, the buffer in vecStlBinary will be released.
	*/
	string resultFilename = "AddSupportData.stl";

	if (typeCode == MeshTypeCode::Normal)
		resultFilename = "AddSupportDataNormal.stl";
	else if (typeCode == MeshTypeCode::Mark)
		resultFilename = "AddSupportDataMark.stl";

	MeshDataToStlFile(vecStlBinary, resultFilename);

	// Step 5: Release.
	ret = SP3_Release();

	printf("%s file has been generated...\n", resultFilename.c_str());
	return 0;
}