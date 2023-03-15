// AutoGenSupDLL.cpp : Defines the exported functions for the DLL application.
//
#include "stdafx.h"
#include "AutoGenSupDLL.h"
#include "SupportMeshsCylinder.h"
#include "SupportMeshsCone.h"
#include "SupportMeshsTree.h"
#include "SupportMeshsMark.h"
#include "SupportPointsList.h"

unique_ptr<SupportMeshsBase> g_pSupport;

void Default_Percentage_Callback(float progress, bool &isCancelled)
{
	//DbgMsg("Progress: %.2f%%", progress);	// Don't print debug message to avoid too many messages and avoid delay the performance.
}

extern "C"
{
	XYZSUPPORT_API int SUP_Create(	PointEditModeCode editMode, // Refer to "SupportCommon.h" for PointEditModeCode definition.
									MeshTypeCode typeCode,		// Refer to "SupportCommon.h" for MeshTypeCode definition.
									bool internalSupport,		// true: enable internal support. false: disable internal support
									float newBaseZScale,		// for honeycomb base. 0 for old base. 0.6f (or 1.2f) for new base.
									bool _EnableEasyReleaseTab,
									int baseType,
									int haveBrim,
									float printerWidth,
									float printerDepth)
	{
		DbgMsg(__FUNCTION__"[IN]===> editMode: %d, typeCode: %d (0: Normal, 1: Mark)", editMode, typeCode);

		if (g_pSupport != nullptr)
			SUP_Release();	// Release g_pSupport if g_pSupport has been created.

		if (typeCode == MeshTypeCode::Normal)
		{
			switch (editMode)
			{
			case PointEditModeCode::Normal:			//1: ===> Auto Support Points
			case PointEditModeCode::User:			//2: ===> Manual Support Points
			case PointEditModeCode::Auto:			//7:
			case PointEditModeCode::Bed:			//8:
				g_pSupport = unique_ptr<SupportMeshsBase>(new SupportMeshsCylinder(editMode, internalSupport, newBaseZScale, _EnableEasyReleaseTab, baseType, haveBrim, printerWidth, printerDepth));
				break;
			case PointEditModeCode::Cone:			//3
				g_pSupport = unique_ptr<SupportMeshsBase>(new SupportMeshsCone());
				break;
			case PointEditModeCode::Tree:			//4
				g_pSupport = unique_ptr<SupportMeshsBase>(new SupportMeshsTree());
				break;
			default:
				return E_INVALID_INPUT;
				break;
			}
		}
		else if(typeCode == MeshTypeCode::Mark)		//generating marks
		{
			g_pSupport = unique_ptr<SupportMeshsBase>(new SupportMeshsMark(editMode));
		}
		else
		{
			return E_INVALID_INPUT;
		}

		DbgMsg(__FUNCTION__"[OUT]");
		return E_SUCCESS;
	}

	// The STL binary data and transformation matrix will be stored in SupportMeshsBase::Cubedata2(Data Type : CubeData)
	XYZSUPPORT_API int SUP_SetModelAndTrans(const unsigned char *buffer, const float trans[16])
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		return g_pSupport->SetModelAndTrans(buffer, trans);
	}

	XYZSUPPORT_API int SUP_AddSupportPointAuto(const wchar_t *density,
												double radius_contactSize,
												const int threshholds[4],
												const wchar_t *tempFolder,
												PERCENTAGE_CALLBACK callback)
	{
		DbgMsg(__FUNCTION__ "[IN]");
		DbgMsg(__FUNCTION__ "===> radius_contactSize: %f", radius_contactSize);
		DbgMsg(__FUNCTION__ "===> threshholds: %d %d %d %d", threshholds[0], threshholds[1], threshholds[2], threshholds[3]);

		if (g_pSupport == nullptr) return E_NO_INTIAL;
		if (g_pSupport->vecModelMeshWor.size() == 0) return E_INVALID_INPUT;

		unique_ptr<SupportPointsList> pSupportPointsList(new SupportPointsList());

		pSupportPointsList->SetProgressCallback((PERCENTAGE_CALLBACK)g_pSupport->GetProgressCallback());

		vector<int> vThresholds(threshholds, threshholds + 4);

		vector<SupportPoint> vSupportPoints = pSupportPointsList->GenerateAuto(density, 
																				radius_contactSize,
																				vThresholds,
																				g_pSupport->vecModelMeshWor, 
																				tempFolder, 
																				callback);
		for (const auto &pt : vSupportPoints)
		{
			unique_ptr<SupportPoint> p(new SupportPoint());

			p->SetPosition(pt.GetPosition());
			p->SetDirection(pt.GetDirection());
			p->SetTouchRadius(pt.GetTouchRadius());
			p->edge = false;
			p->level = 0;

			g_pSupport->Symbols.push_back(move(p));
		}
		DbgMsg(__FUNCTION__ "[OUT]");
		return E_SUCCESS;
	}

	// Add support symbols for cylinder
	XYZSUPPORT_API int SUP_AddSupportPointManual(const double position[3], const double direction[3], double radius)
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		unique_ptr<SupportPoint> p(new SupportPoint());

		p->SetPosition(RHVector3(position[0], position[1], position[2]));
		p->SetDirection(RHVector3(direction[0], direction[1], direction[2]));
		p->SetTouchRadius(radius);
		p->edge = false;
		p->level = 0;

		g_pSupport->Symbols.push_back(move(p));
		return E_SUCCESS;
	}
	// Add support symbols for cone
	XYZSUPPORT_API int SUP_AddSupportSymbolCone(double depthAdjust,
												bool depthAdjustSelected,
												const double position[3],
												const double direction[3],
												double radius_top,
												double radius_base)
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		//DbgMsg(__FUNCTION__ "===> depthAdjust: %f", depthAdjust);
		//DbgMsg(__FUNCTION__ "===> depthAdjustSelected: %s", depthAdjustSelected ? "true":"false");
		//DbgMsg(__FUNCTION__ "===> radius_top: %f", radius_top);
		//DbgMsg(__FUNCTION__ "===> radius_base: %f", radius_base);

		unique_ptr<SupportSymbolCone> p(new SupportSymbolCone());

		p->DepthAdjust = depthAdjust;
		p->DepthAdjustSelected = depthAdjustSelected;
		p->SetPosition(RHVector3(position[0], position[1], position[2]));
		p->SetDirection(RHVector3(direction[0], direction[1], direction[2]));
		p->SetRadiusTop(radius_top);
		p->SetRadiusBase(radius_base);

		if (g_pSupport->editMode == PointEditModeCode::Cone)
			g_pSupport->Symbols.push_back(move(p));
		else if (	g_pSupport->editMode == PointEditModeCode::Normal || 
					g_pSupport->editMode == PointEditModeCode::User ||
					g_pSupport->editMode == PointEditModeCode::Auto)
		{
			SupportMeshsCylinder *pSupportMeshsCylinder = dynamic_cast<SupportMeshsCylinder *>(g_pSupport.get());
			pSupportMeshsCylinder->AddSupportSymbolCone(move(p));
		}
		else
			return E_ILLEGAL_CALL;
		return E_SUCCESS;
	}

	XYZSUPPORT_API int SUP_AddSupportPointTree(	const double position[3],
												const double direction[3],
												double diameter)
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		unique_ptr<SupportPointTree> p(new SupportPointTree());

		p->SetPosition(RHVector3(position[0], position[1], position[2]));
		p->SetDirection(RHVector3(direction[0], direction[1], direction[2]));
		p->SetTouchDiameter(diameter);

		g_pSupport->Symbols.push_back(move(p));
		return E_SUCCESS;
	}
	//																				// Trunk													// Branch
	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	XYZSUPPORT_API int SUP_AddSupportSymbolTree(double depthAdjust,					// 0.5														// 0.5
												const double DepthAdjustNor[3],		// 0, 0, 1													// -0, -0, -1
												bool depthAdjustSelected,
												const double position[3],			// Not used.												// Trunk上面頂點位置
												const double direction[3],			// Not used.												// Branch一端在模型的位置連到 Trunk上面頂點位置的方向								
												const double rootposition[3],		// (x, y, 0)												// Branch一端在模型的位置
												const double topposition[3],		// (x, y, length)											// 與_rootposition一樣但 z + _length
												double radius_top,					// UI: Top Diameter 1 ~ 4  mm	// radius_top: 0.5 ~ 2		// UI: Top Diameter 0.3 ~ 3.7mm		// radius_top: 0.15 ~ 1.85
												double radius_base,					// UI: Base Diameter 2 ~ 5 mm	// radius_base: 1 ~ 2.5		// UI: Base Daimeter 0.6 ~ 4 mm		// radius_base: 0.3 ~ 2
												double length,						// UI: Height
												int type)							// UI: Structure Trunk			// 3: TreeTrunk,			// UI: Structure Branch				// 4: TreeBranch
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		unique_ptr<SupportSymbolTree> p(new SupportSymbolTree());

		p->DepthAdjust = depthAdjust;
		p->DepthAdjustNor = RHVector3(DepthAdjustNor[0], DepthAdjustNor[1], DepthAdjustNor[2]);
		p->DepthAdjustSelected = depthAdjustSelected;
		p->SetPosition(RHVector3(position[0], position[1], position[2]));
		p->SetDirection(RHVector3(direction[0], direction[1], direction[2]));
		p->SetRootPosition(RHVector3(rootposition[0], rootposition[1], rootposition[2]));
		p->SetTopPosition(RHVector3(topposition[0], topposition[1], topposition[2]));
		p->SetRadiusTop(radius_top);
		p->SetRadiusBase(radius_base);
		p->SetLength(length);
		p->Type = (SymbolType)type;	//TreeTrunk (3) or TreeBranch (4)

		g_pSupport->Symbols.push_back(move(p));
		return E_SUCCESS;
	}
	/****************************************************************************/
	XYZSUPPORT_API int SUP_GenSupportDataWOMesh(int &countSupportData)
	{
		DbgMsg(__FUNCTION__"[IN]");

		if (g_pSupport == nullptr) return E_NO_INTIAL;

		g_pSupport->Generate2();
		countSupportData = g_pSupport->Count();

		DbgMsg(__FUNCTION__"===> countSupportData: %d", countSupportData);
		DbgMsg(__FUNCTION__"[OUT]");
		return E_SUCCESS;
	}

	XYZSUPPORT_API int SUP_GetSupportDataWOMesh(int indexSupportData,
													float bufferPosition[3], float bufferScale[3], float bufferRotation[3],
													int &supType,
													double bufferOrientation[3],
													float bufferCurPos[16], float bufferCurPos2[16], float bufferTrans[16],
													double bufferBoundingBox[6],
													double &depthAdjust,
													double &length, double &radius1, double &radius2, float bufferMatrix[16])
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;
		if (g_pSupport->Count() == 0) return E_OTHER;	// No SupportData

		g_pSupport->GetSupportDataWOMesh(indexSupportData,
											bufferPosition, bufferScale, bufferRotation,
											supType,
											bufferOrientation,
											bufferCurPos, bufferCurPos2, bufferTrans,
											bufferBoundingBox,
											depthAdjust,
											length, radius1, radius2, bufferMatrix);
		return E_SUCCESS;
	}
	/****************************************************************************/
	XYZSUPPORT_API unsigned char* SUP_GenSupportDataMesh(int indexSupportData, /*out*/ int &meshSize, float trans[16])
	{
		if (g_pSupport == nullptr) return nullptr;
		if (g_pSupport->Count() == 0) return nullptr;	// No SupportData

		int ret = g_pSupport->GenMesh(indexSupportData);
		return g_pSupport->GetSupportDataMesh(indexSupportData, meshSize, trans);
	}
	/****************************************************************************/

	XYZSUPPORT_API int SUP_GetSupportPointsCount()
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;
		return (int)g_pSupport->Symbols.size();
	}

	XYZSUPPORT_API int SUP_GetSupportPoint(int index, double position[3], double direction[3], double &radius)
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;
		if (g_pSupport->Symbols.size() == 0) return E_OTHER;	// No Symbols.

		// NOTE: this function is only available for g_pSupport is SupportMeshsCylinder object.
		if (g_pSupport->editMode == PointEditModeCode::Normal ||
			g_pSupport->editMode == PointEditModeCode::User ||
			g_pSupport->editMode == PointEditModeCode::Auto)
		{
			SupportPoint *ppoint = dynamic_cast<SupportPoint *>(g_pSupport->Symbols[index].get());
		
			RHVector3 vPos = ppoint->GetPosition();
			position[0] = vPos.x; 
			position[1] = vPos.y; 
			position[2] = vPos.z;

			RHVector3 vDir = ppoint->GetDirection();
			direction[0] = vDir.x; 
			direction[1] = vDir.y;
			direction[2] = vDir.z;

			radius = ppoint->GetTouchRadius();
			return  E_SUCCESS;
		}
		else
			return  E_ILLEGAL_CALL;
	}

	XYZSUPPORT_API int SUP_GetTopoModelProperties(int indexSupportData, 
													bool &selectedTrunkSup,
													bool &branchSupError,
													bool &selectedDepthAdjust)
	{
		if (g_pSupport == nullptr) return E_NO_INTIAL;
		if (g_pSupport->Count() == 0) return E_OTHER;	// No SupportData
		return g_pSupport->GetTopoModelProperties(indexSupportData, selectedTrunkSup, branchSupError, selectedDepthAdjust);
	}

	// Cone / Tree
	XYZSUPPORT_API int SUP_OnAdjustGenerate()
	{
		DbgMsg(__FUNCTION__"[IN]");
		if (g_pSupport == nullptr) return E_NO_INTIAL;

		g_pSupport->OnAdjustGenerate();

		DbgMsg(__FUNCTION__"[OUT]");
		return E_SUCCESS;
	}


	XYZSUPPORT_API int SUP_Release()
	{
		DbgMsg(__FUNCTION__"[IN]");

		if (g_pSupport != nullptr) g_pSupport.reset();

		DbgMsg(__FUNCTION__"[OUT]");
		return E_SUCCESS;
	}
}