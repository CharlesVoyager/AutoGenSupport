#pragma once

#include "MyTools.h"

using namespace MyTools;

namespace XYZSupport
{
	class SupportMeshsBedDbg
	{
	public:

		static string VectorIntToString(vector<int> obj)
		{
			string ret;
			for (const auto i : obj)
				ret += (to_string(i) + " ");

			return ret;
		}

		static string VectorRHVector3ToString(vector<RHVector3> obj)
		{
			string ret;
			for (const auto i : obj)
				ret += (i.ToString() + " ");

			return ret;
		}

		static void DumpObjectToFile(const vector<BedCellPtsInfo> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());

			textFile.Write("-----------------------------------------------------------------\n");
			for (int j = 0; j < object.size(); j++)
			{
				textFile.Write("name[%d]:\n", j);
				textFile.Write("cellpoints: %s\n", VectorRHVector3ToString(object[j].cellpoints).c_str());
				textFile.Write("row: %d\n", object[j].row);
				textFile.Write("column: %s\n",VectorIntToString(object[j].column).c_str()); 
				textFile.Write("intersects: %s\n\n", object[j].intersects?"True":"False");
			}
		}

		static void DumpObjectToFile(const vector<Cell> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());

			textFile.Write("-----------------------------------------------------------------\n");
			textFile.Write("_position           _row     _column    cubiodIntersect   isNull\n");
			textFile.Write("-----------------------------------------------------------------\n");
			for (int j = 0; j < object.size(); j++)
			{
				textFile.Write("[%d] %s   %d  %d  %s  %s\n", j, object[j]._position.ToString().c_str(), 
					object[j]._row, object[j]._column, 
					object[j].cubiodIntersect?"True":"False", 
					object[j].isNull ? "true" : "");
			}
		}

		static void DumpObjectToFile(const vector<TabDesign> &object, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", object.size());

			textFile.Write("-----------------------------------------------------------------\n");
			textFile.Write("candidate           index     density    serviceAreaSample   sideLocation\n");
			textFile.Write("-----------------------------------------------------------------\n");
			for (int j = 0; j < object.size(); j++)
			{
				textFile.Write("-----------------------------------------------------------------\n");
				textFile.Write(" #%d\n", j);
				textFile.Write("-----------------------------------------------------------------\n");
				textFile.Write("Cell candidate: [_position   _row    _column   cubiodIntersect     isNull]\n");
				textFile.Write("%s %d %d %s %s\n", object[j].candidate._position.ToString().c_str(), 
					object[j].candidate._row,
					object[j].candidate._column,
					object[j].candidate.cubiodIntersect?"True":"False",
					object[j].candidate.isNull ? "true" : "");

				textFile.Write("int index: %d\n", object[j].index);
				textFile.Write("vector<int> density:");

				for (int k = 0; k < object[j].density.size(); k++)
				{
					textFile.Write(" %d", object[j].density[k]);
				}
				textFile.Write("\n");
				textFile.Write("double angle: %f\n", object[j].angle);
				textFile.Write("int serviceAreaSample: %d\n", object[j].serviceAreaSample);
				textFile.Write("double sideLocation: %f\n", object[j].sideLocation);
				textFile.Write("RHVector3 endpoint: %s\n", object[j].endpoint.ToString().c_str());
			}
		}
		static void DumpObjectToFile(const CandidateScore &obj, const string name, const string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");

			textFile.Write("-----------------------------------------------------------------\n");
			textFile.Write(" candidate:\n");
			textFile.Write("-----------------------------------------------------------------\n");
			textFile.Write("Cell candidate: [_position   _row    _column   cubiodIntersect     isNull]\n");
			textFile.Write("%s %d %d %s %s\n", obj.candidate.candidate._position.ToString().c_str(),
				obj.candidate.candidate._row,
				obj.candidate.candidate._column,
				obj.candidate.candidate.cubiodIntersect ? "True" : "False",
				obj.candidate.candidate.isNull ? "true" : "");

			textFile.Write("int index: %d\n", obj.candidate.index);
			textFile.Write("vector<int> density:");

			for (int k = 0; k < obj.candidate.density.size(); k++)
			{
				textFile.Write(" %d", obj.candidate.density[k]);
			}
			textFile.Write("\n");
			textFile.Write("double angle: %f\n", obj.candidate.angle);
			textFile.Write("int serviceAreaSample: %d\n", obj.candidate.serviceAreaSample);
			textFile.Write("double sideLocation: %f\n", obj.candidate.sideLocation);
			textFile.Write("RHVector3 endpoint: %s\n", obj.candidate.endpoint.ToString().c_str());

			textFile.Write("-----------------------------------------------------------------\n");
			textFile.Write("candidate           index     density    serviceAreaSample   sideLocation\n");
			textFile.Write("-----------------------------------------------------------------\n");
			for (int j = 0; j < obj.assoc_candidates.size(); j++)
			{
				textFile.Write("-----------------------------------------------------------------\n");
				textFile.Write(" #%d\n", j);
				textFile.Write("-----------------------------------------------------------------\n");
				textFile.Write("Cell candidate: [_position   _row    _column   cubiodIntersect     isNull]\n");
				textFile.Write("%s %d %d %s %s\n", obj.assoc_candidates[j].candidate._position.ToString().c_str(),
					obj.assoc_candidates[j].candidate._row,
					obj.assoc_candidates[j].candidate._column,
					obj.assoc_candidates[j].candidate.cubiodIntersect ? "True" : "False",
					obj.assoc_candidates[j].candidate.isNull ? "true" : "");

				textFile.Write("int index: %d\n", obj.assoc_candidates[j].index);
				textFile.Write("vector<int> density:");

				for (int k = 0; k < obj.assoc_candidates[j].density.size(); k++)
				{
					textFile.Write(" %d", obj.assoc_candidates[j].density[k]);
				}
				textFile.Write("\n");
				textFile.Write("double angle: %f\n", obj.assoc_candidates[j].angle);
				textFile.Write("int serviceAreaSample: %d\n", obj.assoc_candidates[j].serviceAreaSample);
				textFile.Write("double sideLocation: %f\n", obj.assoc_candidates[j].sideLocation);
				textFile.Write("RHVector3 endpoint: %s\n", obj.assoc_candidates[j].endpoint.ToString().c_str());
			}
			textFile.Write("==============================================================================\n");
			textFile.Write("listPosition: %d\n", obj.listPosition);
			textFile.Write("scoreAreaSampling: %d\n", obj.scoreAreaSampling);
			textFile.Write("scoreAngleDifference: %d\n", obj.scoreAngleDifference);
			textFile.Write("totalScore: %d\n", obj.totalScore);
		}
	};
}