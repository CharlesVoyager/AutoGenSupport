#pragma once
#include "SupportCommon.h"

#ifdef XYZSUPPORT_EXPORTS
#define XYZSUPPORT_API __declspec(dllexport)
#elif __APPLE__
#define XYZSUPPORT_API
#endif

using namespace XYZSupport;	// for PointEditModeCode and MeshTypeCode enumeration class.
/* Note that the order of the transformation matrix (Matrix4) used in this header file as the parameter is as below.
C++ Support Module Matrix4 order:
		 | 0  4  8 12 |
		 | 1  5  9 13 |
		 | 2  6 10 14 |
		 | 3  7 11 15 |
*/
typedef void(*PERCENTAGE_CALLBACK)(float progress, bool &isCancelled);
extern "C"
{
	/*  editMode:
		PointEditModeCode::Normal	=1		// Auto Cylinder mesh	
		PointEditModeCode::User		=2		// Manual Cylinder mesh
		PointEditModeCode::Cone		=3		// Cone mesh
		PointEditModeCode::Tree		=4		// Tree mesh
		PointEditModeCode::Auto		=7		// = Normal

		typeCode:
		MeshTypeCode::Normal		=0		// generating supports
		MeshTypeCode::Mark			=1		// generating marks
	*/
	XYZSUPPORT_API int SP3_Create(	PointEditModeCode editMode, 	// Refer to "SupportCommon.h" for PointEditModeCode definition.
									MeshTypeCode typeCode,			// Refer to "SupportCommon.h" for MeshTypeCode definition.
									bool internalSupport,			// true: enable internal support. false: disable internal support.
									float newBaseZScale,			// For honeycomb base. 0 for foot base. 0.6f (or 1.2f) for honeycomb base. 0.6f ==> 1.5mm, 1.2f ==> 3mm
									bool enableEasyReleaseTab,		// true: enable easy release tab. false: disable easy release tab.
						/*callback*/PERCENTAGE_CALLBACK progress);	// 0 ~ 20: Slice, 20 ~ 50: Auto/Manual Symbols, 50 ~ 90: genSupFromList, 90 ~ 95: Bed, 95 ~ 100: GenMesh.
	/*
		stlBuffer: [INPUT] A buffer that has stored the model's mesh in STL binary format.
		trans: [INPUT] Transformation matrix.
	*/
	XYZSUPPORT_API int SP3_SetModelAndTrans( const unsigned char *stlBuffer, const float trans[16] );

	// Add support symbols for PointEditModeCode::Normal mode.
	XYZSUPPORT_API int SP3_AddSupportPointAuto(		const wchar_t *density,		// UI: Low Mid High (Default: Mid)		// density: "LOW", "MED", "HIGH". Not used in the support module. Consider to remove this parameter.
													double radius_contactSize,	// UI: 0.3mm ~ 1.3mm (Default: 0.8mm)	// radius_contactSize: 0.15 ~ 0.65 (Default: 0.4)
													const int threshholds[4],
													const wchar_t *tempFolder,
										/*callback*/PERCENTAGE_CALLBACK callback);

	// Add support symbols for PointEditModeCode::User mode.
	XYZSUPPORT_API int SP3_AddSupportPointManual(	const double position[3],	// point position in world coordinates.
													const double direction[3],
													double radius_contactSize);	// UI: 0.3mm ~ 1.3mm (Default: 0.6mm)	// radius_contactSize: 0.15 ~ 0.65 (Default: 0.3)
	
	// Add support symbols for PointEditModeCode::Cone mode.
	XYZSUPPORT_API int SP3_AddSupportSymbolCone(	double depthAdjust,			// UI: Low ~ High		// depthAdjust: 1, 0.9, ..., 0.5 (default), ..., 0
													const double position[3],
													double radius_top,			// UI: 2mm ~ 9mm		// radius_top: 1 (default) ~ 4.5
													double radius_base);		// UI: 3mm ~ 10mm		// radius_base: 1.5 (default) ~ 5

	// Add support symbols for PointEditModeCode::Tree mode.
	XYZSUPPORT_API int SP3_AddSupportPointTree(const double position[3]);		// diameter is always 0.3mm

	// Add support symbols for PointEditModeCode::Tree mode.
	XYZSUPPORT_API int SP3_AddSupportSymbolTree(double depthAdjust,				// 0.5	(default)											// 0.5 (default)
												const double position_top[3],	// Trunk: 上端的位置											// Branch: 一端在模型的位置
												const double position_base[3],  // Trunk: 下端的位置											// Branch: 一端在Trunk上端的位置
												double radius_top,				// UI: Top Diameter 1 ~ 4  mm	// radius_top: 0.5 ~ 2		// UI: Top Diameter 0.3 ~ 3.7mm		// radius_top: 0.15 ~ 1.85
												double radius_base,				// UI: Base Diameter 2 ~ 5 mm	// radius_base: 1 ~ 2.5		// UI: Base Daimeter 0.6 ~ 4 mm		// radius_base: 0.3 ~ 2
												int type);						// UI: Structure Trunk			// 3: TreeTrunk				// UI: Structure Branch				// 4: TreeBranch
	
	/********************************************************************************************/
	XYZSUPPORT_API int SP3_GenSupportDataWOMesh(/*out*/int &countSupportData);

	XYZSUPPORT_API int SP3_AddSupportDataWOMesh(int indexSupportData,
												int supType,
												double depthAdjust,
												double length, double radius1, double radius2, const float bufferMatrix[16]);
	
	XYZSUPPORT_API int SP3_GetSupportDataWOMesh(int indexSupportData,
												int &supType,
												double &depthAdjust,
												double &length, double &radius1, double &radius2, float bufferMatrix[16]);

	// The return buffer (STL binary) will not be released unless SP3_Release() is executed.
	XYZSUPPORT_API unsigned char* SP3_GenSupportDataMesh(int indexSupportData, /*out*/int &meshSize, /*out*/float trans[16]);
	/********************************************************************************************/
		
	/********************************************************************************************/
	// The two functions below are provided for getting current symbols used after generating meshes.
	// The two functions below are only avaible on PointEditModeCode::Normal, PointEditModeCode::User, and PointEditModeCode::Auto.
	/********************************************************************************************/
	XYZSUPPORT_API int SP3_GetSupportPointsCount();

	XYZSUPPORT_API int SP3_GetSupportPoint(int index, /*out*/double position[3], /*out*/double direction[3], /*out*/double &radius);
	/********************************************************************************************/

	XYZSUPPORT_API int SP3_Release();

	/*  supType:
		ModelType::BED_CELL	(15)
		ModelType::CUBOID_CELL (16)
		ModelType::TAB (18)

		NOTE: This function can be called any time without calling SP3_Create().
			  The return memory address is always valid as long as the DLL is loaded.
	*/
	XYZSUPPORT_API unsigned char* SP3_GenBasicMesh(ModelType supType, /*out*/int &meshSize);
}