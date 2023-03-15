#pragma once
#include <vector>
#include "SupportData.h"
#include "Cubedata.h"
#include "Helper.h"
#include "AutoGenSupDLL.h"

using namespace std;

namespace XYZSupport
{
	class SupportMeshsBase
	{
	private:
		vector<vector<unsigned char>> _vecSupportDataMesh;	// stores support data's all meshes in stl format
		PERCENTAGE_CALLBACK progressSupport = Default_Percentage_Callback;

	public:
		void SetProgressCallback(PERCENTAGE_CALLBACK callback) { progressSupport = callback; }
		void* GetProgressCallback() { return (void*)progressSupport; }
		void OnGenerateMeshsProcessRateUpdate(float value, bool &isCancelled) { progressSupport(value, isCancelled); }

	protected:
		vector<shared_ptr<SupportData>> _items2;
	
	public:
		CubeData Cubedata2;							// Save model data - TopoModel and Trans
		vector<unsigned char> vecModelMeshWor;		// Save model data in stl format with world coordination.	//This is currently only used by SupportPointsList class.

		vector<shared_ptr<SupportSymbol>> Symbols;

		float printerWidth;//mm
		float printerDepth;//mm

		//#region EventSupportMeshs
		STATUS Status = STATUS::Idle;
		//COMMAND Command;	// C++ Module does not need this.
		//#endregion

		PointEditModeCode editMode = PointEditModeCode::Normal;

	public:
		SupportMeshsBase() {}
		/*
		Refer to: https://softwareengineering.stackexchange.com/questions/384738/why-does-the-base-class-need-to-have-a-virtual-destructor-here-if-the-derived-cl
		Topic: Why does the base class need to have a virtual destructor here if the derived class allocates no raw dynamic memory?
		*/
		virtual ~SupportMeshsBase() {}	// NOTE NOTE NOTE: Destructor is MUST. Otherwise, it causes memory leak!!!

		/// <summary>
		/// count support data
		/// </summary>
		int Count() const { return (int)_items2.size(); }

		vector<shared_ptr<SupportData>>& SupportDataVector() { return _items2; }

		virtual void Generate2() = 0;													// generate support data without mesh

		virtual void GenMesh(shared_ptr<SupportData> &supportData) = 0;					// generate mesh by a given support data

		virtual void OnAdjustGenerate() {}

		virtual void AddMark() {}

	public:
		/*
			functions used for extern "C" api in AutoGenSupDLL.h.
		*/
		int SetModelAndTrans(const unsigned char *buffer, const float trans[16])
		{
			// Stores meshes for generating Support Points later
			if (editMode == PointEditModeCode::Normal ||	// Auto Support Points
				editMode == PointEditModeCode::Auto)
				ModelStlBinaryToWorldStlBinary(buffer, trans, vecModelMeshWor);

			StlBinaryToModel2(buffer, Cubedata2.Model);

			Cubedata2.Trans.set(trans);
#if 1
			Cubedata2.Trans.ToDbgMsg("SetModelTransMatrix()===> trans: ");
#endif
			return 0;
		}

		int GetNumOfTriangles(int indexSupportData, int &countTriangles)
		{
			countTriangles = _items2[indexSupportData]->originalModel.triangles.Count();
			return 0;
		}

		int GetSupportDataMeshSize(int indexSupportData)
		{
			int countTriangles = _items2[indexSupportData]->originalModel.triangles.Count();
			if (countTriangles > 0)
				return 84 + countTriangles * 50;
			else
				return 0;
		}

		/******************************************************************/
		int GenMesh(int indexSupportData)
		{
			GenMesh(_items2[indexSupportData]);
			return 0;
		}

		int GetSupportDataMesh(const int indexSupportData, unsigned char *buffer, int size)			//Deprecated. The function is replaced by the function below.
		{
			return ModelToStlBinary(_items2[indexSupportData]->originalModel, buffer, size);
		}

		unsigned char* GetSupportDataMesh(const int indexSupportData, int &size, float trans[16])
		{
			if (Count() == 0) return nullptr;	// Check

			if (_vecSupportDataMesh.size() == 0)	{
				_vecSupportDataMesh.resize(Count());
			}
			if (_vecSupportDataMesh[indexSupportData].size() == 0)
			{
				size = _items2[indexSupportData]->originalModel.Size();
				_vecSupportDataMesh[indexSupportData].resize(size, 0);
				ModelToStlBinary(_items2[indexSupportData]->originalModel, &_vecSupportDataMesh[indexSupportData][0], size);
			}
			memcpy(trans, _items2[indexSupportData]->transMatrix.get(), 16 * sizeof(float));
			return &_vecSupportDataMesh[indexSupportData][0];
		}
		/******************************************************************/
		int AddSupportDataWOMesh(int indexSupportData,
												const float bufferPosition[3],		//in: position
												const float bufferScale[3],			//in: scale
												const float bufferRotation[3],		//in: rotation
												int supType,						//in: supType
												const double bufferOrientation[3],	//in: orientation
												const float bufferCurPos[16],		//in: curPos
												const float bufferCurPos2[16],		//in: curPos2
												const float trans[16],				//in: trans
												const double bufferBoundingBox[6],	//in: BoundingBox
												double depthAdjust,					//in
												double length,						//in
												double radius1,						//in
												double radius2,						//in
												const float bufferMatrix[16])		//in
		{
			//if (_items2.size() != indexSupportData)
			//	return -1;

			unique_ptr<SupportData> pSupportData(new SupportData());

			pSupportData->SetPosition(Coord3D(bufferPosition[0], bufferPosition[1], bufferPosition[2]));
			pSupportData->SetScale(Coord3D(bufferScale[0], bufferScale[1], bufferScale[2]));
			pSupportData->SetRotation(Coord3D(bufferRotation[0], bufferRotation[1], bufferRotation[2]));
			pSupportData->supType = supType;
			pSupportData->orientation = RHVector3(bufferOrientation[0], bufferOrientation[1], bufferOrientation[2]);

			//curPos, curPos2, trans
			pSupportData->curPos.set(bufferCurPos);
			pSupportData->curPos2.set(bufferCurPos2);
			pSupportData->trans.set(trans);

			//BoundingBox
			pSupportData->BoundingBox.minPoint = RHVector3(bufferBoundingBox[0], bufferBoundingBox[1], bufferBoundingBox[2]);
			pSupportData->BoundingBox.maxPoint = RHVector3(bufferBoundingBox[3], bufferBoundingBox[4], bufferBoundingBox[5]);
			
			pSupportData->depthAdjust = depthAdjust;
			pSupportData->length = length;
			pSupportData->radius1 = radius1;
			pSupportData->radius2 = radius2;
			pSupportData->transMatrix.set(bufferMatrix);

			_items2.push_back(move(pSupportData));

			if (supType >= (int)ModelType::BED_CELL /*15*/)	// It means the mesh is for Bed. Generate it first.
				GenMesh(_items2.back());
			return 0;
		}

		int GetSupportDataWOMesh(int indexSupportData,
										float bufferPosition[3],	//out: position
										float bufferScale[3],		//out: scale
										float bufferRotation[3],	//out: rotation
										int &supType,				//out: supType
										double bufferOrientation[3],//out: orientation
										float bufferCurPos[16],		//out: curPos
										float bufferCurPos2[16],	//out: curPos2
										float trans[16],			//out: trans
										double bufferBoundingBox[6],//out: BoundingBox
										double &depthAdjust,		//out
										double &length,				//out
										double &radius1,			//out
										double &radius2,			//out
										float bufferMatrix[16])		//out
		{
			//position
			bufferPosition[0] = _items2[indexSupportData]->GetPosition().x;
			bufferPosition[1] = _items2[indexSupportData]->GetPosition().y;
			bufferPosition[2] = _items2[indexSupportData]->GetPosition().z;
			//scale
			bufferScale[0] = _items2[indexSupportData]->GetScale().x;
			bufferScale[1] = _items2[indexSupportData]->GetScale().y;
			bufferScale[2] = _items2[indexSupportData]->GetScale().z;
			//rotation
			bufferRotation[0] = _items2[indexSupportData]->GetRotation().x;
			bufferRotation[1] = _items2[indexSupportData]->GetRotation().y;
			bufferRotation[2] = _items2[indexSupportData]->GetRotation().z;
			//supType
			supType = _items2[indexSupportData]->supType;
			//orientation
			bufferOrientation[0] = _items2[indexSupportData]->orientation.x;
			bufferOrientation[1] = _items2[indexSupportData]->orientation.y;
			bufferOrientation[2] = _items2[indexSupportData]->orientation.z;
			//curPos, curPos2, trans
			memcpy(bufferCurPos, _items2[indexSupportData]->curPos.get(), 16 * sizeof(float));
			memcpy(bufferCurPos2, _items2[indexSupportData]->curPos2.get(), 16 * sizeof(float));
			memcpy(trans, _items2[indexSupportData]->trans.get(), 16 * sizeof(float));
			//BoundingBox
			bufferBoundingBox[0] = _items2[indexSupportData]->BoundingBox.MinPoint().x;
			bufferBoundingBox[1] = _items2[indexSupportData]->BoundingBox.MinPoint().y;
			bufferBoundingBox[2] = _items2[indexSupportData]->BoundingBox.MinPoint().z;

			bufferBoundingBox[3] = _items2[indexSupportData]->BoundingBox.MaxPoint().x;
			bufferBoundingBox[4] = _items2[indexSupportData]->BoundingBox.MaxPoint().y;
			bufferBoundingBox[5] = _items2[indexSupportData]->BoundingBox.MaxPoint().z;
			
			depthAdjust = _items2[indexSupportData]->depthAdjust;
			length = _items2[indexSupportData]->length;
			radius1 = _items2[indexSupportData]->radius1;
			radius2 = _items2[indexSupportData]->radius2;
			memcpy(bufferMatrix, _items2[indexSupportData]->transMatrix.get(), 16 * sizeof(float));
			return 0;
		}

		int GetTopoModelProperties(const int indexSupportData,	bool &selectedTrunkSup,
																bool &branchSupError,
																bool &selectedDepthAdjust)
		{
			selectedTrunkSup = _items2[indexSupportData]->originalModel.selectedTrunkSup;
			branchSupError = _items2[indexSupportData]->originalModel.branchSupError;
			selectedDepthAdjust = _items2[indexSupportData]->originalModel.selectedDepthAdjust;
			return 0;
		}

		void DumpAllSupportData(string filenme)
		{
			TextFile textFile(filenme);
			if (textFile.IsSuccess() == false)
				return;

			int countHead = 0;
			int countBody = 0;
			int countBodyNoCup = 0;
			int countCylinder = 0;
			int	countFoot = 0;
			int	countBedCell = 0;
			int	countCuboidCell = 0;
			int	countCuboidFoot = 0;
			int	countTab = 0;
			int	countUnknown = 0;

			for (int i = 0; i < _items2.size(); i++)
			{
				textFile.Write("=====================\n");
				textFile.Write("#%d\n", i);
				textFile.Write("=====================\n");
				textFile.Write("position: %s\n", _items2[i]->GetPosition().ToString().c_str());
				textFile.Write("scale: %s\n", _items2[i]->GetScale().ToString().c_str());
				textFile.Write("rotation: %s\n", _items2[i]->GetRotation().ToString().c_str());

				textFile.Write("supType: %d\n", _items2[i]->supType);
				textFile.Write("orientation: %s\n", _items2[i]->orientation.ToString().c_str());
				textFile.Write("curPos:\n");
				textFile.Write("%s\n", _items2[i]->curPos.Row0().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->curPos.Row1().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->curPos.Row2().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->curPos.Row3().ToString().c_str());
				textFile.Write("curPos2:\n");
				textFile.Write("%s\n", _items2[i]->curPos2.Row0().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->curPos2.Row1().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->curPos2.Row2().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->curPos2.Row3().ToString().c_str());
				textFile.Write("trans:\n");
				textFile.Write("%s\n", _items2[i]->trans.Row0().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->trans.Row1().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->trans.Row2().ToString().c_str());
				textFile.Write("%s\n", _items2[i]->trans.Row3().ToString().c_str());

				if (_items2[i]->supType == (int)ModelType::HEAD)
					countHead++;
				else if (_items2[i]->supType == (int)ModelType::BODY)
					countBody++;
				else if (_items2[i]->supType == (int)ModelType::BODY_NOCUP)
					countBodyNoCup++;
				else if (_items2[i]->supType == (int)ModelType::CYLINDER)
					countCylinder++;
				else if (_items2[i]->supType == (int)ModelType::FOOT)
					countFoot++;
				else if (_items2[i]->supType == (int)ModelType::BED_CELL)
					countBedCell++;
				else if (_items2[i]->supType == (int)ModelType::CUBOID_CELL)
					countCuboidCell++;
				else if (_items2[i]->supType == (int)ModelType::CUBOID_FOOT)
					countCuboidFoot++;
				else if (_items2[i]->supType == (int)ModelType::TAB)
					countTab++;
				else
					countUnknown++;
			}
			textFile.Write("==========================\n");
			textFile.Write("= The count of Supports  =\n");
			textFile.Write("==========================\n");
			textFile.Write("HEAD: %d\n", countHead);
			textFile.Write("BODY: %d\n", countBody);
			textFile.Write("BODY_NOCUP: %d\n", countBodyNoCup);
			textFile.Write("CYLINDER: %d\n", countCylinder);
			textFile.Write("FOOT: %d\n", countFoot);
			textFile.Write("BED_CELL: %d\n", countBedCell);
			textFile.Write("CUBOID_CELL: %d\n", countCuboidCell);
			textFile.Write("CUBOID_FOOT: %d\n", countCuboidFoot);
			textFile.Write("TAB: %d\n", countTab);
			textFile.Write("Unknown: %d\n", countUnknown);
		}
	};
}