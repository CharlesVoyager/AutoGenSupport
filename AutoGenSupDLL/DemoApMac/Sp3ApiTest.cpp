// XYZSupportTest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include <iostream>
#include <vector>
#include <limits>       // std::numeric_limits
#include "Sp3ApiTest.h"
#include "Sp3ApiForQt.h"
#include "XYZSupport.h"
#include "MyTools.h"
#include "Matrix4.h"

using namespace std;
using namespace MyTools;

Sp3ApiTest::Sp3ApiTest( string _editMode,
                        string _typeCode,
                        string _stlFilename,
                        string _position,
                        string _scale,
                        string _rotation,
                        string _innerSupport,
                        string _printerName,
                        string _density,
                        string _contactSize,
                        string _newBaseZScale)
{
    strEditMode = _editMode;
    strTypeCode = _typeCode;
    stlFilename = _stlFilename;
    StringToFloatXYZ(_position, position);
    StringToFloatXYZ(_scale, scale);
    StringToFloatXYZ(_rotation, rotation);
    
    if(_innerSupport == "true")
        innerSupport = true;
    else if (_innerSupport == "false")
        innerSupport = false;
    else
        innerSupport = false;
    printerName = _printerName;
    density.assign(_density.begin(), _density.end());
    contactSize = stod(_contactSize);
    newBaseZScale = stof(_newBaseZScale);
    
    // Display the parameter
    printf("Edit Mode: %s\n", strEditMode.c_str());
    printf("Type Code: %s\n", strTypeCode.c_str());
    printf("STL Filename: %s\n", stlFilename.c_str());
    printf("Position (X Y Z): %f %f %f\n", position[0], position[1], position[2]);
    printf("Scale (X Y Z): %f %f %f\n", scale[0], scale[1], scale[2]);
    printf("Rotation (X Y Z): %f %f %f\n", rotation[0], rotation[1], rotation[2]);
    printf("Printer Name: %s\n", printerName.c_str());
    wprintf(L"Density: %ls\n", density.c_str());
    printf("Contact Size: %f\n", contactSize);
    printf("newBaseZScale: %f\n", newBaseZScale);
}
            
void Print_Percentage(float progress, bool &isCancelled)
{
	printf("Progress: %.2f%%\r", progress);
}

int Sp3ApiTest::Run()
{
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
	else if (strEditMode == "Cone")
		return GenSupportCone(typeCode);
	else
		return GenSupportCylinder(PointEditModeCode::Normal, typeCode);
}

unsigned char* Sp3ApiTest::getStlBufferFromStlFile(float trans[16], /*out*/float _uiPosition[3])
{
    printf("Model_Filename: %s\n", stlFilename.c_str());
    
    // Read a STL file
    unsigned char *bufferStl = ReadAllBytes(stlFilename);
    if (bufferStl == nullptr) return nullptr;

    // Get bounding box center
    BoundingBox bbox = GetModelBoundingBox(bufferStl);
    MyTools::Vector3 bboxCenterOri = bbox.GetCenter();
    
    // The bufferStl is world coordinate. It needs to changed to model coordinate.
    ResetVertexPosToBBox(bufferStl, bboxCenterOri);

    printf("MinPoint of Bounding Box: %f %f %f\n", bbox.minPoint.x, bbox.minPoint.y, bbox.minPoint.z);
    printf("MaxPoint of Bounding Box: %f %f %f\n", bbox.maxPoint.x, bbox.maxPoint.y, bbox.maxPoint.z);
    printf("Center of Bounding Box: %f %f %f\n", bboxCenterOri.x, bboxCenterOri.y, bboxCenterOri.z);

    //float uiPosition[3];        // x, y are center of the model. z is the bottom of the model.
    float modelPosition[3];        // x, y, z are all center of the model
    float modelScale[3];
    float modelRotation[3];

    memcpy(_uiPosition, position, sizeof(position));
    memcpy(modelScale, scale, sizeof(scale));
    memcpy(modelRotation, rotation, sizeof(rotation));
    
    modelPosition[0] = _uiPosition[0];
    modelPosition[1] = _uiPosition[1];
    modelPosition[2] = _uiPosition[2] + bboxCenterOri.z - bbox.minPoint.z;    // Note: Subtracting bbox.minPoint.z is to ensure the model's z poistion is measuring from land.

    printf("UI_Position:    %f %f %f\n", _uiPosition[0], _uiPosition[1], _uiPosition[2]);
    printf("Model_Position: %f %f %f\n", modelPosition[0], modelPosition[1], modelPosition[2]);
    printf("Model_Scale:    %f %f %f\n", modelScale[0], modelScale[1], modelScale[2]);
    printf("Model_Rotation: %f %f %f\n", modelRotation[0], modelRotation[1], modelRotation[2]);

    Matrix4 transM4 = Matrix4::ToTransMatrix(    XYZSupport::Vector3(modelPosition[0], modelPosition[1], modelPosition[2]),
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
    modelPosition[2] = modelPosition[2] + (_uiPosition[2] - bbox.minPoint.z);

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
	if ((printerName == "Nobel 1.0A")			//PrinterType.N10A
		|| (printerName == "Nobel 1.0")			//PrinterType.N10
		|| (printerName == "MFGPRO3400 XP")		//PrinterType.NMFGPRO3400XP
		|| (printerName == "PartPro150 xP"))	//PrinterType.NPARTPRO150XP
	{
		if (density == L"MED")
		{
			threshold[0] = 30000;
			threshold[1] = 30000;
			threshold[2] = 30000;
			threshold[3] = 15000;
		}
		else if (density == L"LOW")
		{
			threshold[0] = 40000;
			threshold[1] = 40000;
			threshold[2] = 45000;
			threshold[3] = 25000;
		}
		else
		{
			threshold[0] = 20000;
			threshold[1] = 22000;
			threshold[2] = 22000;
			threshold[3] = 5000;
		}
	}
	else if ((printerName == "Nobel Superfine")	//PrinterType.NSF
		|| (printerName == "CastPro100 xP")		//PrinterType.NCASTPRO100XP
		|| (printerName == "PartPro120 xP")		//PrinterType.NPARTPRO120XP
		|| (printerName == "DentPro100 xP")		//PrinterType.NDENTPRO100XP
		|| (printerName == "PartPro100 xP"))	//PrinterType.NPARTPRO100XP
	{
		if (density == L"MED")
		{
			threshold[0] = 40000;
			threshold[1] = 30000;
			threshold[2] = 30000;
			threshold[3] = 25000;
		}
		else if (density == L"LOW")
		{
			threshold[0] = 50000;
			threshold[1] = 40000;
			threshold[2] = 45000;
			threshold[3] = 35000;
		}
		else
		{
			threshold[0] = 30000;
			threshold[1] = 22000;
			threshold[2] = 22000;
			threshold[3] = 15000;
		}
	}
	return 0;
}

// The following code is referenced from SupportMeshsBed.cs.
float Sp3ApiTest::getNewBaseZScale()
{
    return newBaseZScale;
#if 0
	string printerName = "Nobel Superfine";

	if ((printerName == "Nobel Superfine")		//PrinterType.NSF
		|| (printerName == "CastPro100 xP")		//PrinterType.NCASTPRO100XP
		|| (printerName == "PartPro120 xP")		//PrinterType.NPARTPRO120XP
		|| (printerName == "DentPro100 xP")		//PrinterType.NDENTPRO100XP
		|| (printerName == "PartPro120 xP"))	//PrinterType.NPARTPRO100XP
	{
		return 0.6f;//ori=2.5*1.2=3mm, NSF setting is 2.5*0.6=1.5mm
	}
	else
	{
		return 1.2f;//3mm
	}
#endif
}
int Sp3ApiTest::addSupportPointAuto()
{
	int ret = 0;

	// Generate SupportPointsList and set Symbols
	int threshold[4];
	getThreshold(threshold);

	// Generate Support Points List
    ret = SP3_AddSupportPointAuto(	density.c_str(),
                                    contactSize,
                                    threshold,
									L"./",                                                      // Temp Folder
                                    //L"/Users/cp23dsw/Library/Application Support/",           // Temp Folder
                                    //L"file:///Users/cp23dsw/Library/Application Support/",	// Temp Folder  ==> This does not work!
                                    L"",    // Mac OS does NOT need this parameter. Just input empty wstring.
                                    L"",    // Mac OS does NOT need this parameter. Just input empty wstring.
                                    Print_Percentage);
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
	printf(" GenSupportCylinder Test Start     \n");
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
	int ret = SP3_Create(mode, typeCode,
                         false,             // false means without internal support
                         getNewBaseZScale(),
                         true,              // enable easy release tab
                         nullptr);

	// Step 2: Set TopoModel and transformation
	ret = SP3_SetModelAndTrans(stlBinModel, modelTrans);

	// Step 3: Add Symbols to C++ support module.
	if (mode == PointEditModeCode::Normal)
		ret = addSupportPointAuto();
	else
	{
		double pos1[3] = { uiPosition[0],  uiPosition[1],  uiPosition[2] };
		double pos2[3] = { uiPosition[0] + 3,  uiPosition[1],  uiPosition[2] };
		double pos3[3] = { uiPosition[0],  uiPosition[1] + 3,  uiPosition[2] };
		double dir[3] = { 0, 0, -1 };
		ret = SP3_AddSupportPointManual(pos1, dir, 0.3);
		ret = SP3_AddSupportPointManual(pos2, dir, 0.3);
	}

	// Step 4: Generate supports.
	int countSupportData = 0;
	ret = SP3_GenSupportDataWOMesh(countSupportData);
	printf("===> SupportData Count: %d\n", countSupportData);

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
		// Get SupportDataWOMesh
		SupportDataWOMesh supportDataWOMesh;
		SP3_GetSupportDataWOMesh(i,
									supportDataWOMesh.supType,
									supportDataWOMesh.depthAdjust,
									supportDataWOMesh.length, supportDataWOMesh.radius1, supportDataWOMesh.radius2, supportDataWOMesh.bufferMatrix);

		vecSupportDataWOMesh.push_back(supportDataWOMesh);
		// End of SupportDataWOMesh

#if 1
		float trans[16];
		
		int stlBufferSize = 0;
		unsigned char *stlBuffer = SP3_GenSupportDataMesh(i, stlBufferSize, trans);

		stlBinary.mesh = stlBuffer;
		memcpy(stlBinary.trans, trans, sizeof(trans));
		vecStlBinary.push_back(stlBinary);
#endif
	}

	sw.Stop();
	printf("Elapsed Milli Seconds: %ld\n", sw.ElapsedMilliseconds());

#if 1
	/*
		Dump the support result
		NOTE: This dump must before SP3_Release(); otherwise, the buffer in vecStlBinary will be released.
	*/
	string resultFilename = "supportModel.stl";
	MeshDataToStlFile(vecStlBinary, resultFilename);
	printf("%s file has been generated...\n", resultFilename.c_str());
#endif

	// Step 6: Release.
	ret = SP3_Release();
	free(stlBinModel);

#if 0	// AddSupportDataWOMesh Test
	AddSupportDataWOMeshTest(PointEditModeCode::Normal, typeCode);
#endif
	return 0;
}

int Sp3ApiTest::GenSupportCone(MeshTypeCode typeCode)
{
	printf("============================================\n");
	printf("           Support Cone Test Start   \n");
	printf("============================================\n");
	/*
		Start calling SP3 APIs.
	*/
	Stopwatch sw;
	sw.Start();

	// Step 1: create
	int ret = SP3_Create(PointEditModeCode::Cone, typeCode, false, 0, true, nullptr);

// Step 2: Add Symbols to C++ modules and dump the support points
	double depthAdjust = 0.5;
	double pos[3] = { 30, 17, 20, };
	double radius_top = 1;		//UI: 2mm
	double radius_base = 1.5;	//UI: 3mm

	printf("Print Support Symbol paramters: \n");
	printf("===> depthAdjust: %f\n", depthAdjust);
	printf("===> pos: %f %f %f\n", pos[0], pos[1], pos[2]);
	printf("===> radius_top: %f\n", radius_top);
	printf("===> radius_base: %f\n", radius_base);
	ret = SP3_AddSupportSymbolCone(depthAdjust, pos, radius_top, radius_base);

	// Step 3: Generate supports.
	int countSupportData = 0;
	ret = SP3_GenSupportDataWOMesh(countSupportData);
	printf("===> SupportData Count: %d\n", countSupportData);

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

	printf("%s file has been generated...\n", resultFilename.c_str());
	return 0;
}

int Sp3ApiTest::AddSupportDataWOMeshTest(PointEditModeCode mode, MeshTypeCode typeCode)
{
	printf("============================================\n");
	printf("                Test Start  [AddSupportDataWOMeshTest ===> mode: %d, typeCode: %d]        \n", mode, typeCode);
	printf("============================================\n");

	// Step 1: create
	int ret = SP3_Create(mode, typeCode, true, getNewBaseZScale(), true, nullptr);
	
	printf("===> vecSupportDataWOMesh.size(): %lu\n", vecSupportDataWOMesh.size());

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
		//printf("===> stlBufferSize: %d\n", stlBufferSize);

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
