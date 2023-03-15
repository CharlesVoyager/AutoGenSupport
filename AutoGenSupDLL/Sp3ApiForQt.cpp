// XYZSupport.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "Sp3ApiForQt.h"
#include "SupportMeshsCylinder.h"
#include "SupportMeshsBed.h"
#include "SupportMeshsCone.h"
#include "SupportMeshsTree.h"
#include "SupportMeshsMark.h"
#include "SupportPointsList.h"
#include "AutoGenSupDLL.h"
#include "SupportStructure.h"

using namespace std;
using namespace XYZSupport;
using namespace SupportStructure;

extern unique_ptr<SupportMeshsBase> g_pSupport;

extern "C"
{
	XYZSUPPORT_API int SP3_Create(	PointEditModeCode editMode, 	// Refer to "SupportCommon.h" for PointEditModeCode definition.
									MeshTypeCode typeCode,			// Refer to "SupportCommon.h" for MeshTypeCode definition.
									bool internalSupport,			// true: enable internal support. false: disable internal support
									float newBaseZScale,			// For honeycomb base. 0 for foot base. 0.6f (or 1.2f) for honeycomb base. 0.6f ==> 1.5mm, 1.2f ==> 3mm
									bool enableEasyReleaseTab,
						/*callback*/PERCENTAGE_CALLBACK progress)
	{//the value below need modification when we use dll in QT version
		int ret = SUP_Create(editMode, typeCode, internalSupport, newBaseZScale, enableEasyReleaseTab, 0, 0, 150.0, 150.0);
		if (g_pSupport == nullptr) return E_NULL_PTR;
			
		if(progress)
			g_pSupport->SetProgressCallback(progress);
		
		bool isCancelled = false;
		g_pSupport->OnGenerateMeshsProcessRateUpdate(0, isCancelled);
		return ret;
	}

	XYZSUPPORT_API int SP3_SetModelAndTrans(const unsigned char *buffer, const float trans[16])
	{
		int ret = SUP_SetModelAndTrans(buffer, trans);
		if (g_pSupport == nullptr) return E_NULL_PTR;

		bool isCancelled = false;
		g_pSupport->OnGenerateMeshsProcessRateUpdate(5, isCancelled);
		return ret;
	}

	XYZSUPPORT_API int SP3_AddSupportPointAuto(	const wchar_t *density,
												double radius_contactSize,
												const int threshholds[4],
												const wchar_t *tempFolder,
												PERCENTAGE_CALLBACK callback)
	{
		return SUP_AddSupportPointAuto(density, radius_contactSize, threshholds, tempFolder, callback);
	}

	// Add support symbol for SupportMeshsCylinder
	XYZSUPPORT_API int SP3_AddSupportPointManual(const double position[3], const double direction[3], double radius)
	{
		return SUP_AddSupportPointManual(position, direction, radius);
	}

	// SupportSymbol for cone
	XYZSUPPORT_API int SP3_AddSupportSymbolCone(double depthAdjust,
		const double position[3],
		double radius_top,
		double radius_base)
	{
		double direction[3] = { 0, 0, 0 };
		return SUP_AddSupportSymbolCone(depthAdjust,
										false,
										position,
										direction,
										radius_top,
										radius_base);
	}

	XYZSUPPORT_API int SP3_AddSupportPointTree(	const double position[3])
	{
		double direction[3] = { 0, 0, 0 };
		return SUP_AddSupportPointTree(position, direction, 0.3);
	}

	XYZSUPPORT_API int SP3_AddSupportSymbolTree(double depthAdjust,
												const double position_top[3],
												const double position_base[3],
												double radius_top,
												double radius_base,
												int type)
	{
		if (type == (int)SymbolType::TreeTrunk)//3
		{
			double DepthAdjustNor[3] = { 0, 0, 1 };
			double position[3] = { 0, 0, 0 };
			double direction[3] = { 0, 0, 0 };
			RHVector3 start(position_base[0], position_base[1], position_base[2]);
			RHVector3 end(position_top[0], position_top[1], position_top[2]);
			double length = start.Distance(end);

			return SUP_AddSupportSymbolTree(depthAdjust,
											DepthAdjustNor,
											false,
											position,
											direction,
											position_base,	//rootposition
											position_top,	//topposition
											radius_top,
											radius_base,
											length,
											type);
		}
		else if (type == (int)SymbolType::TreeBranch)//4
		{
			double DepthAdjustNor[3] = { 0, 0, -1 };

			RHVector3 start(position_base[0], position_base[1], position_base[2]);
			RHVector3 end(position_top[0], position_top[1], position_top[2]);
			double length = start.Distance(end);
			RHVector3 branchDirection = RHVector3(start.Subtract(end));
			branchDirection.NormalizeSafe();
			double direction[3] = { branchDirection.x, branchDirection.y, branchDirection.z };
			double topposition[3] = { position_top[0], position_top[1], position_top[2] + length};

			return SUP_AddSupportSymbolTree(depthAdjust,
											DepthAdjustNor,
											false,
											position_base,	//position: Trunk上面頂點位置
											direction,
											position_top,	//rootposition: Branch一端在模型的位置
											topposition,	//topposition
											radius_top,
											radius_base,
											length,
											type);
		}
		return E_INVALID_INPUT;
	}

	XYZSUPPORT_API int SP3_GenSupportDataWOMesh(int &countSupportData)
	{
		int ret = SUP_GenSupportDataWOMesh(countSupportData);
		return ret;
	}

	XYZSUPPORT_API int SP3_AddSupportDataWOMesh(int indexSupportData,
													int supType,
													double depthAdjust,
													double length, double radius1, double radius2, const float bufferMatrix[16])
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		float bufferPosition[3] = { NAN, NAN, NAN };	// IMPORTANT: Set it as NAN, so the later functions can check if position/scale/rotation values are available.
		float bufferScale[3]; float bufferRotation[3];
		double bufferOrientation[3];
		float bufferCurPos[16]; float bufferCurPos2[16]; float bufferTrans[16];
		double bufferBoundingBox[6];

		return g_pSupport->AddSupportDataWOMesh(indexSupportData,
											bufferPosition, bufferScale, bufferRotation,
											supType,
											bufferOrientation,
											bufferCurPos, bufferCurPos2, bufferTrans,
											bufferBoundingBox,
											depthAdjust,
											length, radius1, radius2, bufferMatrix);
	}

	XYZSUPPORT_API int SP3_GetSupportDataWOMesh(int indexSupportData,
													int &supType,
													double &depthAdjust,
													double &length, double &radius1, double &radius2, float bufferMatrix[16])
	{
		float bufferPosition[3]; float bufferScale[3]; float bufferRotation[3];
		double bufferOrientation[3];
		float bufferCurPos[16]; float bufferCurPos2[16]; float bufferTrans[16];
		double bufferBoundingBox[6];
		return SUP_GetSupportDataWOMesh(indexSupportData,
											bufferPosition, bufferScale, bufferRotation,
											supType,
											bufferOrientation,
											bufferCurPos, bufferCurPos2, bufferTrans,
											bufferBoundingBox,
											depthAdjust,
											length, radius1, radius2, bufferMatrix);
	}

	XYZSUPPORT_API unsigned char* SP3_GenSupportDataMesh(int indexSupportData, /*out*/ int &meshSize, float trans[16])
	{
		// 0 ~ 50: Auto/Manual Symbols, 50 ~ 90: genSupFromList,  90 ~ 95: Bed, 95 ~ 100: GenMesh.
		if (g_pSupport != nullptr)
		{
			bool isCancelled = false;
			g_pSupport->OnGenerateMeshsProcessRateUpdate(95.0 + (indexSupportData * 5.0 / g_pSupport->Count()), isCancelled); // 0 ~ 50: Auto/Manual Symbols, 50 ~ 90: genSupFromList, 90 ~ 100: GenMesh. 
		}
	
		return SUP_GenSupportDataMesh(indexSupportData, meshSize, trans);
	}

	XYZSUPPORT_API int SP3_GetSupportPointsCount()
	{
		return SUP_GetSupportPointsCount();
	}

	XYZSUPPORT_API int SP3_GetSupportPoint(int index, double position[3], double direction[3], double &radius)
	{
		return SUP_GetSupportPoint(index, position, direction, radius);
	}

	XYZSUPPORT_API int SP3_Release()
	{
		bool isCancelled = false;
		g_pSupport->OnGenerateMeshsProcessRateUpdate(100.0, isCancelled);
		return SUP_Release();
	}

	XYZSUPPORT_API unsigned char* SP3_GenBasicMesh(ModelType supType, /*out*/int &meshSize)
	{
		if (supType == ModelType::BED_CELL)
		{
			meshSize = sizeof(support_bed_cell_style1_stl);
			return support_bed_cell_style1_stl;
		}
		else if (supType == ModelType::CUBOID_CELL)
		{
			meshSize = sizeof(support_cuboid_cell_stl);
			return support_cuboid_cell_stl;
		}
		else if (supType == ModelType::TAB)
		{
			meshSize = sizeof(support_bed_tab);
			return support_bed_tab;
		}
		meshSize = 0;
		return nullptr;
	}
}
/******************************************************************************************************************/
