#pragma once
#include "../AutoGenSupDLL/Helper/MyTools.h"
#include "Common.h"

using namespace MyTools;
using namespace std;

typedef void(*PERCENTAGE_CALLBACK)(float progress, bool &isCancelled);

/********************************************************************************************************************************/
typedef int(*pSP3_Create)(	PointEditModeCode, 
							MeshTypeCode, 
							bool, 
							float,
							bool,
				/*callback*/PERCENTAGE_CALLBACK callback);

typedef int(*pSP3_SetModelAndTrans)(const unsigned char *, const float *);
typedef int(*pSP3_AddSupportPointAuto)(	const wchar_t *density,
										double contactSize,
										const int threshholds[4],
										const wchar_t *tempFolder,
							/*callback*/PERCENTAGE_CALLBACK callback);
typedef int(*pSP3_AddSupportPointManual)(const double *, const double *, double);
typedef int(*pSP3_AddSupportSymbolCone)(double, const double *, double, double);
typedef int(*pSP3_AddSupportPointTree)(const double *);
typedef int(*pSP3_AddSupportSymbolTree)(double, const double *, const double *, double, double, int);

typedef int(*pSP3_GenSupportDataWOMesh)(/*out*/int &);

typedef int(*pSP3_AddSupportDataWOMesh)(int indexSupportData,
											int supType,
											const double depthAdjust,
											double length, double radius1, double radius2, const float bufferMatrix[16]);

typedef int(*pSP3_GetSupportDataWOMesh)(int indexSupportData,
											int &supType,
											double &depthAdjust,
											double &length, double &radius1, double &radius2, float bufferMatrix[16]);

typedef int(*pSUP_GetSupportDataWOMesh)(int indexSupportData,
												float bufferPosition[3], float bufferScale[3], float bufferRotation[3],		// out
												int &supType,																// out
												double bufferOrientation[3],												// out
												float bufferCurPos[16], float bufferCurPos2[16], float bufferTrans[16],		// out
												double bufferBoundingBox[6],												// out: min point, max point
												double &depthAdjust,														// out
												double &length, double &radius1, double &radius2, float bufferMatrix[16]);	// out

typedef unsigned char*(*pSP3_GenSupportDataMesh)(int indexSupportData, int &stlSize, float trans[16]);

typedef unsigned int(*pSP3_GetSupportPointsCount)();
typedef unsigned int(*pSP3_GetSupportPoint)(int index, /*out*/double position[3], /*out*/double direction[3], /*out*/double &radius);

typedef int(*pSP3_Release)();
/********************************************************************************************************************************/

class Sp3ApiTestBase
{
protected:
	pSP3_Create SP3_Create = nullptr;

	pSP3_SetModelAndTrans SP3_SetModelAndTrans = nullptr;
	pSP3_AddSupportPointManual SP3_AddSupportPointManual = nullptr;
	pSP3_AddSupportPointAuto SP3_AddSupportPointAuto = nullptr;
	pSP3_AddSupportSymbolCone SP3_AddSupportSymbolCone = nullptr;
	pSP3_GenSupportDataWOMesh SP3_GenSupportDataWOMesh = nullptr;
	pSP3_AddSupportDataWOMesh SP3_AddSupportDataWOMesh = nullptr;
	pSP3_GetSupportDataWOMesh SP3_GetSupportDataWOMesh = nullptr;
	pSUP_GetSupportDataWOMesh SUP_GetSupportDataWOMesh = nullptr;
	pSP3_GenSupportDataMesh SP3_GenSupportDataMesh = nullptr;
	
	pSP3_GetSupportPointsCount SP3_GetSupportPointsCount = nullptr;
	pSP3_GetSupportPoint SP3_GetSupportPoint = nullptr;

	pSP3_Release SP3_Release = nullptr;

	string testIniFile;		// file path to the ini file
	string testName;
	wstring sectionNameW;
	HINSTANCE dllHandler;	// DLL handler for XYZSupport.dll

public:
	Sp3ApiTestBase(string testIniFile, HINSTANCE dllHandler);

	virtual int Run() = 0;
};
