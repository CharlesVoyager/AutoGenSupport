#pragma once
#include "stdafx.h"
#include <unordered_map>
#include <utility>      // std::pair
#include "DataCombination.h"
#include "RHVector3.h"
#include "SupportData.h"
#include "CubeMatrix.h"
#include "SupportPoint.h"
#include "TopoVertex.h"
#include "TopoTriangle.h"
#include "SupportMeshsCylinder.h"
#include "RayCollisionDetect.h"
#include "GenBasic.h"
#include "SupportSymbolCone.h"

#define PointF Vector2

namespace XYZSupport
{
	//Styles structure
	struct Style1
	{
		static constexpr float Width = 3.47f;
		static constexpr float Depth = 4.0f;
		static constexpr float Offset_H = 0.7f;
		static constexpr float Offset_V = 1.6f;
		static constexpr float Radius_I = Width / 2.0f;
		static constexpr float Radius_O = Depth / 2.0f;
	};
	struct Style2
	{
		static constexpr float Width = 3.47f;
		static constexpr float Depth = 4.0f;
		static constexpr float Offset_H = 0.87f;
		static constexpr float Offset_V = 1.74f;
		static constexpr float Radius_I = Width / 2.0f;
		static constexpr float Radius_O = Depth / 2.0f;
	};
	struct Style3
	{
		static constexpr float Width = 3.47f;
		static constexpr float Depth = 4.0f;
		static constexpr float Offset_H = 0.49f;
		static constexpr float Offset_V = 1.4f;
		static constexpr float Radius_I = Width / 2.0f;
		static constexpr float Radius_O = Depth / 2.0f;
	};
	struct Style4
	{
		static constexpr float Width = 3.47f;
		static constexpr float Depth = 4.0f;
		static constexpr float Offset_H = 0.49f;
		static constexpr float Offset_V = 1.4f;
		static constexpr float Radius_I = Width / 2.0f;
		static constexpr float Radius_O = Depth / 2.0f;
	};

	enum class BaseCellStyle
	{
		STYLE1 = 1,
		STYLE2 = 2,
		STYLE3 = 3,
		STYLE4 = 4
	};
	
	enum CuboidDensity
	{
		Low = 1, // Cuboid Connection Density with only Single-Connection to other Cuboids
		High = 2 // Cuboid Connection Density with only Two-Connection to other Cuboids
	};

	enum BedCellCount
	{
		GenerateNoTab = 5,
		GenerateOneTab = 35,
		GenerateTwoTab = 70,
		GenerateTriTab = 150,
		GenrateFourTab = 151
	};

	enum EasyTabID
	{
		TabID1 = 1,
		TabID2 = 2,
		TabID3 = 3,
		TabID4 = 4
	};

	enum class TabGenCount
	{
		OneTab = 1,
		TwoTab = 2,
		ThreeTab = 3,
		FourTab = 4
	};

	enum Location
	{
		Top = 1,
		Bottom = 2,
		Left = 3,
		Right = 4
	};

	enum sequencetracker
	{
		off = 0,
		started = 1,
		ended = 2
	};

	struct Cuboid
	{
	public:
		RHVector3 _Start;
		RHVector3 _End;
		RHVector3 _MidPoint;
		bool _IsVirtual;

	public:
		Cuboid(const RHVector3 &start, const RHVector3 &end, const RHVector3 &midpoint, bool isVirtual = false)
		{
			_Start = start;
			_End = end;
			_MidPoint = midpoint;
			_IsVirtual = isVirtual;
		}
		bool operator== (const Cuboid& other) const
		{
			if (_Start == other._Start &&
				_End == other._End &&
				_IsVirtual == other._IsVirtual)
				return true;
			else
				return false;
		}

		string ToString() const
		{
			return "_Start: " + _Start.ToString() + " _End: " + _End.ToString() + " _MidPoint: " + _MidPoint.ToString() + " _IsVirtual: " + (_IsVirtual ? "true" : "false");
		}
	};

	class Cell
	{
	public:
		RHVector3 _position;
		int _row;
		int _column;
		bool cubiodIntersect;

		bool isNull = true;
	public:
		Cell() {}
		Cell(RHVector3 position,/* Polygon polygon,*/ int row, int col)
		{
			_position = position;
			_row = row;
			_column = col;

			isNull = false;
		}
		bool operator==(const Cell &obj) const
		{
			if (isNull == true)
			{
				if (obj.isNull == true)
					return true;
				else
					return false;
			}
			else
			{
				if (obj.isNull == true)
					return false;
				else
				{
					if (_position == obj._position &&
						_row == obj._row &&
						_column == obj._column &&
						cubiodIntersect == obj.cubiodIntersect)
						return true;
					else
						return false;
				}
			}
		}
		string ToString() const
		{
			return "_position: " + _position.ToString() + ", _row: " + to_string(_row) + ", _column: " + to_string(_column) + ", cubiodIntersect: " + (cubiodIntersect ? "true" : "false");
		}
	};

	struct BedCellPtsInfo
	{
	public:
		vector<RHVector3> cellpoints;
		int row;
		vector<int> column;
		bool intersects;

		BedCellPtsInfo()
		{
			intersects = false;
		}
	};

	struct TabInformation
	{
	public:
		int TabID;
		int tabLists_SelectedIndex;
		//public List<TabDesign> TabResultList;
		bool filteredInfo;

		void Reset()
		{
			TabID = 0;
			//TabResultList = null;
			filteredInfo = false;
			tabLists_SelectedIndex = 0;
		}
		TabInformation() {}
	};

	struct TabPosResources
	{
	public:
		Cell tabPosition;
		vector<int> density;
		double sidelocation;
		int serviceAreaSampling;
		Tuple<double, RHVector3> rotationInfo;

		void Reset()
		{
			tabPosition.isNull = true;
			density.clear();
		}
		TabPosResources() {}
		// Charles added.
		bool operator==(const TabPosResources & obj) const
		{
			if (tabPosition == obj.tabPosition &&
				density == obj.density &&
				sidelocation == obj.sidelocation &&
				serviceAreaSampling == obj.serviceAreaSampling)
				return true;
			else
				return false;
		}
		// End of added.
	};

	struct TabSupport
	{
	public:
		TabInformation tab1;
		TabInformation tab2;
		TabInformation tab3;
		TabInformation tab4;
		vector<TabPosResources> tabLists;

	public:
		void Reset()
		{
			tab1.Reset();
			tab2.Reset();
			tab3.Reset();
			tab4.Reset();
			tabLists.clear();
		}
		TabSupport() {}
	};

	enum class Intersection
	{
		None,
		Tangent,
		Intersection,
		Containment
	};

	struct Line
	{
	public:
		static Line Empty;

	private:
		PointF p1;
		PointF p2;

	public:
		Line() {}
		Line(const PointF &value1, const PointF &value2)
		{
			p1 = value1;
			p2 = value2;
		}

		PointF GetP1() const { return p1; }
		void SetP1(PointF value) { p1 = value; }

		PointF GetP2() const { return p2; }
		void SetP2(PointF value) { p2 = value; }

		float GetX1() const { return p1.x; }
		float& X1() { return p1.x; }

		float GetX2() const { return p2.x; }
		float& X2() { return p2.x; }

		float GetY1() const { return p1.y; }
		float& Y1() { return p1.y; }

		float GetY2() const { return p2.y; }
		float& Y2() { return p2.y; }

		double Distance() const {
			return abs(sqrt(pow(GetX2() - GetX1(), 2) + pow(GetY2() - GetY1(), 2)));
		}

		RHVector3 MidPoint() const {
			return RHVector3((double)((GetX1() + GetX2()) / 2.0), (double)((GetY1() + GetY2()) / 2.0), (double)0.0);
		}

		string ToString() const
		{
			return "(" + to_string(p1.x) + "," + to_string(p1.y) + ") - (" + to_string(p2.x) + "," + to_string(p2.y) + ")";
		}
	};

	struct Polygon
	{
	private:
		vector<PointF> points;

	public:
		Polygon() {}
		Polygon(const vector<PointF> &value)
		{
			this->points = value;
		}

		vector<PointF>& GetPoints() { return points; }
		void SetPoints(const vector<PointF> &value) { points = value; }

		int Length() { return (int)points.size(); }

		PointF GetValue(int index) { return points[index]; }
		PointF& SetValue(int index) { return points[index]; }
		PointF& operator[](int index) { return points[index]; }

		void ToFile(string name, string filename)
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("===================================\n");
			textFile.Write("  Dump %s\n", name.c_str());
			textFile.Write("===================================\n");
			textFile.Write("Count: %d\n", points.size());
			for (int j = 0; j < points.size(); j++)
			{
				textFile.Write("%f,%f\n", points[j].x, points[j].y);
			}
		}
	};

	struct TabDesign
	{
	public:
		Cell candidate;
		int index;
		vector<int> density;
		double angle;
		int serviceAreaSample;
		double sideLocation;
		RHVector3 endpoint;

		TabDesign() { Reset(); }
		void Reset()
		{
			candidate = Cell();
			index = 0;
			density.clear();
			angle = 0.0;
			endpoint = RHVector3();
		}

		/************ Charles added **************/
		bool operator==(const TabDesign& other) const
		{

			if (index == other.index &&
				candidate == other.candidate)
				return true;
			else
				return false;
		}

		//bool operator<=(const TabDesign& other) const
		//{
		//	if (index <= other.index)
		//		return true;
		//	else
		//		return false;
		//}
		bool operator<(const TabDesign& other) const
		{
			if (index < other.index)
				return true;
			else
				return false;
		}
		/****************************************/
	};

	class Geometry
	{
	public:
		static Intersection IntersectionOf(Line line, Polygon polygon)// In C#, Line is struct. It is by value.
		{
			if (polygon.Length() == 0)
			{
				return Intersection::None;
			}
			if (polygon.Length() == 1)
			{
				return IntersectionOf(polygon.GetValue(0), line);	// PointF, Line
			}
			bool tangent = false;
			for (int index = 0; index < polygon.Length(); index++)
			{
				int index2 = (index + 1) % polygon.Length();
				Line line2(polygon[index], polygon[index2]);
				Intersection intersection = IntersectionOf(line, line2);
				if (intersection == Intersection::Intersection)
				{
					return intersection;
				}
				if (intersection == Intersection::Tangent)
				{
					tangent = true;
				}
			}
			return tangent ? Intersection::Tangent : IntersectionOf(line.GetP1(), polygon);
		}

		static Intersection IntersectionOf(PointF point, Polygon polygon)
		{
			switch (polygon.Length())
			{
			case 0:
				return Intersection::None;
			case 1:
				if (polygon[0].x == point.x && polygon[0].y == point.y)
				{
					return Intersection::Tangent;
				}
				else
				{
					return Intersection::None;
				}
			case 2:
				return IntersectionOf(point, Line(polygon[0], polygon[1]));
			}

			int counter = 0;
			int i;
			PointF p1;
			int n = polygon.Length();
			p1 = polygon[0];
			if (point == p1)
			{
				return Intersection::Tangent;
			}

			for (i = 1; i <= n; i++)
			{
				PointF p2 = polygon[i % n];
				if (point == p2)
				{
					return Intersection::Tangent;
				}
				if (point.y > min(p1.y, p2.y))
				{
					if (point.y <= max(p1.y, p2.y))
					{
						if (point.x <= max(p1.x, p2.x))
						{
							if (p1.y != p2.y)
							{
								double xinters = (point.y - p1.y) * (p2.x - p1.x) / (p2.y - p1.y) + p1.x;
								if (p1.x == p2.x || point.x <= xinters)
									counter++;
							}
						}
					}
				}
				p1 = p2;
			}
			return (counter % 2 == 1) ? Intersection::Containment : Intersection::None;
		}

		static Intersection IntersectionOf(PointF point, Line line)
		{
			float bottomY = min(line.GetY1(), line.GetY2());
			float topY = max(line.GetY1(), line.GetY2());
			bool heightIsRight = point.y >= bottomY &&
									point.y <= topY;
			//Vertical line, slope is divideByZero error!
			if (line.GetX1() == line.GetX2())
			{
				if (point.x == line.GetX1() && heightIsRight)
				{
					return Intersection::Tangent;
				}
				else
				{
					return Intersection::None;
				}
			}
			float slope = (line.GetX2() - line.GetX1()) / (line.GetY2() - line.GetY1());
			bool onLine = (line.GetY1() - point.y) == (slope * (line.GetX1() - point.x));
			if (onLine && heightIsRight)
			{
				return Intersection::Tangent;
			}
			else
			{
				return Intersection::None;
			}
		}
		// In C#, Line is struct. It is by value.
		// NOTE: Although "line1" and "line2" value will be updated in the function, don't use "by reference" to pass paramters - line1 and line2.
		static Intersection IntersectionOf(Line line1, Line line2)	
		{
			//  Fail if either line segment is zero-length.
			if ( (line1.GetX1() == line1.GetX2() && line1.GetY1() == line1.GetY2()) || (line2.GetX1() == line2.GetX2() && line2.GetY1() == line2.GetY2()) )
				return Intersection::None;

			if ( (line1.GetX1() == line2.GetX1() && line1.GetY1() == line2.GetY1()) || (line1.GetX2() == line2.GetX1() && line1.GetY2() == line2.GetY1()) )
				return Intersection::Intersection;
			if ( (line1.GetX1() == line2.GetX2() && line1.GetY1() == line2.GetY2()) || (line1.GetX2() == line2.GetX2() && line1.GetY2() == line2.GetY2()) )
				return Intersection::Intersection;

			//  (1) Translate the system so that point A is on the origin.
			line1.X2() -= line1.GetX1(); line1.Y2() -= line1.GetY1();
			line2.X1() -= line1.GetX1(); line2.Y1() -= line1.GetY1();
			line2.X2() -= line1.GetX1(); line2.Y2() -= line1.GetY1();

			//  Discover the length of segment A-B.
			double distAB = sqrt(line1.GetX2() * line1.GetX2() + line1.GetY2() * line1.GetY2());

			//  (2) Rotate the system so that point B is on the positive X axis.
			double theCos = line1.GetX2() / distAB;
			double theSin = line1.GetY2() / distAB;
			double newX = line2.GetX1() * theCos + line2.Y1() * theSin;
			line2.Y1() = line2.GetY1() * (float)theCos - line2.GetX1() * (float)theSin; line2.X1() = (float)newX;
			newX = line2.GetX2() * theCos + line2.GetY2() * theSin;
			line2.Y2() = line2.GetY2() * (float)theCos - line2.GetX2() * (float)theSin; line2.X2() = (float)newX;

			//  Fail if segment C-D doesn't cross line A-B.
			if ( (line2.GetY1() < 0 && line2.GetY2() < 0) || (line2.GetY1() >= 0 && line2.GetY2() >= 0) )
				return Intersection::None;

			//  (3) Discover the position of the intersection point along line A-B.
			double posAB = line2.GetX2() + (line2.GetX1() - line2.GetX2()) * line2.GetY2() / (line2.GetY2() - line2.GetY1());

			//  Fail if segment C-D crosses line A-B outside of segment A-B.
			if (posAB < 0 || posAB > distAB)
				return Intersection::None;

			//  (4) Apply the discovered position to line A-B in the original coordinate system.
			return Intersection::Intersection;
		}

		static Intersection IntersectionOf(Polygon polygon1, Polygon polygon2)
		{
			Intersection result = Intersection::None;

			for (int i = 0; i < polygon1.Length(); i++)
			{
				int index1 = i;
				int index2 = i + 1 == polygon1.Length() ? 0 : i + 1;
				Line line1(polygon1[index1], polygon1[index2]);
				result = IntersectionOf(line1, polygon2);

				if (result != Intersection::None)
				{
					return result;
				}
			}

			for (int i = 0; i < polygon2.Length(); i++)
			{
				int index1 = i;
				int index2 = i + 1 == polygon2.Length() ? 0 : i + 1;
				Line line1(polygon2[index1], polygon2[index2]);
				result = IntersectionOf(line1, polygon1);

				if (result != Intersection::None)
				{
					return result;
				}
			}

			return result;
		}

		static Intersection IntersectionOf(PointF point, float radius, Line line)
		{
			float dx, dy, A, B, C, det;

			dx = line.GetP2().x - line.GetP1().x;
			dy = line.GetP2().y - line.GetP1().y;

			A = dx * dx + dy * dy;
			B = 2 * (dx * (line.GetP1().x - point.x) + dy * (line.GetP1().y - point.y));
			C = (line.GetP1().x - point.x) * (line.GetP1().x - point.x) + (line.GetP1().y - point.y) * (line.GetP1().y - point.y) - radius * radius;

			det = B * B - 4 * A * C;

			if ((A <= 0.0000001) || (det < 0))
			{
				return Intersection::None;
			}
			else
			{
				return Intersection::Intersection;
			}
		}

		static Line GetPerpendicularScraperLine(RHVector3 v1, RHVector3 v2)
		{
			//KPPH CP2 SW RCGREY 20180903
			//Determine the line perpendicular to the position of the tab design
			//this line mimics the path of the scraper tool that will first touch upon contact
			double r1 = 38.772922; //distance from cell center to edge of scraper
			double r2 = 37.5; // distance from tab end point to edge of scraper
			Line scraperLine(PointF(-999.0, -999.0), PointF(-999.0, -999.0));

			double d = v1.Distance(v2);

			if ((d > (r1 + r2)) || (d == 0 && r1 == r2) || (d + min(r1, r2) < max(r1, r2)))
				return scraperLine;
			else
			{
				double a = (r1 * r1 - r2 * r2 + d * d) / (2.0 * d);
				double h = sqrt(r1 * r1 - a * a);

				RHVector3 p2((v1.x + (a * (v2.x - v1.x)) / d), (v1.y + a * (v2.y - v1.y) / d), 0.0);

				RHVector3 i1((p2.x + (h * (v2.y - v1.y)) / d), (p2.y - (h * (v2.x - v1.x)) / d), 0.0);
				RHVector3 i2((p2.x - (h * (v2.y - v1.y)) / d), (p2.y + (h * (v2.x - v1.x)) / d), 0.0);

				if (d == (r1 + r2))
					scraperLine = Line(PointF((float)i1.x, (float)i1.y), PointF());
				else
					scraperLine = Line(PointF((float)i1.x, (float)i1.y), PointF((float)i2.x, (float)i2.y));
			}
			return scraperLine;
		}
	};
	
	struct CandidateScore
	{
	public:
		TabDesign candidate;					// tab design candidate in focus
		vector<TabDesign> assoc_candidates;     // list of associated candidates to complete number of tabs to be generated
		int listPosition;						// list position of candidate based on number of area sampling
		int scoreAreaSampling;					// score of candidate based on number of sampled area gathered
		int scoreAngleDifference;				// score of candidate (and its associates) based on angle of differences between its associates 
		int totalScore;							// total score for each assessed candidate
	};

	class SupportMeshsBed : public SupportMeshsBase
	{
		SupportReferenceMesh ReferenceMesh;	// BIG CLASS!!! 放在這裡. SupportMeshsCylinder class建立時後, 此物件建立一次. 不要放在function裡當local object, 造成每次使用此SupportReferenceMesh物件, 都要再一次生成!!!

	private:
		bool debuggingMode = false;
		bool enableEasyReleaseTab = true;

		vector<Cuboid> xCuboids;
		vector<Cell> xBedCells;
		vector<BedCellPtsInfo> xBedCellPoints;
		vector<PointF> xSupportFootPoints;
		vector<vector<PointF>> xClusteringOfSupPoints;
		vector<PointF> clusteredPoints;

		TabSupport vTabs_Data;

		TopoModel baseCellxStyleModel;

		SupportData	vNewBase_Model;	// C#: SupportModel vNewBase_Model;

		int baseCellRows = 0;
		int baseCellCols = 0;
		double bboxOffset_X = 0;
		double bboxOffset_Y = 0;

		double baseCells_StartX = 0;
		double baseCells_StartY = 0;
		double baseCells_StartZ = 0;

		int clusteringThreshold = 0;

		int prog_value = 0;

	public:
		BaseCellStyle baseCellxStyle = BaseCellStyle::STYLE1; // Default Cell Style

		CuboidDensity cuboidsDensity = CuboidDensity::Low; // Default Cuboid Connection Density

	private:
		TabGenCount vNumOfTabDesign = TabGenCount::OneTab;

		float baseCellStyle_Width, baseCellStyle_Depth, baseCellStyle_Offset_H, baseCellStyle_Offset_V, baseCellStyle_Radius_I, baseCellStyle_Radius_O;

		//private Location TabFocus = Location.Top;

		//Ervin 04-12-19 || For Porting use
	private:
		vector<shared_ptr<SupportData>> xSupDataRefrnce;

	protected:
		RHBoundingBox bbox;

		//Ervin 05-22-19
	public:
		float newBaseZScale = 1.2f; //3mm

		//#region for[BETA] Test Version ONLY Raft Style Setup
		bool enableHexagonBase = true;
		bool new_model = true;
		//#endregion

	private:
		unordered_map<double, double> TabAnglePosition;

	private:
		RHBoundingBox& BoundingBoxWOSupport() { return bbox; }

	public:
		SupportMeshsBed(float _NewBaseZScale, bool _EnableEasyReleaseTab = true)
		{
			editMode = PointEditModeCode::Bed;

			newBaseZScale = _NewBaseZScale;
			enableEasyReleaseTab = _EnableEasyReleaseTab;

			TabAnglePosition.insert(
				{	{1.0, 30.0}, {1.5, 60.0}, {2.0, 90.0}, {2.5, 120.0}, {3.0, 150.0},
					{3.5, 180.0}, {4.0, 210.0}, {4.5, 240.0}, {5.0, 270.0}, {5.5, 300.0},
					{6.0, 330.0}, {6.5, 0.0} }
			);
		}
		void Generate2() {}	//implement the virtual function from SuportMeshsBase
		void GenMesh(shared_ptr<SupportData> &pSupportData);
		void Generate2(const vector<unique_ptr<SupportSymbol>> &modelsList, const CubeMatrix &cubeInfo);
		void GenerateBed(const vector<shared_ptr<SupportData>> &models, const RHBoundingBox &_bbox);

	private:
		void includePoint(RHVector3 v);
		void ExtraBboxforGenSupport(double ExtraSpace, const RHBoundingBox &BoundingBoxWOSupport);

		void genSupFromList();
		void initStyleValues();
		void calcTotalCells();
		void genBedCellPoints();
		void execCuboidSpanTree();
		void checkIntersections();
		void SpaceCheck(Cell sample, vector<Cell> listwithIntersects);
		void CoverGaps();
		void genCuboids();
		void genBedCells();
		void CollectCellsWithTabOpenings();
		void CreateEasyReleaseTabs();
		void setSupIntoClusters();
		void DetermineAssociateTabPositions(/*out*/ CandidateScore &input, vector<TabDesign> &candidateList);
		void FinalizeTabDesign(const vector<TabDesign> &candidateList);
		int getMeanDistribBySampleArea(/*out*/ vector<TabDesign> &tD);
		//double CalculateStdDev(IEnumerable<int> values);
		void HighlightPrint(Cell CellTabCandidate, TabPosResources angleref);
		void genTabModelData(Cell cellLocation, double angle, EasyTabID ID);
		vector<int> CheckCellsAdjacentoEachSides(const Cell &cellPosition);
		TabPosResources GetTabRotateValue(vector<int> &directions, Cell &testSample, bool limitfocus = true);
		Triple<double, int, double> GetServiceAreaSampling(double selectedPosition, Cell subjectCell);
		vector<Tuple<double, RHVector3>> ValidateSuppliedRotationValue(Cell cellposition, const vector<double> &angleList);
		bool IsTabSurrounded(Cell cellOrigin, /*out*/RHVector3 &endpoint);
		bool IsScraperLineBlocked(Cell cellposition, RHVector3 endpoint);
		bool IsInAGutter(const Cell &cellposition, double angle);
		bool lineIntersect(const vector<Line> &lines, Line line);
	};
}
