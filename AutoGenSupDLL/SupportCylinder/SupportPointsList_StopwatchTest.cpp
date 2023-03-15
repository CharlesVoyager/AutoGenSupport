/*
Referenced from Trunk - XYZware SVN: 11706
*/
#include "stdafx.h"
#include <map>
#include "SupportPointsList.h"
#include "SupportPointsListDbg.h"
#include "DataCombination.h"
#include "TopoModel.h"
#include "clipper.hpp"
#include "getdatafromSVG.h"
#include "GetMeshfromSVG.h"
#include "GetSegmentsfromSVG.h"

using namespace ClipperLib;

namespace XYZSupport
{
	void SupportPointsList::ToFile(const string filename)
	{
		TextFile textFile(filename);
		if(textFile.IsSuccess() == false) return;
		textFile.Write("PointsList Number: %d\n", _items.size());

		for (int i = 0; i < _items.size(); i++)
		{
			textFile.Write("SupportPoint #%d: %s %s %f\n", i, _items[i]->GetPosition().ToString().c_str(), _items[i]->GetDirection().ToString().c_str(), _items[i]->GetTouchRadius());
		}
	}

	void SupportPointsList::GenerateAuto(	const wstring density,			// "LOW", "MED", "HIGH"
											double contactSize,				// the radius of tocuh point. Defualt is 0.4mm.
											const vector<int> &thresholds,
											const unsigned char *stlBufferWor,
											const wstring wTempFolder,		// Temp Folder for stroing pictureCpp.stl
											const wstring pathForSlicerDll,
											const wstring pathForXYZMeshDll)
	{
		DbgMsg(__FUNCTION__"[IN]");
		vecSw.resize(50);

		vector<Triple<double, RHVector3, RHVector3>> candSupPosList;	// TouchRadius / Position / Direction
		Clear();

		Status = STATUS::Busy;

		wstring wFilename(L"pictureCpp");
		wstring pathForPictureStl = wTempFolder + L"\\" + wFilename + L".stl";

		PolygonPoint xyMinLaction;
		if (false == ReducedTriangles(stlBufferWor, pathForXYZMeshDll, pathForPictureStl, xyMinLaction))
		{
			DbgMsg(__FUNCTION__"[ERROR] Reduced Facets failure..");
			return;
		}
		DbgMsg(__FUNCTION__"==> xyMinLaction: %f, %f", xyMinLaction.x, xyMinLaction.y);

		Stopwatch sw;
		sw.Start();

		//OnGenerateAutoProcessRateUpdate(this, new ProcessRateEventArgs((uint)(0.0)));
		if(false == GetSliceInfo(pathForPictureStl, pathForSlicerDll, xyMinLaction))
			return;
		sw.Stop();

		DbgMsg(__FUNCTION__"===> Slice Time: %ld (ms)", sw.ElapsedMilliseconds());

		thresholdList.clear();
		thresholdList = thresholds;


		sw.Reset();
		sw.Start();
		FindNecessarySupportPointsNew(contactSize, candSupPosList);
		sw.Stop();
		DbgMsg(__FUNCTION__"===> FindNecessarySupportPointsNew time: %ld", sw.ElapsedMilliseconds());


		if (candSupPosList.size()==0 || Command == COMMAND::Abort)
		{
			Status = STATUS::UserAbort;
			Command = COMMAND::None;
			//OnGenerateAutoDone(this, new GenerateDoneEventArgs(GenerateDoneEventArgs.DoneEventCode.UserAbort));
			return;
		}

		// add candidate positions to support mark list and check their distance
		for (int i = 0; i < candSupPosList.size(); i++)
		{
			// C#: 
			//SupportPoint pt(candSupPosList[i].Second(), candSupPosList[i].Third(), candSupPosList[i].First());
			//Add(pt);
			// End of C#

			unique_ptr<SupportPoint> p(new SupportPoint());

			p->SetPosition(candSupPosList[i].Second());
			p->SetDirection(candSupPosList[i].Third());
			p->SetTouchRadius(candSupPosList[i].First());

			_items.push_back(move(p));
		}
		if (Command == COMMAND::Abort)
		{
			//Clear();

			Status = STATUS::UserAbort;
			Command = COMMAND::None;
			//OnGenerateAutoDone(this, new GenerateDoneEventArgs(GenerateDoneEventArgs.DoneEventCode.UserAbort));
			return;
		}
		Status = STATUS::Done;
		//OnGenerateAutoDone(this, new GenerateDoneEventArgs());
	
#if 0//DEBUG	// Delete all temporarily files
		string filename(wFilename.begin(), wFilename.end());
		string tempFolder(wTempFolder.begin(), wTempFolder.end());
		string fileToDel;
		fileToDel = tempFolder + "\\" + filename + ".stl"; remove(fileToDel.c_str());			//pictureCpp.stl
		fileToDel = tempFolder + "\\" + filename + ".svg"; remove(fileToDel.c_str());			//pictureCpp.svg
		fileToDel = tempFolder + "\\" + filename + "_polygons.svg"; remove(fileToDel.c_str());	//pictureCpp_polygons.svg
		fileToDel = tempFolder + "\\" + filename + "_mesh.svg"; remove(fileToDel.c_str());		//pictureCpp_mesh.svg
		fileToDel = tempFolder + "\\" + filename + "_segments.svg"; remove(fileToDel.c_str());	//pictureCpp_segments.svg
		fileToDel = tempFolder + "\\" + filename + "_meshIdx.svg"; remove(fileToDel.c_str());	//pictureCpp_meshIdx.svg
#endif
		/*
		Print function elapsed time
		*/
		int count=0;
		for (auto &sw : vecSw)
		{
			DbgMsg("===> #%d.ElapsedMilliseconds(): %ld", count++, sw.ElapsedMilliseconds());
		}




		DbgMsg(__FUNCTION__"[OUT]");
	}
	
	// Slicer2.0DLL.dll
	typedef int(*pfnSlicer20DLL)(const wchar_t *, double);
	// XYZMesh_64.dll
	typedef bool(*pfnMeshReduceFromData)(float ratio, float *&v, unsigned int &fn, float *&v_new, unsigned int &fn_new);
	typedef void(*pfnFreeVertexArray)();

	bool SupportPointsList::ReducedTriangles(const unsigned char *stlBufferWor, const wstring wstrPathForXYZMeshDll, const wstring pathForPictureStl, PolygonPoint &xyMinLaction)
	{
		unsigned int facet_num = GetTrianglesCount(stlBufferWor);

		if ( facet_num < 10000 || wstrPathForXYZMeshDll == L"" )
		{
			xyMinLaction = GetMinLocation((unsigned char *)stlBufferWor);
			BinFile binFile(pathForPictureStl);
			binFile.Write(stlBufferWor, 1, GetStlFileSize(stlBufferWor));
		}
		else
		{
			HINSTANCE dllHandler = LoadLibrary( wstrPathForXYZMeshDll.c_str());

			if (dllHandler == nullptr)
			{
				DbgMsg(__FUNCTION__"===> Error: Unable to find XYZMesh_64.dll (C++).");
				return false;
			}
			pfnMeshReduceFromData MeshReduceFromData = (pfnMeshReduceFromData)GetProcAddress(dllHandler, "meshReduceFromData");
			pfnFreeVertexArray freeVertexArray = (pfnFreeVertexArray)GetProcAddress(dllHandler, "freeVertexArray");
			if (	MeshReduceFromData == nullptr ||
					freeVertexArray == nullptr)
			{
				DbgMsg(__FUNCTION__"===>  Error: some functions are missed from XYZMesh_x64.dll!!!");
				return false;
			}
			unsigned int facet_num_reduced = 0;

			float *srcPtr1 = GetWorldVerticesXYZ(stlBufferWor);
			float *dstPtr = nullptr;

			MeshReduceFromData(0.5f, srcPtr1, facet_num, dstPtr, facet_num_reduced);
			DbgMsg(__FUNCTION__ "===> facet_num_reduced: %d", facet_num_reduced);

			WorldVerticesXYZToStlBinary(dstPtr, facet_num_reduced, pathForPictureStl, xyMinLaction);

			freeVertexArray();
			free(srcPtr1);
			//bool fFreeResult = FreeLibrary(dllHandler);	// BUG BUG BUG: It will cause crash!!!! Need to check.....
		}
		return true;
	}

	float* SupportPointsList::GetWorldVerticesXYZ(const unsigned char *stlBufferWor)
	{
		if (stlBufferWor == nullptr) return nullptr;

		unsigned char *r = (unsigned char *)(stlBufferWor) + 80;		// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);								// number of triangles

		float *vet = (float *)malloc(nTri * 3 * 3 * sizeof(float));

		int number = 0;
		for (unsigned int i = 0; i < nTri; i++)
		{
			r += 12;	//skip normal 12 bytes
			for (int j = 0; j < 9; j++) // x,y,z, x,y,z, x,y,z
			{
				vet[number] = ReadSingle(r);
				number++;
			}
			r += 2;	//skip two bytes
		}
		return vet;
	}

	void SupportPointsList::WorldVerticesXYZToStlBinary(float *vet_reduced, int facet_num_reduced, const wstring pathForPictureStl, PolygonPoint &xyMinLaction)
	{
		unsigned char *stlBinary = (unsigned char *)malloc(facet_num_reduced * 50 + 84);

		xyMinLaction.x = std::numeric_limits<double>::max();
		xyMinLaction.y = std::numeric_limits<double>::max();

		//UINT8[80] – Header	// 80 bytes
		memset(stlBinary, 0, 80);

		unsigned char *w = stlBinary + 80;

		//UINT32 – Number of triangles
		WriteUInt32(w, facet_num_reduced);

		int index = 0;
		for (int j = 0; j < facet_num_reduced; j++)
		{
			float f = 0;

			//REAL32[3] – Normal vector								// 12 bytes	
			f = 0;
			WriteSingle(w, f);

			f = 0;
			WriteSingle(w, f);

			f = 0;
			WriteSingle(w, f);

			//REAL32[3] – Vertex 1									// 12 bytes
			//REAL32[3] – Vertex 2									// 12 bytes
			//REAL32[3] – Vertex 3									// 12 bytes
			for (unsigned int k = 0; k < 3; k++)
			{
				f = (float)vet_reduced[index]; index++;				//x
				WriteSingle(w, f);
				if (xyMinLaction.x > f) xyMinLaction.x = f;

				f = (float)vet_reduced[index]; index++;				//y
				WriteSingle(w, f);
				if (xyMinLaction.y > f) xyMinLaction.y = f;

				f = (float)vet_reduced[index]; index++;				//z
				WriteSingle(w, f);
			}

			//UINT16 – Attribute byte count							// 2 bytes
			w[0] = 0x00; w++;
			w[0] = 0x00; w++;
		}
		BinFile binFile(pathForPictureStl);
		binFile.Write(stlBinary, 1, facet_num_reduced * 50 + 84);
		free(stlBinary);
	}

	bool SupportPointsList::GetSliceInfo(const wstring wstrStlFilePath, const wstring wstrPathForSlicerDll, PolygonPoint boundaryBox)
	{
		DbgMsg(__FUNCTION__"[IN]");
		HINSTANCE dllHandler = LoadLibrary(wstrPathForSlicerDll.c_str());

		if (dllHandler == nullptr)
		{
			DbgMsg(__FUNCTION__"===> Error: Unable to find Slicer2.0DLL.dll (C++).");
			return false;
		}

		pfnSlicer20DLL fnSlicer20DLL = nullptr;
		fnSlicer20DLL = (pfnSlicer20DLL)GetProcAddress(dllHandler, "fnSlicer20DLL");

		if (!fnSlicer20DLL)
		{
			DbgMsg(__FUNCTION__"===>  Error: Not Found fnSlicer20DLL function");
			return false;
		}
	
		// Charles: TEST
		//int nRet = fnSlicer20DLL(wstrStlFilePath.c_str(), 0.1);

		GetdatafromSVG getDataFromSVG;
		size_t found = wstrStlFilePath.rfind(L".");	// cut extension
		wstring filename = wstrStlFilePath.substr(0, found);

		wstring polygonsPath = filename + L"_polygons.svg";
		getDataFromSVG.LoadSVGFile(layerModel, polygonsPath);

		GetMeshfromSVG getMeshFromSVG;
		wstring meshPath = filename + L"_mesh.svg";
		getMeshFromSVG.LoadSVGFile(meshModel, meshPath);
		meshModel.CaculateNormalVector();

		GetSegmentsfromSVG getSegmentsFromSVG;
		wstring segmentPath = filename + L"_segments.svg";
		getSegmentsFromSVG.LoadSVGFile(segmentModel, segmentPath);
		segmentModel.GetInformation(meshModel.normallist);

		layerModel.SVGdatatoClipperdata(boundaryBox);

		// Charles: DEBUG	===> Checked. Same as AP.
		/*layerModel.ToFile("layerModel_CPP.txt", boundaryBox);
		meshModel.ToFile("meshModel_CPP.txt");
		segmentModel.ToFile("segmentModel_CPP.txt");*/

		bool fFreeResult = FreeLibrary(dllHandler);
		DbgMsg(__FUNCTION__"[OUT]");
		return true;
	}

	void SupportPointsList::FindNecessarySupportPointsNew(double size, /*out*/ vector<Triple<double, RHVector3, RHVector3>> &candSupPosList)
	{
		allGeneratedSupportPoint.clear();
		lowerSolid.clear();
		lowerHole.clear();
		upperSolid.clear();
		upperHole.clear();
		modifiedLowerHole.clear();
		modifiedUpperHole.clear();
		pastLevelSupportPoints.clear();
		pastLevelSupportPointsArea.clear();
		nowLevelSupportPoint.clear();
		nowLevelSupportPointsArea.clear();
		supportPointToPolygonTable.clear();
		nowSupportPointToPolygonTable.clear();
		upperLowerPolygonTable.clear();
		downAreaThreshold.clear();
		upperAreaThreshold.clear();
		//ThresholdList[0]: density setting 
		//ThresholdList[1]: contour setting
		//ThresholdList[2]: average setting
		//ThresholdList[3]: new Area Density Modified setting
		int totalCountOfProgressBar = static_cast<int>(layerModel.alllayer.size());
		int Firstlevel = GetFirstLayer();
		double height = round((Firstlevel * layerHeight + layerHeight / 2)*100.0)/100.0;//double height = Math.Round(Firstlevel * layerHeight + layerHeight / 2, 2, MidpointRounding.AwayFromZero);
		FirstLayerGenerateSupport(Firstlevel, height, size);
		height = round((height + layerHeight)*100.0)/100.0;

		//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPoint._v, "nowLevelSupportPoint", ".\\CPP\\nowLevelSupportPoint_" + to_string(Firstlevel)+ ".txt");	// ===>OK. result is 0
		//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPointsArea, "nowLevelSupportPointsArea", ".\\CPP\\nowLevelSupportPointsArea_"+ to_string(Firstlevel)+ ".txt");
		//SupportPointsListDbg::DumpObjectToFile(pastLevelSupportPointsArea, "pastLevelSupportPointsArea", ".\\CPP\\pastLevelSupportPointsArea_" + to_string(Firstlevel) + ".txt");

		DbgMsg(__FUNCTION__"===> Firstlevel: %d", Firstlevel);
		DbgMsg(__FUNCTION__"===> layerModel.alllayer.size(): %d", layerModel.alllayer.size());
		for (int level = Firstlevel + 1; level < layerModel.alllayer.size(); level++)
		{
			//Charles: TEST
			if (level % 10 == 0)
				DbgMsg(__FUNCTION__"===> level: %d", level);



			currentLayer = level;	// for debugging

			GetNextLayerInfo(level);
			upperAreaThreshold.clear();
		
			// [upperSolid] check OK(2019/8/6). The same as AP as well as the order.
			// [upperLowerPolygonTable] Checked OK (2019/8/6).

			for (int count = 0; count < upperSolid.size(); count++)
			{
				if (upperLowerPolygonTable[count].size() == 0)
				{
					double value = CalculateAreaValue(count);
					if (value <= controlDensityValueAreaThreshold)
						upperAreaThreshold.push_back(true);
					else
						upperAreaThreshold.push_back(false);
					GenerateCompletelyHorizontalFaceSupportPoint(count, height, size);
				}
				else
				{
					vector<int> controlNumber;
					vector<vector<vector<IntPoint>>> needGenerateSupportArea;
					double value = CalculateAreaValue(count);
					bool densityThreshold = GetDensitySettingValue(count, value);
					GetNeedGenerateArea(count, controlNumber, needGenerateSupportArea);

					// Charles: TEST
					//if(currentLayer==1)
					//	SupportPointsListDbg::DumpObjectToFile(needGenerateSupportArea[0], "needGenerateSupportArea[0]", ".\\CPP\\needGenerateSupportArea_" + to_string(level) + "_" + to_string(count) +".txt");

					//haveToGenerateSupportArea[0]: solid
					//haveToGenerateSupportArea[1]: hole
					if (needGenerateSupportArea[0].size() <= 0)
						continue;
					SortPolygons(needGenerateSupportArea[0]);
					for(const auto &poly: needGenerateSupportArea[0]) //foreach(List<IntPoint> poly in needGenerateSupportArea[0])
					{
						vector<vector<IntPoint>> needGenerateSupportAreaHole = needGenerateSupportArea[1];
						queue<vector<IntPoint>> box;
						box.push(poly);
						while (box.size() > 0)
						{
							vector<IntPoint> needSupportPolygon = box.front();	box.pop();
							CheckHole(needSupportPolygon, needGenerateSupportAreaHole);
							double areaValue = CalculateAreaValue(needSupportPolygon, needGenerateSupportAreaHole);
							if (areaValue < needGenerateSupportAreaThreshold)
								continue;
							vector<IntPoint> candidateSupportPointList = GetCandidateSupportPoint(needSupportPolygon);
							if (pastLevelSupportPoints.size() <= 0)
								continue;
							vector<double> distanceList;
							if (densityThreshold == false)
								distanceList = CalculateEveryPointtoSupportMinDistance(candidateSupportPointList, controlNumber, false);
							else
								distanceList = CalculateEveryPointtoSupportMinDistance(candidateSupportPointList, controlNumber, true);
							int maxNumber = FindMaxDistance(distanceList);
							if (distanceList[maxNumber] > 0)
							{
								if (densityThreshold == false)
								{
									RHVector3 normalVector = GenerateNeedSupportAreaSupportPoint(candidateSupportPointList[maxNumber], false, level, count, size, height);
									GetRemainingNeedSupportArea(candidateSupportPointList[maxNumber], normalVector, count, needSupportPolygon, false, 
										box, controlNumber, needGenerateSupportAreaHole);
								}
								else
								{
									RHVector3 normalVector = GenerateNeedSupportAreaSupportPoint(candidateSupportPointList[maxNumber], true, level, count, size, height);
									GetRemainingNeedSupportArea(candidateSupportPointList[maxNumber], normalVector, count, needSupportPolygon, true, 
										box, controlNumber, needGenerateSupportAreaHole);
								}
							}
							else if (distanceList[maxNumber] == 0)
							{
								if (areaValue / value <= needGenerateSupportByAverageThreshold)
									continue;
								GenerateCompletelyHorizontalFaceSupportPoint(needSupportPolygon, needGenerateSupportAreaHole, count, height, size);
							}
						}
					}
				}
			}

			// Dump nowLevelSupportPoint
			//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPoint._v, "nowLevelSupportPoint", ".\\CPP\\nowLevelSupportPoint_" + to_string(currentLayer) + ".txt");

			// Dump nowLevelSupportPointsArea
			//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPointsArea, "nowLevelSupportPointsArea", ".\\CPP\\nowLevelSupportPointsArea_" + to_string(currentLayer) + ".txt");

			pastLevelSupportPoints.clear();
			for(const auto &pt : nowLevelSupportPoint._v)	// foreach(IntPoint pt in nowLevelSupportPoint)
			{
				pastLevelSupportPoints.push_back(pt);
			}
			pastLevelSupportPointsArea.clear();
			for (const auto &polygons : nowLevelSupportPointsArea)	// foreach(List<IntPoint>polygons in nowLevelSupportPointsArea)
			{
				pastLevelSupportPointsArea.push_back(polygons);
			}
			downAreaThreshold.clear();
			for (const auto &key : upperAreaThreshold)	// foreach(bool key in upperAreaThreshold)
			{
				downAreaThreshold.push_back(key);
			}
			lowerSolid.clear();
			for (const auto &one : upperSolid)	// foreach(List<IntPoint> one in upperSolid)
			{
				lowerSolid.push_back(one);
			}
			modifiedLowerHole.clear();
			for (const auto &one : modifiedUpperHole)	// foreach(vector<vector<IntPoint>> one in modifiedUpperHole)
			{
				modifiedLowerHole.push_back(one);
			}
			supportPointToPolygonTable.clear();
			for (const auto &poly : nowSupportPointToPolygonTable)	// foreach(HashSet<Vector3> poly in nowSupportPointToPolygonTable)
			{
				supportPointToPolygonTable.push_back(poly);
			}
			height = round((height + layerHeight) * 100.0) / 100.0;	//height = Math.Round(height + layerHeight, 2, MidpointRounding.AwayFromZero);
			//OnGenerateAutoProcessRateUpdate(this, new ProcessRateEventArgs((uint)((level * 100) / totalCountOfProgressBar)));
		}
		//OnGenerateAutoProcessRateUpdate(this, new ProcessRateEventArgs((uint)(100.0)));

		candSupPosList.clear();
		for(auto &point: allGeneratedSupportPoint) //foreach(SupportPoint point in allGeneratedSupportPoint)
		{
			candSupPosList.push_back(Triple<double, RHVector3, RHVector3>(size, point.GetPosition(), point.GetDirection()));
		}
		if (Command == COMMAND::Abort)
		{
			candSupPosList.clear();
		}
	}

	int SupportPointsList::GetFirstLayer()
	{
		int Firstlevel = 0;
		for (int i = 0; i < layerModel.alllayer.size(); i++)
		{
			if (layerModel.alllayer[i].size() != 0)
			{
				Firstlevel = i;
				break;
			}
		}
		return Firstlevel;
	}

	void SupportPointsList::FirstLayerGenerateSupport(int Firstlevel, double height, double size)
	{
		GetThisLevelAllPolygons(lowerSolid, lowerHole, Firstlevel);
		MergeOverlappingPolygon(lowerSolid, lowerHole);
		modifiedLowerHole = PairSolidandHole(lowerSolid, lowerHole);
		for (int i = 0; i < lowerSolid.size(); i++)
		{
			double value = ClipperLib::Area(lowerSolid[i]);
			if (modifiedLowerHole[i].size() > 0)
			{
				for(const auto &poly : modifiedLowerHole[i])//foreach(List<IntPoint> poly in modifiedLowerHole[i])
				{
					value += ClipperLib::Area(poly);
				}
			}
			if (value <= controlDensityValueAreaThreshold)
				downAreaThreshold.push_back(true);
			else
				downAreaThreshold.push_back(false);
		}
		pastLevelSupportPoints = FindCompletelyHorizontalFace(lowerSolid, modifiedLowerHole, height, size);
		vector<IntPoint> pointArea;
		pastLevelSupportPointsArea.clear();
		for (const auto &pt : pastLevelSupportPoints)//foreach(IntPoint pt in pastLevelSupportPoints)
		{
			pointArea = PointToSupportArea(pt, RHVector3(0, 0, -1), true);
			pastLevelSupportPointsArea.push_back(pointArea);
		}
		MatchPointstoPolygon(height);
	}

	void SupportPointsList::GetNextLayerInfo(int level)
	{
		nowLevelSupportPoint.clear();
		nowLevelSupportPointsArea.clear();
		GetThisLevelAllPolygons(upperSolid, upperHole, level);	// Checked OK. 2019/8/6.
		MergeOverlappingPolygon(upperSolid, upperHole);
		modifiedUpperHole = PairSolidandHole(upperSolid, upperHole);
		upperLowerPolygonTable = GetUpperAndLowerPolygonPairList();
		MatchPointsToPolygon(upperLowerPolygonTable);
	}

	void SupportPointsList::GetThisLevelAllPolygons(vector<vector<IntPoint>> &solid, vector<vector<vector<IntPoint>>> &holelist, int level)
	{
		// TEST TEST TEST
		vecSw[0].Start();


		solid.clear();
		holelist.clear();
		vector<vector<IntPoint>> hole;

		for (int num = 0; num < layerModel.Layers[level].PolygonsColor.size(); num++)
		{
			if (abs(ClipperLib::Area(layerModel.alllayer[level][num])) > 100)
			{
				double v = ClipperLib::Area(layerModel.alllayer[level][num]);
				if (layerModel.Layers[level].PolygonsColor[num] == "B")
				{
					hole.push_back(layerModel.alllayer[level][num]);
				}
				else
				{
					solid.push_back(layerModel.alllayer[level][num]);
					vector<vector<IntPoint>> one;
					holelist.push_back(one);
				}
			}
		}
		SortPolygons(solid);
		vector<int> tag = GetTag(solid, solid);
		for(const auto &poly:hole)	//foreach(vector<IntPoint> poly in hole)
		{
			for (int num = (int)solid.size() - 1; num >= 0; num--)
			{
				vector<vector<vector<IntPoint>>> output = ClipperGeometricOperationPolyTreeType(poly, solid[num], ClipType::ctIntersection, PolyFillType::pftEvenOdd, PolyFillType::pftPositive);
				if (output[0].size() > 0)
				{
					double value1 = -ClipperLib::Area(poly);
					double value2 = ClipperLib::Area(solid[num]);
					if (value2 > value1)
					{
						holelist[tag[num]].push_back(poly);
						break;
					}
				}
			}
		}

		// TEST TEST TEST
		vecSw[0].Stop();
	}
	void SupportPointsList::MergeOverlappingPolygon(vector<vector<IntPoint>> &input, vector<vector<vector<IntPoint>>> &inputhole)
	{
		// TEST TEST TEST
		vecSw[1].Start();

		if (input.size() > 1)
		{
			vector<vector<IntPoint>> polygons;
			vector<vector<IntPoint>> allpolygons;
			for(const auto &poly : input)	//foreach(vector<IntPoint> poly in input)
				allpolygons.push_back(poly);
			for(const auto &inputholes : inputhole)	//foreach(vector<vector<IntPoint>> inputholes in inputhole)
			{
				for(const auto &poly : inputholes)	//C#: foreach(List<IntPoint> poly in inputholes)
				{
					allpolygons.push_back(poly);
				}
			}
			input.clear();
			inputhole.clear();
			vector<vector<vector<IntPoint>>> answer = ClipperGeometricOperationPolyTreeType(allpolygons, polygons, ClipType::ctUnion, PolyFillType::pftPositive, PolyFillType::pftPositive);
			for (const auto &poly : answer[0])	//foreach(vector<IntPoint> poly in answer[0])
			{
				input.push_back(poly);
			}
			vector<vector<IntPoint>> hole;
			for (const auto &poly : answer[1])	//foreach(vector<IntPoint> poly in answer[1])
			{
				hole.push_back(poly);
			}
			inputhole.push_back(hole);
		}

		// TEST TEST TEST
		vecSw[1].Stop();
	}
	//1
	vector<vector<vector<IntPoint>>> SupportPointsList::ClipperGeometricOperationPolyTreeType(const vector<IntPoint> &subject, 
										const vector<IntPoint> &clip, ClipType clipType, PolyFillType subtype, PolyFillType cliptype)
	{
		// TEST TEST TEST
		vecSw[2].Start();


		Clipper clipper;
		PolyTree sol;
		vector<vector<vector<IntPoint>>> output;

		clipper.AddPath(subject, PolyType::ptSubject, true);
		clipper.AddPath(clip, PolyType::ptClip, true);
		clipper.Execute(clipType, sol, subtype, cliptype);
		vector<vector<IntPoint>> solid;
		vector<vector<IntPoint>> hole;
		for (int i = sol.ChildCount()-1; i>=0; i--)
		{
			bool test = sol.Childs[i]->IsHole();
			if (test == true)
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					hole.push_back(poly);
				//hole.Add(sol.m_AllPolys[i].Contour);
			}
			else
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					solid.push_back(poly);
				//solid.Add(sol.m_AllPolys[i].Contour);
			}
		}
		output.push_back(solid);
		output.push_back(hole);

		// TEST TEST TEST
		vecSw[2].Stop();

		return output;
	}
	//2
	vector<vector<IntPoint>> SupportPointsList::ClipperGeometricOperationNormalType(const vector<vector<IntPoint>> &subject, 
								const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype)
	{
		// TEST TEST TEST
		vecSw[3].Start();

		Clipper clipper;
		vector<vector<IntPoint>> sol;
		vector<vector<IntPoint>> output;
		clipper.AddPaths(subject, PolyType::ptSubject, true);
		for (const auto &clip : clips)	// C#: foreach(List<IntPoint> clip in clips)
		{
			clipper.AddPath(clip, PolyType::ptClip, true);
		}
		clipper.Execute(clipType, sol, subtype, cliptype);

		ClipperLib::CleanPolygons(sol, output, 1.415);

		// TEST TEST TEST
		vecSw[3].Stop();

		return output;
	}
	//3
	// Test Results:
	// Input: subject, clips ===> same as AP
	// Return: "output" ===> from "sol". The "sol" data is the same as AP's, but the order is not the same.
	vector<vector<IntPoint>> SupportPointsList::ClipperGeometricOperationNormalType(const vector<IntPoint> &subject, 
								const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype)
	{
		// TEST TEST TEST
		vecSw[4].Start();

		Clipper clipper;
		vector<vector<IntPoint>> sol;
		vector<vector<IntPoint>> output;
		clipper.AddPath(subject, PolyType::ptSubject, true);
		for(const auto &clip : clips)	//foreach(List<IntPoint> clip in clips)
		{
			clipper.AddPath(clip, PolyType::ptClip, true);
		}
		clipper.Execute(clipType, sol, subtype, cliptype);
		ClipperLib::CleanPolygons(sol, output, 1.415);

		// TEST TEST TEST
		vecSw[4].Stop();

		return output;
	}

	//4
	vector<vector<vector<IntPoint>>> SupportPointsList::ClipperGeometricOperationPolyTreeType(const vector<vector<IntPoint>> &subject, 
								const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype)
	{
		// TEST TEST TEST
		vecSw[5].Start();

		Clipper clipper;
		PolyTree sol;
		vector<vector<vector<IntPoint>>> output;
		clipper.AddPaths(subject, PolyType::ptSubject, true);
		for(const auto &clip : clips)	//foreach(List<IntPoint> clip in clips)
		{
			clipper.AddPath(clip, PolyType::ptClip, true);
		}
		clipper.Execute(clipType, sol, subtype, cliptype);
		vector<vector<IntPoint>> solid;
		vector<vector<IntPoint>> hole;

#if 1	// must use this one; otherwise, upperSolid[] won't be the same AP.
		for (int i = sol.ChildCount() - 1; i >= 0; i--)	// To make the order of the result is the same as AP's.
		{
			bool test = sol.Childs[i]->IsHole();
			if (test == true)
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					hole.push_back(poly);
				//hole.Add(sol.m_AllPolys[i].Contour);
			}
			else
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					solid.push_back(poly);
				//solid.Add(sol.m_AllPolys[i].Contour);
			}
		}
#else
		for (int i = 0; i < sol.AllNodes.size(); i++)
		{
			bool test = sol.AllNodes[i]->IsHole();
			if (test == true)
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.AllNodes[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					hole.push_back(poly);
				//hole.Add(sol.m_AllPolys[i].Contour);
			}
			else
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.AllNodes[i]->Contour, poly, 1.415);

				if (poly.size() > 0)
					solid.push_back(poly);
				//solid.Add(sol.m_AllPolys[i].Contour);
			}
		}

#endif
		output.push_back(solid);
		output.push_back(hole);

		// TEST TEST TEST
		vecSw[5].Stop();
		return output;
	}

	//4_1: This function is only called in GetNeedGenerateArea() function. GetNeedGenerateArea() does not use the Cliper function above because of the order of the elements in "needGenerateSupportArea".
	//     GetNeedGenerateArea() calls the cliper function below to obtain "needGenerateSupportArea" with the same order as the AP.
	vector<vector<vector<IntPoint>>> SupportPointsList::ClipperGeometricOperationPolyTreeType4_1(const vector<vector<IntPoint>> &subject,
		const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype)
	{
		// TEST TEST TEST
		vecSw[6].Start();

		Clipper clipper;
		PolyTree sol;
		vector<vector<vector<IntPoint>>> output;
		clipper.AddPaths(subject, PolyType::ptSubject, true);
		for (const auto &clip : clips)	//foreach(List<IntPoint> clip in clips)
		{
			clipper.AddPath(clip, PolyType::ptClip, true);
		}
		clipper.Execute(clipType, sol, subtype, cliptype);
		vector<vector<IntPoint>> solid;
		vector<vector<IntPoint>> hole;

		for (int i = 0; i< sol.ChildCount(); i++)
		{
			bool test = sol.Childs[i]->IsHole();
			if (test == true)
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					hole.push_back(poly);
				//hole.Add(sol.m_AllPolys[i].Contour);
			}
			else
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					solid.push_back(poly);
				//solid.Add(sol.m_AllPolys[i].Contour);
			}
		}

		output.push_back(solid);
		output.push_back(hole);

		// TEST TEST TEST
		vecSw[6].Stop();

		return output;
	}
	//5
	vector<vector<vector<IntPoint>>> SupportPointsList::ClipperGeometricOperationPolyTreeType(const vector<IntPoint> &subject, 
								const vector<vector<IntPoint>> &clips, ClipType clipType, PolyFillType subtype, PolyFillType cliptype)
	{
		// TEST TEST TEST
		vecSw[7].Start();

		Clipper clipper;
		PolyTree sol;
		vector<vector<vector<IntPoint>>> output;
		clipper.AddPath(subject, PolyType::ptSubject, true);
		for(const auto &clip : clips)	// C#: foreach(List<IntPoint> clip in clips)
		{
			clipper.AddPath(clip, PolyType::ptClip, true);
		}
		clipper.Execute(clipType, sol, subtype, cliptype);
		vector<vector<IntPoint>> solid;
		vector<vector<IntPoint>> hole;

		for (int i = sol.ChildCount() - 1; i >= 0; i--)
		{
			bool test = sol.Childs[i]->IsHole();
			if (test == true)
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					hole.push_back(poly);
				//hole.Add(sol.m_AllPolys[i].Contour);
			}
			else
			{
				vector<IntPoint> poly;
				CleanPolygon(sol.Childs[i]->Contour, poly, 1.415);
				if (poly.size() > 0)
					solid.push_back(poly);
				//solid.Add(sol.m_AllPolys[i].Contour);
			}
		}
		output.push_back(solid);
		output.push_back(hole);

		// TEST TEST TEST
		vecSw[7].Stop();
		return output;
	}

	vector<vector<IntPoint>> SupportPointsList::ClipperOffest(const vector<IntPoint> &intput, int value)
	{
		// TEST TEST TEST
		vecSw[8].Start();

		PolyTree sol;
		vector<vector<IntPoint>> newarea;
		vector<vector<IntPoint>> output;
		ClipperOffset clipperoffest;
		clipperoffest.AddPath(intput, JoinType::jtMiter, EndType::etClosedPolygon);
		clipperoffest.Execute(sol, value);
		ClipperLib::ClosedPathsFromPolyTree(sol, newarea);
		output = RemoveCrushArea(newarea);

		// TEST TEST TEST
		vecSw[8].Stop();
		return output;
	}
	vector<vector<IntPoint>> SupportPointsList::RemoveCrushArea(const vector<vector<IntPoint>> &intput)
	{
		// TEST TEST TEST
		vecSw[9].Start();

		vector<vector<IntPoint>> output;
		for(const auto &onearea : intput)	//foreach(vector<IntPoint> onearea in intput)
		{
			vector<IntPoint> values;
			double kvalue = abs(ClipperLib::Area(onearea));
			if (kvalue > 4000000)
			{
				for (const auto &pt : onearea)	//foreach(IntPoint pt in onearea)
				{
					values.push_back(pt);
				}
				output.push_back(values);
			}
		}

		// TEST TEST TEST
		vecSw[9].Stop();
		return output;
	}

	void SupportPointsList::GenerateCompletelyHorizontalFaceSupportPoint(int count, double height, double size)
	{
		// TEST TEST TEST
		vecSw[10].Start();

		vector<vector<IntPoint>> needSupportAreas; needSupportAreas.push_back(upperSolid[count]);
		vector<vector<vector<IntPoint>>> hole = PairSolidandHole(needSupportAreas, modifiedUpperHole);
		vector<IntPoint> generatedPoint = FindCompletelyHorizontalFace(needSupportAreas, hole, height, size);
		for(auto &pt : generatedPoint)// C#: foreach(IntPoint pt in generatedPoint)
		{
			nowLevelSupportPoint.Add(pt);
			vector<IntPoint> supportArea = PointToSupportArea(pt, RHVector3(0, 0, -1), true);
			nowLevelSupportPointsArea.push_back(supportArea);
			nowSupportPointToPolygonTable[count].push_back(Vector3((float)pt.X, (float)pt.Y, (float)height));
		}

		// TEST TEST TEST
		vecSw[10].Stop();
	}

	void SupportPointsList::GenerateCompletelyHorizontalFaceSupportPoint(vector<IntPoint> &solid, vector<vector<IntPoint>> &hole, int count, double height, double size)
	{
		// TEST TEST TEST
		vecSw[11].Start();


		vector<vector<IntPoint>> areas;	areas.push_back(solid);
		vector<vector<vector<IntPoint>>> areaHoles;	areaHoles.push_back(hole);
		vector<vector<vector<IntPoint>>> modifiedHole = PairSolidandHole(areas, areaHoles);
		vector<IntPoint> generatedPoint = FindCompletelyHorizontalFace(areas, modifiedHole, height, size);
		for(const auto &pt: generatedPoint)	//foreach(IntPoint pt in generatedPoint)
		{
			nowLevelSupportPoint.Add((pt));
			vector<IntPoint> supportArea = PointToSupportArea(pt, RHVector3(0, 0, -1), true);
			nowLevelSupportPointsArea.push_back(supportArea);
			nowSupportPointToPolygonTable[count].push_back(Vector3((float)pt.X, (float)pt.Y, (float)height));
		}

		// TEST TEST TEST
		vecSw[11].Stop();
	}

	RHVector3 SupportPointsList::GenerateNeedSupportAreaSupportPoint(IntPoint point, bool isDensityTooHigh, int level, int count, double size, double height)
	{
		// TEST TEST TEST
		vecSw[12].Start();


		int meshid = 0;
		bool getMeshId = GetTriangleId(point, height, /*out*/ meshid);									//#41: 0s
		RHVector3 newLocation(0, 0, 0);
		if (getMeshId == false)
		{
			meshid = FindMeshId(point, level);															//#42: 
			newLocation = FindNewLocation(point, height, meshid, size);									//#44: 0s
		}
		else
		{
			newLocation = FindNewLocation(point, meshModel.Meshs[meshid].Vector3ds[0].z, meshid, size);
		}
		RHVector3 normalVector = AddSupportPoint(newLocation, isDensityTooHigh, meshid, size, count);	//#48: 0s

		// TEST TEST TEST
		vecSw[12].Stop();

		return normalVector;
	}

	vector<IntPoint> SupportPointsList::FindCompletelyHorizontalFace(const vector<vector<IntPoint>> &solid, const vector<vector<vector<IntPoint>>> &hole, double h, double size)
	{
		// TEST TEST TEST
		vecSw[13].Start();

		vector<IntPoint> Points;

		if (h > 0)
		{
			for (int num = 0; num < solid.size(); num++)
			{
				vector<IntPoint> pointslocation;
				vector<RHVector3> supportpoints;
				if (hole[num].size() > 0)
				{
					AddInternalAverage(solid[num], hole[num], h, supportpoints, pointslocation, size);
					CheckPoint(pointslocation, h, Points, size);
				}
				else
				{
					AddNoHoleInternalAverage(solid[num], h, supportpoints, pointslocation, size);
					CheckPoint(pointslocation, h, Points, size);
				}
			}
		}
		// TEST TEST TEST
		vecSw[13].Stop();

		return Points;
	}

	void SupportPointsList::AddInternalAverage(	const vector<IntPoint> &polygon, 
												const vector<vector<IntPoint>> &hole, 
												double h, 
												vector<RHVector3> &supportpoints, 
												vector<IntPoint> &pointslocation, double size)
	{
		// TEST TEST TEST
		vecSw[14].Start();

		int threshold = (int)((size + 0.1) * 10000);
		vector<vector<IntPoint>> output1 = ClipperOffest(polygon, -threshold);
		if (output1.size() > 0)
		{
			for(auto &poly : output1)	// C#: foreach(List<IntPoint> poly in output1)
			{
				AddWithCheckContour(poly, h, supportpoints, pointslocation);
			}
			for (const auto &poly : hole) // C#: foreach(List<IntPoint> poly in hole)
			{
				vector<vector<IntPoint>> output2 = ClipperOffest(poly, threshold);
				AddWithCheckContour(output2[0], h, supportpoints, pointslocation);
			}
			vector<vector<IntPoint>> shrinkarea = ClipperOffest(polygon, -10000);
			for (const auto &poly : shrinkarea) // C#: foreach(List<IntPoint> poly in shrinkarea)
			{
				IntPoint minaabb = poly[0];
				IntPoint maxaabb(0, 0);
				for(const auto &pt: poly) // C#: foreach(IntPoint pt in poly)
				{
					if (maxaabb.X < pt.X)
						maxaabb.X = pt.X;
					if (maxaabb.Y < pt.Y)
						maxaabb.Y = pt.Y;
					if (minaabb.X > pt.X)
						minaabb.X = pt.X;
					if (minaabb.Y > pt.Y)
						minaabb.Y = pt.Y;
				}
				IntPoint xaabb = IntPoint(static_cast<cInt>(minaabb.X + thresholdList[2] / 2), static_cast<cInt>(minaabb.Y + thresholdList[2] / 2 * sqrt(3)));
				int xnum = (int)((maxaabb.X - minaabb.X) / thresholdList[2]);
				int ynum = (int)((maxaabb.Y - minaabb.Y) / (thresholdList[2] * sqrt(3)));
				int xnum2 = (int)((maxaabb.X - xaabb.X) / thresholdList[2]);
				int ynum2 = (int)((maxaabb.Y - xaabb.Y) / (thresholdList[2] * sqrt(3)));
				for (int x = 0; x <= xnum; x++)
				{
					for (int y = 0; y <= ynum; y++)
					{
						int test = ClipperLib::PointInPolygon(IntPoint(static_cast<cInt>(minaabb.X + x * thresholdList[2]), static_cast<cInt>(minaabb.Y + y * sqrt(3) * thresholdList[2])), poly);
						int holetest = 0;
						for(const auto &holepoly:hole)	//foreach(List<IntPoint> holepoly in hole)
						{
							int value = ClipperLib::PointInPolygon(IntPoint(static_cast<cInt>(minaabb.X + x * thresholdList[2]), static_cast<cInt>(minaabb.Y + y * sqrt(3) * thresholdList[2])), holepoly);
							if (value != 0)
							{
								holetest = 1;
								break;
							}
						}
						if ((test == 1) && (holetest == 0))
						{
							RHVector3 point(static_cast<double>(minaabb.X + x * thresholdList[2]), static_cast<double>(minaabb.Y + y * sqrt(3) * thresholdList[2]), h);
							AddWithTestDistance(point, pointslocation, supportpoints, h);
						}
					}
				}
				for (int x = 0; x <= xnum2; x++)
				{
					for (int y = 0; y <= ynum2; y++)
					{
						int test = ClipperLib::PointInPolygon(IntPoint(static_cast<cInt>(xaabb.X + x * thresholdList[2]), static_cast<cInt>(xaabb.Y + y * sqrt(3) * thresholdList[2])), poly);
						int holetest = 0;
						for(const auto &holepoly : hole)	//foreach(vector<IntPoint> holepoly in hole)
						{
							int value = ClipperLib::PointInPolygon(IntPoint(static_cast<cInt>(xaabb.X + x * thresholdList[2]), static_cast<cInt>(xaabb.Y + y * sqrt(3) * thresholdList[2])), holepoly);
							if (value != 0)
							{
								holetest = 1;
								break;
							}
						}
						if ((test == 1) && (holetest == 0))
						{
							RHVector3 point(static_cast<double>(xaabb.X + x * thresholdList[2]), static_cast<double>(xaabb.Y + y * sqrt(3) * thresholdList[2]), h);
							AddWithTestDistance(point, pointslocation, supportpoints, h);
						}
					}
				}
			}
		}
		else
		{
			IntPoint spoint(0, 0);
			for(const auto &pt: polygon)	//foreach(IntPoint pt in polygon)
			{
				spoint.X = spoint.X + pt.X;
				spoint.Y = spoint.Y + pt.Y;
			}
			spoint.X = spoint.X / polygon.size();
			spoint.Y = spoint.Y / polygon.size();
			int testthreshold = 0;
			for(const auto &holepoly:hole)	//foreach(List<IntPoint> holepoly in hole)
			{
				int test = ClipperLib::PointInPolygon(spoint, holepoly);
				if (test != 0)
				{
					testthreshold = 1;
					break;
				}
			}
			int test2threshold = ClipperLib::PointInPolygon(spoint, polygon);
			if ((testthreshold == 0) & (test2threshold != 0))
			{
				supportpoints.push_back(RHVector3((double)spoint.X, (double)spoint.Y, h));
				pointslocation.push_back(spoint);
			}
			else
			{
				supportpoints.push_back(RHVector3((double)polygon[0].X, (double)polygon[0].Y, h));
				pointslocation.push_back(polygon[0]);
			}
		}

		// TEST TEST TEST
		vecSw[14].Stop();
	}
	void SupportPointsList::AddNoHoleInternalAverage(const vector<IntPoint> &polygon, double h, vector<RHVector3> &supportpoints, vector<IntPoint> &pointslocation, double size)
	{
		// TEST TEST TEST
		vecSw[15].Start();

		int threshold = (int)((size + 0.1) * 10000);
		vector<vector<IntPoint>> output1 = ClipperOffest(polygon, -threshold);
		if (output1.size() > 0)
		{
			for(auto &poly : output1)//foreach(List<IntPoint> poly in output1)
			{
				AddWithCheckContour(poly, h, supportpoints, pointslocation);
			}
			vector<vector<IntPoint>> shrinkarea = ClipperOffest(polygon, -10000);
			for(const auto &poly : shrinkarea)	//foreach(List<IntPoint> poly in shrinkarea)
			{
				IntPoint minaabb = poly[0];
				IntPoint maxaabb(0, 0);
				for(const auto &pt : poly)	//foreach(IntPoint pt in poly)
				{
					if (maxaabb.X < pt.X)
						maxaabb.X = pt.X;
					if (maxaabb.Y < pt.Y)
						maxaabb.Y = pt.Y;
					if (minaabb.X > pt.X)
						minaabb.X = pt.X;
					if (minaabb.Y > pt.Y)
						minaabb.Y = pt.Y;
				}
				IntPoint xaabb(static_cast<cInt>(minaabb.X + thresholdList[2] / 2), static_cast<cInt>(minaabb.Y + thresholdList[2] / 2 * sqrt(3)));
				int xnum = (int)((maxaabb.X - minaabb.X) / thresholdList[2]);
				int ynum = (int)((maxaabb.Y - minaabb.Y) / (thresholdList[2] * sqrt(3)));
				int xnum2 = (int)((maxaabb.X - xaabb.X) / thresholdList[2]);
				int ynum2 = (int)((maxaabb.Y - xaabb.Y) / (thresholdList[2] * sqrt(3)));
				for (int x = 0; x <= xnum; x++)
				{
					for (int y = 0; y <= ynum; y++)
					{
						int test = ClipperLib::PointInPolygon(IntPoint(static_cast<cInt>(minaabb.X + x * thresholdList[2]), static_cast<cInt>(minaabb.Y + y * sqrt(3) * thresholdList[2])), poly);
						if (test == 1)
						{
							RHVector3 point((double)minaabb.X + x * thresholdList[2], (double)minaabb.Y + y * sqrt(3) * thresholdList[2], h);
							AddWithTestDistance(point, pointslocation, supportpoints, h);
						}
					}
				}
				for (int x = 0; x <= xnum2; x++)
				{
					for (int y = 0; y <= ynum2; y++)
					{
						int test = ClipperLib::PointInPolygon(IntPoint(static_cast<cInt>(xaabb.X + x * thresholdList[2]), static_cast<cInt>(xaabb.Y + y * sqrt(3) * thresholdList[2])), poly);
						if (test == 1)
						{
							RHVector3 point((double)xaabb.X + x * thresholdList[2], (double)xaabb.Y + y * sqrt(3) * thresholdList[2], h);
							AddWithTestDistance(point, pointslocation, supportpoints, h);
						}
					}
				}
			}
		}
		else
		{
			IntPoint spoint(0, 0);
			for(const auto &pt : polygon)	//foreach(IntPoint pt in polygon)
			{
				spoint.X = spoint.X + pt.X;
				spoint.Y = spoint.Y + pt.Y;
			}
			spoint.X = spoint.X / polygon.size();
			spoint.Y = spoint.Y / polygon.size();
			int test = ClipperLib::PointInPolygon(spoint, polygon);
			if (test != 0)
			{
				supportpoints.push_back(RHVector3((double)spoint.X, (double)spoint.Y, h));
				pointslocation.push_back(spoint);
			}
			else
			{
				supportpoints.push_back(RHVector3((double)polygon[0].X, (double)polygon[0].Y, h));
				pointslocation.push_back(polygon[0]);
			}
		}

		// TEST TEST TEST
		vecSw[15].Stop();
	}
	void SupportPointsList::AddWithCheckContour(vector<IntPoint> &polygon, double h, vector<RHVector3> &supportpoints, vector<IntPoint> &location)
	{
		// TEST TEST TEST
		vecSw[16].Start();

		if (polygon.size() > 1)
		{
			int anglethreshold = 150;
			double edgelength = 0;
			polygon.push_back(polygon[0]);
			for (int num = 1; num < polygon.size(); num++)
			{
				double angle = CaculateAngle(polygon[num], polygon[(num - 1) % (polygon.size() - 1)], polygon[(num + 1) % (polygon.size() - 1)]);
				if (angle < anglethreshold)
				{
					RHVector3 cpoint((double)polygon[num].X, (double)polygon[num].Y, h);
					AddWithTestDistance(cpoint, location, supportpoints, h);
					double oneedgelength = sqrt(pow((polygon[num].X - polygon[num - 1].X), 2) + pow((polygon[num].Y - polygon[num - 1].Y), 2));
					edgelength = edgelength + oneedgelength;
					while (edgelength >= thresholdList[1] * 1.5)
					{
						RHVector3 dirvec(polygon[num].X - polygon[num - 1].X, polygon[num].Y - polygon[num - 1].Y, (cInt)0);
						dirvec.NormalizeSafe();//方向向量
						dirvec.Scale(edgelength - thresholdList[1]);
						RHVector3 nowpoint((double)polygon[num].X, (double)polygon[num].Y, h);
						RHVector3 supportpt = nowpoint.Subtract(dirvec);//新支撐點位子
						AddWithTestDistance(supportpt, location, supportpoints, h);
						edgelength = edgelength - thresholdList[1];
					}
					edgelength = 0;
				}
				else
				{
					double oneedgelength = sqrt(pow((polygon[num].X - polygon[num - 1].X), 2) +pow((polygon[num].Y - polygon[num - 1].Y), 2));
					edgelength = edgelength + oneedgelength;
					while (edgelength >= thresholdList[1])
					{
						RHVector3 dirvec(polygon[num].X - polygon[num - 1].X, polygon[num].Y - polygon[num - 1].Y, (cInt)0);
						dirvec.NormalizeSafe();//方向向量
						dirvec.Scale(edgelength - thresholdList[1]);
						RHVector3 nowpoint((double)polygon[num].X, (double)polygon[num].Y, h);
						RHVector3 supportpt = nowpoint.Subtract(dirvec);//新支撐點位子
						AddWithTestDistance(supportpt, location, supportpoints, h);
						edgelength = edgelength - thresholdList[1];
					}
				}
			}
			if (supportpoints.size() == 0)
			{
				IntPoint spoint(0,0);
				for(const auto &pt:polygon)	//foreach(IntPoint pt in polygon)
				{
					spoint.X = spoint.X + pt.X;
					spoint.Y = spoint.Y + pt.Y;
				}
				spoint.X = spoint.X / polygon.size();
				spoint.Y = spoint.Y / polygon.size();
				supportpoints.push_back(RHVector3(spoint.X, spoint.Y, (cInt)h));
				location.push_back(spoint);
			}
		}
		else
		{
			RHVector3 pt((double)polygon[0].X, (double)polygon[0].Y, h);
			location.push_back(polygon[0]);
			supportpoints.push_back(pt);
		}

		// TEST TEST TEST
		vecSw[16].Stop();
	}
	void SupportPointsList::AddWithTestDistance(RHVector3 pt, vector<IntPoint> &location, vector<RHVector3> &supportpoints, double h)
	{
		// TEST TEST TEST
		vecSw[17].Start();

		IntPoint point((cInt)pt.x, (cInt)pt.y);
		if (location.size() > 0)
		{
			double distance = sqrt(pow(pt.x - location[0].X, 2) + pow(pt.y - location[0].Y, 2));
			int number = 0;
			for (int num = 1; num < location.size(); num++)
			{
				double distancex = sqrt(pow(pt.x - location[num].X, 2) + pow(pt.y - location[num].Y, 2));
				if (distance > distancex)
				{
					distance = distancex;
					number = num;
				}
			}
			if (distance < thresholdList[1] * 1 / 2)
			{
				IntPoint spoint((location[number].X + (cInt)pt.x) / 2, (location[number].Y + (cInt)pt.y) / 2);
				supportpoints.erase(supportpoints.begin() + number);
				location.erase(location.begin() + number);
				location.push_back(spoint);
				supportpoints.push_back(RHVector3(spoint.X, spoint.Y, (cInt)h));
			}
			else
			{
				location.push_back(point);
				supportpoints.push_back(pt);
			}
		}
		else
		{
			location.push_back(point);
			supportpoints.push_back(pt);
		}

		// TEST TEST TEST
		vecSw[17].Stop();
	}
	//????
	double SupportPointsList::CaculateAngle(IntPoint pt1, IntPoint pt2, IntPoint pt3)
	{
		// TEST TEST TEST
		vecSw[18].Start();

		double distance1 = sqrt(pow(pt1.X - pt2.X, 2) + pow(pt1.Y - pt2.Y, 2));
		double distance2 = sqrt(pow(pt1.X - pt3.X, 2) + pow(pt1.Y - pt3.Y, 2));
		Vector2 vector1(static_cast<float>(pt2.X - pt1.X), static_cast<float>(pt2.Y - pt1.Y));
		Vector2 vector2(static_cast<float>(pt3.X - pt1.X), static_cast<float>(pt3.Y - pt1.Y));
		double dotvalue = vector1.dot(vector2);		//double dotvalue = Vector2.Dot(vector1, vector2);
		double angle = acos(dotvalue / (distance1 * distance2)) * 180.0 / M_PI;


		// TEST TEST TEST
		vecSw[18].Stop();

		return angle;
	}

	void SupportPointsList::CheckPoint(const vector<IntPoint> &pointslocation, double h, vector<IntPoint> &supportPoints, double size)
	{
		// TEST TEST TEST
		vecSw[19].Start();

		for (int i = 0; i < pointslocation.size(); i++)
		{
			int test = 1;
			for(auto &supportpoint : allGeneratedSupportPoint)//foreach(SupportPoint supportpoint in allGeneratedSupportPoint)
			{
				if (h - supportpoint.GetPosition().z <= 0.3)
				{
					double value = sqrt(pow(pointslocation[i].X - supportpoint.GetPosition().x * 10000, 2) + pow(pointslocation[i].Y - supportpoint.GetPosition().y * 10000, 2));
					if (value < 10000)
					{
						test = 0;
						break;
					}
				}
			}
			if (test == 1)
			{
				supportPoints.push_back(pointslocation[i]);
				RHVector3 OriginalPoint(pointslocation[i].X * 0.0001, pointslocation[i].Y * 0.0001, h);
				RHVector3 normalvector(0, 0, -1);
				SupportPoint pt(OriginalPoint, normalvector, size);
				allGeneratedSupportPoint.push_back(pt);
			}
		}

		// TEST TEST TEST
		vecSw[19].Stop();
	}
	void SupportPointsList::MatchPointstoPolygon(double height)
	{
		// TEST TEST TEST
		vecSw[20].Start();

		for (int i = 0; i < lowerSolid.size(); i++)
		{
			vector<Vector3> one;
			supportPointToPolygonTable.push_back(one);
		}
		for(const auto &pt: pastLevelSupportPoints)	//foreach(IntPoint pt in pastLevelSupportPoints)
		{
			for (int i = 0; i < lowerSolid.size(); i++)
			{
				int k = ClipperLib::PointInPolygon(pt, lowerSolid[i]);
				if (k != 0)//point is in polygon
				{
					supportPointToPolygonTable[i].push_back(Vector3((float)pt.X, (float)pt.Y, (float)height));
					break;
				}
			}
		}

		// TEST TEST TEST
		vecSw[20].Stop();
	}

	void SupportPointsList::MatchPointsToPolygon(const vector<vector<int>> &updowntable)
	{
		// TEST TEST TEST
		vecSw[21].Start();

		nowSupportPointToPolygonTable.clear();
		for (int i = 0; i < upperSolid.size(); i++)
		{
			vector<Vector3> one;
			for(const auto &num : updowntable[i])	//foreach(int num in updowntable[i])
			{
				for(const auto &pt: supportPointToPolygonTable[num])	//foreach(Vector3 pt in supportPointToPolygonTable[num])
				{
					one.push_back(Vector3(pt));
				}
			}
			nowSupportPointToPolygonTable.push_back(one);
		}

		// TEST TEST TEST
		vecSw[21].Stop();
	}

	vector<IntPoint> SupportPointsList::PointToSupportArea(IntPoint point, RHVector3 in_vector, bool isDensityTooHigh)
	{
		// TEST TEST TEST
		vecSw[22].Start();

		vector<IntPoint> onesupportarea;
		//double angle = Math.Asin( / ) * 180 / Math.PI;
		int threshold = 0;

		if (in_vector.z > -0.333)
			threshold = 40000;
		else if (in_vector.z > -0.666 && in_vector.z <= -0.333)
			threshold = 20000;
		else if (in_vector.z > -0.9 && in_vector.z <= -0.666)
			threshold = 0;
		else if (in_vector.z <= -0.9)
			threshold = -15000;
		if (isDensityTooHigh == true)
			threshold = -thresholdList[3];

		IntPoint pt1 = IntPoint(point.X + (thresholdList[0] + threshold), point.Y);
		IntPoint pt2 = IntPoint(point.X + (thresholdList[0] + threshold) / 2, static_cast<cInt>(point.Y + sqrt(3) * (thresholdList[0] + threshold) / 2.0));
		IntPoint pt3 = IntPoint(point.X - (thresholdList[0] + threshold) / 2, static_cast<cInt>(point.Y + sqrt(3) * (thresholdList[0] + threshold) / 2.0));
		IntPoint pt4 = IntPoint(point.X - (thresholdList[0] + threshold), point.Y);
		IntPoint pt5 = IntPoint(point.X - (thresholdList[0] + threshold) / 2, static_cast<cInt>(point.Y - sqrt(3) * (thresholdList[0] + threshold) / 2.0));
		IntPoint pt6 = IntPoint(point.X + (thresholdList[0] + threshold) / 2, static_cast<cInt>(point.Y - sqrt(3) * (thresholdList[0] + threshold) / 2.0));
		onesupportarea.push_back(pt1);
		onesupportarea.push_back(pt2);
		onesupportarea.push_back(pt3);
		onesupportarea.push_back(pt4);
		onesupportarea.push_back(pt5);
		onesupportarea.push_back(pt6);
		
		// TEST TEST TEST
		vecSw[22].Stop();
		return onesupportarea;
	}

	vector<vector<int>> SupportPointsList::GetUpperAndLowerPolygonPairList()
	{
		// TEST TEST TEST
		vecSw[23].Start();

		vector<vector<int>> targetlist;
		for (int num1 = 0; num1 < upperSolid.size(); num1++)
		{
			vector<int> one;
			for (int num2 = 0; num2 < lowerSolid.size(); num2++)
			{
				vector<vector<IntPoint>> up;
				vector<vector<IntPoint>> down;
				up.push_back(upperSolid[num1]);
				for(const auto &poly : modifiedUpperHole[num1])	//foreach(List<IntPoint> poly in modifiedUpperHole[num1])
				{
					up.push_back(poly);
				}
				down.push_back(lowerSolid[num2]);
				for (const auto &poly : modifiedLowerHole[num2])	//foreach(List<IntPoint> poly in modifiedLowerHole[num2])
				{
					down.push_back(poly);
				}
				vector<vector<vector<IntPoint>>> output = ClipperGeometricOperationPolyTreeType(up, down, ClipType::ctIntersection, PolyFillType::pftEvenOdd, PolyFillType::pftEvenOdd);
				if (output[0].size() > 0)
				{
					one.push_back(num2);
				}
			}
			targetlist.push_back(one);
		}

		// TEST TEST TEST
		vecSw[23].Stop();
		return targetlist;
	}

	vector<vector<vector<IntPoint>>> SupportPointsList::PairSolidandHole(vector<vector<IntPoint>> &solid, vector<vector<vector<IntPoint>>> &uphole)
	{
		// TEST TEST TEST
		vecSw[24].Start();


		vector<vector<vector<IntPoint>>> newhole;
		vector<vector<IntPoint>> allhole;
		for (const auto &onegroup : uphole) // C#: foreach(List<List<IntPoint>> onegroup in uphole)
		{
			for(const auto &ones : onegroup) // C#: foreach(List<IntPoint> ones in onegroup)
			{
				allhole.push_back(ones);
			}
		}
		for (int i = 0; i < solid.size(); i++)
		{
			vector<vector<IntPoint>> group;
			newhole.push_back(group);
		}
		SortPolygons(solid);
		vector<int> tag = GetTag(solid, solid);
		for(const auto &poly : allhole)	// C#: foreach(List<IntPoint> poly in allhole)
		{
			for (int num = (int)solid.size() - 1; num >= 0; num--)
			{
				vector<vector<vector<IntPoint>>> output = ClipperGeometricOperationPolyTreeType(poly, solid[num], ClipType::ctIntersection, PolyFillType::pftEvenOdd, PolyFillType::pftPositive);
				if (output[0].size() > 0)
				{
					double value1 = -ClipperLib::Area(poly);
					double value2 = ClipperLib::Area(solid[num]);
					if (value2 > value1)
					{
						newhole[tag[num]].push_back(poly);
						break;
					}
				}
			}
		}

		// TEST TEST TEST
		vecSw[24].Stop();
		return newhole;
	}

	double SupportPointsList::CalculateAreaValue(int count)
	{
		// TEST TEST TEST
		vecSw[25].Start();


		double value = ClipperLib::Area(upperSolid[count]);
		if (modifiedUpperHole[count].size() > 0)
		{
			for(const auto &poly: modifiedUpperHole[count])//foreach(List<IntPoint> poly in modifiedUpperHole[count])
			{
				value += ClipperLib::Area(poly);
			}
		}

		// TEST TEST TEST
		vecSw[25].Stop();
		return value;
	}

	bool SupportPointsList::GetDensitySettingValue(int count, double value)
	{
		// TEST TEST TEST
		vecSw[26].Start();

		bool tag = true;
		for(const auto &i : upperLowerPolygonTable[count])	//foreach(int i in upperLowerPolygonTable[count])
		{
			if (downAreaThreshold[i] == false)
			{
				tag = false;
				break;
			}
		}
		if (tag == false)
		{
			upperAreaThreshold.push_back(false);
			tag = false;
		}
		else
		{
			if (value > controlDensityValueAreaThreshold)
			{
				upperAreaThreshold.push_back(false);
				tag = false;
			}
			else
				upperAreaThreshold.push_back(true);
		}

		// TEST TEST TEST
		vecSw[26].Stop();


		return tag;
	}

	void SupportPointsList::GetNeedGenerateArea(int count, /*out*/ vector<int> &controlNumber, /*out*/ vector<vector<vector<IntPoint>>> &needGenerateSupportArea)
	{
		// TEST TEST TEST
		vecSw[27].Start();


		vector<vector<IntPoint>> allLowerPolygons = GetPolygons(upperLowerPolygonTable[count]);
		vector<vector<IntPoint>> allUpperPolygons = GetPolygons(upperSolid[count], modifiedUpperHole[count]);
		controlNumber = GetPointInUpperSolid(count);
		UpdateNowSupportPointTable(count, controlNumber);
		vector<vector<IntPoint>> haveSupportProtectArea = GetSupportArea(controlNumber);
		vector<vector<IntPoint>> noNeedSupportArea = ClipperGeometricOperationNormalType(allLowerPolygons, haveSupportProtectArea, ClipType::ctUnion, PolyFillType::pftPositive, PolyFillType::pftPositive);
		//no Need Support Area = have Support Protect Area + lower polygons
		needGenerateSupportArea = ClipperGeometricOperationPolyTreeType4_1(allUpperPolygons, noNeedSupportArea, ClipType::ctDifference, PolyFillType::pftEvenOdd, PolyFillType::pftEvenOdd);
		//have To Generate Support Area = upper polygons - no Need Support Area

		// TEST TEST TEST
		vecSw[27].Stop();
	}

	void SupportPointsList::GetRemainingNeedSupportArea(IntPoint point, RHVector3 normalVector, int count, const vector<IntPoint> &needSupportPolygon, bool isDensityTooHigh,
		/*out*/ queue<vector<IntPoint>> &box, /*out*/ vector<int> &controlNumber, /*out*/ vector<vector<IntPoint>> &needGenerateSupportAreaHole)
	{
		// TEST TEST TEST
		vecSw[28].Start();
		
		vector<vector<IntPoint>> newSupportPointArea;
		newSupportPointArea.push_back(PointToSupportArea(point, normalVector, isDensityTooHigh));
		pastLevelSupportPoints.push_back(point);
		pastLevelSupportPointsArea.push_back(PointToSupportArea(point, normalVector, isDensityTooHigh));
		controlNumber = GetPointInUpperSolid(count);
		vector<vector<IntPoint>> needGenerateSupportPolygons = GetPolygons(needSupportPolygon, needGenerateSupportAreaHole);
		vector<vector<vector<IntPoint>>> remainingHaveToGenerateSupportArea = ClipperGeometricOperationPolyTreeType(needGenerateSupportPolygons, 
			newSupportPointArea, ClipType::ctDifference, PolyFillType::pftPositive, PolyFillType::pftPositive);
		if (remainingHaveToGenerateSupportArea[0].size() > 0)
		{
			SortPolygons(remainingHaveToGenerateSupportArea[0]);
			for(const auto &newPolygon : remainingHaveToGenerateSupportArea[0]) // foreach(List<IntPoint> newPolygon in remainingHaveToGenerateSupportArea[0])
			{
				box.push(newPolygon);
			}
			needGenerateSupportAreaHole.clear();
			for (const auto &newHolePolygon : remainingHaveToGenerateSupportArea[1]) // foreach(List<IntPoint> newHolePolygon in remainingHaveToGenerateSupportArea[1])
			{
				needGenerateSupportAreaHole.push_back(newHolePolygon);
			}
		}

		// TEST TEST TEST
		vecSw[28].Stop();
	}
	double SupportPointsList::CalculateAreaValue(vector<IntPoint> &testpoly, vector<vector<IntPoint>> &unSafeAreaHole)
	{
		// TEST TEST TEST
		vecSw[29].Start();


		double areavalue = ClipperLib::Area(testpoly);
		for(const auto &holepoly : unSafeAreaHole)	// C#: foreach(List<IntPoint> holepoly in unSafeAreaHole)
		{
			areavalue += ClipperLib::Area(holepoly);
		}


		// TEST TEST TEST
		vecSw[29].Stop();

		return areavalue;
	}

	vector<vector<IntPoint>> SupportPointsList::GetPolygons(const vector<int> &UpDownTable)
	{
		// TEST TEST TEST
		vecSw[30].Start();


		vector<vector<IntPoint>> output;
		for (int i = 0; i < UpDownTable.size(); i++)
		{
			output.push_back(lowerSolid[UpDownTable[i]]);
			for(const auto &poly : modifiedLowerHole[UpDownTable[i]]) //foreach(List<IntPoint> poly in modifiedLowerHole[UpDownTable[i]])
			{
				output.push_back(poly);
			}
		}
		// TEST TEST TEST
		vecSw[30].Stop();
		return output;
	}

	vector<vector<IntPoint>> SupportPointsList::GetPolygons(const vector<IntPoint> &testPoly, const vector<vector<IntPoint>> &unSafeAreaHole)
	{
		// TEST TEST TEST
		vecSw[31].Start();


		vector<vector<IntPoint>> output;
		output.push_back(testPoly);
		for(const auto &hole : unSafeAreaHole)	//foreach(List<IntPoint> hole in unSafeAreaHole)
			output.push_back(hole);


		// TEST TEST TEST
		vecSw[31].Stop();

		return output;
	}
	/* Test Result:
	Model: Hedgehog.stl
	Total Layers: 615
	This function - ClipperGeometricOperationPolyTreeType() will totally take 74s during FindNecessarySupportPointNew() process on my Asus Core i5 notebook.
	*/
	vector<int> SupportPointsList::GetPointInUpperSolid(int count)
	{
		// TEST TEST TEST
		vecSw[32].Start();

		vector<int> number;
		for (int i = 0; i < pastLevelSupportPointsArea.size(); i++)
		{
			vector<vector<vector<IntPoint>>> output = ClipperGeometricOperationPolyTreeType(upperSolid[count],					//#2: 3s
				pastLevelSupportPointsArea[i], ClipType::ctIntersection, PolyFillType::pftPositive, PolyFillType::pftPositive);
			if (output[0].size() > 0)
			{
				number.push_back(i);
			}
		}

		// TEST TEST TEST
		vecSw[32].Stop();

		return number;
	}
	void SupportPointsList::UpdateNowSupportPointTable(int count, const vector<int> &number)
	{
		// TEST TEST TEST
		vecSw[33].Start();


		if (nowSupportPointToPolygonTable[count].size() == number.size())
		{
		}
		else
		{
			map<Vector2, float> dic;
			for(const auto &point : nowSupportPointToPolygonTable[count])	//foreach(Vector3 point in nowSupportPointToPolygonTable[count])
			{
				map<Vector2, float>::const_iterator got = dic.find(Vector2(point.x, point.y));	//		bool test = dic.ContainsKey(Vector2(point.x, point.y));
				if (got == dic.end())
					dic.insert(pair<Vector2, float>(Vector2(point.x, point.y), point.z));
				else
				{
					dic.erase(Vector2(point.x, point.y));
					dic.insert(pair<Vector2, float>(Vector2(point.x, point.y), point.z));
				}
			}
			nowSupportPointToPolygonTable[count].clear();
			for(const auto &num : number)	//foreach(int num in number)
			{
				float height;
				height = dic[Vector2((float)pastLevelSupportPoints[num].X, (float)pastLevelSupportPoints[num].Y)];	//dic.TryGetValue(Vector2(pastLevelSupportPoints[num].X, pastLevelSupportPoints[num].Y), out height);
				nowSupportPointToPolygonTable[count].push_back(Vector3((float)pastLevelSupportPoints[num].X, (float)pastLevelSupportPoints[num].Y, height));
			}
		}

		// TEST TEST TEST
		vecSw[33].Stop();

	}
	vector<vector<IntPoint>> SupportPointsList::GetSupportArea(const vector<int> &number)
	{
		// TEST TEST TEST
		vecSw[34].Start();


		//SupportPointsListDbg::DumpObjectToFile(number, "number", ".\\CPP\\number_" + to_string(currentLayer) + "_" + to_string(count) + ".txt");
		//SupportPointsListDbg::DumpObjectToFile(pastLevelSupportPointsArea, "pastLevelSupportPointsArea", ".\\CPP\\pastLevelSupportPointsArea_IN_" + to_string(currentLayer) + "_" + to_string(count) + ".txt");
		//SupportPointsListDbg::DumpObjectToFile(pastLevelSupportPoints, "pastLevelSupportPoints", ".\\CPP\\pastLevelSupportPoints_IN_" + to_string(currentLayer) + "_" + to_string(count) + ".txt");
		//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPointsArea, "nowLevelSupportPointsArea[GetSupportArea][IN]", ".\\CPP\\nowLevelSupportPointsArea_IN_" + to_string(currentLayer) + "_" + to_string(count) + ".txt");
		//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPoint, "nowLevelSupportPoint", ".\\CPP\\nowLevelSupportPoint_IN_" + to_string(currentLayer) + "_" + to_string(count) + ".txt");
		
		vector<vector<IntPoint>> output;
		if (number.size() == 1)
		{
			output.push_back(pastLevelSupportPointsArea[number[0]]);
			int checkCount1 = (int)nowLevelSupportPoint.size();

			nowLevelSupportPoint.Add(pastLevelSupportPoints[number[0]]);	//nowLevelSupportPoint.push_back(pastLevelSupportPoints[number[0]]);
			
			int checkCount2 = (int)nowLevelSupportPoint.size();
			if (checkCount1 != checkCount2)
				nowLevelSupportPointsArea.push_back(pastLevelSupportPointsArea[number[0]]);
		}
		else if (number.size() > 1)
		{
			int checkCount1 = (int)nowLevelSupportPoint.size();

			nowLevelSupportPoint.Add(pastLevelSupportPoints[number[0]]);	//nowLevelSupportPoint.push_back(pastLevelSupportPoints[number[0]]);

			int checkCount2 = (int)nowLevelSupportPoint.size();
			if (checkCount1 != checkCount2)
				nowLevelSupportPointsArea.push_back(pastLevelSupportPointsArea[number[0]]);
			vector<vector<IntPoint>> polygons;
			for (int i = 1; i < number.size(); i++)
			{
				polygons.push_back(pastLevelSupportPointsArea[number[i]]);
				checkCount1 = (int)nowLevelSupportPoint.size();

				nowLevelSupportPoint.Add(pastLevelSupportPoints[number[i]]);	//nowLevelSupportPoint.push_back(pastLevelSupportPoints[number[i]]);

				checkCount2 = (int)nowLevelSupportPoint.size();
				if (checkCount1 == checkCount2)
					continue;
				nowLevelSupportPointsArea.push_back(pastLevelSupportPointsArea[number[i]]);
			}
			output = ClipperGeometricOperationNormalType(pastLevelSupportPointsArea[number[0]], polygons, ClipType::ctUnion, PolyFillType::pftPositive, PolyFillType::pftPositive);
		}
		//SupportPointsListDbg::DumpObjectToFile(nowLevelSupportPoint, "nowLevelSupportPoint", ".\\CPP\\nowLevelSupportPoint_" + to_string(currentLayer) + "_" + to_string(count) + ".txt");
		
		// TEST TEST TEST
		vecSw[34].Stop();
		
		return output;
	}
	void SupportPointsList::SortPolygons(vector<vector<IntPoint>> &allpolygons)
	{
		// TEST TEST TEST
		vecSw[35].Start();


		vector<double> areavalue;
		vector<double> xareavalue;
		vector<vector<IntPoint>> xallpolygons;
		for(const auto &poly : allpolygons)//foreach(List<IntPoint> poly in allpolygons)
		{
			double value = ClipperLib::Area(poly);
			areavalue.push_back(ClipperLib::Area(poly));
			xareavalue.push_back(ClipperLib::Area(poly));
			xallpolygons.push_back(poly);
		}
		sort(areavalue.begin(), areavalue.end());	//areavalue.sort();
		allpolygons.clear();
		for (int i0 = (int)areavalue.size() - 1; i0 >= 0; i0--)
		{
			for (int i = 0; i < xareavalue.size(); i++)
			{
				if (areavalue[i0] == xareavalue[i])
				{
					allpolygons.push_back(xallpolygons[i]);
					xareavalue.erase(xareavalue.begin() + i);		
					xallpolygons.erase(xallpolygons.begin() + i);
					break;
				}
			}
		}

		// TEST TEST TEST
		vecSw[35].Stop();
	}
	vector<int> SupportPointsList::GetTag(vector<vector<IntPoint>> &allpolygons, vector<vector<IntPoint>> &sortpolygons)
	{
		// TEST TEST TEST
		vecSw[36].Start();


		vector<int> tag;
		for (int num1 = 0; num1 < sortpolygons.size(); num1++)
		{
			for (int num2 = 0; num2 < allpolygons.size(); num2++)
			{
				if (sortpolygons[num1] == allpolygons[num2])
				{
					tag.push_back(num2);
				}
			}
		}


		// TEST TEST TEST
		vecSw[36].Stop();


		return tag;
	}

	void SupportPointsList::CheckHole(vector<IntPoint> &subjectpoly, vector<vector<IntPoint>> &allholepolygon)
	{
		// TEST TEST TEST
		vecSw[37].Start();


		vector<vector<IntPoint>> allhole;
		for(const auto &poly : allholepolygon)//foreach(List<IntPoint> poly in allholepolygon)
		{
			vector<vector<vector<IntPoint>>> test = ClipperGeometricOperationPolyTreeType(subjectpoly, poly, 
				ClipType::ctIntersection, PolyFillType::pftEvenOdd, PolyFillType::pftEvenOdd);
			if (test[0].size() > 0)
			{
				allhole.push_back(poly);
			}
		}
		allholepolygon.clear();
		for(const auto &poly : allhole)	//foreach(List<IntPoint> poly in allhole)
		{
			allholepolygon.push_back(poly);
		}

		// TEST TEST TEST
		vecSw[37].Stop();
	}

	vector<IntPoint> SupportPointsList::GetCandidateSupportPoint(vector<IntPoint> &testpoly)
	{
		// TEST TEST TEST
		vecSw[38].Start();


		vector<IntPoint> output;
		for (int i = 0; i < testpoly.size(); i++)
		{
			output.push_back(testpoly[i]);
			double k = sqrt(pow(testpoly[i].X - testpoly[(i + 1) % testpoly.size()].X, 2) + pow(testpoly[i].Y - testpoly[(i + 1) % testpoly.size()].Y, 2));
			if (k > thresholdList[1] * 1.5)
			{
				k = k / thresholdList[1];
				Vector2 xy((float)testpoly[(i + 1) % testpoly.size()].X - testpoly[i].X, (float)testpoly[(i + 1) % testpoly.size()].Y - testpoly[i].Y);
				xy.normalize();
				for (int i2 = 1; i2 < k; i2++)
				{
					output.push_back(IntPoint(static_cast<cInt>(testpoly[i].X + xy.x * thresholdList[1] * i2), static_cast<cInt>(testpoly[i].Y + xy.y * thresholdList[1] * i2)));
				}
			}
		}

		// TEST TEST TEST
		vecSw[38].Stop();

		return output;
	}
	//checked OK. 2019/8/13.
	vector<double> SupportPointsList::CalculateEveryPointtoSupportMinDistance(const vector<IntPoint> &polygon, const vector<int> &number, bool isDensityTooHigh)
	{
		// TEST TEST TEST
		vecSw[39].Start();


		vector<double> distancelist;
		for(const auto &pt:polygon)	// C#: foreach(IntPoint pt in polygon)
		{
			double distance = sqrt(pow(pt.X - pastLevelSupportPoints[0].X, 2) + pow(pt.Y - pastLevelSupportPoints[0].Y, 2));
			for(const auto &i:number) // C#foreach(int i in number)
			{
				double distance2 = sqrt(pow(pt.X - pastLevelSupportPoints[i].X, 2) + pow(pt.Y - pastLevelSupportPoints[i].Y, 2));
				//int threshold = 0;
				//if (isDensityTooHigh == true)
				//    threshold = -thresholdList[3];
				//if (distance2 <= (thresholdList[0] + threshold) * 1.1)
				//{
				//    distance = 0;
				//    break;
				//}
				if (distance2 <= 15500)
				{
					distance = 0;
					break;
				}
				if (distance > distance2)
				{
					distance = distance2;
				}
			}
			distancelist.push_back(distance);
		}

		// TEST TEST TEST
		vecSw[39].Stop();


		return distancelist;
	}

	int SupportPointsList::FindMaxDistance(const vector<double> &distancelist)
	{
		// TEST TEST TEST
		vecSw[40].Start();


		int output = 0;
		double distance = distancelist[0];
		for (int i = 1; i < distancelist.size(); i++)
		{
			if (distance < distancelist[i])
			{
				distance = distancelist[i];
			}
		}
		for (int i = 0; i < distancelist.size(); i++)
		{
			if (distance == distancelist[i])
			{
				output = i;
				break;
			}
		}

		// TEST TEST TEST
		vecSw[40].Stop();

		return output;
	}

	bool SupportPointsList::GetTriangleId(IntPoint point, double height, /*out*/ int &meshid)
	{
		// TEST TEST TEST
		vecSw[41].Start();


		unordered_map<int, double>::const_iterator getvalue = meshModel.horizontaltriangle.find(static_cast<int>(height - 0.1));
		unordered_map<int, double>::const_iterator getvalue2 = meshModel.horizontaltriangle.find((int)height);

		bool v = false;
		if (getvalue != meshModel.horizontaltriangle.end() || getvalue2 != meshModel.horizontaltriangle.end())
			v = true;
		meshid = 0;
		vector<int> meshidlist;
		bool output = false;
		if (v == true)
		{
			for(const auto &kvp : meshModel.horizontaltriangle)	// C#: foreach(KeyValuePair<int, double> kvp in meshModel.horizontaltriangle)
			{
				if (kvp.second == height || kvp.second == (height - 0.1))
					meshidlist.push_back(kvp.first);
			}
			int k = 0;
			double distance = numeric_limits<double>::infinity();	// double.MaxValue;
			for(const auto &i : meshidlist)	//foreach(int i in meshidlist)
			{
				Vector2d pa(meshModel.Meshs[i].Vector3ds[0].x - point.X / 10000.0, meshModel.Meshs[i].Vector3ds[0].y - point.Y / 10000.0);
				Vector2d pb(meshModel.Meshs[i].Vector3ds[1].x - point.X / 10000.0, meshModel.Meshs[i].Vector3ds[1].y - point.Y / 10000.0);
				Vector2d pc(meshModel.Meshs[i].Vector3ds[2].x - point.X / 10000.0, meshModel.Meshs[i].Vector3ds[2].y - point.Y / 10000.0);
				if (distance > sqrt(pow(pa.x, 2) + pow(pa.y, 2)))
				{
					k = i;
					distance = sqrt(pow(pa.x, 2) + pow(pa.y, 2));
				}
				if (distance > sqrt(pow(pb.x, 2) + pow(pb.y, 2)))
				{
					k = i;
					distance = sqrt(pow(pb.x, 2) + pow(pb.y, 2));
				}
				if (distance > sqrt(pow(pc.x, 2) + pow(pc.y, 2)))
				{
					k = i;
					distance = sqrt(pow(pc.x, 2) + pow(pc.y, 2));
				}
				double Crossproductpapb = round((pa.x * pb.y - pb.x * pa.y)*100.0) / 100.0;	//double Crossproductpapb = Math.Round(pa.X * pb.Y - pb.X * pa.Y, 2, MidpointRounding.AwayFromZero);
				double Crossproductpbpc = round((pb.x * pc.y - pc.x * pb.y)*100.0) / 100.0;	//double Crossproductpbpc = Math.Round(pb.X * pc.Y - pc.X * pb.Y, 2, MidpointRounding.AwayFromZero);
				double Crossproductpcpa = round((pc.x * pa.y - pa.x * pc.y)*100.0) / 100.0;	//double Crossproductpcpa = Math.Round(pc.X * pa.Y - pa.X * pc.Y, 2, MidpointRounding.AwayFromZero);
				if (Crossproductpcpa * Crossproductpapb >= 0 && Crossproductpapb * Crossproductpbpc >= 0 && Crossproductpbpc * Crossproductpcpa >= 0)
				{
					meshid = i;
					output = true;
					break;
				}
			}
			if (output == false)
			{
				if (distance < 0.3)
				{
					meshid = k;
					output = true;
				}
			}
		}

		// TEST TEST TEST
		vecSw[41].Stop();

		return output;
	}

	int SupportPointsList::FindMeshId(IntPoint point, int level)
	{
		// TEST TEST TEST
		vecSw[42].Start();


		bool findpoint = false;
		int levelcount = -1;
		int polycount = -1;
		int pointcount = -1;
		double value = numeric_limits<double>::infinity(); // C#: double.MaxValue;
		int valuelevel = -1;
		int valuepoly = -1;
		int valuepoint = -1;
		vector<vector<vector<IntPoint>>> &originalpoly = layerModel.alllayer;	//*****************
		vector<ModelLayer> &Polygons = layerModel.Layers;						//*****************
		vector<RHVector3> &normallist = meshModel.normallist;					//*****************
		vector<map<Vector4d, int>> &dictionary = segmentModel.dictionary;		//*****************
		for (int L = 0; L <= 1; L++)
		{
			for (int t = 0; t < originalpoly[level - L].size(); t++)
			{
				if (findpoint == false)
				{
					for (int c = 0; c < originalpoly[level - L][t].size() - 1; c++)
					{
						IntPoint PA(point.X - originalpoly[level - L][t][c].X, point.Y - originalpoly[level - L][t][c].Y);
						IntPoint PB(point.X - originalpoly[level - L][t][(c + 1) % originalpoly[level - L][t].size()].X, point.Y - originalpoly[level - L][t][(c + 1) % originalpoly[level - L][t].size()].Y);
						if (PA.X == 0 && PA.Y == 0)
						{
							findpoint = true;
							levelcount = level - L;
							polycount = t;
							if (c == 0)
							{
								int meshid1 = GetId(Vector4d(Polygons[level - L].Polygons[t].Points[c].x, Polygons[level - L].Polygons[t].Points[c].y, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].x, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].y), dictionary[level - L]);
								int meshid2 = GetId(Vector4d(Polygons[level - L].Polygons[t].Points[Polygons[level - L].Polygons[t].Points.size() - 1].x, Polygons[level - L].Polygons[t].Points[Polygons[level - L].Polygons[t].Points.size() - 1].y, Polygons[level - L].Polygons[t].Points[c].x, Polygons[level - L].Polygons[t].Points[c].y), dictionary[level - L]);
								if (normallist[meshid1].z < normallist[meshid2].z)
									pointcount = c;
								else
									pointcount = (int)Polygons[level - L].Polygons[t].Points.size() - 1;
							}
							else
							{
								int meshid1 = GetId(Vector4d(Polygons[level - L].Polygons[t].Points[c].x, Polygons[level - L].Polygons[t].Points[c].y, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].x, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].y), dictionary[level - L]);
								int meshid2 = GetId(Vector4d(Polygons[level - L].Polygons[t].Points[c - 1].x, Polygons[level - L].Polygons[t].Points[c - 1].y, Polygons[level - L].Polygons[t].Points[c].x, Polygons[level - L].Polygons[t].Points[c].y), dictionary[level - L]);
								if (normallist[meshid1].z < normallist[meshid2].z)
									pointcount = c;
								else
									pointcount = c - 1;
							}
							break;
						}
						else if (PB.X == 0 && PB.Y == 0)
						{
							findpoint = true;
							levelcount = level - L;
							polycount = t;
							int meshid1 = GetId(Vector4d(Polygons[level - L].Polygons[t].Points[c].x, Polygons[level - L].Polygons[t].Points[c].y, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].x, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].y), dictionary[level - L]);
							int meshid2 = GetId(Vector4d(Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].x, Polygons[level - L].Polygons[t].Points[(c + 1) % Polygons[level - L].Polygons[t].Points.size()].y, Polygons[level - L].Polygons[t].Points[(c + 2) % Polygons[level - L].Polygons[t].Points.size()].x, Polygons[level - L].Polygons[t].Points[(c + 2) % Polygons[level - L].Polygons[t].Points.size()].y), dictionary[level - L]);
							if (normallist[meshid1].z < normallist[meshid2].z)
								pointcount = c;
							else
								pointcount = (c + 1) % Polygons[level - L].Polygons[t].Points.size();
							break;
						}
						IntPoint AB(originalpoly[level - L][t][(c + 1) % originalpoly[level - L][t].size()].X - originalpoly[level - L][t][c].X, originalpoly[level - L][t][(c + 1) % originalpoly[level - L][t].size()].Y - originalpoly[level - L][t][c].Y);
						double crossvalue = static_cast<double>(PA.X * AB.Y - PA.Y * AB.X);
						if (AB.X == 0 && AB.Y == 0)
						{
						}
						else
						{
							double lengthPA = sqrt(PA.X * PA.X + PA.Y * PA.Y);
							double lengthPB = sqrt(PB.X * PB.X + PB.Y * PB.Y);
							double lengthAB = sqrt(AB.X * AB.X + AB.Y * AB.Y);
							double abscrossvalue = abs(crossvalue);
							double h = abscrossvalue / lengthAB;
							if (lengthPB * lengthPB >= lengthPA * lengthPA + lengthAB * lengthAB)
							{
								if (value > lengthPA)
								{
									value = lengthPA;
									valuelevel = level - L;
									valuepoly = t;
									valuepoint = c;
								}
							}
							else if (lengthPA * lengthPA >= lengthPB * lengthPB + lengthAB * lengthAB)
							{
								if (value > lengthPB)
								{
									value = lengthPB;
									valuelevel = level - L;
									valuepoly = t;
									valuepoint = c;
								}
							}
							else if (value > h)
							{
								value = h;
								valuelevel = level - L;
								valuepoly = t;
								valuepoint = c;
							}
						}
						double dotvalue = static_cast<double>(PA.X * AB.X + PA.Y * AB.Y);
						double test1 = static_cast<double>(PA.X * AB.X);
						double test2 = static_cast<double>(PA.Y * AB.Y);
						double distance1 = static_cast<double>(PA.X * PA.X + PA.Y * PA.Y);
						double distance2 = static_cast<double>(AB.X * AB.X + AB.Y * AB.Y);
						if (crossvalue == 0 && dotvalue > 0 && distance2 > distance1)
						{
							findpoint = true;
							levelcount = level - L;
							polycount = t;
							pointcount = c;
							break;
						}
					}
				}
				else
					break;
			}
		}
		if (findpoint == false)
		{
			levelcount = valuelevel;
			polycount = valuepoly;
			pointcount = valuepoint;
		}
		Vector4d segment(Polygons[levelcount].Polygons[polycount].Points[pointcount].x, Polygons[levelcount].Polygons[polycount].Points[pointcount].y, Polygons[levelcount].Polygons[polycount].Points[(pointcount + 1) % Polygons[levelcount].Polygons[polycount].Points.size()].x, Polygons[levelcount].Polygons[polycount].Points[(pointcount + 1) % Polygons[levelcount].Polygons[polycount].Points.size()].y);
		int meshID = GetId(segment, dictionary[levelcount]);	//#43: 0s

		// TEST TEST TEST
		vecSw[42].Stop();
		return meshID;
	}

	int SupportPointsList::GetId(Vector4d segment, map<Vector4d, int> &dictionary)
	{
		// TEST TEST TEST
		vecSw[43].Start();


		//unordered_set<Vector4, int>::const_iterator got = dictionary.find(segment);	//	bool test = dictionary.ContainsKey(segment);
		//if (test == false)
		//errorlist.Add(segment);
		int meshID = 0;
		meshID = dictionary[segment];	// dictionary.TryGetValue(segment, out meshID);

		// TEST TEST TEST
		vecSw[43].Stop();

		return meshID;
	}

	RHVector3 SupportPointsList::FindNewLocation(IntPoint supportpoint, double height, int meshID, double size)
	{
		// TEST TEST TEST
		vecSw[44].Start();

		
		//adjst the location from edge to face
		vector<RHVector3> output;
		double threshold = (size + 0.1) * 10000;
		RHVector3 center(0, 0, 0);
		for(const auto &pt : meshModel.Meshs[meshID].Vector3ds) //foreach(Vector3d pt in meshModel.Meshs[meshID].Vector3ds)
		{
			center.x = center.x + pt.x;
			center.y = center.y + pt.y;
			center.z = center.z + pt.z;
		}
		center.x = center.x / 3;
		center.y = center.y / 3;
		center.z = center.z / 3;
		center.x = center.x * 10000;
		center.y = center.y * 10000;
		RHVector3 point((double)supportpoint.X, (double)supportpoint.Y, height);
		RHVector3 vector = center.Subtract(point);
		double l = vector.Length();
		if (threshold >= l)
		{
	
			// TEST TEST TEST
			vecSw[44].Stop();

			return center;
		}
		else
		{
			vector.Scale(threshold / l);
			RHVector3 finalpoint = point.Add(vector);

			// TEST TEST TEST
			vecSw[44].Stop();

			return finalpoint;
		}
	}

	vector<IntPoint> SupportPointsList::AdjustSupportPoint(int maxnumber, const vector<IntPoint> &uppoly, const vector<IntPoint> &originalpoly, double size)
	{
		// TEST TEST TEST
		vecSw[45].Start();

		double threshold = (size + 0.1) * 10000;
		vector<IntPoint> output;
		set<IntPoint> x;
		for(const auto & p : originalpoly)	//foreach(IntPoint p in originalpoly)
			x.insert(p);

		set<IntPoint>::const_iterator got = x.find(uppoly[maxnumber]);	//	bool q = x.Contains(uppoly[maxnumber]);

		RHVector3 vector1(0, 0, 0);
		RHVector3 vector2(0, 0, 0);
		IntPoint p1(0, 0);
		IntPoint p2(0, 0);
		if (got != x.end())
		{
			int num = 0;
			for (int i = 0; i < originalpoly.size(); i++)
			{
				if (uppoly[maxnumber] == originalpoly[i])
				{
					num = i;
					break;
				}
			}
			vector1 = RHVector3(originalpoly[(num + 1) % originalpoly.size()].X - originalpoly[num].X, originalpoly[(num + 1) % originalpoly.size()].Y - originalpoly[num].Y, (cInt)0);
			if (num == 0)
				vector2 = RHVector3(originalpoly[originalpoly.size() - 1].X - originalpoly[num].X, originalpoly[originalpoly.size() - 1].Y - originalpoly[num].Y, (cInt)0);
			else
				vector2 = RHVector3(originalpoly[num - 1].X - originalpoly[num].X, originalpoly[num - 1].Y - originalpoly[num].Y, (cInt)0);
			if (vector1.Length() > threshold)
				vector1.Scale(threshold / vector1.Length());
			if (vector2.Length() > threshold)
				vector2.Scale(threshold / vector2.Length());
			p1 = IntPoint(static_cast<cInt>(uppoly[maxnumber].X + vector1.x), static_cast<cInt>(uppoly[maxnumber].Y + vector1.y));
			p2 = IntPoint(static_cast<cInt>(uppoly[maxnumber].X + vector2.x), static_cast<cInt>(uppoly[maxnumber].Y + vector2.y));
			output.push_back(p1);
			output.push_back(p2);
		}
		else
		{
			p1 = IntPoint(uppoly[maxnumber].X, uppoly[maxnumber].Y);
			output.push_back(p1);
		}


		// TEST TEST TEST
		vecSw[45].Stop();
		return output;
	}
	vector<RHVector3> SupportPointsList::CheckNormalvector(vector<IntPoint> adjustpointlist, int maxnumber, const vector<IntPoint> &uppoly, const vector<vector<IntPoint>> &downpoly)
	{
		// TEST TEST TEST
		vecSw[46].Start();


		vector<RHVector3> output;
		if (adjustpointlist.size() == 1)
		{

			RHVector3 v1 = FindNormalvector(adjustpointlist[0], uppoly[(maxnumber + 1) % uppoly.size()], downpoly);
			output.push_back(v1);
		}
		else
		{
			RHVector3 v1 = FindNormalvector(uppoly[maxnumber], adjustpointlist[0], downpoly);
			RHVector3 v2 = FindNormalvector(adjustpointlist[1], uppoly[maxnumber], downpoly);
			output.push_back(v1);
			output.push_back(v2);
		}

		// TEST TEST TEST
		vecSw[46].Stop();

		return output;
	}
	RHVector3 SupportPointsList::FindNormalvector(IntPoint point1, IntPoint point2, const vector<vector<IntPoint>> &downpoly)
	{
		// TEST TEST TEST
		vecSw[47].Start();

		double minvalue = sqrt(pow(point1.X - downpoly[0][0].X, 2) + pow(point1.Y - downpoly[0][0].Y, 2));
		int number1 = 0;
		int number2 = 0;
		for (int polynum = 0; polynum < downpoly.size(); polynum++)
		{
			for (int pointnum = 0; pointnum < downpoly[polynum].size(); pointnum++)
			{
				double value = sqrt(pow(point1.X - downpoly[polynum][pointnum].X, 2) + pow(point1.Y - downpoly[polynum][pointnum].Y, 2));
				if (minvalue > value)
				{
					minvalue = value;
					number1 = polynum;
					number2 = pointnum;
				}
			}
		}
		RHVector3 v1(downpoly[number1][number2].X - point1.X, downpoly[number1][number2].Y - point1.Y, (cInt)-0.1);
		RHVector3 v2(point2.X - point1.X, point2.Y - point1.Y, (cInt)0);
		v1.x = v1.x / 10000;
		v1.y = v1.y / 10000;
		v2.x = v2.x / 10000;
		v2.y = v2.y / 10000;
		//v1.NormalizeSafe();
		//v2.NormalizeSafe();
		RHVector3 output = v1.CrossProduct(v2);
		output.NormalizeSafe();
		if (v1.x * v2.y == v2.x * v1.y)
			output = RHVector3(0, 0, -1);

		// TEST TEST TEST
		vecSw[47].Stop();
		return output;
	}

	RHVector3 SupportPointsList::AddSupportPoint(RHVector3 point, bool isDensityTooHigh, int meshId, double size, int count)
	{
		// TEST TEST TEST
		vecSw[48].Start();


		RHVector3 normalvector = meshModel.normallist[meshId];
		if (normalvector.z < 0)
		{
			RHVector3 OriginalPoint(point.x * 0.0001, point.y * 0.0001, point.z);
			SupportPoint pt(OriginalPoint, normalvector, size);
			allGeneratedSupportPoint.push_back(pt);
			nowLevelSupportPoint.Add(IntPoint((cInt)point.x, (cInt)point.y));//nowLevelSupportPoint.push_back(IntPoint(point.x, point.y));
			vector<IntPoint> supportArea = PointToSupportArea(IntPoint((cInt)point.x, (cInt)point.y), normalvector, isDensityTooHigh);
			nowLevelSupportPointsArea.push_back(supportArea);
			nowSupportPointToPolygonTable[count].push_back(Vector3((float)point.x, (float)point.y, (float)point.z));
		}
		else if (normalvector.z == 0)
		{
			RHVector3 OriginalPoint(point.x * 0.0001, point.y * 0.0001, point.z);
			normalvector.SubtractInternal(RHVector3(0, 0, 0.1));
			normalvector.NormalizeSafe();
			SupportPoint pt(OriginalPoint, normalvector, size);
			allGeneratedSupportPoint.push_back(pt);
			nowLevelSupportPoint.Add(IntPoint((cInt)point.x, (cInt)point.y));//nowLevelSupportPoint.push_back(IntPoint(point.x, point.y));
			vector<IntPoint> supportArea = PointToSupportArea(IntPoint((cInt)point.x, (cInt)point.y), normalvector, isDensityTooHigh);
			nowLevelSupportPointsArea.push_back(supportArea);
			nowSupportPointToPolygonTable[count].push_back(Vector3((float)point.x, (float)point.y, (float)point.z));
		}

		// TEST TEST TEST
		vecSw[48].Stop();
		return normalvector;
	}
}