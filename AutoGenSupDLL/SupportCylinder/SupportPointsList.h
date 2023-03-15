#pragma once
#include "SupportMeshsBase.h"
#include "SupportData.h"
#include "SupportPoint.h"
#include "clipper.hpp"
#include "LayerModel.h"
#include "DataCombination.h"
#include "MeshModel.h"
#include "SegmentModel.h"

using namespace std;
using namespace ClipperLib;

namespace XYZSupport
{
	class HashSet_IntPoint
	{
	private:
		unordered_map<__int64, int> _hash;	// <hash value, index>
		vector<IntPoint> _v;

	public:
		void clear()
		{
			_v.clear();
			_hash.clear();
		}
		void Add(const IntPoint &pt)
		{	// Check if the new added point already exists or not.
			__int64 temp = pt.X * 5915587277 + pt.Y * 1500450271;
			std::unordered_map<__int64, int>::const_iterator got = _hash.find(temp);
			if (got == _hash.end())
			{
				_hash.insert({ temp, (int)_v.size() });
				_v.push_back(pt);
			}
		}
		size_t size() { return _v.size(); }
		IntPoint operator[](int index) const	// subscript operator v[0], v[1]
		{
			return _v[index];
		}
		IntPoint * begin()
		{
			return &_v.front();
		}
		IntPoint * end()
		{
			return &_v[size()];
		}
	};

	class SupportPointsList : public SupportMeshsBase
	{
	private:
		int currentLayer;	// just use for print debug message.
		PERCENTAGE_CALLBACK progressSupportPointAuto = Default_Percentage_Callback;

	public:
		void Generate2() {};

		vector<SupportPoint> GenerateAuto(const wstring density,				// "LOW", "MED", "HIGH"
											double contactSize,					// the radius of tocuh point. Defualt is 0.4mm.
											const vector<int> &thresholds,
											const vector<unsigned char> &vecModelMeshWor,
											const wstring wTempFolder,			// Temp Folder for stroing pictureCpp.stl
											PERCENTAGE_CALLBACK callback);

		void GenMesh(shared_ptr<SupportData> &supportData);		// C#: GenMarkMesh

	private:
		vector<SupportPoint> allGeneratedSupportPoint;
		LayerModel layerModel;
		MeshModel meshModel;
		SegmentModel segmentModel;
		vector<int> thresholdList;
		vector<vector<IntPoint>> lowerSolid;
		vector<vector<vector<IntPoint>>> lowerHole;
		vector<vector<vector<IntPoint>>> modifiedLowerHole;
		vector<vector<IntPoint>> upperSolid;
		vector<vector<vector<IntPoint>>> upperHole;
		vector<vector<vector<IntPoint>>> modifiedUpperHole;
		vector<IntPoint> pastLevelSupportPoints;
		vector<vector<IntPoint>> pastLevelSupportPointsArea;
		HashSet_IntPoint nowLevelSupportPoint;				// C#: HashSet<IntPoint> nowLevelSupportPoint;	// NOTE: Don't use "orodered_set" because the order of elments in the unordered_set is Not the same as the order of insert.
		vector<vector<IntPoint>> nowLevelSupportPointsArea;		// C#: HashSet<List<IntPoint>> nowLevelSupportPointsArea;
		vector<vector<Vector3>> supportPointToPolygonTable;		// C#: List<HashSet<Vector3>> supportPointToPolygonTable;
		vector<vector<Vector3>> nowSupportPointToPolygonTable;	// C#: List<HashSet<Vector3>> nowSupportPointToPolygonTable;
		vector<vector<int>> upperLowerPolygonTable;
		vector<bool> downAreaThreshold;
		vector<bool> upperAreaThreshold;

		const int needGenerateSupportAreaThreshold = 10000000;//0.1 mm*mm
		const double needGenerateSupportByAverageThreshold = 0.3;//if one / all area too big, generate support
		const double layerHeight = 0.1;
		const double controlDensityValueAreaThreshold = 546000000;

	private:
		bool ReducedTriangles(const unsigned char *stlBufferWor, const wstring wstrPathForXYZMeshDll, const wstring pathForPictureStl, PolygonPoint &xyMinLaction);
		float* GetWorldVerticesXYZ(const unsigned char *stlBufferWor);
		void WorldVerticesXYZToStlBinary(float *vet_reduced, int facet_num_reduced, const wstring pathForPictureStl, PolygonPoint &xyMinLaction);

		bool GetSliceInfo(const wstring wstrStlFilePath, const wstring wstrPathForSlicerDll, PolygonPoint boundaryBox);

		void FindNecessarySupportPointsNew(double size, /*out*/ vector<Triple<double, RHVector3, RHVector3>> &candSupPosList);
		int GetFirstLayer();
		void FirstLayerGenerateSupport(int Firstlevel, double height, double size);

		void GetNextLayerInfo(int level);

		double CalculateAreaValue(int count);
		double CalculateAreaValue(vector<IntPoint> &testpoly, vector<vector<IntPoint>> &unSafeAreaHole);
		vector<vector<IntPoint>> GetPolygons(const vector<int> &UpDownTable);
		vector<vector<IntPoint>> GetPolygons(const vector<IntPoint> &testPoly, const vector<vector<IntPoint>> &unSafeAreaHole);

		vector<IntPoint> GetCandidateSupportPoint(vector<IntPoint> &testpoly);
		vector<double> CalculateEveryPointtoSupportMinDistance(const vector<IntPoint> &polygon, const vector<int> &number, bool isDensityTooHigh);
		RHVector3 GenerateNeedSupportAreaSupportPoint(IntPoint point, bool isDensityTooHigh, int level, int count, double size, double height);
		vector<IntPoint> FindCompletelyHorizontalFace(const vector<vector<IntPoint>> &solid, const vector<vector<vector<IntPoint>>> &hole, double h, double size);
		void AddInternalAverage(const vector<IntPoint> &polygon,
								const vector<vector<IntPoint>> &hole, 
								double h,
								vector<RHVector3> &supportpoints,
								vector<IntPoint> &pointslocation, double size);

		void AddNoHoleInternalAverage(const vector<IntPoint> &polygon, double h, vector<RHVector3> &supportpoints, vector<IntPoint> &pointslocation, double size);

		bool GetTriangleId(IntPoint point, double height, /*out*/ int &meshid);
		void GetRemainingNeedSupportArea(IntPoint point, RHVector3 normalVector, int count, const vector<IntPoint> &needSupportPolygon, bool isDensityTooHigh,
			/*out*/ queue<vector<IntPoint>> &box, /*out*/ vector<int> &controlNumber, /*out*/ vector<vector<IntPoint>> &needGenerateSupportAreaHole);

		void GenerateCompletelyHorizontalFaceSupportPoint(int count, double height, double size);
		void GenerateCompletelyHorizontalFaceSupportPoint(vector<IntPoint> &solid, vector<vector<IntPoint>> &hole, int count, double height, double size);

		vector<vector<vector<IntPoint>>> PairSolidandHole(vector<vector<IntPoint>> &solid, vector<vector<vector<IntPoint>>> &uphole);

		bool GetDensitySettingValue(int count, double value);
		double CaculateAngle(IntPoint pt1, IntPoint pt2, IntPoint pt3);
		void CheckPoint(const vector<IntPoint> polygon, const vector<IntPoint> &pointslocation, double h, vector<IntPoint> &supportPoints, double size);
		void MatchPointstoPolygon(double height);
		void MatchPointsToPolygon(const vector<vector<int>> &updowntable);
		vector<IntPoint> PointToSupportArea(IntPoint point, RHVector3 vector, bool isDensityTooHigh);
		vector<vector<int>> GetUpperAndLowerPolygonPairList();

		void AddWithCheckContour(vector<IntPoint> &polygon, double h, vector<RHVector3> &supportpoints, vector<IntPoint> &location);
		void AddWithTestDistance(RHVector3 pt, vector<IntPoint> &location, vector<RHVector3> &supportpoints, double h);

		void GetNeedGenerateArea(int count, /*out*/ vector<int> &controlNumber, /*out*/ vector<vector<vector<IntPoint>>> &needGenerateSupportArea);
		vector<vector<IntPoint>> RemoveCrushArea(const vector<vector<IntPoint>> &intput);
		void SortPolygons(vector<vector<IntPoint>> &allpolygons);
		vector<int> GetPointInUpperSolid(int count);
		void UpdateNowSupportPointTable(int count, const vector<int> &number);
		vector<vector<IntPoint>> GetSupportArea(const vector<int> &number);

		void CheckHole(vector<IntPoint> &subjectpoly, vector<vector<IntPoint>> &allholepolygon);
		void GetThisLevelAllPolygons(vector<vector<IntPoint>> &solid, vector<vector<vector<IntPoint>>> &holelist, int level);
		void MergeOverlappingPolygon(vector<vector<IntPoint>> &input, vector<vector<vector<IntPoint>>> &inputhole);
		//1
		vector<vector<vector<IntPoint>>> ClipperGeometricOperationPolyTreeType(const vector<IntPoint> &subject,
			const vector<IntPoint> &clip, ClipType clipType, PolyFillType subtype, PolyFillType cliptype);
		//2
		vector<vector<IntPoint>> ClipperGeometricOperationNormalType(const vector<vector<IntPoint>> &subject,
			const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype);
		//3
		vector<vector<IntPoint>> ClipperGeometricOperationNormalType(const vector<IntPoint> &subject,
			const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype);
		//4
		vector<vector<vector<IntPoint>>> ClipperGeometricOperationPolyTreeType(const vector<vector<IntPoint>> &subject,
			const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype);
		//5
		vector<vector<vector<IntPoint>>> ClipperGeometricOperationPolyTreeType(const vector<IntPoint> &subject,
			const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype);

		vector<vector<IntPoint>> ClipperOffest(const vector<IntPoint> &intput, int value);

		vector<int> GetTag(vector<vector<IntPoint>> &allpolygons, vector<vector<IntPoint>> &sortpolygons);

		int FindMaxDistance(const vector<double> &distancelist);
		int FindMeshId(IntPoint point, int level);
		int GetId(Vector4d segment, map<Vector4d, int> &dictionary);
		RHVector3 FindNewLocation(IntPoint supportpoint, double height, int meshID, double size);

		vector<IntPoint> AdjustSupportPoint(int maxnumber, const vector<IntPoint> &uppoly, const vector<IntPoint> &originalpoly, double size);
		vector<RHVector3> CheckNormalvector(vector<IntPoint> adjustpointlist, int maxnumber, const vector<IntPoint> &uppoly, const vector<vector<IntPoint>> &downpoly);
		RHVector3 FindNormalvector(IntPoint point1, IntPoint point2, const vector<vector<IntPoint>> &downpoly);
		RHVector3 AddSupportPoint(RHVector3 point, bool isDensityTooHigh, int meshId, double size, int count);
	};
}