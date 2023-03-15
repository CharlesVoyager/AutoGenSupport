#pragma once
#include <vector>
#include <unordered_map>
#include "RHVector3.h"
using namespace std;

namespace XYZSupport
{
	struct Vector3d
	{
		double x;
		double y;
		double z;
	};

	class LayerVector3
	{
	public:
		vector<Vector3d> Vector3ds;
		void AddPoint()
		{
			Vector3d polygon;
			Vector3ds.push_back(polygon);
		}
	};

	class MeshModel
	{
	public:
		vector<LayerVector3> Meshs;
		vector<RHVector3> normallist;
		//public List<int> angles = new List<int>();
		unordered_map<int, double> horizontaltriangle;

		void AddPolygon()
		{
			LayerVector3 Vector3;
			Meshs.push_back(Vector3);
		}
		void CaculateNormalVector()
		{
			for (int i = 0; i < Meshs.size(); i++)
			{
				RHVector3 a1(0, 0, 0);
				a1.x = Meshs[i].Vector3ds[1].x - Meshs[i].Vector3ds[0].x;
				a1.y = Meshs[i].Vector3ds[1].y - Meshs[i].Vector3ds[0].y;
				a1.z = Meshs[i].Vector3ds[1].z - Meshs[i].Vector3ds[0].z;
				RHVector3 a2(0, 0, 0);
				a2.x = Meshs[i].Vector3ds[2].x - Meshs[i].Vector3ds[0].x;
				a2.y = Meshs[i].Vector3ds[2].y - Meshs[i].Vector3ds[0].y;
				a2.z = Meshs[i].Vector3ds[2].z - Meshs[i].Vector3ds[0].z;
				RHVector3 a3 = a1.CrossProduct(a2);
				a3.NormalizeSafe();
				normallist.push_back(a3);
				if (a3.x == 0 && a3.y == 0 && a3.z == -1)
				{
					double a = (Meshs[i].Vector3ds[0].z - 0.05) * 10 + 1;
					a = round(a);													//	a = Math.Round(a, MidpointRounding.AwayFromZero);
					double level = round(((a - 1) * 0.1 + 0.05) * 100.0) / 100.0;	//	double level = Math.Round((a - 1) * 0.1 + 0.05, 2, MidpointRounding.AwayFromZero);
					if (level > 0.05)
						horizontaltriangle.insert(pair<int, double>(i, level));
				}
			}
		}
		//public void CacaulateAngles()
		//{
		//    CaculateNormalVector();
		//    foreach (RHVector3 vector in normallist)
		//    {
		//        RHVector3 v0 = new RHVector3(vector.x, vector.y, 0);
		//        double angle = vector.Angle(v0);
		//        angle = angle * 180 / Math.PI;
		//        if (vector.z < 0)
		//            angle = -angle;
		//        angle = Math.Round(angle, 0, MidpointRounding.AwayFromZero);
		//        angles.Add((int)angle);
		//    }
		//}

		void ToFile(string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("Meshs size: %d\n", Meshs.size());

			for (int i = 0; i < Meshs.size(); i++)
			{			
				textFile.Write("[%i] ", i);
				for (int j = 0; j < Meshs[i].Vector3ds.size(); j++)
				{
					textFile.Write(" %f %f %f", Meshs[i].Vector3ds[j].x, Meshs[i].Vector3ds[j].y, Meshs[i].Vector3ds[j].z);
				}
				textFile.Write("\n");
			}
		}
	};

}
