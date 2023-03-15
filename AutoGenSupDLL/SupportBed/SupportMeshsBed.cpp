#include "stdafx.h"
#include <unordered_map>
#include <set>
#include <utility>      // std::pair
#include <algorithm>    // std::sort
#include <math.h>
#include "boolinq.h"
#include "DataCombination.h"
#include "RHVector3.h"
#include "SupportData.h"
#include "CubeMatrix.h"
#include "SupportPoint.h"
#include "TopoVertex.h"
#include "TopoTriangle.h"
#include "SupportMeshsBed.h"
#include "RayCollisionDetect.h"
#include "GenBasic.h"
#include "SupportMeshsBedDbg.h"

using namespace boolinq;

namespace XYZSupport
{
	//Ervin 04-12-2019 Ported from other support classes
	void SupportMeshsBed::Generate2(const vector<unique_ptr<SupportSymbol>> &modelsList, const CubeMatrix &cubeInfo)
	{
		bbox.Clear();
		for(const auto &v : Cubedata2.Model.vertices._v)
		{
			includePoint(v.pos);
		}
		Cubedata2.Model.boundingBox = bbox;
		int CubeSpan = 2;
		ExtraBboxforGenSupport(CubeSpan, BoundingBoxWOSupport());
		//genSupFromList();
	}

	// joshua 05-07-2019
	void SupportMeshsBed::GenerateBed(const vector<shared_ptr<SupportData>> &models, const RHBoundingBox &_bbox)
	{
		DbgMsg(__FUNCTION__ "[IN]");
		DbgMsg(__FUNCTION__"===> newBaseZScale: %f", newBaseZScale);
		DbgMsg(__FUNCTION__"===> enableEasyReleaseTab: %s", enableEasyReleaseTab?"true":"false");

		Stopwatch sw;
		sw.Start();
		xSupDataRefrnce = models;

		bbox.Clear();

#if 0	// C# AP: Calculate bounding box according cube data.
		for(const auto &v : Cubedata2.Model.vertices._v)
		{
			includePoint(v.pos);
		}
		Cubedata2.Model.boundingBox = bbox;
		int CubeSpan = 2;
		ExtraBboxforGenSupport(CubeSpan, BoundingBoxWOSupport());
#else
		bbox = _bbox;
#endif
		genSupFromList();

		sw.Stop();
		DbgMsg(__FUNCTION__"===> Total SupportData (_items2.size()): %d", _items2.size());
		DbgMsg(__FUNCTION__"===> GenerateBed() time: %ld (ms)", sw.ElapsedMilliseconds());
		DbgMsg(__FUNCTION__ "[OUT]");
	}

	void SupportMeshsBed::includePoint(RHVector3 v)
	{
		float x, y, z;
		Vector4 *pv4 = v.asVector4();

		x = Cubedata2.Trans.Column0().dot(*pv4);
		y = Cubedata2.Trans.Column1().dot(*pv4);
		z = Cubedata2.Trans.Column2().dot(*pv4);
		bbox.Add(RHVector3(x, y, z));
		delete pv4;
	}

	void SupportMeshsBed::ExtraBboxforGenSupport(double ExtraSpace, const RHBoundingBox &BoundingBoxWOSupport)
	{
		bbox.minPoint.x = BoundingBoxWOSupport.xMin() - ExtraSpace;
		bbox.maxPoint.x = BoundingBoxWOSupport.xMax() + ExtraSpace;
		bbox.minPoint.y = BoundingBoxWOSupport.yMin() - ExtraSpace;
		bbox.maxPoint.y = BoundingBoxWOSupport.yMax() + ExtraSpace;
	}
	//#endregion

	void SupportMeshsBed::genSupFromList()
	{
		Status = STATUS::Busy;

#if 0	// The setting is related to hardware type.
		PrinterType machineType = PrinterSettings.Instance.ValidPrinterType;
		if ((machineType == PrinterType.NSF)
		|| (machineType == PrinterType.NCASTPRO100XP)
		|| (machineType == PrinterType.NPARTPRO120XP)
		|| (machineType == PrinterType.NDENTPRO100XP)
		|| (machineType == PrinterType.NPARTPRO100XP))
		{
			newBaseZScale = 0.6f;//ori=2.5*1.2=3mm, NSF setting is 2.5*0.6=1.5mm
		}
#endif
		cuboidsDensity = CuboidDensity::High;	// NOTE: AP sets this value as CuboidDensity::High.

		xSupportFootPoints.clear();

		xSupportFootPoints = from(xSupDataRefrnce).where([](const shared_ptr<SupportData> &c) {return c->supType == (int)ModelType::CUBOID_FOOT; }).select([](const shared_ptr<SupportData> &a) { return PointF(a->GetPosition().x, a->GetPosition().y); }).toStdVector();

		xSupportFootPoints = from(xSupportFootPoints).orderBy([](const PointF &p) { return (p.x * 1000) + p.y; }).toStdVector();	// order by p.x first then by p.y.

		//xSupportFootPoints = from(xSupportFootPoints).orderBy([&](const PointF &p) { return abs(sqrt(pow(p.x - xSupportFootPoints[0].x, 2) + pow(p.y - xSupportFootPoints[0].y, 2))); }).toStdVector();

		if (xSupportFootPoints.size() == 0)
		{
			Status = STATUS::Done;
			//OnGenerateMeshsDone(this, new GenerateDoneEventArgs());
			return;
		}
		// Charles: TEST
		//for (int i = 0; i < xSupportFootPoints.size(); i++)
		//	DbgMsg(__FUNCTION__"===> SupportFootPoints[%d]: %s", i, xSupportFootPoints[i].ToString().c_str());
		// End of Test

		xClusteringOfSupPoints.clear();
		setSupIntoClusters();

		for (auto &cluster : xClusteringOfSupPoints)
		{
			prog_value = 0;

			xSupportFootPoints.clear();
			xSupportFootPoints = cluster;
			
			xCuboids.clear();
			xBedCells.clear();
			xBedCellPoints.clear();
			vTabs_Data.Reset();

			initStyleValues();

			calcTotalCells();

			genBedCellPoints();

			execCuboidSpanTree();

			genCuboids();

			if (enableHexagonBase)
			{
				checkIntersections();
				genBedCells();

				if (enableEasyReleaseTab)
				{
					CollectCellsWithTabOpenings();
					CreateEasyReleaseTabs();
				}
			}
		}
		// Charles: TEST
		//SupportMeshsBedDbg::DumpObjectToFile(xBedCells, "xBedCells", "xBedCells.txt");
		//SupportMeshsBedDbg::DumpObjectToFile(xBedCellPoints, "xBedCellPoints", "xBedCellPoints.txt");

		xSupportFootPoints.clear();
		xSupDataRefrnce.clear();
		xCuboids.clear();
		xBedCells.clear();
		xBedCellPoints.clear();

		//KPPH CP2 SW RCGREY 20190705
		/*
		if (vNewBase_Model.originalModel.vertices.Count > 0 && !debuggingMode)
		{
				SupportModel resultbase = new SupportModel(drawer);
				if (!enableHexagonBase)
				{
					resultbase.name = "[Cuboid Module]";
					resultbase.supType = (int)SupportModel.ModelType.CUBOID_CELL;
				}
				else
				{
					resultbase.name = "[BedSupport Module]";
					resultbase.supType = (int)SupportModel.ModelType.BED_CELL;
				}

				resultbase.Scale = new Coord3D((float)1.0, (float)1.0, (float)1.0);
				resultbase.Rotation = new Coord3D(0, 0, 0);
				resultbase.Position = new Coord3D(Cubedata2.Trans.M41, Cubedata2.Trans.M42, Cubedata2.Trans.M43);
				resultbase.UpdateMatrix();
				resultbase.originalModel.Merge(vNewBase_Model.originalModel, resultbase.invTrans, null);
				resultbase.originalModel.UpdateVertexNumbers();

				//vNewBase_Model.ConvexHull3D();
				resultbase.UpdateBoundingBox();
				resultbase.SupportModelToSupportData(resultbase);
				_items2.Add(resultbase.SupportData);
		}
		*/

		Status = STATUS::Done;
		//OnGenerateMeshsDone(this, new GenerateDoneEventArgs());

		//DumpSupportData
		//for (int i = 0; i < _items2.size(); i++)
		//	_items2[i]->ToFile(".\\0_DLL\\BedSD_" + to_string(i) + ".txt");
		//End of Dump
		
		// Test
		//SupportDataToStlFileWOModel("MeshBedDLL.stl");
	}

	void SupportMeshsBed::initStyleValues()
	{
		switch (baseCellxStyle)
		{
		case BaseCellStyle::STYLE1:
			baseCellStyle_Width = Style1::Width;
			baseCellStyle_Depth = Style1::Depth;
			baseCellStyle_Offset_H = Style1::Offset_H;
			baseCellStyle_Offset_V = Style1::Offset_V;
			baseCellStyle_Radius_I = Style1::Radius_I;
			baseCellStyle_Radius_O = Style1::Radius_O;
			baseCellxStyleModel = ReferenceMesh.BedCellStyle1();
			break;
		case BaseCellStyle::STYLE2:
			baseCellStyle_Width = Style2::Width;
			baseCellStyle_Depth = Style2::Depth;
			baseCellStyle_Offset_H = Style2::Offset_H;
			baseCellStyle_Offset_V = Style2::Offset_V;
			baseCellStyle_Radius_I = Style2::Radius_I;
			baseCellStyle_Radius_O = Style2::Radius_O;
			baseCellxStyleModel = ReferenceMesh.BedCellStyle2();
			break;
		case BaseCellStyle::STYLE3:
			baseCellStyle_Width = Style3::Width;
			baseCellStyle_Depth = Style3::Depth;
			baseCellStyle_Offset_H = Style3::Offset_H;
			baseCellStyle_Offset_V = Style3::Offset_V;
			baseCellStyle_Radius_I = Style3::Radius_I;
			baseCellStyle_Radius_O = Style3::Radius_O;
			baseCellxStyleModel = ReferenceMesh.BedCellStyle3();
			break;
		case BaseCellStyle::STYLE4:
			baseCellStyle_Width = Style4::Width;
			baseCellStyle_Depth = Style4::Depth;
			baseCellStyle_Offset_H = Style4::Offset_H;
			baseCellStyle_Offset_V = Style4::Offset_V;
			baseCellStyle_Radius_I = Style4::Radius_I;
			baseCellStyle_Radius_O = Style4::Radius_O;
			baseCellxStyleModel = ReferenceMesh.BedCellStyle4();
			break;
		}
	}

	void SupportMeshsBed::calcTotalCells()
	{
		baseCellRows = (int)round(BoundingBoxWOSupport().Size().y / (baseCellStyle_Depth - baseCellStyle_Offset_V));
		baseCellCols = (int)round(BoundingBoxWOSupport().Size().x / (baseCellStyle_Width - baseCellStyle_Offset_H));

		bboxOffset_X = baseCellCols * (baseCellStyle_Width - baseCellStyle_Offset_H) - BoundingBoxWOSupport().Size().x;
		bboxOffset_Y = baseCellRows * (baseCellStyle_Depth - baseCellStyle_Offset_V) - BoundingBoxWOSupport().Size().y;

		baseCellRows++;
		baseCellCols++;

		baseCells_StartX = BoundingBoxWOSupport().xMin() - (bboxOffset_X / 2.0);
		baseCells_StartY = BoundingBoxWOSupport().yMin() - (bboxOffset_Y / 2.0);
	}

	void SupportMeshsBed::genBedCellPoints()
	{
		xBedCellPoints.clear();

		for (int row = 0; row < baseCellRows; row++)
		{
			BedCellPtsInfo points;
			int index = 1;

			for (int col = 0; col < (row % 2 != 0 ? baseCellCols - 1 : baseCellCols); col++)
			{
				RHVector3 point(
					baseCells_StartX + (col * (baseCellStyle_Width - baseCellStyle_Offset_H)) + (row % 2 != 0 ? (baseCellStyle_Width - baseCellStyle_Offset_H) / 2.0 : 0.0),
					baseCells_StartY + (row * (baseCellStyle_Depth - baseCellStyle_Offset_V)),
					baseCells_StartZ
				);
				points.cellpoints.push_back(point);

				if ((row % 2 != 0 ? (baseCellStyle_Width - baseCellStyle_Offset_H) / 2.0 : 0.0) == 0.0)
				{
					if (col == 0)
						points.column.push_back(col);
					else
						points.column.push_back(col * 2);
				}
				else
				{
					points.column.push_back(col + index);
					index++;
				}

			}
			points.row = row;
			xBedCellPoints.push_back(points);
		}
	}

	void SupportMeshsBed::execCuboidSpanTree()
	{
		RHVector3 start, end;
		vector<Line> lines;
		vector<PointF> points = xSupportFootPoints;
		vector<Line> tempLines;

		if (points.size() >= 2)
		{
			//method1
			/*for (int i = 1; i < points.Count(); i++)
			{
				lines.Add(new Line(points[i - 1], points[i]));
			}*/

			//method2
			lines.push_back(Line(points[0], points[1]));
			points.erase(points.begin() + IndexOf(points, lines[0].GetP1()));
			points.erase(points.begin() + IndexOf(points, lines[0].GetP2()));

			while (points.size() > 0)
			{

				int i = lines.size() - 1;

				Line currentLine;
				tempLines.clear();

				for (int j = 0; j < points.size(); j++)
				{
					Line currentLine(lines[i].GetP2(), points[j]);
					tempLines.push_back(currentLine);
				}

				tempLines = from(tempLines).orderBy([](const Line &p) {return p.Distance(); }).toStdVector();


				if (int(tempLines[0].Distance()) > clusteringThreshold)
				{
					PointF farPoint = tempLines[0].GetP2();
					tempLines.clear();

					for (int k = 0; k < lines.size(); k++)
					{
						Line currentLine(lines[k].GetP2(), farPoint);
						tempLines.push_back(currentLine);
					}
					tempLines = from(tempLines).orderBy([](const Line &p) {return p.Distance(); }).toStdVector();
					lines.push_back(tempLines[0]);
					points.erase(points.begin() + IndexOf(points, tempLines[0].GetP2()));
				}
				else
				{
					lines.push_back(tempLines[0]);
					points.erase(points.begin() + IndexOf(points, tempLines[0].GetP2()));
				}
			}

		}


		for (int i = 0; i < lines.size(); i++)
		{
			RHVector3 start(lines[i].GetX1(), lines[i].GetY1(), 0.0);
			RHVector3 end(lines[i].GetX2(), lines[i].GetY2(), 0.0);
			Cuboid cuboid(start, end, lines[i].MidPoint());
			xCuboids.push_back(cuboid);
		}
	}

	void SupportMeshsBed::checkIntersections()
	{
		PointF p1, p2;
		vector<PointF> cellPolygonPoints;
		Line xline;
		vector<Cell> bcIntersects;
		bool cuboid_intersect;
		bool itsaPoint;

		bcIntersects.clear();

		for (int x = 0; x < xBedCellPoints.size(); x++)
		{
			for (int y = 0; y < xBedCellPoints[x].cellpoints.size(); y++)
			{
				cellPolygonPoints.clear();
				// Top-Middle Point
				cellPolygonPoints.push_back(PointF((float)xBedCellPoints[x].cellpoints[y].x, (float)xBedCellPoints[x].cellpoints[y].y - (baseCellStyle_Depth / 2.0f)));
				// Top-Right Point
				cellPolygonPoints.push_back(PointF((float)xBedCellPoints[x].cellpoints[y].x - (baseCellStyle_Width / 2), (float)xBedCellPoints[x].cellpoints[y].y - (baseCellStyle_Depth / 2.0f) + 1.0f));
				// Bottom-Right Point
				cellPolygonPoints.push_back(PointF((float)xBedCellPoints[x].cellpoints[y].x - (baseCellStyle_Width / 2), (float)xBedCellPoints[x].cellpoints[y].y + (baseCellStyle_Depth / 2.0f) - 1.0f));
				// Bottom-Middle Point
				cellPolygonPoints.push_back(PointF((float)xBedCellPoints[x].cellpoints[y].x, (float)xBedCellPoints[x].cellpoints[y].y + (baseCellStyle_Depth / 2.0f)));
				// Bottom-Left Point
				cellPolygonPoints.push_back(PointF((float)xBedCellPoints[x].cellpoints[y].x + (baseCellStyle_Width / 2), (float)xBedCellPoints[x].cellpoints[y].y + (baseCellStyle_Depth / 2.0f) - 1.0f));
				// Top-Left Point
				cellPolygonPoints.push_back(PointF((float)xBedCellPoints[x].cellpoints[y].x + (baseCellStyle_Width / 2), (float)xBedCellPoints[x].cellpoints[y].y - (baseCellStyle_Depth / 2.0f) + 1.0f));

				Polygon cellPolygon(cellPolygonPoints);	//20191210: Checked OK.

				if (xCuboids.size() > 0)
				{
					cuboid_intersect = false;
					for (int i = 0; i < xCuboids.size(); i++)
					{
						itsaPoint = false;
						if (xCuboids[i]._IsVirtual) continue;

						p1 = PointF((float)xCuboids[i]._Start.x, (float)xCuboids[i]._Start.y);
						p2 = PointF((float)xCuboids[i]._End.x, (float)xCuboids[i]._End.y);

						if (p1.x == p2.x && p1.y == p2.y)
						{
							itsaPoint = true;
							if (Geometry::IntersectionOf(p1, cellPolygon) == Intersection::Intersection)
							{
								bcIntersects.push_back(Cell(xBedCellPoints[x].cellpoints[y], xBedCellPoints[x].row, xBedCellPoints[x].column[y]));
								bcIntersects.back().cubiodIntersect = true;
								cuboid_intersect = true;
								break;
							}
						}

						if (!itsaPoint)
						{
							xline = Line(p1, p2);

							float dx = p2.x - p1.x; //delta x
							float dy = p2.y - p1.y; //delta y
							float linelength = (float)sqrt(dx * dx + dy * dy);
							dx /= linelength;
							dy /= linelength;
							//Ok, (dx, dy) is now a unit vector pointing in the direction of the line
							//A perpendicular vector is given by (-dy, dx)
							float thickness = 1.5f; //Some number
							float px = 0.5f * thickness * (-dy); //perpendicular vector with lenght thickness * 0.5
							float py = 0.5f * thickness * dx;
							vector<Line> cline;
							cline.clear();
							cline.push_back(xline);
							cline.push_back(Line(PointF(p1.x + px, p1.y + py), PointF(p2.x + px, p2.y + py)));
							cline.push_back(Line(PointF(p1.x + px, p1.y + py), PointF(p2.x - px, p2.y - py)));
							cline.push_back(Line(PointF(p1.x - px, p1.y - py), PointF(p2.x + px, p2.y + py)));
							cline.push_back(Line(PointF(p1.x - px, p1.y - py), PointF(p2.x - px, p2.y - py)));
							cline.push_back(Line(PointF(p1.x + px, p1.y + py), PointF(p1.x - px, p1.y - py)));
							cline.push_back(Line(PointF(p2.x + px, p2.y + py), PointF(p2.x - px, p2.y - py)));

							for(auto &line : cline)
							{
								if (Geometry::IntersectionOf(line, cellPolygon) == Intersection::Intersection)
								{
									bcIntersects.push_back(Cell(xBedCellPoints[x].cellpoints[y], xBedCellPoints[x].row, xBedCellPoints[x].column[y]));
									bcIntersects.back().cubiodIntersect = true;
									cuboid_intersect = true;
									break;
								}
							}
						}
						if (cuboid_intersect)
							break;
					}
				}
				else
				{
					if (xSupportFootPoints.size() == 1)
					{
						p1 = xSupportFootPoints[0];

						if (Geometry::IntersectionOf(p1, cellPolygon) != Intersection::None)
						{
							bcIntersects.push_back(Cell(xBedCellPoints[x].cellpoints[y], xBedCellPoints[x].row, xBedCellPoints[x].column[y]));
							bcIntersects.back().cubiodIntersect = true;
						}
					}
				}
			}
		}

		//Add space checker
		//vector<Tuple<Cell, int>>SpaceChecker;
		//vector<Triple<int, int, int>> newBaseDesign; // row, mincol, maxcol
		//SpaceChecker.clear();
		bcIntersects = from(bcIntersects).orderBy([](const Cell &p) { return p._row; }).toStdVector();

		for (const BedCellPtsInfo &r : xBedCellPoints)
		{
			//DbgMsg(__FUNCTION__"===>Now is row number: %d", r.row);
			if (from(bcIntersects).where([&](const Cell &u) {return u._row == r.row; }).count() > 0)
			{
				int mincol = from(bcIntersects).where([&](const Cell &p) {return p._row == r.row; }).min([](const Cell &q) {return q._column; })._column;
				int maxcol = from(bcIntersects).where([&](const Cell &p) {return p._row == r.row; }).max([](const Cell &q) {return q._column; })._column;


				for (auto &cell_col : r.column)
				{

					if (cell_col >= mincol && cell_col <= maxcol)
					{

						int cellIndex = IndexOf(r.column, cell_col);
						Cell sample(r.cellpoints[cellIndex], r.row, cell_col);
						//DbgMsg(__FUNCTION__"===>bcIntersects.count(): %d", from(bcIntersects).where([&](const Cell &p) {return p._row == r.row && p._column == cell_col; }).count());
						if (from(bcIntersects).where([&](const Cell &p) {return p._row == r.row && p._column == cell_col; }).count() > 0)
						{
							//sample.cubiodIntersect = from(bcIntersects).where([&](const Cell &p) {return p._row == r.row && p._column == cell_col; }).first().cubiodIntersect;
							xBedCells.push_back(sample);
							continue;
						}
						else
						{
							//sample.cubiodIntersect = false;
							SpaceCheck(sample, bcIntersects);
						}


						//SpaceCheck(sample, bcIntersects, SpaceChecker);
					}
				}



				//newBaseDesign.push_back(Triple<int, int, int>(r.row, mincol, maxcol));
			}
		}
		CoverGaps();
	}

	void SupportMeshsBed::SpaceCheck(Cell sample, vector<Cell> listwithIntersects)
	{
		{
			int currentRow = sample._row;
			int currentCol = sample._column;
			bool spacecheck = false;
			if (from(listwithIntersects).where([&](const Cell &p) {return p._row == currentRow - 1 && p._column == currentCol - 1; }).count() > 0)
			{
				spacecheck = true;
			}
			else if (from(listwithIntersects).where([&](const Cell &p) {return p._row == currentRow - 1 && p._column == currentCol + 1; }).count() > 0)
			{
				spacecheck = true;
			}
			else if (from(listwithIntersects).where([&](const Cell &p) {return p._row == currentRow && p._column == currentCol - 2; }).count() > 0)
			{
				spacecheck = true;
			}
			else if (from(listwithIntersects).where([&](const Cell &p) {return p._row == currentRow && p._column == currentCol + 2; }).count() > 0)
			{
				spacecheck = true;
			}
			else if (from(listwithIntersects).where([&](const Cell &p) {return p._row == currentRow + 1 && p._column == currentCol - 1; }).count() > 0)
			{
				spacecheck = true;
			}
			else if (from(listwithIntersects).where([&](const Cell &p) {return p._row == currentRow + 1 && p._column == currentCol + 1; }).count() > 0)
			{
				spacecheck = true;
			}




			if (spacecheck)
			{
				xBedCells.push_back(sample);
			}

		}
	}

	void SupportMeshsBed::CoverGaps()
	{
		vector<Cell> addGap;
		int minrow = from(xBedCells).min([](const Cell &p) {return p._row; })._row;		// C#: int minrow = xBedCells.Min(p => p._row);
		int maxrow = from(xBedCells).max([](const Cell &p) {return p._row; })._row;		// C#: int maxrow = xBedCells.Max(p => p._row);

		// C#: xBedCells = xBedCells.OrderBy(p => p._row).OrderBy(q => q._column).ToList();	// NOTE: It is NOT "OrderBy" -> "ThenBy"!!!
		xBedCells = from(xBedCells).orderBy([](const Cell &p) { return (p._column * 1000) + p._row; }).toStdVector();
		vector<Cell> CloneBC = xBedCells;

		for (int i = minrow; i <= maxrow; i++)
		{
			if (!from(xBedCells).any([&](const Cell &p) { return p._row == i + 1; })) continue;
			if (!from(xBedCells).any([&](const Cell &p) { return p._row == i - 1; })) continue;

			for (const Cell &c1 : from(xBedCells).where([&](const Cell &p) { return p._row == i - 1;}).toStdVector())
			{
				for (const Cell &c2 : from(CloneBC).where([&](const Cell &p) { return p._row == i + 1; }).toStdVector())
				{
					if (c1._column == c2._column)
					{
						if (from(xBedCells).any([&](const Cell &p) { return p._row == i && p._column == c1._column - 1;}))
							continue;
						else
						{
							BedCellPtsInfo temp = from(xBedCellPoints).where([&](const BedCellPtsInfo &p) { return p.row == i; }).first();
							int index = IndexOf(temp.column, c1._column - 1);
							if (index >= 0 && index <= IndexOf(temp.column, from(temp.column).max([](int p) {return p; })))
								addGap.push_back(Cell(temp.cellpoints[index], i, temp.column[index]));
						}
					}
				}
			}
		}

		if (addGap.size()>0)
		{
			for(const Cell &a : addGap)
				xBedCells.push_back(a);
		
			// C#: xBedCells = xBedCells.OrderBy(p => p._row).OrderBy(q => q._column).ToList(); // NOTE: It is NOT "OrderBy" -> "ThenBy"!!!
			xBedCells = from(xBedCells).orderBy([](const Cell &p) { return (p._column * 1000) + p._row; }).toStdVector();
		}
	}

	void SupportMeshsBed::genCuboids()
	{
		/*
		vNewBase_Model = SupportModel(drawer);

		for (int i = 0; i < xCuboids.size(); i++)
		{
			SupportModel CuboidCell = new SupportModel(drawer);

			double RotationAngle = atan2(xCuboids[i]._Start.y - xCuboids[i]._End.y, xCuboids[i]._End.x - xCuboids[i]._Start.x) * (180.0 / M_PI);
			double distance = abs(sqrt(pow(xCuboids[i]._End.x - xCuboids[i]._Start.x, 2) + pow(xCuboids[i]._End.y - xCuboids[i]._Start.y, 2))) + (double)Diameter::SMALL;

			CuboidCell.originalModel = ReferenceMesh.CuboidCell;
			CuboidCell.name = "[Cuboid Cell]";
			CuboidCell.supType = (int)ModelType::CUBOID_CELL;
			CuboidCell.Scale = Coord3D((float)distance, (float)1.80, newBaseZScale);
			CuboidCell.Position = Coord3D((float)xCuboids[i]._MidPoint.x, (float)xCuboids[i]._MidPoint.y, (float)baseCells_StartZ);
			CuboidCell.Rotation.z = (float)RotationAngle;

			if (!debuggingMode)
			{
				//KPPH CP2 SW RCGREY 20190705
				CuboidCell.UpdateMatrix();
				vNewBase_Model.originalModel.Merge(CuboidCell.ActiveModel, CuboidCell.trans, null);
			}
			else
			{
				//Ervin 04-12-2019 Adds to the Support Data List
				CuboidCell = CuboidCell.SupportModelToSupportData(CuboidCell);
				_items2.push_back(CuboidCell.SupportData);
			}
		}
		*/
		for (int i = 0; i < xCuboids.size(); i++)
		{
			double RotationAngle = atan2(xCuboids[i]._Start.y - xCuboids[i]._End.y, xCuboids[i]._End.x - xCuboids[i]._Start.x) * (180.0 / M_PI);
			double distance = abs(sqrt(pow(xCuboids[i]._End.x - xCuboids[i]._Start.x, 2) + pow(xCuboids[i]._End.y - xCuboids[i]._Start.y, 2))) + (double)Diameter::SMALL;
			unique_ptr<SupportData> pSupportData(new SupportData());
			//pSupportData->originalModel = ReferenceMesh.CuboidCell();	// C++: Moved to GenMesh().
			pSupportData->name = "[Cuboid Cell]";
			pSupportData->supType = (int)ModelType::CUBOID_CELL;
			pSupportData->SetScale(Coord3D((float)distance, (float)1.80, newBaseZScale));
			pSupportData->SetPosition(Coord3D((float)xCuboids[i]._MidPoint.x, (float)xCuboids[i]._MidPoint.y, (float)baseCells_StartZ));
			pSupportData->SetRotation(Coord3D(pSupportData->GetRotation().x, pSupportData->GetRotation().y, (float)RotationAngle));
			pSupportData->UpdateMatrix();
			_items2.push_back(move(pSupportData));
		}
	}

	void SupportMeshsBed::genBedCells()
	{

		for (int i = 0; i < xBedCells.size(); i++)
		{
#if 0	// C# AP
			SupportModel bedCell = new SupportModel(drawer);

			bedCell.name = "xCell " + "[" + xBedCells[i]._column.ToString() + "]" + " " + "[" + xBedCells[i]._row.ToString() + "]";
			bedCell.supType = (int)ModelType::BED_CELL;
			bedCell.originalModel = baseCellxStyleModel;
			bedCell.Scale = Coord3D(1.0f, 1.0f, newBaseZScale);
			bedCell.Position = Coord3D((float)xBedCells[i]._position.x, (float)xBedCells[i]._position.y, (float)xBedCells[i]._position.z);

			if (!debuggingMode)
			{
				//KPPH CP2 SW RCGREY 20190705
				bedCell.UpdateMatrix();
				vNewBase_Model.originalModel.Merge(bedCell.ActiveModel, bedCell.trans, null);
			}
			else
			{
				bedCell = bedCell.SupportModelToSupportData(bedCell);
				_items2.push_back(bedCell.SupportData);
			}
#endif
			unique_ptr<SupportData> pSupportData(new SupportData());
			pSupportData->name = "xCell [" + to_string(xBedCells[i]._column) + "] [" + to_string(xBedCells[i]._row) + "]";
			pSupportData->supType = (int)ModelType::BED_CELL;
			//pSupportData->originalModel = baseCellxStyleModel;	// C++: Moved to GenMesh().
			pSupportData->SetScale(Coord3D(1.0f, 1.0f, newBaseZScale));
			pSupportData->SetPosition(Coord3D((float)xBedCells[i]._position.x, (float)xBedCells[i]._position.y, (float)xBedCells[i]._position.z));
			pSupportData->UpdateMatrix();
			_items2.push_back(move(pSupportData));
		}
	}

	void SupportMeshsBed::CollectCellsWithTabOpenings()
	//KPPH CP2 SW RCGREY 20180614
	//Initial collection of cell candidates for new tab/s position
	{
		bool limitFocus = true; //WARNING!! changing this setting may alter result. disabling limit focus will loosen tab angle search
		vTabs_Data.Reset();
		vTabs_Data.tabLists.clear();

		if (xBedCells.size() >= (int)BedCellCount::GenrateFourTab)
			limitFocus = false; //better results for four tabs category

		for (int i = 0; i < xBedCells.size(); i++)
		{
			if (xBedCells[i].cubiodIntersect == true)
			{
				vector<int> dlist = CheckCellsAdjacentoEachSides(xBedCells[i]); //acquire cells with free space side	

				TabPosResources angleref = GetTabRotateValue(dlist, xBedCells[i], limitFocus); //get valid angles for tab positioning	//******************************
				angleref.tabPosition.cubiodIntersect = xBedCells[i].cubiodIntersect;

				if (angleref.rotationInfo.First() == 999.0) //no valid angle found
					continue;
				if (debuggingMode)
					HighlightPrint(xBedCells[i], angleref);

				vTabs_Data.tabLists.push_back(angleref);
			}
		}
	}

	void SupportMeshsBed::CreateEasyReleaseTabs()
	//KPPH CP2-SW RCGREY 20180905 
	// distribute collected tab-cell positions and determine a logical position for each easy remove tab
	//  the number of tabs to be generate is dependent to the total number of cells generated for the new base
	{
		bool isCancelled = false;

		TabDesign candidatetemp;
		vector<TabDesign> tabs_workingList;
		vector<TabDesign> list_temp;
		vector<TabDesign> final_tabDesign;
		CandidateScore ctemp;
		vector<CandidateScore> assessed_List;

		int index = 0;
		int var_ref = 0;
		bool count_similar = false;

		if (xBedCells.size() <= (int)BedCellCount::GenerateNoTab)
			return; //no need tab
		else if (xBedCells.size() <= (int)BedCellCount::GenerateOneTab)
			vNumOfTabDesign = TabGenCount::OneTab;
		else if (xBedCells.size() <= (int)BedCellCount::GenerateTwoTab)
			vNumOfTabDesign = TabGenCount::TwoTab;
		else if (xBedCells.size() <= (int)BedCellCount::GenerateTriTab)
			vNumOfTabDesign = TabGenCount::ThreeTab;
		else
			vNumOfTabDesign = TabGenCount::FourTab;

		DbgMsg(__FUNCTION__"===> vNumOfTabDesign: %d", vNumOfTabDesign);

		if (vTabs_Data.tabLists.size() > 0)
		{
			for(const TabPosResources &t : vTabs_Data.tabLists)
			{
				candidatetemp.candidate = t.tabPosition;
				candidatetemp.index = IndexOf(vTabs_Data.tabLists, t);
				candidatetemp.candidate.cubiodIntersect = t.tabPosition.cubiodIntersect;
				candidatetemp.density = t.density;
				candidatetemp.angle = t.rotationInfo.First();
				candidatetemp.endpoint = t.rotationInfo.Second();
				candidatetemp.sideLocation = t.sidelocation;
				candidatetemp.serviceAreaSample = t.serviceAreaSampling;
				tabs_workingList.push_back(candidatetemp);
			}
			tabs_workingList = from(tabs_workingList).orderBy([](const TabDesign &p) { return p.serviceAreaSample; }).reverse().toStdVector();		// C#: tabs_workingList = tabs_workingList.OrderByDescending(p => p.serviceAreaSample).ToList();

			//KPPH CP2-SW RCGREY 20190118
			// Assessment of tab design candidates based on sampling Area for each tab design and the angles of different for multiple tab generation
			// The assessment is done thru determining the tab design by score accumulatations.
			//candidate's Area Sampling score per hierarchy position: (first) 3, (second) 2, (third) 1;
			//

			var_ref = from(tabs_workingList).max([](const TabDesign &p) {return p.serviceAreaSample; }).serviceAreaSample;			
			list_temp = from(tabs_workingList).where([&](const TabDesign &v) {return v.serviceAreaSample == var_ref; }).toStdVector();			
			
			// Charles: added sort by index to make the order is the same as the C# XYZware AP
			list_temp = from(list_temp).orderBy([](const TabDesign &p) { return p.index; }).toStdVector();
			
			int count = 0;
			//#region gather tab design candidates with the largest sampling area
			for(const TabDesign &td : list_temp)
			{
				ctemp.candidate = td;
				ctemp.listPosition = 1;
				ctemp.scoreAreaSampling = 3;
				DetermineAssociateTabPositions(/*out*/ ctemp, tabs_workingList);

				if (assessed_List.size() > 0)
				{
					count_similar = false;
					for(const CandidateScore &cs : assessed_List)
					{
						int cnt1 = (int)cs.assoc_candidates.size();
						int cnt2 = 0;
						for(const TabDesign &cs_assoc : cs.assoc_candidates)
						{
							for(const TabDesign &tmp : ctemp.assoc_candidates)
							{
								if (cs_assoc.candidate.isNull == false && tmp.candidate.isNull == false)
								{
									if (cs_assoc.candidate._position == tmp.candidate._position)
									{
										cnt2++;
										break;
									}
								}
							}
						}
						if (cnt1 == cnt2)
						{
							count_similar = true;
							break;
						}
					}
				}
				if (!count_similar)
					assessed_List.push_back(ctemp);

				index++;
				count++;
				OnGenerateMeshsProcessRateUpdate(90.0 + (count / list_temp.size() * 2.0), isCancelled); // C#: prog_rate = ProcessRateEventArgs((unsigned int)((count / list_temp.size()) * 20));
				if (isCancelled == true) return;
			}
			prog_value = 92;
			//#endregion

			//#region gather and add tab design candidates with the second largest sampling area
			count = 0;
			if (index >= tabs_workingList.size())
				goto finish;
			else if (from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == tabs_workingList[index].serviceAreaSample;}).count() > 0)
			{
				if (var_ref != from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == tabs_workingList[index].serviceAreaSample; }).first().serviceAreaSample)
				{
					var_ref = from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == tabs_workingList[index].serviceAreaSample; }).first().serviceAreaSample;

					list_temp = from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == var_ref; }).toStdVector();

					// Charles: added sort by index to make the order is the same as the C# XYZware AP
					list_temp = from(list_temp).orderBy([](const TabDesign &p) { return p.index; }).toStdVector();
					for(const TabDesign td : list_temp)
					{
						ctemp.candidate = td;
						ctemp.listPosition = 2;
						ctemp.scoreAreaSampling = 2;
						DetermineAssociateTabPositions(/*out*/ ctemp, tabs_workingList);

						if (assessed_List.size() > 0)
						{
							count_similar = false;
							for(const CandidateScore &cs : assessed_List)
							{
								int cnt1 = (int)cs.assoc_candidates.size();
								int cnt2 = 0;
								for(const TabDesign &cs_assoc : cs.assoc_candidates)
								{
									for(const TabDesign &tmp : ctemp.assoc_candidates)
									{
										if (cs_assoc.candidate.isNull == false && tmp.candidate.isNull == false)
										{
											if (cs_assoc.candidate._position == tmp.candidate._position)
											{
												cnt2++;
												break;
											}
										}
									}
								}
								if (cnt1 == cnt2)
								{
									count_similar = true;
									break;
								}
							}
						}
						if (!count_similar)
							assessed_List.push_back(ctemp);

						index++;
						count++;
						OnGenerateMeshsProcessRateUpdate( (count / list_temp.size() * 1.0) + prog_value, isCancelled); 	// C#: prog_rate = new ProcessRateEventArgs((uint)(((count / list_temp.Count()) * 20) + prog_value));
						if (isCancelled == true) return;
					}
					prog_value = 93;
				}
			}
			//#endregion

			//#region gather and add tab design candidates with the third largest sampling area
			count = 0;
			if (index >= tabs_workingList.size())
				goto finish;
			else if (from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == tabs_workingList[index].serviceAreaSample; }).count() > 0)
			{
				if (var_ref != from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == tabs_workingList[index].serviceAreaSample; }).first().serviceAreaSample)
				{
					var_ref = from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == tabs_workingList[index].serviceAreaSample; }).first().serviceAreaSample;

					list_temp = from(tabs_workingList).where([&](const TabDesign &v) { return v.serviceAreaSample == var_ref; }).toStdVector();

					// Charles: added sort by index to make the order is the same as the C# XYZware AP
					list_temp = from(list_temp).orderBy([](const TabDesign &p) { return p.index; }).toStdVector();

					for(const TabDesign &td : list_temp)
					{
						ctemp.candidate = td;
						ctemp.listPosition = 3;
						ctemp.scoreAreaSampling = 1;
						DetermineAssociateTabPositions(/*ref*/ ctemp, tabs_workingList);

						if (assessed_List.size() > 0)
						{
							count_similar = false;
							for(const CandidateScore &cs : assessed_List)
							{
								int cnt1 = (int)cs.assoc_candidates.size();
								int cnt2 = 0;
								for(const TabDesign &cs_assoc : cs.assoc_candidates)
								{
									for(const TabDesign &tmp : ctemp.assoc_candidates)
									{
										if (cs_assoc.candidate.isNull == false && tmp.candidate.isNull == false)
										{
											if (cs_assoc.candidate._position == tmp.candidate._position)
											{
												cnt2++;
												break;
											}
										}
									}
								}
								if (cnt1 == cnt2)
								{
									count_similar = true;
									break;
								}
							}
						}
						if (!count_similar)
							assessed_List.push_back(ctemp);

						index++;
						count++;
						OnGenerateMeshsProcessRateUpdate( (count / list_temp.size() * 1.0) + prog_value, isCancelled);	//C#: prog_rate = new ProcessRateEventArgs((uint)(((count / list_temp.Count()) * 20) + prog_value));
						if(isCancelled == true) return;
					}
					prog_value = 94;
				}
			}
			//#endregion

		finish :
			//the assessed design with the highest score will be applied as the tab/s for the new base
			// C#: assessed_List = (from p in assessed_List where p.totalScore == assessed_List.Max(q = > q.totalScore) select p).ToList();
			assessed_List = from(assessed_List).where([&](const CandidateScore &p) { return p.totalScore == from(assessed_List).max([](const CandidateScore &q) { return q.totalScore; }).totalScore; }).toStdVector();

			//final_tabDesign = assessed_List.front().assoc_candidates;
			if (assessed_List.size() > 1)
			{
				from(assessed_List).orderBy([](const CandidateScore &p) { return p.totalScore; });
				int size = assessed_List.size();
				int mid = size / 2;
				int median = (size % 2 != 0) ? mid : mid + (mid - 1) / 2;
				final_tabDesign = assessed_List[median].assoc_candidates;
			}
			else if (assessed_List.size() == 1)
				final_tabDesign = assessed_List.front().assoc_candidates;

			if (final_tabDesign.size()>0) //select final tab/s design and generate
				FinalizeTabDesign(final_tabDesign);

			OnGenerateMeshsProcessRateUpdate(95, isCancelled); 		//C#: prog_rate = new ProcessRateEventArgs((uint)100);	// NOTE: C++, 90 ~ 95: Bed
			prog_value = 95;
		}
	}
	void SupportMeshsBed::DetermineAssociateTabPositions(/*out*/ CandidateScore &input, vector<TabDesign> &candidateList)
	{
		//KPPH CP2-SW RCGREY 20190118
		//Angle Score for One Tab: {90, 270}= 5, {0, 180}= 3, the rest is 1; only one tab is being use for reference here.
		//Angle Score for Two Tab: {180}= 5, {120, 150, 210, 240}= 3, {90, 270}= 1, the rest is zero(0); the difference in angle between two tabs for two tabs separation 
		//Angle Score for Three Tab: {120, 150}= 5, {90, 180, 210, 240, 270}= 3, {60, 300}= 1, the rest is -7; the difference in angle between two tabs for 3 tabs separation
		//Angle Score for Four Tab: {180, 90}= 5, {60, 120, 150}= 3, {30}= 1, the rest is -7; the difference in angle between two tabs for 4 tabs separation
		double angle_difference = 0;
		int minRowDistSeparation = (from(xBedCells).max([](const Cell &p) {return p._row; })._row - from(xBedCells).min([](const Cell &p) {return p._row; })._row) / 3;
		int minColDistSeparation = (from(xBedCells).max([](const Cell &p) {return p._column; })._column - from(xBedCells).min([](const Cell &p) {return p._column; })._column) / 3;

		set<Tuple<TabDesign, int>> tab2_candidates;
		set<Triple<TabDesign, TabDesign, int>> tab3_candidates;

		set<TabDesign> c1List;
		for (vector<TabDesign>::iterator it = candidateList.begin(); it != candidateList.end(); ++it)
			c1List.insert(*it);

		set<TabDesign> c2List;
		set<TabDesign> c3List;

		input.assoc_candidates.clear();
		RHVector3 ref_position = input.candidate.candidate._position;

		switch (vNumOfTabDesign)
		{
		case TabGenCount::OneTab:
			//#region determine OneTab Design Output
			if (input.candidate.angle == 90.0 || input.candidate.angle == 270.0)
				input.scoreAngleDifference = 5;
			else if (input.candidate.angle == 0.0 || input.candidate.angle == 180.0)
				input.scoreAngleDifference = 3;
			else
				input.scoreAngleDifference = 1;

			input.assoc_candidates.push_back(input.candidate);
			input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			break;
			//#endregion

		case TabGenCount::TwoTab:
		{
			//#region determine TwoTab Design Output
			set<Tuple<TabDesign, int>> nxtAssoc_candidate; // Tuple<candidate, score of angle difference>

			c1List.erase(from(c1List).where([&](const TabDesign &v) { return v.candidate._position == ref_position; }).first());			//C#: c1List.Remove((from v in c1List where v.candidate._position == ref_position select v).First());

			for (const TabDesign &td : c1List)
			{
				if (abs(input.candidate.candidate._row - td.candidate._row) >= minRowDistSeparation && abs(input.candidate.candidate._column - td.candidate._column) >= minColDistSeparation)
				{
					angle_difference = abs(input.candidate.angle - td.angle);
					if (angle_difference == 180.0)
						nxtAssoc_candidate.insert(Tuple<TabDesign, int>(td, 5));
					else if (angle_difference == 120.0 || angle_difference == 150.0 || angle_difference == 210.0 || angle_difference == 240.0)
						nxtAssoc_candidate.insert(Tuple<TabDesign, int>(td, 3));
					else if (angle_difference == 90.0 || angle_difference == 270.0)
						nxtAssoc_candidate.insert(Tuple<TabDesign, int>(td, 1));
					else
						nxtAssoc_candidate.insert(Tuple<TabDesign, int>(td, 0));
				}
				else
					nxtAssoc_candidate.insert(Tuple<TabDesign, int>(td, 0));
			}

			tab2_candidates = set<Tuple<TabDesign, int>>(from(nxtAssoc_candidate).where([&](const Tuple<TabDesign, int> &r) { return r.Second() == from(nxtAssoc_candidate).max([](const Tuple<TabDesign, int> &s) { return s.Second(); }).Second(); }).toStdSet());

			if (tab2_candidates.size() == 1)
			{
				input.scoreAngleDifference = from(tab2_candidates).elementAt(0).Second();
				input.assoc_candidates.push_back(input.candidate);
				input.assoc_candidates.push_back(from(tab2_candidates).elementAt(0).First());
				input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			}
			else if (tab2_candidates.size() == 0)
			{
				input.scoreAngleDifference = 0;
				input.assoc_candidates.push_back(input.candidate);
				input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			}
			else
			{
				vector<TabDesign> multiple_result = from(tab2_candidates).select([](const Tuple<TabDesign, int> &p) { return p.First(); }).toStdVector();
				int index = getMeanDistribBySampleArea(/*out*/ multiple_result);
				input.scoreAngleDifference = from(tab2_candidates).elementAt(0).Second();
				input.assoc_candidates.push_back(input.candidate);
				input.assoc_candidates.push_back(multiple_result[index]);
				input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			}
			nxtAssoc_candidate.clear();
			break;
			//#endregion
		}

		case TabGenCount::ThreeTab:
		{
			//#region determine ThreeTab Design Output
			set<Triple<TabDesign, TabDesign, int>> partial_result3;

			c1List.erase(from(c1List).where([&](const TabDesign &v) { return v.candidate._position == ref_position; }).first());			//C#: c1List.Remove((from v in c1List where v.candidate._position == ref_position select v).First());
			for (const TabDesign &td : c1List)
			{
				TabDesign tab2;
				int anglescoretemp1;

				if (abs(input.candidate.candidate._row - td.candidate._row) >= minRowDistSeparation && abs(input.candidate.candidate._column - td.candidate._column) >= minColDistSeparation)
				{
					tab2 = td;
					angle_difference = abs(input.candidate.angle - td.angle);
					if (angle_difference == 120.0 || angle_difference == 150.0)
						anglescoretemp1 = 5;
					else if (angle_difference == 90.0 || angle_difference == 240.0 || angle_difference == 210.0 || angle_difference == 270.0 || angle_difference == 180.0)
						anglescoretemp1 = 3;
					else if (angle_difference == 60.0 || angle_difference == 300.0)
						anglescoretemp1 = 1;
					else
						anglescoretemp1 = 0;
				}
				else
					anglescoretemp1 = 0;

				c2List = c1List;

				if (tab2.candidate.isNull == false)
				{
					ref_position = tab2.candidate._position;

					c2List.erase(from(c2List).where([&](const TabDesign &v) { return v.candidate._position == ref_position; }).first());
				}

				for (const TabDesign &td2 : c2List)
				{
					TabDesign tab2tmp = tab2;
					TabDesign tab3;
					int anglescoretemp2 = anglescoretemp1;

					if (tab2.candidate.isNull == false)
					{
						if (abs(tab2.candidate._row - td2.candidate._row) >= minRowDistSeparation || abs(tab2.candidate._column - td2.candidate._column) >= minColDistSeparation)
						{
							angle_difference = abs(tab2.angle - td2.angle);
							if (angle_difference == 120.0 || angle_difference == 150.0)
								anglescoretemp2 += 5;
							else if (angle_difference == 90.0 || angle_difference == 240.0 || angle_difference == 210.0 || angle_difference == 270.0 || angle_difference == 180.0)
								anglescoretemp2 += 3;
							else if (angle_difference == 60.0 || angle_difference == 300.0)
								anglescoretemp2 += 1;
							else
								anglescoretemp2 += 0;
						}
						else
						{
							tab2tmp.Reset();
							anglescoretemp2 -= anglescoretemp1;
						}
					}
					else
						anglescoretemp2 += 0;

					if (abs(input.candidate.candidate._row - td2.candidate._row) >= minRowDistSeparation && abs(input.candidate.candidate._column - td2.candidate._column) >= minColDistSeparation)
					{
						tab3 = td2;
						angle_difference = abs(input.candidate.angle - td2.angle);
						if (angle_difference == 120.0 || angle_difference == 150.0)
							anglescoretemp2 += 5;
						else if (angle_difference == 90.0 || angle_difference == 240.0 || angle_difference == 210.0 || angle_difference == 270.0 || angle_difference == 180.0)
							anglescoretemp2 += 3;
						else if (angle_difference == 60.0 || angle_difference == 300.0)
							anglescoretemp2 += 1;
						else
							anglescoretemp2 += 0;
					}
					else
						anglescoretemp2 += 0;

					tab3_candidates.insert(Triple<TabDesign, TabDesign, int>(tab2tmp, tab3, anglescoretemp2));
				}

				vector<Triple<TabDesign, TabDesign, int>> selectedTab3;
				int index = 0;
				if (tab3_candidates.size() > 0)
				{
					selectedTab3 = from(tab3_candidates).where([&](const Triple<TabDesign, TabDesign, int> &p) { return p.Third() == from(tab3_candidates).max([](const Triple<TabDesign, TabDesign, int> &q) { return q.Third(); }).Third(); }).toStdVector();

					vector<TabDesign> multiple_result = from(selectedTab3).where([](const Triple<TabDesign, TabDesign, int> &p) { return p.Second().candidate.isNull == false; }).select([](const Triple<TabDesign, TabDesign, int> &q) { return q.Second(); }).toStdVector();

					if (multiple_result.size() > 0)
					{
						index = getMeanDistribBySampleArea(/*out*/ multiple_result);

						index = IndexOf(selectedTab3, 
							from(selectedTab3).where([&](const Triple<TabDesign, TabDesign, int> &v) { return v.Second().candidate.isNull == false && v.Second().candidate._position == multiple_result[index].candidate._position; }).first());
					}
				}

				if (selectedTab3.size() > 0)
					partial_result3.insert(Triple<TabDesign, TabDesign, int>(selectedTab3[index].First(), selectedTab3[index].Second(), selectedTab3[index].Third()));
				else
					partial_result3.insert(Triple<TabDesign, TabDesign, int>(tab2, TabDesign(), anglescoretemp1));

				selectedTab3.clear();
			}

			if (partial_result3.size() > 0)
				tab3_candidates = from(partial_result3).where([&](const Triple<TabDesign, TabDesign, int> &r) { return r.Third() == from(partial_result3).max([](const Triple<TabDesign, TabDesign, int> &s) { return s.Third(); }).Third(); }).toStdSet();

			if (tab3_candidates.size() > 0)
			{
				input.scoreAngleDifference = from(tab3_candidates).elementAt(0).Third();
				input.assoc_candidates.push_back(input.candidate);
				input.assoc_candidates.push_back(from(tab3_candidates).elementAt(0).First());
				input.assoc_candidates.push_back(from(tab3_candidates).elementAt(0).Second());
				input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			}
			else
			{
				input.scoreAngleDifference = 0;
				input.assoc_candidates.push_back(input.candidate);
				input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			}

			partial_result3.clear();

			break;
			//#endregion
		}
		case TabGenCount::FourTab:
		{
			//#region determine FourTabDesign Output
			minRowDistSeparation = (from(xBedCells).max([](const Cell &p) { return p._row; })._row - from(xBedCells).min([](const Cell &p) { return p._row; })._row) * 3/ 8;
			minColDistSeparation = (from(xBedCells).max([](const Cell &p) { return p._column; })._column - from(xBedCells).min([](const Cell &p) { return p._column; })._column) * 3 / 8;

			set<Quadruple<TabDesign, TabDesign, TabDesign, int>> tab4_candidates;

			c1List.erase(from(c1List).where([&](const TabDesign &v) { return v.candidate._position == ref_position; }).first());
			
			double *applyconfig = nullptr;
			double config1[] = {90.0, 5.0, 60.0, 4.0, 120.0, 150.0, 3.0, 30.0, 1.0, 0.0};
			double config2[] = { 180.0, 5.0, 150.0, 4.0, 90.0, 60.0, 3.0, 30.0, 1.0, 0.0 };

			for (const TabDesign &td2 : c1List)
			{
				TabDesign tab2ofFour;
				int anglescoretemp2ofFour = 0;
				if (abs(input.candidate.candidate._row - td2.candidate._row) >= minRowDistSeparation || abs(input.candidate.candidate._column - td2.candidate._column) >= minColDistSeparation)
				{
					tab2ofFour = td2;
					applyconfig = config1;

					if (abs(input.candidate.candidate._row - td2.candidate._row) >= minRowDistSeparation && abs(input.candidate.candidate._column - td2.candidate._column) >= minColDistSeparation)
						anglescoretemp2ofFour = 20;

					if (td2.angle == input.candidate.angle)
						angle_difference = 0;
					else if (td2.angle > input.candidate.angle)
						angle_difference = td2.angle - input.candidate.angle;
					else if (td2.angle < input.candidate.angle)
						angle_difference = (td2.angle + 360.0) - input.candidate.angle;

					if (angle_difference == applyconfig[0])
						anglescoretemp2ofFour += (int)applyconfig[1];
					else if (angle_difference == applyconfig[2])
						anglescoretemp2ofFour += (int)applyconfig[3];
					else if (angle_difference == applyconfig[4] || angle_difference == applyconfig[5])
						anglescoretemp2ofFour += (int)applyconfig[6];
					else if (angle_difference == applyconfig[7])
						anglescoretemp2ofFour += (int)applyconfig[8];
					else
						anglescoretemp2ofFour += (int)applyconfig[9];

					if (angle_difference == 0)
						tab2ofFour = TabDesign();
				}
				else
					anglescoretemp2ofFour = 0;

				if (tab2ofFour.candidate.isNull == false && anglescoretemp2ofFour != 0)
				{
					if (tab2_candidates.size() == 0)
						tab2_candidates.insert(Tuple<TabDesign, int>(tab2ofFour, anglescoretemp2ofFour));
					else
					{
						int max_score = from(tab2_candidates).max([](const Tuple<TabDesign, int> &p) {return p.Second(); }).Second();

						if (anglescoretemp2ofFour == max_score)
							tab2_candidates.insert(Tuple<TabDesign, int>(tab2ofFour, anglescoretemp2ofFour));
						else if (anglescoretemp2ofFour > max_score)
						{
							tab2_candidates = set<Tuple<TabDesign, int>>();
							tab2_candidates.insert(Tuple<TabDesign, int>(tab2ofFour, anglescoretemp2ofFour));
						}
					}
				}
			}

			TabDesign tab3ofFour;
			int anglescoretemp3ofFour = 0;

			c2List = c1List;
			applyconfig = config1;

			if (tab2_candidates.size() == 0)
			{
				tab2_candidates.insert(Tuple<TabDesign, int>(input.candidate, 0));
				applyconfig = config2;
			}

			if (tab2_candidates.size() > 0)
			{
				for (Tuple<TabDesign, int> t2 : tab2_candidates)
				{
					//c2List = unordered_set<TabDesign>(c1List);

					ref_position = t2.First().candidate._position;
					
					if (from(c2List).contains(t2.First()))
						c2List.erase(from(c2List).where([&](const TabDesign &v) { return v.candidate._position == ref_position; }).first());				//c2List.Remove((from v in c2List where v.candidate._position == ref_position select v).First());

					for (TabDesign td3 : c2List)
					{
						tab3ofFour = TabDesign();
						anglescoretemp3ofFour = t2.Second();

						if (abs(t2.First().candidate._row - td3.candidate._row) >= minRowDistSeparation || abs(t2.First().candidate._column - td3.candidate._column) >= minColDistSeparation)
						{
							if (abs(t2.First().candidate._row - td3.candidate._row) >= minRowDistSeparation && abs(t2.First().candidate._column - td3.candidate._column) >= minColDistSeparation)
								anglescoretemp3ofFour += 20;

							tab3ofFour = td3;
							if (td3.angle == t2.First().angle)
								angle_difference = 0;
							else if (td3.angle > t2.First().angle)
								angle_difference = td3.angle - t2.First().angle;
							else if (td3.angle < t2.First().angle)
								angle_difference = (td3.angle + 360.0) - t2.First().angle;

							if (angle_difference == applyconfig[0])
								anglescoretemp3ofFour += (int)applyconfig[1];
							else if (angle_difference == applyconfig[2])
								anglescoretemp3ofFour += (int)applyconfig[3];
							else if (angle_difference == applyconfig[4] || angle_difference == applyconfig[5])
								anglescoretemp3ofFour += (int)applyconfig[6];
							else if (angle_difference == applyconfig[7])
								anglescoretemp3ofFour += (int)applyconfig[8];
							else
								anglescoretemp3ofFour += (int)applyconfig[9];

							if (angle_difference == 0)
								tab3ofFour = TabDesign();
						}
						else
							anglescoretemp3ofFour += 0;

						if (abs(input.candidate.candidate._row - td3.candidate._row) < minRowDistSeparation || abs(input.candidate.candidate._column - td3.candidate._column) < minColDistSeparation)
						{
							tab3ofFour = TabDesign();
							anglescoretemp3ofFour = t2.Second();
						}

						if (tab3ofFour.candidate.isNull == false && anglescoretemp3ofFour != 0)
						{
							if (tab3_candidates.size() == 0)
								tab3_candidates.insert(Triple<TabDesign, TabDesign, int>(t2.First(), tab3ofFour, anglescoretemp3ofFour));
							else
							{
								int max_score = from(tab3_candidates).max([](const Triple<TabDesign, TabDesign, int> &p) {return p.Third(); }).Third();

								if (anglescoretemp3ofFour == max_score)
									tab3_candidates.insert(Triple<TabDesign, TabDesign, int>(t2.First(), tab3ofFour, anglescoretemp3ofFour));
								else if (anglescoretemp3ofFour > max_score)
								{
									tab3_candidates = set<Triple<TabDesign, TabDesign, int>>();
									tab3_candidates.insert(Triple<TabDesign, TabDesign, int>(t2.First(), tab3ofFour, anglescoretemp3ofFour));
								}
							}
						}
					}
				}
			}
			TabDesign tab4ofFour;
			int anglescoretemp4ofFour = 0;
			applyconfig = config1;
			c3List = c2List;
			if (tab3_candidates.size() == 0)
			{
				tab3_candidates.insert(Triple<TabDesign, TabDesign, int>(input.candidate, TabDesign(), 0));
				applyconfig = config2;
			}
			if (tab3_candidates.size() > 0)
			{
				for (Triple<TabDesign, TabDesign, int> t3 : tab3_candidates)
				{
					//c3List = unordered_set<TabDesign>(c1List);

					RHVector3 ref_position2;
					RHVector3 ref_position3;

					if (t3.First().candidate.isNull == false )
						ref_position2 = t3.First().candidate._position;
					if (t3.Second().candidate.isNull == false )
						ref_position3 = t3.Second().candidate._position;

					if (t3.First().candidate.isNull == false && from(c3List).contains(t3.First()))
						c3List.erase(from(c3List).where([&](const TabDesign &v) { return v.candidate._position == ref_position2; }).first());
					if (t3.Second().candidate.isNull == false && from(c3List).contains(t3.Second()))
						c3List.erase(from(c3List).where([&](const TabDesign &v) { return v.candidate._position == ref_position3; }).first());

					for (const TabDesign &td4 : c3List)
					{
						tab4ofFour = TabDesign();
						anglescoretemp4ofFour = t3.Third();

						if (t3.Second().candidate.isNull == false)
						{
							if (abs(t3.Second().candidate._row - td4.candidate._row) >= minRowDistSeparation || abs(t3.Second().candidate._column - td4.candidate._column) >= minColDistSeparation)
							{
								if (abs(t3.Second().candidate._row - td4.candidate._row) >= minRowDistSeparation && abs(t3.Second().candidate._column - td4.candidate._column) >= minColDistSeparation)
									anglescoretemp3ofFour += 20;

								if (td4.angle == t3.Second().angle)
									angle_difference = 0;
								else if (td4.angle > t3.Second().angle)
									angle_difference = td4.angle - t3.Second().angle;
								else if (td4.angle < t3.Second().angle)
									angle_difference = (td4.angle + 360.0) - t3.Second().angle;

								if (angle_difference == applyconfig[0])
									anglescoretemp3ofFour += (int)applyconfig[1];
								else if (angle_difference == applyconfig[2])
									anglescoretemp3ofFour += (int)applyconfig[3];

								else if (angle_difference == applyconfig[4] || angle_difference == applyconfig[5])
									anglescoretemp3ofFour += (int)applyconfig[6];
								else if (angle_difference == applyconfig[7])
									anglescoretemp3ofFour += (int)applyconfig[8];
								else
									anglescoretemp3ofFour += (int)applyconfig[9];

								if (abs(input.candidate.candidate._row - td4.candidate._row) >= minRowDistSeparation && abs(input.candidate.candidate._column - td4.candidate._column) >= minColDistSeparation)
								{
									anglescoretemp3ofFour += 20;

									tab4ofFour = td4;
									if (td4.angle == input.candidate.angle)
										angle_difference = 0;
									else if (td4.angle > input.candidate.angle)
										angle_difference = (input.candidate.angle + 360.0) - td4.angle;
									else if (td4.angle < input.candidate.angle)
										angle_difference = input.candidate.angle - td4.angle;

									if (angle_difference == applyconfig[0])
										anglescoretemp3ofFour += (int)applyconfig[1];
									else if (angle_difference == applyconfig[2])
										anglescoretemp3ofFour += (int)applyconfig[3];

									else if (angle_difference == applyconfig[4] || angle_difference == applyconfig[5])
										anglescoretemp3ofFour += (int)applyconfig[6];
									else if (angle_difference == applyconfig[7])
										anglescoretemp3ofFour += (int)applyconfig[8];
									else
										anglescoretemp3ofFour += (int)applyconfig[9];

									if (angle_difference == 0)
										tab4ofFour = TabDesign();
								}
								else
									anglescoretemp4ofFour += 0;

								if (abs(t3.First().candidate._row - td4.candidate._row) < minRowDistSeparation || abs(t3.First().candidate._column - td4.candidate._column) < minColDistSeparation)
								{
									tab4ofFour = TabDesign();
									anglescoretemp4ofFour = t3.Third();
								}
							}
							else
								anglescoretemp4ofFour += 0;
						}
						else
							anglescoretemp4ofFour += 0;

						if (tab4ofFour.candidate.isNull == false && anglescoretemp4ofFour != 0 && t3.Second().angle != td4.angle)
						{
							if (tab4_candidates.size() == 0)
								tab4_candidates.insert(Quadruple<TabDesign, TabDesign, TabDesign, int>(t3.First(), t3.Second(), tab4ofFour, anglescoretemp4ofFour));
							else
							{
								int max_score = from(tab4_candidates).max([](const Quadruple<TabDesign, TabDesign, TabDesign, int> &p) { return p.Fourth(); }).Fourth();
								if (anglescoretemp4ofFour == max_score)
									tab4_candidates.insert(Quadruple<TabDesign, TabDesign, TabDesign, int>(t3.First(), t3.Second(), tab4ofFour, anglescoretemp4ofFour));
								else if (anglescoretemp4ofFour > max_score)
								{
									tab4_candidates = set<Quadruple<TabDesign, TabDesign, TabDesign, int>>();
									tab4_candidates.insert(Quadruple<TabDesign, TabDesign, TabDesign, int>(t3.First(), t3.Second(), tab4ofFour, anglescoretemp4ofFour));
								}
							}
						}
					}
				}
			}
			int size, mid, median;
			if (tab4_candidates.size() > 0)
			{
				set<Quadruple<TabDesign, TabDesign, TabDesign, int>> partial_result4of4(from(tab4_candidates).where([&](const Quadruple<TabDesign, TabDesign, TabDesign, int> &v) { return v.Fourth() == from(tab4_candidates).max([](const Quadruple<TabDesign, TabDesign, TabDesign, int> &p) { return p.Fourth(); }).Fourth(); }).toStdSet());

				//set<Quadruple<TabDesign, TabDesign, TabDesign, int>> partial_result4of4 = set<Quadruple<TabDesign, TabDesign, TabDesign, int>>(tab4_candidates);

				//Console.WriteLine("4 of 4 Tabs number of results returned: " + partial_result4of4.Count());

				if (partial_result4of4.size() > 1)
				{
					size = partial_result4of4.size();
					mid = size / 2;
					median = (size % 2 != 0) ? mid : mid + (mid - 1) / 2;
					input.scoreAngleDifference = from(partial_result4of4).elementAt(median).Fourth();
					input.assoc_candidates.push_back(input.candidate);
					input.assoc_candidates.push_back(from(partial_result4of4).elementAt(median).First());
					input.assoc_candidates.push_back(from(partial_result4of4).elementAt(median).Second());
					input.assoc_candidates.push_back(from(partial_result4of4).elementAt(median).Third());
					input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
				}
				else if (partial_result4of4.size() == 1)
				{
					input.scoreAngleDifference = partial_result4of4.begin()->Fourth();
					input.assoc_candidates.push_back(input.candidate);
					input.assoc_candidates.push_back(partial_result4of4.begin()->First());
					input.assoc_candidates.push_back(partial_result4of4.begin()->Second());
					input.assoc_candidates.push_back(partial_result4of4.begin()->Third());
					input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
				}
				partial_result4of4.clear();
			}
			else if (tab3_candidates.size() > 0)
			{
				set<Triple<TabDesign, TabDesign, int>> partial_result3of4(from(tab3_candidates).where([&](const Triple<TabDesign, TabDesign, int> &v) { return v.Third() == from(tab3_candidates).max([](const Triple<TabDesign, TabDesign, int> &p) { return p.Third(); }).Third(); }).toStdSet());
				//set<Triple<TabDesign, TabDesign, int>> partial_result3of4 = set<Triple<TabDesign, TabDesign, int>>(tab3_candidates);
				if (partial_result3of4.size() > 1)
				{
					size = partial_result3of4.size();
					mid = size / 2;
					median = (size % 2 != 0) ? mid : mid + (mid - 1) / 2;
					input.scoreAngleDifference = from(partial_result3of4).elementAt(median).Third();
					input.assoc_candidates.push_back(input.candidate);
					input.assoc_candidates.push_back(from(partial_result3of4).elementAt(median).First());
					input.assoc_candidates.push_back(from(partial_result3of4).elementAt(median).Second());
					input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
				}
				else if (partial_result3of4.size() == 1)
				{
					input.scoreAngleDifference = partial_result3of4.begin()->Third();
					input.assoc_candidates.push_back(input.candidate);
					input.assoc_candidates.push_back(partial_result3of4.begin()->First());
					input.assoc_candidates.push_back(partial_result3of4.begin()->Second());
					input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;

				}
				partial_result3of4.clear();
			}
			else if (tab2_candidates.size() > 0)
			{
				set<Tuple<TabDesign, int>> partial_result2of4(from(tab2_candidates).where([&](const Tuple<TabDesign, int> &v) { return v.Second() == from(tab2_candidates).max([](const Tuple<TabDesign, int> &p) { return p.Second(); }).Second(); }).toStdSet());
				//set<Tuple<TabDesign, int>> partial_result2of4 = set<Tuple<TabDesign, int>>(tab2_candidates);
				if (partial_result2of4.size() > 1)
				{
					size = partial_result2of4.size();
					mid = size / 2;
					median = (size % 2 != 0) ? mid : mid + (mid - 1) / 2;
					input.scoreAngleDifference = from(partial_result2of4).elementAt(median).Second();
					input.assoc_candidates.push_back(input.candidate);
					input.assoc_candidates.push_back(from(partial_result2of4).elementAt(median).First());
					input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
				}
				else if (partial_result2of4.size() == 1)
				{
					input.scoreAngleDifference = partial_result2of4.begin()->Second();
					input.assoc_candidates.push_back(input.candidate);
					input.assoc_candidates.push_back(partial_result2of4.begin()->First());
					input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
				}
				partial_result2of4.clear();
			}
			else
			{
				//Console.WriteLine("Only 1 of 4 Tabs has been returned");
				input.scoreAngleDifference = 0;
				input.assoc_candidates.push_back(input.candidate);
				input.totalScore = input.scoreAreaSampling + input.scoreAngleDifference;
			}
			tab4_candidates.clear();
			break;
			//#endregion
		}	// case TabGenCount.FourTab: 
		
		}
		tab2_candidates.clear();
		tab3_candidates.clear();
		c1List.clear();
		c2List.clear();
		c3List.clear();
	}

	// compile error
	void SupportMeshsBed::FinalizeTabDesign(const vector<TabDesign> &candidateList)
	{
		//KPPH CP2-SW RCGREY 20190118
		for(const TabDesign &td : candidateList)
		{
			if (td.candidate.isNull == false)
			{
				switch (IndexOf(candidateList, td))
				{
				case 0:
					vTabs_Data.tab1.TabID = (int)EasyTabID::TabID1;
					vTabs_Data.tab1.tabLists_SelectedIndex = IndexOf(vTabs_Data.tabLists, from(vTabs_Data.tabLists).where([&](const TabPosResources &p) { return p.tabPosition._position == td.candidate._position; }).first());
			  	    vTabs_Data.tab1.filteredInfo = true;
					genTabModelData(td.candidate, td.angle, EasyTabID::TabID1);
					break;
				case 1:
					vTabs_Data.tab2.TabID = (int)EasyTabID::TabID2;
					vTabs_Data.tab2.tabLists_SelectedIndex = IndexOf(vTabs_Data.tabLists, from(vTabs_Data.tabLists).where([&](const TabPosResources &p) { return p.tabPosition._position == td.candidate._position; }).first());
					vTabs_Data.tab2.filteredInfo = true;
					genTabModelData(td.candidate, td.angle, EasyTabID::TabID2);
					break;
				case 2:
					vTabs_Data.tab3.TabID = (int)EasyTabID::TabID3;
					vTabs_Data.tab3.tabLists_SelectedIndex = IndexOf(vTabs_Data.tabLists, from(vTabs_Data.tabLists).where([&](const TabPosResources &p) { return p.tabPosition._position == td.candidate._position; }).first());
					vTabs_Data.tab3.filteredInfo = true;
					genTabModelData(td.candidate, td.angle, EasyTabID::TabID3);
					break;
				case 3:
					vTabs_Data.tab4.TabID = (int)EasyTabID::TabID4;
					vTabs_Data.tab4.tabLists_SelectedIndex = IndexOf(vTabs_Data.tabLists, from(vTabs_Data.tabLists).where([&](const TabPosResources &p) { return p.tabPosition._position == td.candidate._position; }).first());
					vTabs_Data.tab4.filteredInfo = true;
					genTabModelData(td.candidate, td.angle, EasyTabID::TabID4);
					break;
				}
			}
		}
	}

	int SupportMeshsBed::getMeanDistribBySampleArea(/*out*/ vector<TabDesign> &tD)
	{
		//KPPH CP2-SW RCGREY 20190118
		int meanIndex = 0;
		double median = 0;
		double average;

		int lowerleft, upperright;

		if (tD.size() == 0 || tD.size() == 1)
			return 0;

		int idx = 0;
		//if (TabFocus == Location.Bottom || TabFocus == Location.Top)
		tD = from(tD).orderBy([](const TabDesign &p) {return p.candidate._column; }).toStdVector();
		//else
			//tD = tD.OrderBy(p => p.candidate._row).ToList();

		tD = from(tD).select([&](TabDesign p) {p.index = idx; idx++; return p; }).toStdVector();		//C#: tD = tD.Select(p => { p.index = idx; idx++; return p; }).ToList();

		median = (double)(tD.size() + 1) / 2;

		double intpart;
		if (modf(median, &intpart) == 0.5)
		{
			lowerleft = from(tD).where([&](const TabDesign &p) {return (double)p.index < median - 1; }).last().index;
			upperright = from(tD).where([&](const TabDesign &p) {return (double)p.index > median - 1; }).first().index;

			average = from(tD).where([&](const TabDesign &p) {return (double)p.index == lowerleft; }).first().serviceAreaSample + 
						from(tD).where([&](const TabDesign &p) {return (double)p.index == upperright; }).first().serviceAreaSample / 2;

			meanIndex = abs(average - from(tD).where([&](const TabDesign &p) {return p.index == lowerleft; }).first().serviceAreaSample) <
						abs(average - from(tD).where([&](const TabDesign &p) {return p.index == upperright; }).first().serviceAreaSample) ?
						lowerleft : upperright;

			if (abs(average - from(tD).where([&](const TabDesign &p) {return p.index == lowerleft; }).first().serviceAreaSample) ==
					abs(average - from(tD).where([&](const TabDesign &p) {return p.index == upperright; }).first().serviceAreaSample) )
				meanIndex = from(tD).where([&](const TabDesign &p) {return p.index == lowerleft; }).first().serviceAreaSample >
					(from(tD).where([&](const TabDesign &p) {return p.index == upperright; }).first().serviceAreaSample) ? lowerleft : upperright;
		}
		else
			meanIndex = (int)median - 1;

		return meanIndex;
	}

	//double SupportMeshsBed::CalculateStdDev(IEnumerable<int> values)
	//{
	//	double ret = 0;
	//	if (values.Count() > 0)
	//	{
	//		//Compute the Average      
	//		double avg = (double)values.Average();
	//		//Perform the Sum of (value-avg)_2_2      
	//		double sum = (double)values.Sum(d = > Math.Pow(d - avg, 2));
	//		//Put it all together      
	//		ret = sqrt((sum) / (values.Count() - 1));
	//	}
	//	return ret;
	//}

	//For tab debugging and test purposes only..
	//#region
	void SupportMeshsBed::HighlightPrint(Cell CellTabCandidate, TabPosResources angleref)
	{
		string id = "xCell [" + to_string(CellTabCandidate._column) + "]" + " " + "[" + to_string(CellTabCandidate._row) + "]";
		auto query = from(_items2).where([&](const shared_ptr<SupportData> sm) {return sm->name == id; });		//C#: var query = from sm in _items2 where(sm.name == id) select sm;

		if (query.count() == 1)
		{
			vector<shared_ptr<SupportData>> result = query.toStdVector();
			int index = IndexOf(_items2, result[0]);
			_items2[index]->name = "orange";
		}
		//Console.WriteLine("row: " + CellTabCandidate._row + "  column: " + CellTabCandidate._column + "  area: " + angleref.serviceAreaSampling + "  angle: " + angleref.rotationInfo.First);
		//genTabModelData(CellTabCandidate, angleref.rotationInfo.First, EasyTabID.TabID1);
	}
	//#endregion

	void SupportMeshsBed::genTabModelData(Cell cellLocation, double angle, EasyTabID ID)
	//KPPH CP2-SW RCGREY 20180605 
	// generate the tab model data from the selected cell position and rotation value
	{
		/*
		SupportModel tab = new SupportModel(drawer);

		tab.name = "[Tab Cell]" + ((int)ID).ToString();
		tab.originalModel = ReferenceMesh.TabCell;
		tab.supType = (int)ModelType::TAB;
		tab.Position = Coord3D((float)cellLocation._position.x, (float)cellLocation._position.y, (float)cellLocation._position.z);
		double Offset = 0.0;

		if (angle % 60 == 0)
			Offset = 3.792738; //rotation along the CORNER of the cell
		else
			Offset = 4.0606872; // rotation along the SIDE of the cell

		tab.Position.x = (float)(cellLocation._position.x - (Offset * Math.Sin((angle * Math.PI) / 180)));
		tab.Position.y = (float)(cellLocation._position.y - (Offset * Math.Cos((angle * Math.PI) / 180)));
		//Added by ERVIN| Scales the old tab support to level with the hexagonal base cell
		tab.Scale.z = newBaseZScale;
		tab.Rotation.z = (float)angle;

		if (!debuggingMode)
		{
			//KPPH CP2 SW RCGREY 20190705
			tab.UpdateMatrix();
			vNewBase_Model.originalModel.Merge(tab.ActiveModel, tab.trans, null);
		}
		else
		{
			//Ervin 04-12-2019 Adds to the Support Data List
			tab = tab.SupportModelToSupportData(tab);
			_items2.push_back(tab.SupportData);
		}
		*/
		unique_ptr<SupportData> pSupportData(new SupportData());
		pSupportData->name = "[Tab Cell]" + to_string((int)ID);
		//pSupportData->originalModel = ReferenceMesh.TabCell();	// C++: Moved to GenMesh()
		pSupportData->supType = (int)ModelType::TAB;
		pSupportData->SetPosition(Coord3D((float)cellLocation._position.x, (float)cellLocation._position.y, (float)cellLocation._position.z));
		double Offset = 0.0;

		if (fmod(angle, 60) == 0)
			Offset = 3.792738; //rotation along the CORNER of the cell
		else
			Offset = 4.0606872; // rotation along the SIDE of the cell

		float tempTabPosX = (float)(cellLocation._position.x - (Offset * sin((angle * M_PI) / 180.0)));
		float tempTabPosY = (float)(cellLocation._position.y - (Offset * cos((angle * M_PI) / 180.0)));
		pSupportData->SetPosition(Coord3D(tempTabPosX, tempTabPosY, (float)cellLocation._position.z));
		//Added by ERVIN| Scales the old tab support to level with the hexagonal base cell
		pSupportData->SetScale(Coord3D(pSupportData->GetScale().x, pSupportData->GetScale().y, newBaseZScale));
		pSupportData->SetRotation(Coord3D(pSupportData->GetRotation().x, pSupportData->GetRotation().y, (float)angle));
		pSupportData->UpdateMatrix();
		_items2.push_back(move(pSupportData));
	}

	vector<int> SupportMeshsBed::CheckCellsAdjacentoEachSides(const Cell &cellPosition)
	{
		//KPPH CP2-SW RCGREY 20180605 
		// check each side of the cell has adjacent cells beside it. no adjacent cell means space
		// collect data to further analyze validity of free space - this data will be useful to determine
		// the orientation angle of the tab design for each cell candidate
		vector<int> result;
		result.push_back(1);
		//side 1
		if (from(xBedCells).where([&](const Cell &v) { return v._row == cellPosition._row - 1 && v._column==cellPosition._column - 1;}).count() > 0)
			result.push_back(1);
		else
			result.push_back(0);

		//side 2
		if (from(xBedCells).where([&](const Cell &v) { return v._row == cellPosition._row && v._column == cellPosition._column - 2; }).count() > 0)
			result.push_back(1);
		else
			result.push_back(0);

		//side 3
		if (from(xBedCells).where([&](const Cell &v) { return v._row == cellPosition._row + 1 && v._column == cellPosition._column - 1; }).count() > 0)
			result.push_back(1);
		else
			result.push_back(0);

		//side 4
		if (from(xBedCells).where([&](const Cell &v) { return v._row == cellPosition._row + 1 && v._column == cellPosition._column + 1; }).count() > 0)
			result.push_back(1);
		else
			result.push_back(0);

		//side 5
		if (from(xBedCells).where([&](const Cell &v) { return v._row == cellPosition._row && v._column == cellPosition._column + 2; }).count() > 0)
			result.push_back(1);
		else
			result.push_back(0);

		//side 6
		if (from(xBedCells).where([&](const Cell &v) { return v._row == cellPosition._row - 1 && v._column == cellPosition._column + 1; }).count() > 0)
			result.push_back(1);
		else
			result.push_back(0);

		return result;
	}

	TabPosResources SupportMeshsBed::GetTabRotateValue(vector<int> &directions, Cell &testSample, bool limitfocus)
	//KPPH CP2-SW RCGREY 20180605 
	//analyzes the result from the list of density of cells for each side of the cell candidate
	//and look for openings (space) outward. ("directions" listed elements with zero value indicate a "space" or "opening")
	//analyze the openings to determine the rotation value to  
	//position the tab outward the support bed group
	//(index of "directions" list indicates the side of the cell)
	{
		Triple<double, int, double> serviceAreaSampling;
		TabPosResources returnTabInfo;
		returnTabInfo.tabPosition = testSample;
		returnTabInfo.density = directions;
		returnTabInfo.rotationInfo = Tuple<double, RHVector3>(999.0, RHVector3()); //invalid return angle value 

		vector<Triple<double,int,double>> angleList;
		double centerPosition = 0.0;
		vector<int> sideList;

		sequencetracker tracker = sequencetracker::off;

		// index values 1 to 6 refers to the sides of the candidate cell
		for (int i = 0; i < directions.size(); i++)
		{
			if (directions[i] == 0) // cell opens outward??
				sideList.push_back(i);
		}

		if (sideList.size() == 0)
			return returnTabInfo;
			//this indicates the cell has no available space for tab position 
			//thus is not a good candidate for tab position;

		//arrange sequentially identified sides - combine sequential sides with zero density 
		vector<int> sequentialList;
		//bool seqncebroken = false;

		if (from(sideList).contains(6) && from(sideList).contains(1))
		{
			for (int i = 0; i < sideList.size(); i++)
			{
				if (sideList[0] == 1 || (sideList[i] - (sideList[i - 1] - 6) == 1))
					sideList[i] += 6;
				else
					break;
			}

			//sort updated list
			sideList = from(sideList).orderBy([](int p) {return p; }).toStdVector();
		}
		double lead = 0.0;
		double trail = 0.0;
		double first = 0.0;
		double last = 0.0;
		bool sequenceDetected = false;
		for(const int side : sideList)
		{
			int tempSide = side > 6 ? side - 6 : side;
			int index = IndexOf(sideList, side);

			if (limitfocus == false) //depending on the bed cell count.. relax focus when tab generate is 1 or 2
			{  // consider each side in evaluation
				serviceAreaSampling = GetServiceAreaSampling(tempSide, testSample);
				//service Area Sampling = Triple<tab position, service area count, angle>

				if (serviceAreaSampling.Second() > 0)
				{
					//if (angleList.Where(p => p.First == tempSide).Count() == 0)
					//	angleList.Add(serviceAreaSampling);
					if (from(angleList).where([&](const Triple<double, int, double> &p) { return p.First() == tempSide;}).count() == 0)
						angleList.push_back(serviceAreaSampling);
				}
			}
			tempSide = side;

			sequentialList.push_back(tempSide);

			if (sequentialList.size() == 1)
				first = trail = lead = (double)sequentialList.front();
			else if (sequentialList.back() - trail == 1)
			{
				last = trail = (double)sequentialList.back();
				tracker = sequencetracker::started;
			}
			else //end of sequence
			{
				//seqncebroken = true;
				if (tracker == sequencetracker::started)
					tracker = sequencetracker::ended;
			}

			if (tracker != sequencetracker::ended && sequentialList.size() > 1 && limitfocus == false)
				sequenceDetected = true;

			if (IndexOf(sideList, side) == sideList.size() - 1 && sideList.size() > 1 && tracker > (int)sequencetracker::off)
				//seqncebroken = true;
				tracker = sequencetracker::ended;

			//analyze selected side/s
			if ( (tracker == sequencetracker::ended && from(sequentialList).any()) || (sideList.size() - 1 == IndexOf(sideList, side)) || sequenceDetected)
			{
				//determine possible corners for tab position
				centerPosition = (lead + trail) / 2.0;

				// center position with +0.5 value indicates the rotation 
				// is pointing across a CORNER position of the cell
				// (CORNER is between two adjoining sides)
				// other whole number values indicates that the                    
				// TAB is pointing across the SIDE position of the CELL
				if (centerPosition == 6.5) centerPosition = 6.5;
				else if (centerPosition > 6.5) centerPosition -= 6.0;

				//acquire the corresponding angle of position based on the prepared list
				/*Triple<double, int, double>*/ serviceAreaSampling = GetServiceAreaSampling(centerPosition, testSample);
				//service Area Sampling = Triple<tab position, service area count, angle>

				if (serviceAreaSampling.Second() > 0)
				{
					if (from(angleList).where([&](const Triple<double, int, double> &v) { return v.First() == centerPosition; }).count() == 0)
						angleList.push_back(serviceAreaSampling);
				}

				if (tracker == sequencetracker::ended)
				{
					centerPosition = (first + last) / 2.0;
					if (centerPosition == 6.5) centerPosition = 6.5;
					else if (centerPosition > 6.5) centerPosition -= 6.0;
					serviceAreaSampling = GetServiceAreaSampling(centerPosition, testSample);
					if (serviceAreaSampling.Second() > 0)
					{
						if (from(angleList).where([&](const Triple<double, int, double> &v) { return v.First() == centerPosition; }).count() == 0)
							angleList.push_back(serviceAreaSampling);
					}
					sequentialList.clear();
					//seqncebroken = false;
					tracker = sequencetracker::off;
				}
				else if (sequenceDetected)
					lead = trail;

				sequenceDetected = false;
			}
		}
		//subject each angle from the angle list to determine which orientation and the tab design that will be applied for each candidate cell
		if (angleList.size() > 0)
		{
			vector<double> filteredList = from(angleList).select([](const Triple<double, int, double> &v) { return v.Third(); }).toStdVector();
			vector<Tuple <double, RHVector3>> rotationResult = ValidateSuppliedRotationValue(testSample, filteredList);	//***********************************

			if (rotationResult.size() > 0)
			{
				vector<Quadruple<double, int, double, RHVector3>> finalList;
				//final list = side, service area, angle, endpoint
				for(Tuple<double, RHVector3> t : rotationResult)
				{
					Triple<double, int, double> a = from(angleList).where([&](const Triple<double, int, double> &v) { return v.Third() == t.First(); }).first();
					finalList.push_back(Quadruple<double, int, double, RHVector3>(a.First(), a.Second(), a.Third(), t.Second()));
				}
				int maxarea = from(finalList).max([](const Quadruple<double, int, double, RHVector3> &p) {return p.Second(); }).Second();
				Quadruple<double, int, double, RHVector3> selectedValue = from(finalList).where([&](const Quadruple<double, int, double, RHVector3> &v) { return v.Second() == maxarea; }).first();
				
				returnTabInfo.rotationInfo = Tuple<double, RHVector3>(selectedValue.Third(), selectedValue.Fourth());
				returnTabInfo.serviceAreaSampling = selectedValue.Second();
				returnTabInfo.sidelocation = selectedValue.First();
			}
		}
		return returnTabInfo;
	}

	Triple<double, int, double> SupportMeshsBed::GetServiceAreaSampling(double selectedPosition, Cell subjectCell)
	{
		//sampling Area is a method of acquiring information on how dense is the area OPPOSITE from
		//each selected side of the candidate cell for tab design
		//this will be helpful in selecting the final tab design by acquiring the bigger area.
		// an easy remove tab design can serve or be efficient.
		Triple<double, int, double> result;
		double opposite = selectedPosition > 3.5 ? (selectedPosition - 3) : (selectedPosition + 3);
		int count = 0;
		int cntmp = 0;
		int cref;
		double angle = TabAnglePosition[selectedPosition];
		int maxrow = from(xBedCells).max([](const Cell &p) {return p._row; })._row;
		int minrow = from(xBedCells).min([](const Cell &p) {return p._row; })._row;
		int mincol = from(xBedCells).min([](const Cell &p) {return p._column; })._column;
		int maxcol = from(xBedCells).max([](const Cell &p) {return p._column; })._column;

		if (opposite == 1)
		{
			//#region service area adjacent to side1
			int mincol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).min([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column - 2; i >= mincol;)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i -= 2;
				}
			}
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			//#endregion
		}
		else if (opposite == 2)
		{
			//#region service area adjacent to side2
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			mincol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).min([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column - 2; i >= mincol;)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i -= 2;
				}
			}
			//#endregion
		}
		else if (opposite == 3)
		{
			//#region service area adjacent to side3
			mincol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).min([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column - 2; i >= mincol;)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i -= 2;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			//#endregion
		}
		else if (opposite == 4)
		{
			//#region service area adjacent to side4
			maxcol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).max([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column + 2; i <= maxcol;)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i += 2;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			//#endregion
		}
		else if (opposite == 5)
		{
			//#region service area adjacent to side5
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			maxcol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).max([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column + 2; i <= maxcol;)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i += 2;
				}
			}
			//#endregion
		}
		else if (opposite == 6)
		{
			//#region service area adjacent to side6
			maxcol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).max([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column + 2; i <= maxcol; )
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i += 2;
				}
			}
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			//#endregion
		}
		else if (opposite == 1.5)
		{
			//#region service area adjacent to corner1.5
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				from(xBedCells).where([&](const Cell &p) {return  p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			mincol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).min([](const Cell &p) { return p._column; })._column;
			for (int i = subjectCell._column - 2; i >= mincol;)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i -= 2;
				}
			}
			//#endregion
		}
		else if (opposite == 2.5)
		{
			//#region service area adjacent to corner2.5
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			mincol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).min([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column - 2; i >= mincol; )
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i -= 2;
				}
			}
			//#endregion
		}
		else if (opposite == 3.5)
		{
			//#region service area adjacent to corner 3.5
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			for (int i = subjectCell._row + 1; i <= maxrow; )
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column - 1; }).count() +
					from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + 1; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += 1;
					i++;
					cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column; }).count();
					if (cntmp == 0)
						break;
					else
					{
						count += cntmp;
						i++;
					}
				}
			}
			//#endregion
		}
		else if (opposite == 4.5)
		{
			//#region service area adjacent to corner 4.5
			maxcol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).max([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column + 2; i <= maxcol; )
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i += 2;
				}
			}
			cref = 1;
			for (int i = subjectCell._row + 1; i <= maxrow; i++)
			{
				cntmp = from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					cref++;
					count += cntmp;
				}
			}
			//#endregion
		}
		else if (opposite == 5.5)
		{
			//#region service area adjacent to corner 5.5
			maxcol = from(xBedCells).where([&](const Cell &p) {return p._row == subjectCell._row; }).max([](const Cell &p) {return p._column; })._column;
			for (int i = subjectCell._column + 2; i <= maxcol; )
			{
				cntmp = from(xBedCells).where([&](Cell p) {return p._row == subjectCell._row && p._column == i; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					i += 2;
				}
			}

			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](Cell p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			//#endregion
		}
		else if (opposite == 6.5) //corner @zero
		{
			//#region service area adjacent to corner 0
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](Cell p) {return p._row == i && p._column == subjectCell._column + cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			cref = 1;
			for (int i = subjectCell._row - 1; i >= minrow; i--)
			{
				cntmp = from(xBedCells).where([&](Cell p) {return p._row == i && p._column == subjectCell._column - cref; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += cntmp;
					cref++;
				}
			}
			for (int i = subjectCell._row - 1; i >= minrow; )
			{
				cntmp = from(xBedCells).where([&](Cell p) {return p._row == i && p._column == subjectCell._column - i; }).count() +
					from(xBedCells).where([&](Cell p) {return p._row == i && p._column == subjectCell._column + 1; }).count();
				if (cntmp == 0)
					break;
				else
				{
					count += 1;
					i--;
					cntmp = from(xBedCells).where([&](Cell p) {return p._row == i && p._column == subjectCell._column; }).count();
					if (cntmp == 0)
						break;
					else
					{
						count += cntmp;
						i--;
					}
				}
			}
			//#endregion
		}
		return result = Triple<double, int, double>(selectedPosition, count, angle);
	}

	vector<Tuple<double, RHVector3>> SupportMeshsBed::ValidateSuppliedRotationValue(Cell cellposition, const vector<double> &angleList)
	{ //KPPH CP2 SW RCGREY 20180904
		// - validate and return selected tab rotation angle, tab's tip end position
		// - check if tab end position has approached the edge of the platform
		// - check if tab design position is surrounded or not by other cells of the new base
		// - subject tab end position to scraper line test - checks perpendicular path of the tab 
		//                                                   with respect to surrounding cells of the new base
		//    *if the new base blocks the path of the scraper line before it reaches the proposed tab design
		//       the tab design will be considered invalid and be removed from the proposed list.
		// - check if the tab position has a notch. A notch will invalidate the tab design
		// all passed angle in a list will be returned to select final angle for tab design
		vector<Tuple<double, RHVector3>> result;

		RHVector3 endpoint;
		double offset = 0.0;
		double xtab, ytab;

		for(const double angle : angleList)
		{
			if ((int)angle % 60 == 0)
				offset = 9.5854761; //length of tab exceeding from the corner of a cell candidate
			else
				offset = 9.8534271; //length of tab exceeding from the side of a cell candidate

			//tab's end tip/point position
			xtab = cellposition._position.x - (offset * sin((angle * M_PI) / 180.0));
			ytab = cellposition._position.y - (offset * cos((angle * M_PI) / 180.0));

			//check tab exceeds platform settings               
			if (xtab >= printerWidth || xtab <= 0.0 || ytab >= printerDepth || ytab <= 0.0)
				continue;

			endpoint = RHVector3(xtab, ytab, 0.0);

			//check if tab is surrounded by other cells
			if (IsTabSurrounded(cellposition, endpoint))
				continue;
			//check if scraper line will be blocked by cell
			if (IsScraperLineBlocked(cellposition, endpoint))
				continue;
			//check if there is a notch in the tab
			if (IsInAGutter(cellposition, angle))
				continue;
			result.push_back(Tuple<double, RHVector3>(angle, endpoint));
		}
		return result;
	}

	bool SupportMeshsBed::IsTabSurrounded(Cell cellOrigin, /*out*/RHVector3 &endpoint)
	{
		//KPPH CP2 SW RCGREY 20180903
		// Check if tab position is within the new base or is surrounded by other cells
		bool result = false;
		TopoTriangle triModel;// = null;
		RHVector3 vertical(0, 0, -1);
		RHVector3 direction;// = null;
		RHVector3 currentcell = cellOrigin._position.Subtract(RHVector3(0,0,0));

		//double u, v, t;

		//check if new tab position is within surrounding cells
		endpoint.z = 1;
		currentcell.z = 1;

#if 0	// It seems there is NO object in _items. Deleted the code.
		foreach(SupportModel sm in _items.Where(i = > i.name.Contains("xCell")))
		{
			foreach(TopoTriangle ts in sm.ActiveModel.triangles.triangles)
			{
				if (sm.Position.Equals(cellOrigin._position))
					continue;

				sm.ActiveModel.getTriInWorld(sm.trans, ts, out triModel);

				direction = currentcell.Subtract(new RHVector3(sm.Position.x, sm.Position.y, sm.Position.z));

				direction.NormalizeSafe();

				result = triModel.IntersectsLineTest(endpoint, direction, out t, out u, out v);

				if (result == true)
					break;
			}

			if (result == true)
				break;
		}
#endif
		return result;
	}

	bool SupportMeshsBed::IsScraperLineBlocked(Cell cellposition, RHVector3 endpoint)
	{
		//KPPH CP2 SW RCGREY 20180903
		//the line perpendicular to the end point of the tab design is tested if there will
		//cells that will intersect to this line - this line is the path of the scraper tool
		//the scraper tool must atleast touch first the easy remove tab before any cell from 
		//the new base is encountered
		bool result = false;
		Intersection intersects = Intersection::None;
		Line scraperLine = Geometry::GetPerpendicularScraperLine(cellposition._position, endpoint);
		if (scraperLine.GetP1() != PointF(-999.0, -999.0) || scraperLine.GetP2() != PointF(-999.0, -999.0))	// C#: if (scraperLine.P1 != null || scraperLine.P2 != null)
		{
			for(const Cell &bc : xBedCells)
			{
				vector<PointF> cellPolygonPoints;
				// Top-Middle Point
				cellPolygonPoints.push_back(PointF((float)bc._position.x, (float)bc._position.y - (baseCellStyle_Depth / 2.0f)));
				// Top-Right Point
				cellPolygonPoints.push_back(PointF((float)bc._position.x - (baseCellStyle_Width / 2.0f), (float)bc._position.y - (baseCellStyle_Depth / 2.0f) + 1.0f));
				// Bottom-Right Point
				cellPolygonPoints.push_back(PointF((float)bc._position.x - (baseCellStyle_Width / 2.0f), (float)bc._position.y + (baseCellStyle_Depth / 2.0f) - 1.0f));
				// Bottom-Middle Point
				cellPolygonPoints.push_back(PointF((float)bc._position.x, (float)bc._position.y + (baseCellStyle_Depth / 2.0f)));
				// Bottom-Left Point
				cellPolygonPoints.push_back(PointF((float)bc._position.x + (baseCellStyle_Width / 2.0f), (float)bc._position.y + (baseCellStyle_Depth / 2.0f) - 1.0f));
				// Top-Left Point
				cellPolygonPoints.push_back(PointF((float)bc._position.x + (baseCellStyle_Width / 2.0f), (float)bc._position.y - (baseCellStyle_Depth / 2.0f) + 1.0f));
		
				Polygon cellPolygon(cellPolygonPoints);
				intersects = Geometry::IntersectionOf(scraperLine, cellPolygon);
				if (intersects == Intersection::None)
					continue;
				else
					break;
			}
		}
		if (intersects != Intersection::None)
			result = true;
		
		return result;
	}

	bool SupportMeshsBed::IsInAGutter(const Cell &cellposition, double angle)
	{
		//KPPH CP2 SW RCGREY 20180903
		//check if the cell is compressed between cells or if there is a notch or overlapping cells
		//that may reduce the effectiveness of the tab design
		bool result = false;

		int minrow = from(xBedCells).min([](const Cell &p) {return p._row; })._row;
		int maxrow = from(xBedCells).max([](const Cell &p) {return p._row; })._row;
		int mincol = from(xBedCells).min([](const Cell &p) {return p._column; })._column;
		int maxcol = from(xBedCells).max([](const Cell &p) {return p._column; })._column;

		int refcol = 0;
		int refrow = 0;
		int cntr = 0;
		switch ((int)angle)
		{
		case 0:
			//#region @angle 0
			for (int i = 3; i <= 6; i++)
			{
				if(	from(xBedCells).where([&](const Cell &p) {return p._column == cellposition._column + i && (p._row == cellposition._row - 1 || p._row == cellposition._row - 2); }).count() > 0 ||
					from(xBedCells).where([&](const Cell &p) {return p._column == cellposition._column - i && (p._row == cellposition._row - 1 || p._row == cellposition._row - 2); }).count() > 0)
				{
					result = true;
					break;
				}
			}
			//#endregion
			break;
		case 30:
			//#region @angle 30
			cntr = 1;
			for (int i = cellposition._column - 4; i >= mincol; )
			{
				if(from(xBedCells).where([&](const Cell &p) {return p._row == cellposition._row && p._column == i; }).count() > 0)
				{
					result = true;
					break;
				}
				i -= 2;
				if (cntr == 4)
					break;
				cntr++;
			}
			if (result == false)
			{
				refcol = 2;
				cntr = 1;
				for (int i = cellposition._row - 2; i >= minrow; i--)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == cellposition._column - refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					if (cntr == 4)
						break;
					cntr++;
				}
			}
			//#endregion
			break;
		case 60:
			//#region @angle 60
			refcol = 1;
			refrow = 0;
			for (int i = cellposition._row + 1; i <= maxrow; i++)
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column < cellposition._column - refcol; }).count() > 0)
				{
					result = true;
					break;
				}
				refcol++;
				refrow++;
				if (refrow == 3)
					break;

			}
			refcol = 1;
			refrow = 0;
			if (result == false)
			{
				for (int i = cellposition._row - 1; i >= minrow; i--)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column < cellposition._column + refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					refrow++;
					if (refrow == 3)
						break;
				}
			}
			//#endregion
			break;
		case 90:
			//#region @angle 90
			refcol = 0;
			for (int i = 2; i <= 4; i++)
			{
				if (i % 2 == 0)
					refcol = 0;
				else
					refcol = +1;

				if (from(xBedCells).where([&](const Cell &v) {return v._column < cellposition._column + refcol &&
										(v._row == cellposition._row - i || v._row == cellposition._row + i); }).count() > 0)
				{
					result = true;
					break;
				}
			}
			//#endregion
			break;
		case 120:
			//#region @angle 120
			refcol = 1;
			refrow = 0;
			for (int i = cellposition._row + 1; i <= maxrow; i++)
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column < cellposition._column + refcol; }).count() > 0)
				{
					result = true;
					break;
				}
				refcol++;
				refrow++;
				if (refrow == 3)
					break;
			}
			if (result == false)
			{
				refcol = 1;
				refrow = 0;
				for (int i = cellposition._row; i >= mincol; i--)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column < cellposition._column - refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					refrow++;
					if (refrow == 3)
						break;
				}
			}
			//#endregion
			break;
		case 150:
			//#region @angle 150
			cntr = 1;
			for (int i = cellposition._column - 4; i >= mincol; )
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == cellposition._row && p._column == i; }).count() > 0)
				{
					result = true;
					break;
				}
				i -= 2;
				if (cntr == 4)
					break;
				cntr++;
			}
			if (result == false)
			{
				refcol = 2;
				cntr = 1;
				for (int i = cellposition._row + 2; i <= maxrow; i++)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == cellposition._column + refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					if (cntr == 4)
						break;
					cntr++;
				}
			}
			//#endregion
			break;
		case 180:
			//#region @angle 180
			for (int i = 3; i <= 6; i++)
			{
				if (from(xBedCells).where([&](const Cell &p) {return (p._row == cellposition._row + 1 || p._row == cellposition._row + 2) && p._column == cellposition._column + i; }).count() > 0||
					from(xBedCells).where([&](const Cell &p) {return (p._row == cellposition._row + 1 || p._row == cellposition._row + 2) && p._column == cellposition._column - i; }).count() > 0)
				{
					result = true;
					break;
				}
			}
			//#endregion
			break;
		case 210:
			//#region @angle 210
			cntr = 1;
			for (int i = cellposition._column + 4; i <= maxcol; )
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == cellposition._row  && p._column == i; }).count() > 0)
				{
					result = true;
					break;
				}
				i += 2;
				if (cntr == 4)
					break;
				cntr++;
			}
			if (result == false)
			{
				refcol = 2;
				cntr = 1;
				for (int i = cellposition._row + 2; i <= maxrow; i++)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == cellposition._column - refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					if (cntr == 4)
						break;
					cntr++;
				}
			}
			//#endregion
			break;
		case 240:
			//#region @angle 240
			refcol = 1;
			refrow = 0;
			for (int i = cellposition._row + 1; i <= maxrow; i++)
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column > cellposition._column - refcol; }).count() > 0)
				{
					result = true;
					break;
				}
				refcol++;
				refrow++;
				if (refrow == 3)
					break;
			}
			if (result == false)
			{
				refcol = 1;
				refrow = 0;
				for (int i = cellposition._row - 1; i >= minrow; i--)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column > cellposition._column + refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					refrow++;
					if (refrow == 3)
						break;
				}
			}
			//#endregion
			break;
		case 270:
			//#region @angle 270
			refcol = 0;
			for (int i = 2; i <= 4; i++)
			{
				if (i % 2 == 0)
					refcol = 0;
				else
					refcol = -1;
				
				if (from(xBedCells).where([&](const Cell &v) {return v._column > cellposition._column + refcol &&
							(v._row == cellposition._row + i || v._row == cellposition._row - i); }).count() > 0)
				{
					result = true;
					break;
				}
			}
			//#endregion
			break;
		case 300:
			//#region @angle 300
			refcol = 1;
			refrow = 0;
			for (int i = cellposition._row + 1; i <= maxrow; i++)
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column > cellposition._column + refcol; }).count() > 0)
				{
					result = true;
					break;
				}
				refcol++;
				refrow++;
				if (refrow == 3)
					break;

			}
			refcol = 1;
			refrow = 0;
			for (int i = cellposition._row - 1; i >= minrow; i--)
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column > cellposition._column - refcol; }).count() > 0)
				{
					result = true;
					break;
				}
				refcol++;
				refrow++;
				if (refrow == 3)
					break;
			}

			//#endregion
			break;
		case 330:
			//#region @angle 330
			cntr = 1;
			for (int i = cellposition._column + 4; i <= maxcol; )
			{
				if (from(xBedCells).where([&](const Cell &p) {return p._row == cellposition._row  && p._column == i; }).count() > 0)
				{
					result = true;
					break;
				}
				i += 2;
				if (cntr == 4)
					break;
				cntr++;
			}
			if (result == false)
			{
				refcol = 2;
				cntr = 1;
				for (int i = cellposition._row - 2; i >= minrow; i--)
				{
					if (from(xBedCells).where([&](const Cell &p) {return p._row == i && p._column == cellposition._column - refcol; }).count() > 0)
					{
						result = true;
						break;
					}
					refcol++;
					if (cntr == 4)
						break;
					cntr++;
				}
			}
			//#endregion
			break;
		}
		return result;
	}

	bool SupportMeshsBed::lineIntersect(const vector<Line> &lines, Line line)
	{
		bool Intersect = false;

		for (int i = 0; i < lines.size(); i++)
		{
			if (lines[i].MidPoint() == line.MidPoint()) continue;

			if (lines[i].GetP1() == line.GetP1() || lines[i].GetP1() == line.GetP2() || lines[i].GetP2() == line.GetP2() || lines[i].GetP2() == line.GetP1()) continue;

			//if (Geometry.IntersectionOf(lines[i], line) != Intersection.None)
			//{
			//    Cuboids[i] = new Cuboid(Cuboids[i]._Start, Cuboids[i]._End, Cuboids[i]._MidPoint, true);
			//    Intersect = true;
			//    break;
			//}
		}
		return Intersect;
	}

	void SupportMeshsBed::GenMesh(/*in*//*out*/shared_ptr<SupportData> &pSupportData)
	{
		if (pSupportData->supType == (int)ModelType::BED_CELL)
		{
			switch(baseCellxStyle)
			{
			case BaseCellStyle::STYLE1:
				pSupportData->originalModel = ReferenceMesh.BedCellStyle1();
				break;
			case BaseCellStyle::STYLE2:
				pSupportData->originalModel = ReferenceMesh.BedCellStyle2();
				break;
			case BaseCellStyle::STYLE3:
				pSupportData->originalModel = ReferenceMesh.BedCellStyle3();
				break;
			case BaseCellStyle::STYLE4:
				pSupportData->originalModel = ReferenceMesh.BedCellStyle4();
				break;
			}
		}
		else if (pSupportData->supType == (int)ModelType::CUBOID_CELL)
		{
			pSupportData->originalModel = ReferenceMesh.CuboidCell();
		}
		else if (pSupportData->supType == (int)ModelType::TAB)
		{
			pSupportData->originalModel = ReferenceMesh.TabCell();
		}
		return;
	}

	void SupportMeshsBed::setSupIntoClusters()
	{

		vector<PointF> tempList = xSupportFootPoints;
		double bboxThreshold;
		double tabLength = 9.8534271;
		double bboxXSize = bbox.maxPoint.x - bbox.minPoint.x;
		double bboxYSize = bbox.maxPoint.y - bbox.minPoint.y;
		bboxThreshold = (bboxYSize < bboxXSize ? bboxXSize : bboxYSize) / 10;
		clusteringThreshold = (int)ceil(bboxThreshold > tabLength ? bboxThreshold : tabLength) + 3; //avoid tab inserting to honey comb base // 3 is honeycomb size(2.4)
		//DbgMsg(__FUNCTION__"===> clusteringThreshold: %d", clusteringThreshold);
		while (tempList.size() > 0)
		{
			//DbgMsg(__FUNCTION__"===> tempListSize: %d", tempList.size());
			clusteredPoints.clear();

			bool keepFinding = true;
			clusteredPoints.push_back(tempList[0]);

			while (keepFinding)
			{
				keepFinding = false;

				for (int i = clusteredPoints.size() - 1; i >= 0; i--)//從最後加入的點找
				{
					bool belowThreshold = false;

					for (auto &supFootPoint : tempList)
					{

						if (abs(sqrt(pow(clusteredPoints[i].x - supFootPoint.x, 2) + pow(clusteredPoints[i].y - supFootPoint.y, 2))) < clusteringThreshold)//below threshold
						{
							belowThreshold = true;
							clusteredPoints.push_back(supFootPoint);
							tempList.erase(tempList.begin() + IndexOf(tempList, supFootPoint));
							break;
						}

					}

					if (belowThreshold)
					{
						keepFinding = true;
						break;
					}
				}
			}
			//DbgMsg(__FUNCTION__"===> clusteredPointsSize: %d", clusteredPoints.size());
			if (clusteredPoints.size() > 0)
				xClusteringOfSupPoints.push_back(clusteredPoints);
		}
		DbgMsg(__FUNCTION__"===> xClusteringOfSupPointsSize: %d", xClusteringOfSupPoints.size());
	}

}