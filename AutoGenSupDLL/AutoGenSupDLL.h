#pragma once

#include "SupportCommon.h"

#ifdef XYZSUPPORT_EXPORTS
#define XYZSUPPORT_API __declspec(dllexport)
#elif __APPLE__
#define XYZSUPPORT_API
#endif

using namespace XYZSupport;
void Default_Percentage_Callback(float progress, bool &isCancelled);

/* Note that the order of the transformation matrix (Matrix4) used in this header file as the parameter is as below.
C++ Support Module Matrix4 order:
		 | 0  4  8 12 |
		 | 1  5  9 13 |
		 | 2  6 10 14 |
		 | 3  7 11 15 |
*/
typedef void(*PERCENTAGE_CALLBACK)(float progress, bool &isCancelled);
//********************************************************************************************
extern "C"
{
	XYZSUPPORT_API int SUP_Create(	PointEditModeCode editMode, // Refer to "SupportCommon.h" for PointEditModeCode definition.
									MeshTypeCode typeCode,		// Refer to "SupportCommon.h" for MeshTypeCode definition.
									bool internalSupport,		// true: enable internal support. false: disable internal support
									float newBaseZScale,		// for honeycomb base.  0 for old base. 0.6f (or 1.2f) for new base.
									bool _EnableEasyReleaseTab,
									int baseType,
									int haveBrim,
									float printerWidth,
									float printerDepth);
	/*
		stlBuffer: [INPUT] A buffer that has stored the model's mesh in STL binary format.
		trans: [INPUT] Transformation matrix.
	*/
	XYZSUPPORT_API int SUP_SetModelAndTrans(const unsigned char *buffer, const float trans[16]);

	// Add support symbols for PointEditModeCode::Normal mode.
	XYZSUPPORT_API int SUP_AddSupportPointAuto(	const wchar_t *density,		// UI: Low Mid High (Default: Mid)		// density: "LOW", "MED", "HIGH". Not used in the support module. Consider to remove this parameter.
												double radius_contactSize,	// UI: 0.3mm ~ 1.3mm (Default: 0.8mm)	// radius_contactSize: 0.15 ~ 0.65 (Default: 0.4)
												const int threshholds[4],
												const wchar_t *tempFolder,
									/*callback*/PERCENTAGE_CALLBACK callback);

	XYZSUPPORT_API int SUP_AddSupportPointManual(	const double position[3], 
													const double direction[3], 
													double radius);			// UI: 0.3mm ~ 1.3mm (Default: 0.6mm)	// radius_contactSize: 0.15 ~ 0.65 (Default: 0.3)

	XYZSUPPORT_API int SUP_AddSupportSymbolCone(double depthAdjust,			// UI: Low ~ High		// depthAdjust: 1, 0.9, ..., 0.5 (default), ..., 0
												bool depthAdjustSelected,
												const double position[3],
												const double direction[3],
												double radius_top,			// UI: 2mm ~ 9mm		// radius_top: 1 (default) ~ 4.5
												double radius_base);		// UI: 3mm ~ 10mm		// radius_base: 1.5 (default) ~ 5

	XYZSUPPORT_API int SUP_AddSupportPointTree(	const double position[3],
												const double direction[3],
												double diameter);

	XYZSUPPORT_API int SUP_AddSupportSymbolTree(double depthAdjust,
												const double DepthAdjustNor[3],
												bool depthAdjustSelected,
												const double position[3],
												const double direction[3],
												const double rootposition[3],
												const double topposition[3],
												double radius_top,
												double radius_base,
												double length,
												int type);
	/********************************************************************************************/
	XYZSUPPORT_API int SUP_GenSupportDataWOMesh(int &countSupportData);

	XYZSUPPORT_API int SUP_GetSupportDataWOMesh(int indexSupportData,
															float bufferPosition[3], float bufferScale[3], float bufferRotation[3],		// out
															int &supType,																// out
															double bufferOrientation[3],												// out
															float bufferCurPos[16], float bufferCurPos2[16], float bufferTrans[16],		// out
															double bufferBoundingBox[6],												// out: min point, max point ===> NOTE: Bounding Box won't be available untill meshes are generated.
															double &depthAdjust,														// out
															double &length,	double &radius1, double &radius2, float bufferMatrix[16]);	// out

	// The return buffer (STL binary) will not be released unless SP2_Release() is executed.
	XYZSUPPORT_API unsigned char* SUP_GenSupportDataMesh(int indexSupportData, /*out*/int &meshSize, /*out*/float trans[16]);
	/********************************************************************************************/
	   
	XYZSUPPORT_API int SUP_GetSupportPointsCount();

	XYZSUPPORT_API int SUP_GetSupportPoint(int index, double position[3], double direction[3], double &radius);

	XYZSUPPORT_API int SUP_GetTopoModelProperties(int indexSupportData,
													bool &selectedTrunkSup,
													bool &branchSupError,
													bool &selectedDepthAdjust);

	XYZSUPPORT_API int SUP_OnAdjustGenerate();

	XYZSUPPORT_API int SUP_Release();
}