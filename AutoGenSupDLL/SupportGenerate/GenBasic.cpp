#include <stdafx.h>
#include "GenBasic.h"
#include "SupportMeshsBed.h"

namespace XYZSupport
{
	//AddMesh from C# AP: private void AddTopSphere(SupportData treesupportmodel, RHVector3 center, double radius)
	// Input: num_phi, num_theta,
	// Output: model
	void AddTopSphere(/*out*/TopoModel &model, const int num_phi, const int num_theta, RHVector3 center, double radius)
	{
		double phi0, theta0;
		double dphi = M_PI / num_phi;
		double dtheta = 2 * M_PI / num_theta;

		phi0 = 0;
		double z0 = radius * cos(phi0);
		double r0 = radius * sin(phi0);
		for (int i = 0; i < num_phi / 2; i++)
		{
			double phi1 = phi0 + dphi;
			double z1 = radius * cos(phi1);
			double r1 = radius * sin(phi1);

			// Point ptAB has phi value A and theta value B.
			// For example, pt01 has phi = phi0 and theta = theta1.
			// Find the points with theta = theta0.
			theta0 = 0;
			RHVector3 pt00 = RHVector3(
				center.x + r0 * cos(theta0),
				center.y + r0 * sin(theta0),
				center.z + z0);

			RHVector3 pt10 = RHVector3(
				center.x + r1 * cos(theta0),
				center.y + r1 * sin(theta0),
				center.z + z1);
			for (int j = 0; j < num_theta; j++)
			{
				// Find the points with theta = theta1.
				double theta1 = theta0 + dtheta;
				RHVector3 pt01 = RHVector3(
					center.x + r0 * cos(theta1),
					center.y + r0 * sin(theta1),
					center.z + z0);
				RHVector3 pt11 = RHVector3(
					center.x + r1 * cos(theta1),
					center.y + r1 * sin(theta1),
					center.z + z1);

				TopoVertex v1 = model.addVertex(pt00);
				TopoVertex v2 = model.addVertex(pt10);
				TopoVertex v3 = model.addVertex(pt11);
				TopoTriangle triangle(v1, v2, v3);
				model.AddTriangle(triangle);
#if 0	// C# AP
				v1 = model.addVertex(pt00);
				v2 = model.addVertex(pt11);
				v3 = model.addVertex(pt01);
				triangle = TopoTriangle(v1, v2, v3);
				model.AddTriangle(triangle);
#else	// Optimized Code
				TopoVertex v4 = model.addVertex(pt01);
				triangle = TopoTriangle(v1, v3, v4);
				model.AddTriangle(triangle);			//Don't use "model.triangles.Add(triangle)". It need to check if the trangle is degenerated.
#endif
				// Move to the next value of theta.
				theta0 = theta1;
				pt00 = pt01;
				pt10 = pt11;
			}
			// Move to the next value of phi.
			phi0 = phi1;
			z0 = z1;
			r0 = r1;
		}
	}

	void AddTree(/*out*/TopoModel &model, const int num_sides, RHVector3 end_point, double radius1, double radius2, bool isNeedToMakeTopEndCap)
	{
		// Get two vectors perpendicular to the axis.
		RHVector3 p1, p2, p3, p4;
		RHVector3 start_point;
		TopoVertex v1, v2, v3, v4;
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
		}

		start_point = RHVector3(end_point.x, end_point.y, 0.0);
		theta = 0;

		for (int i = 0; i < num_sides; i++)
		{
			double x2 = radius2 * cos(theta);
			double y2 = radius2 * sin(theta);

			p3 = RHVector3(start_point.x + x2, start_point.y + y2, start_point.z);
			theta += dtheta;
			x2 = radius2 * cos(theta);
			y2 = radius2 * sin(theta);

			p4 = RHVector3(start_point.x + x2, start_point.y + y2, start_point.z);
			v1 = model.addVertex(start_point);
			v2 = model.addVertex(p4);
			v3 = model.addVertex(p3);

			triangle = TopoTriangle(v1, v2, v3);
			if (isNeedToMakeTopEndCap == true)
			{
				model.AddTriangle(triangle);	//Don't use "model.triangles.Add(triangle)". It need to check if the trangle is degenerated.
			}
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

#if 0		// C# AP
			v1 = model.addVertex(p1);
			v2 = model.addVertex(p3);
			v3 = model.addVertex(p2);
			triangle = TopoTriangle(v1, v2, v3);
			model.AddTriangle(triangle);

			v1 = model.addVertex(p2);
			v2 = model.addVertex(p3);
			v3 = model.addVertex(p4);
			triangle = TopoTriangle(v1, v2, v3);
			model.AddTriangle(triangle);
#else		// Optimized Code.
			v1 = model.addVertex(p1);
			v2 = model.addVertex(p2);
			v3 = model.addVertex(p3);
			v4 = model.addVertex(p4);
			triangle = TopoTriangle(v1, v3, v2);
			model.AddTriangle(triangle);

			triangle = TopoTriangle(v2, v3, v4);
			model.AddTriangle(triangle);
#endif
		}
	}

	void AddBtmSphere(/*out*/TopoModel &model, const int num_phi, const int num_theta, RHVector3 center, double radius)
	{
		double phi0, theta0;
		double dphi = M_PI / num_phi;
		double dtheta = 2 * M_PI / num_theta;

		phi0 = 0;
		double z0 = radius * cos(phi0);
		double r0 = radius * sin(phi0);

		//center = new RHVector3(center.x, center.y, center.z + radius);

		for (int i = 0; i < num_phi; i++)
		{
			double phi1 = phi0 + dphi;
			double z1 = radius * cos(phi1);
			double r1 = radius * sin(phi1);

			// Point ptAB has phi value A and theta value B.
			// For example, pt01 has phi = phi0 and theta = theta1.
			// Find the points with theta = theta0.
			theta0 = 0;
			RHVector3 pt00 = RHVector3(
				center.x + r0 * cos(theta0),
				center.y + r0 * sin(theta0),
				center.z + z0);

			RHVector3 pt10 = RHVector3(
				center.x + r1 * cos(theta0),
				center.y + r1 * sin(theta0),
				center.z + z1);
			for (int j = 0; j < num_theta; j++)
			{
				// Find the points with theta = theta1.
				double theta1 = theta0 + dtheta;
				RHVector3 pt01 = RHVector3(
					center.x + r0 * cos(theta1),
					center.y + r0 * sin(theta1),
					center.z + z0);
				RHVector3 pt11 = RHVector3(
					center.x + r1 * cos(theta1),
					center.y + r1 * sin(theta1),
					center.z + z1);

				if (i >= num_phi / 2)
				{
					TopoVertex v1 = model.addVertex(pt00);
					TopoVertex v2 = model.addVertex(pt10);
					TopoVertex v3 = model.addVertex(pt11);
					TopoTriangle triangle(v1, v2, v3);
					model.AddTriangle(triangle);
#if 0	// C# AP
					v1 = model.addVertex(pt00);
					v2 = model.addVertex(pt11);
					v3 = model.addVertex(pt01);
					triangle = TopoTriangle(v1, v2, v3);
					model.AddTriangle(triangle);
#else	// Optimized Code
					TopoVertex v4 = model.addVertex(pt01);
					triangle = TopoTriangle(v1, v3, v4);
					model.AddTriangle(triangle);//Don't use "model.triangles.Add(triangle)". It need to check if the trangle is degenerated.
#endif				
				}
				// Move to the next value of theta.
				theta0 = theta1;
				pt00 = pt01;
				pt10 = pt11;
			}

			// Move to the next value of phi.
			phi0 = phi1;
			z0 = z1;
			r0 = r1;
		}
	}

	void AddTreeSymbol(TopoModel &treesupportmodel, const int num_sides, ModelType supType, RHVector3 end_point, double radius1, double radius2)
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
		if (supType == ModelType::SYMBOL_TRUNK)
			start_point = RHVector3(end_point.x, end_point.y, 0.0);
		else if (supType == ModelType::SYMBOL_BRANCH)
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

	void GenerateCylinder(int num_sides, RHVector3 EndPoint, RHVector3 StartPoint, double TopRadius, double BottomRadius, /*out*/ TopoModel &model)
	{
		RHVector3 Points[4];
		TopoVertex Vertices[3];
		TopoTriangle Triangle;

		double Theta = 0;
		double dTheta = 2.0 * M_PI / num_sides;

		// Top Part
		for (int i = 0; i < num_sides; i++)
		{
			double x1 = TopRadius * cos(Theta);
			double y1 = TopRadius * sin(Theta);

			Points[0] = RHVector3(StartPoint.x + x1, StartPoint.y + y1, StartPoint.z);

			Theta += dTheta;

			x1 = TopRadius * cos(Theta);
			y1 = TopRadius * sin(Theta);

			Points[1] = RHVector3(StartPoint.x + x1, StartPoint.y + y1, StartPoint.z);

			Vertices[0] = model.addVertex(StartPoint);
			Vertices[1] = model.addVertex(Points[0]);
			Vertices[2] = model.addVertex(Points[1]);
			Triangle = TopoTriangle(Vertices[0], Vertices[1], Vertices[2]);
			model.AddTriangle(Triangle);
		}
		Theta = 0;

		// Bottom Part
		for (int i = 0; i < num_sides; i++)
		{
			double x2 = BottomRadius * cos(Theta);
			double y2 = BottomRadius * sin(Theta);

			Points[2] = RHVector3(EndPoint.x + x2, EndPoint.y + y2, EndPoint.z);

			Theta += dTheta;

			x2 = BottomRadius * cos(Theta);
			y2 = BottomRadius * sin(Theta);

			Points[3] = RHVector3(EndPoint.x + x2, EndPoint.y + y2, EndPoint.z);

			Vertices[0] = model.addVertex(EndPoint);
			Vertices[1] = model.addVertex(Points[3]);
			Vertices[2] = model.addVertex(Points[2]);
			Triangle = TopoTriangle(Vertices[0], Vertices[1], Vertices[2]);
			model.AddTriangle(Triangle);
		}
		Theta = 0;

		// Side Part
		for (int i = 0; i < num_sides; i++)
		{
			double SideTop_X = TopRadius * cos(Theta);
			double SideTop_Y = TopRadius * sin(Theta);

			double SideBottom_X = BottomRadius * cos(Theta);
			double SideBottom_Y = BottomRadius * sin(Theta);

			Points[0] = RHVector3(StartPoint.x + SideTop_X, StartPoint.y + SideTop_Y, StartPoint.z);
			Points[2] = RHVector3(EndPoint.x + SideBottom_X, EndPoint.y + SideBottom_Y, EndPoint.z);

			Theta += dTheta;

			SideTop_X = TopRadius * cos(Theta);
			SideTop_Y = TopRadius * sin(Theta);

			SideBottom_X = BottomRadius * cos(Theta);
			SideBottom_Y = BottomRadius * sin(Theta);

			Points[1] = RHVector3(StartPoint.x + SideTop_X, StartPoint.y + SideTop_Y, StartPoint.z);
			Points[3] = RHVector3(EndPoint.x + SideBottom_X, EndPoint.y + SideBottom_Y, EndPoint.z);

			Vertices[0] = model.addVertex(Points[0]);
			Vertices[1] = model.addVertex(Points[2]);
			Vertices[2] = model.addVertex(Points[1]);
			Triangle = TopoTriangle(Vertices[0], Vertices[1], Vertices[2]);
			model.AddTriangle(Triangle);

			Vertices[0] = model.addVertex(Points[1]);
			Vertices[1] = model.addVertex(Points[2]);
			Vertices[2] = model.addVertex(Points[3]);
			Triangle = TopoTriangle(Vertices[0], Vertices[1], Vertices[2]);
			model.AddTriangle(Triangle);
		}
	}
	void AddBrim(TopoModel &model, RHBoundingBox bbox, RHVector3 center, float printerWidth, float printerDepth)//for recent version are Width and Depth of partpro120
	{
		RHVector3 p1, p2, p3, p4, p5, p6, p7, p8;
		TopoVertex v1, v2, v3, v4, v5, v6, v7, v8;
		TopoTriangle Triangle;

		double xMax = center.x + bbox.xMax();
		double xMin = center.x + bbox.xMin();
		double yMax = center.y + bbox.yMax();
		double yMin = center.y + bbox.yMin();
		double zMin = 0;
		double zBrim = 0.5;

		//We need to adjust the brim edge only if the distance of the brim edge is larger than printer.
		//If the model is on the edge of the printer, the model with support(tails and brim) will move back from the printer edge after auto generating.
		if (xMax - xMin > printerWidth)
		{
			xMin = center.x + 0;
			xMax = center.x + printerWidth;
		}
		if (yMax - yMin > printerDepth)
		{
			yMin = center.y + 0;
			yMax = center.y + printerDepth;
		}

			
		
		// 8 vertex of the brim
		p1 = RHVector3(xMin, yMin, zMin);
		p2 = RHVector3(xMin, yMax, zMin);
		p3 = RHVector3(xMax, yMin, zMin);
		p4 = RHVector3(xMax, yMax, zMin);
		p5 = RHVector3(xMin, yMin, zBrim);
		p6 = RHVector3(xMin, yMax, zBrim);
		p7 = RHVector3(xMax, yMin, zBrim);
		p8 = RHVector3(xMax, yMax, zBrim);

		v1 = model.addVertex(p1);
		v2 = model.addVertex(p2);
		v3 = model.addVertex(p3);
		v4 = model.addVertex(p4);
		v5 = model.addVertex(p5);
		v6 = model.addVertex(p6);
		v7 = model.addVertex(p7);
		v8 = model.addVertex(p8);

		//Top and Bottom
		Triangle = TopoTriangle(v1, v2, v3);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v2, v4, v3);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v5, v7, v6);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v6, v7, v8);
		model.AddTriangle(Triangle);

		//Body
		Triangle = TopoTriangle(v1, v3, v5);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v3, v7, v5);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v3, v4, v7);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v4, v8, v7);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v4, v2, v8);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v2, v6, v8);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v2, v1, v6);
		model.AddTriangle(Triangle);
		Triangle = TopoTriangle(v1, v5, v6);
		model.AddTriangle(Triangle);

		
		
	}
}