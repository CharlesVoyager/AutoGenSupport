#pragma once
#include "SupportSymbolCone.h"
#include "SupportData.h"
#include "DataCombination.h"
#include "GenBasic.h"

namespace XYZSupport
{
	class SupportMeshsCone : public SupportMeshsBase
	{
	private:
		const int num_phi = 24;				
		const int num_theta = 48;			
		const int num_sides = 48;	

	public:
		SupportMeshsCone()
		{
			editMode = PointEditModeCode::Cone;
		}

		// [input] psymbol: SupportSymbolCone for cone
		void OnAdjustGenerate()
		{
			DbgMsg(__FUNCTION__"[IN]");
			unique_ptr<SupportData> conesupport(new SupportData());
			SupportSymbolCone *psym = dynamic_cast<SupportSymbolCone *>(Symbols[0].get());
			RHVector3 tempPosition;

			conesupport->supType = (int)ModelType::CONE;
			double newZpozition = psym->GetPosition().z - (psym->GetRadiusTop() * psym->DepthAdjust); // default cone height (less 50% of top radius) less depth adjust
			tempPosition = RHVector3(psym->GetPosition().x, psym->GetPosition().y, newZpozition);
			AddTopSphere(conesupport->originalModel, num_phi, num_theta, RHVector3(0, 0, tempPosition.z), psym->GetRadiusTop());
			AddCone(conesupport->originalModel, RHVector3(0, 0, tempPosition.z), psym->GetRadiusTop(), psym->GetRadiusBase());
			
			// C#: ((SupportSymbolCone)symbol).DepthAdjustSelected = false;	//set in XYZSupportWrapper
			
			// C#: conesupport->symbol = (SupportSymbolCone)symbol;;		//set in XYZSupportWrapper
			conesupport->SetPosition(Coord3D(tempPosition.x, tempPosition.y, 0));
			conesupport->originalModel.selectedDepthAdjust = true;			//set in XYZSupportWrapper
			_items2.push_back(move(conesupport));							//在C++ module, 每次作Cone adjust, 都會重新SUP_Initial(), 因此這個_items2只會存一個SupportData.

			//OnGenerateMeshsDone(this, new GenerateDoneEventArgs());

			DbgMsg(__FUNCTION__"[OUT]");	
		}
		void Generate2()
		{
			generateConeMesh();

			bool isCancelled = false;
			OnGenerateMeshsProcessRateUpdate(95.0, isCancelled);	// Added for Mark/Cone/Tree generate. Just set the progress to 95.
		}

		void GenMesh(shared_ptr<SupportData> &supportData)		// C#: GenMarkMesh
		{
			RHVector3 endPoint(0, 0, supportData->length);
			int num_phi = 24;
			int num_theta = 48;

			AddTopSphere(supportData->originalModel, num_phi, num_theta, endPoint, supportData->radius1);
			AddCone(supportData->originalModel, endPoint, supportData->radius1, supportData->radius2);
		}
	private:
		/// <summary>
		/// Generate cone support mesh using STL format
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="radius_top"></param>
		/// <param name="radius_base"></param>
		/// <returns></returns>
		int generateConeMesh()
		{
			DbgMsg(__FUNCTION__"[IN]");
			DbgMsg(__FUNCTION__"===> Symbols.Count(): %d", Symbols.size());

			//Debug.Assert(false, "Not yet implement.");
			RHVector3 tempPosition;
			double newZpozition;
			if (Symbols.size() == 0)
			{
			/*	Status = STATUS.Done;
				OnGenerateMeshsDone(this, new GenerateDoneEventArgs());*/
				return 0;
			}

			Status = STATUS::Busy;

			int totalCountOfProgressBar = 2 * static_cast<int>(Symbols.size());
			int cnt = 0;

			for (int i = 0; i < Symbols.size(); i++)	//foreach(SupportSymbolCone sym in Symbols)
			{
				SupportSymbolCone *psym = dynamic_cast<SupportSymbolCone *>(Symbols[i].get());	//IMPORTANT: Because we need to update the Support data, we have to use pointer.

				DbgMsg(__FUNCTION__"===> psym->GetPosition(): %s", psym->GetPosition().ToString().c_str());

				unique_ptr<SupportData> conesupport(new SupportData());
				conesupport->supType = (int)ModelType::CONE;
				conesupport->originalModel.selectedDepthAdjust = false;

				newZpozition = psym->GetPosition().z - (psym->GetRadiusTop() * psym->DepthAdjust); //the default cone height is less than 50% of the top radius
				tempPosition = RHVector3(psym->GetPosition().x, psym->GetPosition().y, newZpozition);

				if (psym->DepthAdjustSelected)
				{
					conesupport->originalModel.selectedDepthAdjust = true;
					psym->DepthAdjustSelected = false;
				}

				//AddTopSphere(conesupport->originalModel, num_phi, num_theta, tempPosition, psym->GetRadiusTop());
				conesupport->depthAdjust = psym->DepthAdjust;
				conesupport->length = newZpozition;
				Coord3D pos((float)tempPosition.x, (float)tempPosition.y, 0);
				conesupport->SetPosition(pos);
				conesupport->radius1 = psym->GetRadiusTop();
				//AddCone(conesupport->originalModel, tempPosition, psym->GetRadiusTop(), psym->GetRadiusBase());
				conesupport->radius2 = psym->GetRadiusBase();
				cnt++;
				conesupport->psymbol = (SupportSymbol *)psym;
				conesupport->UpdateMatrix();

				_items2.push_back(move(conesupport));
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
			DbgMsg(__FUNCTION__"[OUT]");
			return 0;
		}

		// Add a cone with smooth sides.
		void AddCone(TopoModel &conesupportmodel, RHVector3 end_point, double radius1, double radius2)
		{
			// Get two vectors perpendicular to the axis.
			RHVector3 p1, p2, p3, p4;
			TopoVertex v1, v2, v3;
			TopoTriangle triangle;

			Tuple<RHVector3, double> ConePoint = Tuple<RHVector3, double>(end_point, radius2);

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
				//v1 = conesupportmodel.ActiveModel.addVertex(end_point);
				//v2 = conesupportmodel.ActiveModel.addVertex(p1);
				//v3 = conesupportmodel.ActiveModel.addVertex(p2);
				//triangle = new TopoTriangle(v1, v2, v3);
				//conesupportmodel.ActiveModel.AddTriangle(triangle);
			}

			// Make the bottom end cap.
			RHVector3 end_point2 = RHVector3(end_point.x, end_point.y, 0.0);
			theta = 0;
			for (int i = 0; i < num_sides; i++)
			{
				double x2 = radius2 * cos(theta);
				double y2 = radius2 * sin(theta);

				p3 = RHVector3(end_point2.x + x2, end_point2.y + y2, 0.0);

				theta += dtheta;

				x2 = radius2 * cos(theta);
				y2 = radius2 * sin(theta);

				p4 = RHVector3(end_point2.x + x2, end_point2.y + y2, 0.0);

				v1 = conesupportmodel.addVertex(end_point2);
				v2 = conesupportmodel.addVertex(p4);
				v3 = conesupportmodel.addVertex(p3);
				triangle = TopoTriangle(v1, v2, v3);
				conesupportmodel.AddTriangle(triangle);

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
				p3 = RHVector3(end_point2.x + sidebot_x, end_point2.y + sidebot_y, 0.0);

				theta += dtheta;

				sidetop_x = radius1 * cos(theta);
				sidetop_y = radius1 * sin(theta);

				sidebot_x = radius2 * cos(theta);
				sidebot_y = radius2 * sin(theta);

				p2 = RHVector3(end_point.x + sidetop_x, end_point.y + sidetop_y, end_point.z);
				p4 = RHVector3(end_point2.x + sidebot_x, end_point2.y + sidebot_y, 0.0);

				v1 = conesupportmodel.addVertex(p1);
				v2 = conesupportmodel.addVertex(p3);
				v3 = conesupportmodel.addVertex(p2);
				triangle = TopoTriangle(v1, v2, v3);
				conesupportmodel.AddTriangle(triangle);

				v1 = conesupportmodel.addVertex(p2);
				v2 = conesupportmodel.addVertex(p3);
				v3 = conesupportmodel.addVertex(p4);
				triangle = TopoTriangle(v1, v2, v3);
				conesupportmodel.AddTriangle(triangle);
			}
		}
	};
}