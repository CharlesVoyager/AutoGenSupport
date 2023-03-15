#pragma once
#include "stdafx.h"
#include "SupportMeshsBase.h"
#include "SupportData.h"
#include "SupportSymbolTree.h"
#include "RHVector3.h"
#include "SupportReferenceMesh.h"
#include "SupportPointTree.h"
#include "GenBasic.h"

namespace XYZSupport
{
	class SupportMeshsMark : public SupportMeshsBase
	{
		SupportReferenceMesh ReferenceMesh;	// BIG CLASS!!! 放在這裡. SupportMeshsCylinder class建立時後, 此物件建立一次. 不要放在function裡當local object, 造成每次使用此SupportReferenceMesh物件, 都要再一次生成!!!

	private:
		const int MODEL_SCALE = 15; // value 15, if support diameter is 2.0mm, then scale to 2.0mm/15 = 0.1333mm
		//public static int MODEL_SCALE = 10; // if support diameter is 2.0mm, then scale to 2.0mm/10 = 0.2000mm
		//static int SUP_DIAMETER_DEFAULT = (int)Diameter.LARGE;
		//private static Double SCALE_FACTOR_DEFAULT = (Double)SUP_DIAMETER_DEFAULT / (Double)MODEL_SCALE;  // value 0.1333, if support diameter is 2.0, then scale factor is 2.0/15 = 0.1333
		const int num_phi = 24;
		const int num_theta = 48;
		const int num_sides = 48;
	
	public:
		SupportMeshsMark(PointEditModeCode value) { this->editMode = value; }

		void Generate2()	// The input paramter is not used in this function!
		{
			if (Symbols.size() == 0) return;

			for (int i = 0; i < Symbols.size(); i++)
			{
				SupportSymbol *psymbol = Symbols[i].get();

				if (psymbol->Type == SymbolType::TreePoint)
				{
					SupportPointTree *ppoint = dynamic_cast<SupportPointTree *>(psymbol);
					ppoint->Type = SymbolType::NormalPoint;
					AddPoint(ppoint->GetPosition(), ppoint->GetDirection(), ppoint);
				}
				else if(psymbol->Type == SymbolType::TreeTrunk || psymbol->Type == SymbolType::TreeBranch)
				{
					SupportSymbolTree *psym = dynamic_cast<SupportSymbolTree *>(psymbol);
					AddMark(psym);
				}
				else if (psymbol->Type == SymbolType::ConePoint)
				{
					SupportSymbolCone *ppoint = dynamic_cast<SupportSymbolCone *>(psymbol);
					addMark(ppoint->GetPosition());
				}
				else if (psymbol->Type == SymbolType::NormalPoint)
				{
					SupportPoint *ppoint = dynamic_cast<SupportPoint *>(psymbol);
					addMark(ppoint->GetTouchRadius(), ppoint->GetPosition(), ppoint->GetDirection());
				}
			}
			bool isCancelled = false;
			OnGenerateMeshsProcessRateUpdate(95.0, isCancelled);	// Added for Mark/Cone/Tree generate. Just set the progress to 95.
		}

		void GenMesh(shared_ptr<SupportData> &supportData)		// C#: GenMarkMesh
		{
			RHVector3 origin(0, 0, 0);
			int num_phi = 12;		// Charles20191206: The previous version was 16, but Tim updated to 12 for consistency.
			int num_theta = 12;
			int num_sides = 12;

			if (supportData->supType == (int)ModelType::MARK
				|| supportData->supType == (int)ModelType::MARK_CONE
				|| supportData->supType == (int)ModelType::MARK_BRANCH)
			{
				AddTopSphere(supportData->originalModel, num_phi, num_theta, origin, supportData->radius1);
				AddBtmSphere(supportData->originalModel, num_phi, num_theta, origin, supportData->radius1);
				supportData->UpdateMatrix();
				supportData->UpdateBoundingBox();
			}
			else if (supportData->supType == (int)ModelType::SYMBOL_BRANCH)
			{
				RHVector3 endPoint(0, 0, supportData->length);
				AddTopSphere(supportData->originalModel, num_phi, num_theta, endPoint, supportData->radius1);
				AddTreeSymbol(supportData->originalModel, num_sides, ModelType::SYMBOL_BRANCH, endPoint, supportData->radius1, supportData->radius2);
				AddBtmSphere(supportData->originalModel, num_phi, num_theta, origin, supportData->radius2);

				supportData->UpdateMatrix();
				supportData->UpdateBoundingBox();
			}
			else if (supportData->supType == (int)ModelType::SYMBOL_TRUNK)
			{
				RHVector3 endPoint(0, 0, supportData->length);
				AddTopSphere(supportData->originalModel, num_phi, num_theta, endPoint, supportData->radius1);
				AddTreeSymbol(supportData->originalModel, num_sides, ModelType::SYMBOL_TRUNK, endPoint, supportData->radius1, supportData->radius2);

				supportData->UpdateMatrix();
				supportData->UpdateBoundingBox();
			}
		}

	private:
		/// <summary>
		/// Add support mark model.
		/// This function is used for the manual mode of support editor.
		/// </summary>
		/// <param name="nearModel">The model is that the support model to be add.</param>XX
		/// <param name="supRadius">Radius of support contact point.</param>
		/// <param name="supTouchWor">World coordinate of vertex that support to be generate at.</param>
		/// <param name="supTouchNor">The normal vector at the vertex.</param>XX
		/// <returns>Mesh(SupportData) of mark</returns>
		void addMark(double supRadius, RHVector3 supTouchWor, RHVector3 supTouchNor)	// Add marks for Cylinder
		{
			unique_ptr<SupportData> supMark(new SupportData());

			//supMark.parentModel = BelongModel;
			supMark->supType = (int)ModelType::MARK;
			supMark->SetPosition(Coord3D((float)supTouchWor.x, (float)supTouchWor.y, (float)supTouchWor.z));
			supMark->radius1 = supRadius;
			supMark->orientation = supTouchNor;	// Charles: added for storing the support point's direction.
			//supMark->SetScale(Coord3D((float)supRadius / 5, (float)supRadius / 5, (float)supRadius / 5));
			//supMark.UpdateBoundingBox();
			supMark->UpdateMatrix();
			supMark->UpdateBoundingBox();
			if (supMark->GetPosition().z - supRadius < 0)
			{
				supMark->Clear();
				return;
			}
			_items2.push_back(move(supMark));
			return;
		}

		/// <summary>
		/// Add support mark model.
		/// This function is used for the manual mode of support editor.
		/// </summary>
		/// <param name="nearModel">The model is that the support model to be add.</param>
		/// <param name="supRadius">Radius of support contact point.</param>
		/// <param name="supTouchWor">World coordinate of vertex that support to be generate at.</param>
		/// <param name="supTouchNor">The normal vector at the vertex.</param>
		/// <returns>Mesh(SupportModel) of mark</returns>
		void addMark(RHVector3 supTouchWor)	// cone mark --> a small ball	===> C#: SupportMeshsMark.cs in SupportCone folder.
		{
			unique_ptr<SupportData> supMark(new SupportData());

			//supMark.parentModel = BelongModel;
			supMark->supType = (int)ModelType::MARK_CONE;
			supMark->SetPosition(Coord3D((float)supTouchWor.x, (float)supTouchWor.y, (float)supTouchWor.z));
			// supMark.Scale = new Coord3D((float)supRadius / 10, (float)supRadius / 10, (float)supRadius / 10);
			//supMark->SetScale(Coord3D(0.08f, 0.08f, 0.08f)); //new Coord3D(0.15f, 0.15f, 0.15f);
			supMark->radius1 = 0.5f;
			//supMark.UpdateBoundingBox();
			supMark->UpdateMatrix();
			supMark->UpdateBoundingBox();
			if (supMark->GetPosition().z - 0.08f < 0)
			{
				supMark->Clear();
				return;
			}
			_items2.push_back(move(supMark));

			return;
		}

		void AddMark(const SupportSymbolTree *ptree)	// Tree trunk mark ---> a cylinder, C#: SupportMeshsMark.cs in SupportTree folder
		{
			// Branch Support Symbol - initial reference parameters
			double refAngle45 = 0.7854; // minimum reference angle for a good branch support
			RHVector3 branchSupOrigin = RHVector3(0, 0, 0); //initial branch position @origin vector(0,0,0)
			RHVector3 branchSupEnd = RHVector3(0, 0, ptree->GetLength()); //initial branch end reference to origin(0,0,0)
			//--
			//Debug.Assert(MeshType == MeshTypeCode.Mark);
			unique_ptr<SupportData> supSymbol(new SupportData());
			//if (Command == COMMAND.Abort)
			//{
			//	supSymbol.Clear();
			//	Status = STATUS.UserAbort;
			//	OnGenerateMeshsDone(this, new GenerateDoneEventArgs(GenerateDoneEventArgs.DoneEventCode.UserAbort));
			//	return null;
			//}

			//supSymbol.parentModel = BelongModel;
			supSymbol->psymbol = (SupportSymbol *) ptree;

			//Generate Trunk Support Symbol mesh
			if (ptree->Type == SymbolType::TreeTrunk)
			{
				supSymbol->supType = (int)ModelType::SYMBOL_TRUNK;

				//AddTopSphere(supSymbol->originalModel, num_phi, num_theta, RHVector3(0, 0, ptree->GetTopPosition().z), 0.5);
				//AddTreeSymbol(supSymbol->originalModel, (ModelType)supSymbol->supType, RHVector3(0, 0, ptree->GetTopPosition().z), 0.5, 0.5);
				supSymbol->length = ptree->GetTopPosition().z;
				supSymbol->radius1 = 0.5;
				supSymbol->radius2 = 0.5;
				// C# AP: supSymbol->originalModel = supSymbol->originalModel.Copy();
				supSymbol->SetPosition(Coord3D((float)ptree->GetTopPosition().x, (float)ptree->GetTopPosition().y, 0));
			}
			//Generate Branch Support Symbol mesh 
			else if (ptree->Type == SymbolType::TreeBranch)
			{
				supSymbol->supType = (int)ModelType::SYMBOL_BRANCH;

				//AddTopSphere(supSymbol->originalModel, num_phi, num_theta, branchSupEnd, 0.2);
				//AddTreeSymbol(supSymbol->originalModel, (ModelType)supSymbol->supType, branchSupEnd, 0.2, 0.3);
				//AddBtmSphere(supSymbol->originalModel, num_phi, num_theta, branchSupOrigin, 0.3);
				supSymbol->length = ptree->GetLength();
				supSymbol->radius1 = 0.2;
				supSymbol->radius2 = 0.3;
				// C# AP: supSymbol.originalModel = supSymbol.originalModel.Copy();

				supSymbol->SetPosition(Coord3D((float)ptree->GetPosition().x, (float)ptree->GetPosition().y, (float)ptree->GetPosition().z));
				supSymbol->orientation = ptree->GetDirection();

				double angle = supSymbol->RotateModel(); //Rotate the branch symbol to the specified direction

				//Check branch support structure integrity - check angle
				if (angle >= 0 && angle <= refAngle45)
					supSymbol->originalModel.branchSupError = false;
				else
					supSymbol->originalModel.branchSupError = true;
			}

			//supSymbol.UpdateBoundingBox();
			supSymbol->UpdateMatrix();
			supSymbol->UpdateBoundingBox();
			_items2.push_back(move(supSymbol));

			if (ptree->Type == SymbolType::TreeBranch)
			{
				//if (multiBranchIdx++ % multiBranchProcessStep == 0)
				//	OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(((double)multiBranchIdx / multiBranchCountTotal) * 100.0)));
			}
		}

		void AddPoint(RHVector3 supTouchWor, RHVector3 supTouchNor, const SupportPointTree *point)	// for Tree Support, C#: SuuportMeshsMark.cs in SupportTree.cs
		{
			//Debug.Assert(MeshType == MeshTypeCode.Mark);

			unique_ptr<SupportData> supPoint(new SupportData());

			//supPoint.parentModel = BelongModel;
			supPoint->supType = (int)ModelType::MARK_BRANCH;
			supPoint->SetPosition(Coord3D((float)supTouchWor.x, (float)supTouchWor.y, (float)supTouchWor.z));
			//supPoint->SetScale(Coord3D(0.07f, 0.07f, 0.07f)); //new Coord3D(0.10f, 0.10f, 0.10f);
			supPoint->radius1 = 0.5f;
			supPoint->orientation = supTouchNor;
			supPoint->pbranchpoint = (SupportSymbol *)point;	// cannot use static_cast because point is declared as 'const'
			//supPoint.UpdateBoundingBox();
			if (supPoint->GetPosition().z - 0.07f < 0)
			{
				supPoint->Clear();
				return;
			}
			supPoint->UpdateMatrix();

			_items2.push_back(move(supPoint));
			return;
		}
	};
}