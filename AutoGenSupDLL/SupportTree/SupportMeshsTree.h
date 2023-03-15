#pragma once
#include "stdafx.h"
#include "SupportSymbolTree.h"
#include "SupportData.h"
#include "DataCombination.h"

namespace XYZSupport
{
	class SupportMeshsTree : public SupportMeshsBase
	{
	private:
		const int num_phi = 24;				
		const int num_theta = 48;			
		const int num_sides = 48;	
		const double refAngle45 = 0.7854; // minimum reference angle for a good branch support
	
	private:
		RHVector3 ScaleVector(RHVector3 vector, double length)
		{
			double scale = length / vector.Length();
			return RHVector3(
				vector.x * scale,
				vector.y * scale,
				vector.z * scale);
		}

	public:
		SupportMeshsTree()
		{
			editMode = PointEditModeCode::Tree;
		}
		// to apply collision depth adjustment for branch/s create new symbol
		// after touched position has been adjusted- RC Grey 03012017
		SupportSymbolTree OnAdjustSupportTouchNormal(const SupportSymbolTree &origSymbol)
		{
			SupportSymbolTree outSymbol;

			double adjustvalue = origSymbol.GetRadiusTop() * origSymbol.DepthAdjust;
			RHVector3 normalpos = origSymbol.DepthAdjustNor;
			normalpos = ScaleVector(normalpos, adjustvalue);
			RHVector3 branchStart = origSymbol.GetRootPosition().Add(normalpos);
			RHVector3 branchposition = origSymbol.GetPosition();
			double branchHeight = branchStart.Distance(branchposition);
			RHVector3 branchEnd = RHVector3(branchStart.Add(RHVector3(0, 0, branchHeight)));
			RHVector3 branchDirection = RHVector3(branchposition.Subtract(branchStart));
			double branch_radiustop = origSymbol.GetRadiusTop();
			double branch_radiusbtm = origSymbol.GetRadiusBase();

			outSymbol = SupportSymbolTree(branchStart, branchEnd, branchposition, branchDirection, branch_radiustop, branch_radiusbtm, branchHeight);
			outSymbol.Type = SymbolType::TreeBranch;
			outSymbol.DepthAdjust = origSymbol.DepthAdjust;

			return outSymbol;
		}
		void OnAdjustGenerate()
		{
			DbgMsg(__FUNCTION__"[IN]");

			unique_ptr<SupportData> treesupport(new SupportData());
			SupportSymbolTree symbolTree = *(dynamic_cast<SupportSymbolTree *>(Symbols[0].get()));
			SupportSymbolTree sym = OnAdjustSupportTouchNormal(symbolTree);
			RHVector3 branchSupOrigin(0, 0, 0); //initial branch position @origin vector(0,0,0)

			if (sym.Type == SymbolType::TreeBranch)
			{
				RHVector3 branchSupEnd(0, 0, sym.GetLength()); //initial branch end reference to origin(0,0,0)

				treesupport->supType = (int)ModelType::TREE_BRANCH;
				AddTopSphere(treesupport->originalModel, num_phi, num_theta, branchSupEnd, sym.GetRadiusTop());
				AddTree(treesupport->originalModel, (ModelType)treesupport->supType, branchSupEnd, sym.GetRadiusTop(), sym.GetRadiusBase());
				AddBtmSphere(treesupport->originalModel, num_phi, num_theta, branchSupOrigin, sym.GetRadiusBase());
				treesupport->SetPosition(Coord3D((float)sym.GetPosition().x, (float)sym.GetPosition().y, (float)sym.GetPosition().z));
				treesupport->orientation = sym.GetDirection();

				double angle = treesupport->RotateModel(); //Rotate the branch symbol to the specified direction

				if (angle >= 0 && angle <= refAngle45)
					treesupport->originalModel.branchSupError = false;
				else
					treesupport->originalModel.branchSupError = true;

				//((SupportSymbolTree *)Symbols._p[0].get())->DepthAdjustSelected = false;	//Updated in in XYZSupportWrapper.
				//treesupport->psymbol = Symbols._p[0].get();								//Updated in in XYZSupportWrapper.
				treesupport->originalModel.selectedDepthAdjust = true;

				_items2.push_back(move(treesupport));
			}
			//OnGenerateMeshsDone(this, new GenerateDoneEventArgs());
			DbgMsg(__FUNCTION__"[OUT]");
			return;
		}

		void Generate2()
		{
			generateTreeMesh();
			bool isCancelled = false;
			OnGenerateMeshsProcessRateUpdate(95.0, isCancelled);	// Added for Mark/Cone/Tree generate. Just set the progress to 95.
		}

		void GenMesh(shared_ptr<SupportData> &supportData)
		{
			RHVector3 origin(0, 0, 0);
			RHVector3 endPoint(0, 0, supportData->length);
			if (supportData->supType == (int)ModelType::TREE_TRUNK)
			{
				AddTopSphere(supportData->originalModel, num_phi, num_theta, endPoint, supportData->radius1);
				AddTree(supportData->originalModel, (ModelType)supportData->supType, endPoint, supportData->radius1, supportData->radius2);
			}
			else if (supportData->supType == (int)ModelType::TREE_BRANCH)
			{
				AddTopSphere(supportData->originalModel, num_phi, num_theta, endPoint, supportData->radius1);
				AddTree(supportData->originalModel, (ModelType)supportData->supType, endPoint, supportData->radius1, supportData->radius2);
				AddBtmSphere(supportData->originalModel, num_phi, num_theta, origin, supportData->radius2);
			}
		}
	private:



		/// <summary>
		/// Generate tree support mesh using STL format
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="radius_top"></param>
		/// <param name="radius_base"></param>
		/// <returns></returns>
		int generateTreeMesh()
		{
			//DbgMsg(__FUNCTION__"[IN]");

			RHVector3 branchSupOrigin = RHVector3(0, 0, 0); //initial branch position @origin vector(0,0,0)
			RHVector3 branchSupEnd;

			if (Symbols.size() == 0)
			{
				Status = STATUS::Done;
				//OnGenerateMeshsDone(this, new GenerateDoneEventArgs());
				return 0;
			}

			Status = STATUS::Busy;

			int totalCountOfProgressBar = 2 * static_cast<int>(Symbols.size());
			int cnt = 0;

			for(const auto &psym : Symbols)	//foreach(SupportSymbolTree sym in Symbols)
			{
				//DbgMsg(__FUNCTION__"===> psym->GetPosition(): %s", psym->GetPosition().ToString().c_str());
				//DbgMsg(__FUNCTION__"===> psym->Type: %d (3: TRUNK, 4: Branch)", psym->Type);

				unique_ptr<SupportData> treesupport(new SupportData());
				if (psym->Type == SymbolType::TreeTrunk)
				{
					treesupport->supType = (int)ModelType::TREE_TRUNK;
					//AddTopSphere(treesupport->originalModel, num_phi, num_theta, RHVector3(0, 0, psym->GetTopPosition().z), psym->GetRadiusTop());
					//AddTree(treesupport->originalModel, (ModelType)treesupport->supType, RHVector3(0, 0, psym->GetTopPosition().z), psym->GetRadiusTop(), psym->GetRadiusBase());
					treesupport->SetPosition(Coord3D((float)psym->GetTopPosition().x, (float)psym->GetTopPosition().y, 0));
					treesupport->depthAdjust = dynamic_cast<SupportSymbolTree *>(psym.get())->DepthAdjust;
					treesupport->length = psym->GetTopPosition().z;
					treesupport->radius1 = psym->GetRadiusTop();
					treesupport->radius2 = psym->GetRadiusBase();
				}
				if (psym->Type == SymbolType::TreeBranch)
				{
					//if (symbranch.DepthAdjust > 0 && symbranch.DepthAdjust < 1)
					//{
						SupportSymbolTree symbranch = OnAdjustSupportTouchNormal( *(dynamic_cast<SupportSymbolTree *>(psym.get())) );
						branchSupEnd = RHVector3(0, 0, symbranch.GetLength()); //initial branch end reference to origin(0,0,0)
					//}
					//else
					//branchSupEnd = new RHVector3(0, 0, symbranch.Length); //initial branch end reference to origin(0,0,0)

					treesupport->supType = (int)ModelType::TREE_BRANCH;
					//AddTopSphere(treesupport->originalModel, num_phi, num_theta, branchSupEnd, symbranch.GetRadiusTop());
					//AddTree(treesupport->originalModel, (ModelType)treesupport->supType, branchSupEnd, symbranch.GetRadiusTop(), symbranch.GetRadiusBase());
					//AddBtmSphere(treesupport->originalModel, num_phi, num_theta, branchSupOrigin, symbranch.GetRadiusBase());
					treesupport->SetPosition(Coord3D((float)symbranch.GetPosition().x, (float)symbranch.GetPosition().y, (float)symbranch.GetPosition().z));
					treesupport->orientation = symbranch.GetDirection();
					treesupport->depthAdjust = symbranch.DepthAdjust;
					treesupport->length = symbranch.GetLength();
					treesupport->radius1 = psym->GetRadiusTop();
					treesupport->radius2 = psym->GetRadiusBase();

					double angle = treesupport->RotateModel(); //Rotate the branch symbol to the specified direction

					//Check branch support structure integrity - check angle
					if (angle >= 0 && angle <= refAngle45)
						treesupport->originalModel.branchSupError = false;
					else
						treesupport->originalModel.branchSupError = true;
				}
				cnt++;

				treesupport->psymbol = psym.get();

				treesupport->UpdateMatrix();

				_items2.push_back(move(treesupport));
			}

			totalCountOfProgressBar = 2 * static_cast<int>(Symbols.size());
			if (totalCountOfProgressBar == 0)
				totalCountOfProgressBar = 1;

			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));

			//if (Command == COMMAND.Abort)
			//{
			//	Command = COMMAND.None;
			//	Status = STATUS.UserAbort;
			//	OnGenerateMeshsDone(this, new GenerateDoneEventArgs(GenerateDoneEventArgs.DoneEventCode.UserAbort));
			//	return 0;
			//}

			Status = STATUS::Done;
			//OnGenerateMeshsDone(this, new GenerateDoneEventArgs());
			//DbgMsg(__FUNCTION__"[OUT]");
			return 0;
		}

		// Add a Tree with smooth sides.
		void AddTree(TopoModel &treesupportmodel, ModelType supType, RHVector3 end_point, double radius1, double radius2)
		{
			// Get two vectors perpendicular to the axis.
			RHVector3 p1, p2, p3, p4;
			RHVector3 start_point;
			TopoVertex v1, v2, v3;
			TopoTriangle triangle;

			// Make the top end cap.
			double theta = 0;
			double dtheta = 2 * M_PI / num_sides;

			for (int i = 0; i < num_sides; i++)
			{
				double x1 = radius1 * cos(theta);
				double y1 = radius1 * sin(theta);

				p1 = RHVector3(end_point.x + x1, end_point.y + y1, end_point.z);

				theta += dtheta;

				x1 = radius1 * cos(theta);
				y1 = radius1 * sin(theta);

				p2 = RHVector3(end_point.x + x1, end_point.y + y1, end_point.z);

				// Top side
				//v1 = treesupportmodel.ActiveModel.addVertex(end_point);
				//v2 = treesupportmodel.ActiveModel.addVertex(p1);
				//v3 = treesupportmodel.ActiveModel.addVertex(p2);
				//triangle = new TopoTriangle(v1, v2, v3);
				//treesupportmodel.ActiveModel.AddTriangle(triangle);
			}

			//mesh start point
			if (supType == ModelType::TREE_TRUNK)
				start_point = RHVector3(end_point.x, end_point.y, 0.0);
			else if (supType == ModelType::TREE_BRANCH)
				start_point = RHVector3(0, 0, 0);

			theta = 0;
			// Make the bottom end cap.
			for (int i = 0; i < num_sides; i++)
			{
				double x2 = radius2 * cos(theta);
				double y2 = radius2 * sin(theta);

				p3 = RHVector3(start_point.x + x2, start_point.y + y2, start_point.z);

				theta += dtheta;

				x2 = radius2 * cos(theta);
				y2 = radius2 * sin(theta);

				p4 = RHVector3(start_point.x + x2, start_point.y + y2, start_point.z);

				v1 = treesupportmodel.addVertex(start_point);
				v2 = treesupportmodel.addVertex(p4);
				v3 = treesupportmodel.addVertex(p3);
				triangle = TopoTriangle(v1, v2, v3);
				treesupportmodel.AddTriangle(triangle);
			}

			// Make the sides.
			theta = 0;
			for (int i = 0; i < num_sides; i++)
			{
				double sidetop_x = radius1 * cos(theta);
				double sidetop_y = radius1 * sin(theta);

				double sidebot_x = radius2 * cos(theta);
				double sidebot_y = radius2 * sin(theta);

				p1 = RHVector3(end_point.x + sidetop_x, end_point.y + sidetop_y, end_point.z);
				p3 = RHVector3(start_point.x + sidebot_x, start_point.y + sidebot_y, start_point.z);

				theta += dtheta;

				sidetop_x = radius1 * cos(theta);
				sidetop_y = radius1 * sin(theta);

				sidebot_x = radius2 * cos(theta);
				sidebot_y = radius2 * sin(theta);

				p2 = RHVector3(end_point.x + sidetop_x, end_point.y + sidetop_y, end_point.z);
				p4 = RHVector3(start_point.x + sidebot_x, start_point.y + sidebot_y, start_point.z);

				v1 = treesupportmodel.addVertex(p1);
				v2 = treesupportmodel.addVertex(p3);
				v3 = treesupportmodel.addVertex(p2);
				triangle = TopoTriangle(v1, v2, v3);
				treesupportmodel.AddTriangle(triangle);

				v1 = treesupportmodel.addVertex(p2);
				v2 = treesupportmodel.addVertex(p3);
				v3 = treesupportmodel.addVertex(p4);
				triangle = TopoTriangle(v1, v2, v3);
				treesupportmodel.AddTriangle(triangle);
			}
		}
	};
}