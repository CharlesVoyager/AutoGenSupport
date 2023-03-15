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

namespace XYZSupport
{
	void SupportMeshsCylinder::Generate2(const vector<unique_ptr<SupportSymbol>> &symbols)
	{
		Stopwatch sw;
		sw.Start();
		DbgMsg(__FUNCTION__ "[IN]");
		DbgMsg(__FUNCTION__ "===> Symbols.size: %d (Manual touch points)", Symbols.size());	//ISupportSymbols Symbols ===> touch points
		DbgMsg(__FUNCTION__ "===> symbols.size: %d (Cone number)", symbols.size());

		if (Symbols.size() > 0)
		{
			bbox.Clear();
			for(const auto &v : Cubedata2.Model.vertices._v)	//for(int i=0; i< Cubedata2.Model.vertices._v.size(); i++)		// C#: foreach(TopoVertex v in Cubedata2.Model.vertices.v)
			{
				includePoint(v.pos);	// set bbox
			}
			Cubedata2.Model.boundingBox = bbox;
			DbgMsg(__FUNCTION__ "===> Cubedata2.Model.boundingBox: %s", Cubedata2.Model.boundingBox.ToString().c_str());
			
			UpdateSpacePartition(); // Generate the cubeMarix cubeSpaceforNewsup
			DbgMsg(__FUNCTION__ "===> CubeMatrix cubeSpaceforNewsup: %s", cubeSpaceforNewsup.ToString().c_str());
						
			float CubeSpan = 2.0;
			SetCubeCenterpoint(CubeSpan);									//set cubeSpaceforNewsup
			UpdateConeSupportCubeInfo(symbols);
			UpdateInfoforConnectPlateform(CubeSpan);//newsup紀錄cube資訊 
			genSupFromList(symbols);
		}
		sw.Stop();
		DbgMsg(__FUNCTION__"===> Generate2() time: %ld (ms)", sw.ElapsedMilliseconds());
		DbgMsg(__FUNCTION__ "[OUT]");
	}

	void SupportMeshsCylinder::UpdateConeSupportCubeInfo(const vector<unique_ptr<SupportSymbol>> &symbols)
	{
		vector<Tuple<RHVector3, double>> ConeList;
		for(const auto &s : symbols)		//C# AP: foreach(SupportCone.SupportSymbolCone supMark in symbols)
		{
			Tuple<RHVector3, double> Cone = Tuple<RHVector3, double>(s->GetPosition(), s->GetRadiusBase());
			ConeList.push_back(Cone);
		}
		for(const auto &Cone : ConeList)	//for (int k = 0; k < ConeList.size(); k++)//C# AP: foreach(Tuple<RHVector3, double> Cone in ConeList)
		{
			//計算支撐點在哪個cube
			int CubeX, CubeY, CubeZ;
			CubeX = lrint(floor((Cone.First().x - cubeSpaceforNewsup.MinPoint.x) / 2));//changespan
			CubeY = lrint(floor((Cone.First().y - cubeSpaceforNewsup.MinPoint.y) / 2));
			CubeZ = lrint(floor((Cone.First().z - cubeSpaceforNewsup.MinPoint.z) / 2));
			int ConeRadius = lrint(ceil((Cone.Second() - 1) / 2) + 1);
			for (int z = CubeZ; z >= 0; z--)
			{
				cubeSpaceforNewsup.GetCubeByIndex(CubeX, CubeY, z).EmptyCube = false;
			}
			for (int i = 1; i < ConeRadius; i++)
			{
				for (int j = 0; j < i * 2; j++)
				{
					for (int z = CubeZ; z >= 0; z--)
					{
						cubeSpaceforNewsup.GetCubeByIndex(CubeX - i + j, CubeY + i, z).EmptyCube = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX - i + j, CubeY + i, z).ConnectPlatform = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX + i - j, CubeY - i, z).EmptyCube = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX + i - j, CubeY - i, z).ConnectPlatform = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX + i, CubeY + i - j, z).EmptyCube = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX + i - j, CubeY - i, z).ConnectPlatform = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX - i, CubeY - i + j, z).EmptyCube = false;
						cubeSpaceforNewsup.GetCubeByIndex(CubeX + i - j, CubeY - i, z).ConnectPlatform = false;
					}
				}
			}
		}
	}

	void SupportMeshsCylinder::UpdateInfoforConnectPlateform(float nspan)//newsup
	{
		int span = 1;

		for (int i = 1; i < cubeSpaceforNewsup.XNum() - span; i++)
		{
			for (int j = 1; j < cubeSpaceforNewsup.YNum() - span; j++)
			{
				int z = 0;
				if (cubeSpaceforNewsup.GetCubeByIndex(i, j, z).EmptyCube == true)
				{
					if (cubeSpaceforNewsup.GetCubeByIndex(i + span, j, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i, j + span, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i - span, j, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i, j - span, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i + span, j + span, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i + span, j - span, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i - span, j + span, z).EmptyCube == true
						&& cubeSpaceforNewsup.GetCubeByIndex(i - span, j - span, z).EmptyCube == true
						//boundingbox需要加大
						)
					{
						cubeSpaceforNewsup.GetCubeByIndex(i, j, z).ConnectPlatform = true;
						cubeSpaceforNewsup.GetCubeByIndex(i, j, z).ObjectBottomEdge = false;
					}
					// else cubeSpaceforNewsup[i, j, 0].ObjectBottomEdge = true;
				}
			}
		}
		for (int i = 0; i < cubeSpaceforNewsup.XNum(); i++)
		{
			for (int j = 0; j < cubeSpaceforNewsup.YNum(); j++)
			{
				for (int k = 0; k < cubeSpaceforNewsup.ZNum() - 1; k++)
				{
					if (cubeSpaceforNewsup.GetCubeByIndex(i, j, k).EmptyCube == false)
						break;
					if (cubeSpaceforNewsup.GetCubeByIndex(i, j, k).ConnectPlatform == true && cubeSpaceforNewsup.GetCubeByIndex(i, j, k + 1).EmptyCube == true)
						cubeSpaceforNewsup.GetCubeByIndex(i, j, k + 1).ConnectPlatform = true;
				}
				for (int k = 0; k < cubeSpaceforNewsup.ZNum() - 1; k++)
				{
					if (cubeSpaceforNewsup.GetCubeByIndex(i, j, k).ObjectBottomEdge == false)
						cubeSpaceforNewsup.GetCubeByIndex(i, j, k + 1).ObjectBottomEdge = false;
				}
			}
		}
	}

	bool SupportMeshsCylinder::CheckConeDistance(SupportPoint &point, const vector<unique_ptr<SupportSymbol>> &symbols)
	{
		bool output = false;
		vector<Tuple<RHVector3, double>> ConeList;
		for (const auto &psupMark : symbols)	// C# AP: foreach(SupportCone.SupportSymbolCone supMark in symbols)
		{
			Tuple<RHVector3, double> Cone = Tuple<RHVector3, double>(psupMark->GetPosition(), psupMark->GetRadiusBase());
			ConeList.push_back(Cone);
		}
		if (ConeList.size() > 0)
		{
			for (int k = 0; k < ConeList.size(); k++)//	C# AP: foreach(Tuple<RHVector3, double> Cone in ConeList)
			{
				Tuple<RHVector3, double> Cone = ConeList[k];
				int ConeRadius = lrint(ceil((Cone.Second() - 1) / 2) + 1);
				double distance = sqrt(pow(point.GetPosition().x - Cone.First().x, 2) + pow(point.GetPosition().y - Cone.First().y, 2));
				if (ConeRadius > distance)
				{
					output = true;
					break;
				}
			}
		}
		return output;
	}
	// Note: The input argument should be model coordinate. 
	//       For example, the model is "Cube". The vertices of coordinate are like (-5, -5, -5), (5, -5, -5), .....
	void SupportMeshsCylinder::includePoint(RHVector3 v)
	{
		float x, y, z;
		Vector4 *pv4 = v.asVector4();

		x = Cubedata2.Trans.Column0().dot(*pv4);
		y = Cubedata2.Trans.Column1().dot(*pv4);
		z = Cubedata2.Trans.Column2().dot(*pv4);
		bbox.Add(RHVector3(x, y, z));
		delete pv4;
	}
	void SupportMeshsCylinder::SetCubeCenterpoint(float nspan)//newsup	//C# AP: GetCubeCenterpoint() --> changed name to SetCubeCenterpoint() for match its meaning
	{
		for (int i = 0; i < cubeSpaceforNewsup.XNum(); i++)
		{
			for (int j = 0; j < cubeSpaceforNewsup.YNum(); j++)
			{
				for (int k = 0; k < cubeSpaceforNewsup.ZNum(); k++)
				{
					RHVector3 CenterPoint(cubeSpaceforNewsup.GetCubeByIndex(i, j, k).XLowBound() + (nspan / 2), cubeSpaceforNewsup.GetCubeByIndex(i, j, k).YLowBound() + (nspan / 2), cubeSpaceforNewsup.GetCubeByIndex(i, j, k).ZLowBound() + (nspan / 2));//changespan              
					cubeSpaceforNewsup.GetCubeByIndex(i, j, k).Centerpoint = CenterPoint;
				}
			}
		}
	}

	void SupportMeshsCylinder::UpdateSpacePartition()
	{
		calVtxWorldCoordinate();
		OriginalBboxMax = RHVector3(bbox.maxPoint);
		OriginalBboxMin = RHVector3(bbox.minPoint);

		double spanX = (bbox.xMax() - bbox.xMin());
		double spanY = (bbox.yMax() - bbox.yMin());
		double spanZ = (bbox.zMax() - bbox.zMin());
		double spanSmall = 0;
		if (spanX < spanY)
			spanSmall = spanX;
		else
			spanSmall = spanY;
		if (spanZ < spanSmall)
			spanSmall = spanZ;
		int span = (int)(spanSmall / 10);
		if (span < 1) span = 1;
			
		cubeSpace.Clean();
	
		BuildUniformGridSpaceStructureForCD((float)span, 1.01, cubeSpace);

		float LargeSpan = (float)(2); //change span int nspan = 1; 
		float SmallSpan = (float)(1);
	
		cubeSpaceforNewsup.Clean();
		
		BuildUniformGridSpaceStructureForNewsup(/*in*/Cubedata2.Model, LargeSpan, 1.01, bbox, BoundingBoxWOSupport(), /*out*/cubeSpaceforNewsup);// BoundingBoxWOSupport() ==> return bbox
		dirtySpaceInfo = false;
	}

	// The function below is required when the model is rotated. Checked OK 2019/6/11.
	void SupportMeshsCylinder::calVtxWorldCoordinate()
	{
		int vId;
		TopoTriangle triWor;
		int totalTriangle;

		vtxPosWorldCor.clear();
		triNormalWorldCor.clear();

		vtxPosWorldCor.resize(Cubedata2.Model.vertices.Count());
		triNormalWorldCor.resize(Cubedata2.Model.triangles.Count());
		totalTriangle = Cubedata2.Model.triangles.Count();

		for (int triIdx = 0; triIdx < totalTriangle; triIdx++)    // 模型總三角網格數
		{
			triWor = getTriWorByMesh(triIdx, Cubedata2.Model, Cubedata2.Trans);
			triNormalWorldCor[triIdx] = triWor.normal;
			for (int i = 0; i < 3; i++)
			{
				// store vertex position by vertex id
				vId = Cubedata2.Model.triangles._t[triIdx].vertices[i].id;
				vtxPosWorldCor[vId] = triWor.vertices[i].pos;
			}
		}
	}

	void SupportMeshsCylinder::BuildUniformGridSpaceStructureForNewsup(/*in*/TopoModel &model, 
																		float span, 
																		double upperBound, 
																		RHBoundingBox &bbox, 
																		RHBoundingBox &BoundingBoxWOSupport, 
																		/*out*/CubeMatrix &cubeMatrixforNewsup)
	{
		/* If no model information, we build CubeMatrix based on current BoundingBox. So, marked the code below.
		if (0 == model.triangles.Count())
		{
			cubeMatrixforNewsup = CubeMatrix();
			return;
		}*/
		//BBOX往外加一個cube的空間方便長sup
		double ExtraSpace = 2;
		ExtraBboxforGenSupport(ExtraSpace, bbox, BoundingBoxWOSupport);	// Charles: bbox and BoundingBoxWOSupport are actually the same.
		
		RHVector3 floating(bbox.MinPoint().x, bbox.MinPoint().y, 0.0);//從平台開始切for懸空

		cubeMatrixforNewsup = CubeMatrix(floating, bbox.MaxPoint(), span);
		cubeMatrixforNewsup.BuildForNewsup(model, vtxPosWorldCor, upperBound);	// C# AP: cubeMatrixforNewsup.BuildForNewsup(model, vtxPosWorldCor, upperBound, cubeMatrixforNewsup); NOTE NOTE NOTE: The last argument is redundant.
		cubeMatrixforNewsup.CheckCubeEmptyandObjectBottom(true, Cubedata2.Model, Cubedata2.Trans);
	}

	void SupportMeshsCylinder::ExtraBboxforGenSupport(double ExtraSpace, RHBoundingBox &bbox, const RHBoundingBox &BoundingBoxWOSupport)
	{
		bbox.minPoint.x = BoundingBoxWOSupport.xMin() - ExtraSpace;
		bbox.maxPoint.x = BoundingBoxWOSupport.xMax() + ExtraSpace;
		bbox.minPoint.y = BoundingBoxWOSupport.yMin() - ExtraSpace;
		bbox.maxPoint.y = BoundingBoxWOSupport.yMax() + ExtraSpace;
	}

	void SupportMeshsCylinder::BuildUniformGridSpaceStructureForCD(float span, double upperBound, CubeMatrix &cubeMatrix)
	{
		if (0 == Cubedata2.Model.triangles.Count())
		{
			cubeMatrix = CubeMatrix();
			return;
		}
		cubeMatrix = CubeMatrix(bbox.MinPoint(), bbox.MaxPoint(), span);
		cubeMatrix.BuildForCD(Cubedata2.Model, vtxPosWorldCor, upperBound);
	}

	//SupHead
	Triple<RHVector3, RHVector3, CubeNum> SupportMeshsCylinder::CalSupportHeadPositionforNewsup(double supRadius, RHVector3 supTouchWor, RHVector3 supTouchNor, CubeMatrix &modelcube)//newsup
	{
		RHVector3 FirstHeadPosition;
		unique_ptr<SupportData> FistHead(new SupportData());
		FistHead->supType = (int)ModelType::HEAD;
		FistHead->orientation = supTouchNor;

		FistHead->orientation.NormalizeSafe();
		RHVector3 FirstHeadLength = FistHead->orientation;
		if (FirstHeadLength.Length() == 0) //切層Bug,法向量可能為0
		{
			FirstHeadLength.z = -1;
		}
		FirstHeadLength.Scale(0.3);

		FirstHeadPosition = supTouchWor.Add(FirstHeadLength);
		FistHead->SetPosition(Coord3D((float)FirstHeadPosition.x, (float)FirstHeadPosition.y, (float)FirstHeadPosition.z));

		CubeNum num = CalCubeNumforPoint(FirstHeadPosition, modelcube);
		vector<Triple<RHVector3, double, CubeNum>> SupHeadPathList = CalSupportHead(FirstHeadPosition, supTouchNor, modelcube, Largespan, num);	// Checked ok. 2019/6/12
		Tuple<RHVector3, CubeNum> SupportHeadEndandCubeNum = ChooseHeadCube(SupHeadPathList, modelcube, num, FirstHeadPosition);

		//內部支撐距離太近無法長一般支撐
		if (SupportHeadEndandCubeNum.First() == RHVector3())
		{
			SupHeadStartEnd = (Triple<RHVector3, RHVector3, CubeNum>(RHVector3(), RHVector3(), SupportHeadEndandCubeNum.Second()));//輸出起點,終點,cube num
			return SupHeadStartEnd;
		}
		GenNeedleSup(supRadius, FistHead);													// generate needle support
		_items2.push_back(move(FistHead));
		GenSupHead(SupportHeadEndandCubeNum, supRadius, FirstHeadPosition);					// generate head support
		SupHeadStartEnd = Triple<RHVector3, RHVector3, CubeNum>(supTouchWor, SupportHeadEndandCubeNum.First(), SupportHeadEndandCubeNum.Second());//輸出起點,終點,cube num
		return SupHeadStartEnd;
	}

	CubeNum SupportMeshsCylinder::CalCubeNumforPoint(RHVector3 FirstHeadPosition, const CubeMatrix &modelcube)
	{
		int CubeX = lrint(floor((FirstHeadPosition.x - modelcube.MinPoint.x) / Largespan));//changespan
		int CubeY = lrint(floor((FirstHeadPosition.y - modelcube.MinPoint.y) / Largespan));
		int CubeZ = lrint(floor((FirstHeadPosition.z - modelcube.MinPoint.z) / Largespan));

		return CubeNum (CubeX, CubeY, CubeZ);			//newsup//存cube
	}

	vector<Triple<RHVector3, double, CubeNum>> SupportMeshsCylinder::CalSupportHead(RHVector3 supTouchWorCor, RHVector3 supTouchNor, CubeMatrix &modelcube, float span, CubeNum num)//newsup
	{
		//計算支撐點在哪個cube
		vector<Triple<RHVector3, double, CubeNum>> SupHeadList;
		vector<double> SupHeadAngleList;
		vector<RHVector3> TestVector;

		int CubeX, CubeY, CubeZ;
		CubeX = num.x;
		CubeY = num.y;
		CubeZ = num.z;
		for (int n = 1; n <= 1; n++)
		{
			RHVector3 testvector1;
			if (CubeZ - 1 > 0)
			{
				testvector1 = modelcube.GetCubeByIndex(CubeX, CubeY, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector1);
				if (modelcube.GetCubeByIndex(CubeX, CubeY, CubeZ - 1).EmptyCube == true)
				{
					//if (GetTouchPerpendicularRHVector3forHead(model, num,  modelcube, supTouchWorCor, testvector1) == false)
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector1) == false)
						CalHeadAngle(modelcube, testvector1, supTouchNor, supTouchWorCor, CubeX, CubeY, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			RHVector3 testvector2;
			if (CubeX + 1 < modelcube.XNum() && CubeZ - 1 > 0)
			{
				testvector2 = modelcube.GetCubeByIndex(CubeX + 1, CubeY, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector2);
				if (modelcube.GetCubeByIndex(CubeX + 1, CubeY, CubeZ - 1).EmptyCube == true)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector2) == false)
						CalHeadAngle(modelcube, testvector2, supTouchNor, supTouchWorCor, CubeX + 1, CubeY, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////       
			RHVector3 testvector3;
			if (CubeX - 1 >= 0 && CubeZ - 1 > 0)
			{
				testvector3 = modelcube.GetCubeByIndex(CubeX - 1, CubeY, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector3);
				if (modelcube.GetCubeByIndex(CubeX - 1, CubeY, CubeZ - 1).EmptyCube == true)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector3) == false)
						CalHeadAngle(modelcube, testvector3, supTouchNor, supTouchWorCor, CubeX - 1, CubeY, CubeZ - 1, SupHeadList);
				}
			}
			/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
			RHVector3 testvector4;

			if (CubeY + 1 < modelcube.YNum() && CubeZ - 1 > 0)
			{
				testvector4 = modelcube.GetCubeByIndex(CubeX, CubeY + 1, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector4);
				if (modelcube.GetCubeByIndex(CubeX, CubeY + 1, CubeZ - 1).EmptyCube == true)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector4) == false)
						CalHeadAngle(modelcube, testvector4, supTouchNor, supTouchWorCor, CubeX, CubeY + 1, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			RHVector3 testvector5;
			if (CubeY - 1 >= 0 && CubeZ - 1 > 0)
			{
				testvector5 = modelcube.GetCubeByIndex(CubeX, CubeY - 1, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector5);
				if (modelcube.GetCubeByIndex(CubeX, CubeY - 1, CubeZ - 1).EmptyCube == true)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector5) == false)
						CalHeadAngle(modelcube, testvector5, supTouchNor, supTouchWorCor, CubeX, CubeY - 1, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////           
			RHVector3 testvector6;
			if (CubeX + 1 < modelcube.XNum() && CubeY + 1 < modelcube.YNum() && CubeZ - 1 > 0)
			{
				testvector6 = modelcube.GetCubeByIndex(CubeX + 1, CubeY + 1, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector6);
				if (modelcube.GetCubeByIndex(CubeX + 1, CubeY + 1, CubeZ - 1).EmptyCube == true/* && modelcube.CubeMat[a + 1, b, c - 1].EmptyCube == true && modelcube.CubeMat[a, b + 1, c - 1].EmptyCube == true*/)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector6) == false)
						CalHeadAngle(modelcube, testvector6, supTouchNor, supTouchWorCor, CubeX + 1, CubeY + 1, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////               
			RHVector3 testvector7;
			if (CubeX - 1 >= 0 && CubeY - 1 >= 0 && CubeZ - 1 > 0)
			{
				testvector7 = modelcube.GetCubeByIndex(CubeX - 1, CubeY - 1, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector7);
				if (modelcube.GetCubeByIndex(CubeX - 1, CubeY - 1, CubeZ - 1).EmptyCube == true /*&& modelcube.CubeMat[a - 1, b, c - 1].EmptyCube == true && modelcube.CubeMat[a, b - 1, c - 1].EmptyCube == true*/)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector7) == false)
						CalHeadAngle(modelcube, testvector7, supTouchNor, supTouchWorCor, CubeX - 1, CubeY - 1, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////               
			RHVector3 testvector8;
			if (CubeX + 1 < modelcube.XNum() && CubeY - 1 >= 0 && CubeZ - 1 > 0)
			{
				testvector8 = modelcube.GetCubeByIndex(CubeX + 1, CubeY - 1, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector8);
				if (modelcube.GetCubeByIndex(CubeX + 1, CubeY - 1, CubeZ - 1).EmptyCube == true /*&& modelcube.CubeMat[a + 1, b, c - 1].EmptyCube == true && modelcube.CubeMat[a, b - 1, c - 1].EmptyCube == true*/)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector8) == false)
						CalHeadAngle(modelcube, testvector8, supTouchNor, supTouchWorCor, CubeX + 1, CubeY - 1, CubeZ - 1, SupHeadList);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////                         
			RHVector3 testvector9;
			if (CubeX - 1 >= 0 && CubeY + 1 < modelcube.YNum() && CubeZ - 1 > 0)
			{
				testvector9 = modelcube.GetCubeByIndex(CubeX - 1, CubeY + 1, CubeZ - 1).Centerpoint.Subtract(supTouchWorCor);
				TestVector.push_back(testvector9);
				if (modelcube.GetCubeByIndex(CubeX - 1, CubeY + 1, CubeZ - 1).EmptyCube == true /*&& modelcube.CubeMat[a - 1, b, c - 1].EmptyCube == true && modelcube.CubeMat[a, b + 1, c - 1].EmptyCube == true*/)
				{
					if (GetTouchPerpendicularRHVector3forHead2(num, modelcube, supTouchWorCor, testvector9) == false)
						CalHeadAngle(modelcube, testvector9, supTouchNor, supTouchWorCor, CubeX - 1, CubeY + 1, CubeZ - 1, SupHeadList);
				}
			}
		}
		return SupHeadList;
	}

	Tuple<RHVector3, CubeNum> SupportMeshsCylinder::ChooseHeadCube(vector<Triple<RHVector3, double, CubeNum>> SupHeadList, CubeMatrix &modelcube, CubeNum num, RHVector3 supTouchWorCor)
	{
		int Num = -1;
		if (SupHeadList.size() != 0)
		{
			double minAngle = std::numeric_limits<double>::infinity();// = //double.PositiveInfinity;
			for (int i = 0; i < SupHeadList.size(); i++)
			{
				double angle;
				angle = SupHeadList[i].Second() * 180 / 3.14;
				if (angle < minAngle)
				{
					minAngle = angle;
					{
						Num = i;
					}
				}
			}

			//角度太大無法長一般支撐
			if (modelcube.GetCubeByIndex(SupHeadList[Num].Third().x, SupHeadList[Num].Third().y, SupHeadList[Num].Third().z).EmptyCube != true)
			{
				//num[0] = CubeX; num[1] = CubeY; num[2] = CubeZ;//存cube
				supHeadend = supTouchWorCor;
				SupHeadList.clear();
				return Tuple<RHVector3, CubeNum>(RHVector3(), num);
			}

			supHeadend = SupHeadList[Num].First();
			num.x = SupHeadList[Num].Third().x; num.y = SupHeadList[Num].Third().y; num.z = SupHeadList[Num].Third().z;//存cube              
			SupHeadList.clear();

			if (internalSupport == InternalSupportMode::Disable)
			{
				if (modelcube.GetCubeByIndex(num.x, num.y, num.z).ConnectPlatform == false)
				{
					return Tuple<RHVector3, CubeNum>(RHVector3(), num);
				}
			}
			return (Tuple<RHVector3, CubeNum>(supHeadend, num));
		}
		//沒得長
		//num[0] = CubeX; num[1] = CubeY; num[2] = CubeZ;//存cube
		supHeadend = supTouchWorCor;
		SupHeadList.clear();
		return (Tuple<RHVector3, CubeNum>(RHVector3(), num));
	}

	vector<Triple<RHVector3, double, CubeNum>> SupportMeshsCylinder::CalHeadAngle(CubeMatrix &modelcube, RHVector3 testvector, RHVector3 supTouchNor, RHVector3 supTouchWorCor, 
		int a, int b, int c, vector<Triple<RHVector3, double, CubeNum>> &list)//newsup
	{
		CubeNum cubeNum(a, b, c);		// C# AP: int CubeNum[] = { a, b, c };
		RHVector3 vertical(0, 0, -1);

		double innerProduct;
		double cosine = 0;
		double angleBetween;
		testvector.NormalizeSafe();
		vertical.NormalizeSafe();
		innerProduct = vertical.ScalarProduct(testvector);
		cosine = innerProduct / (vertical.Length() * testvector.Length());
		angleBetween = acos(cosine); // unit: radian
		list.push_back(Triple<RHVector3, double, CubeNum>(modelcube.GetCubeByIndex(a, b, c).Centerpoint, angleBetween, cubeNum));
		return list;
	}
	

	/// <summary>
	/// Generate support model from the list of support mark.
	/// </summary>
	void SupportMeshsCylinder::genSupFromList(const vector<unique_ptr<SupportSymbol>> &ptsCones)
	{
		DbgMsg(__FUNCTION__ "[IN]" );

		if (Symbols.size() == 0)
		{
			Status = STATUS::Done;
			return;
		}
		Status = STATUS::Busy;

#if DEBUG_SUPPORT_ADD_FAILURE
		infoSupHeadDropList.Clear();
		infoSupBodyDropList.Clear();
		infoSupFailList.Clear();
		infoSupPtAdjList.Clear();
#endif

		// The vertex arranged in front of the original list is important.
		// Traversal the list from end of list, so reverse the list for inserting important vertex first
		//Symbols.Reverse();		// Charles: C# AP has this code, but seems useless.
		///////////////////////////new sup
		SupHeadStartEndList.clear();
		SupTailStartEndList.clear();
		ThinBodyList.clear();
		SupSubBodyStartEndList.clear();
		InnerSupBodyList.clear();

		MergeBranchList.clear();
		CandidateInnerSupportPos.clear();
		HaveTopGenInnerSupportList.clear();
		HaveTopGenTailList.clear();

		/////////////////////////////////////////////////////////////////////////////////找垂直表面的支撐頭
		int totalCountOfProgressBar = 4 * static_cast<int>(Symbols.size());// +supParentHeadHeiList.Count;
		int cnt = 0;

		int HeadNum = 0;
		int ListCount = -1;

		int SupNum = static_cast<int>(Symbols.size()) - 1;

		//檢查支撐點
		for (int i = SupNum; i >= 0; i--)	// Charles: From the last support point starting...
		{
			SupportPoint *ppoint = dynamic_cast<SupportPoint *>(Symbols[i].get());	//IMPORTANT: Because we need to update the Support data, we have to use pointer.

			//double SupportDiameter = (point.TouchDiameter / 10) * 2;
			////////太靠近平台無法長一般支撐//////////////////
			////小於最小高度移除支撐點
			if (ppoint->GetTouchRadius() > 0.65)
			{
				ppoint->SetTouchRadius(ppoint->GetTouchRadius() / 5 );
			}
			//if (CheckHeight(ppoint->GetPosition().z, i, Symbols) == true)	// check if the height of the Symbol is below 2. If so, remove this symbol and go to process next Symbol.
			//	continue;
			//////太靠近cone刪除支撐點
			if (CheckConeDistance(*ppoint, ptsCones) == true)
			{
				Symbols.erase(Symbols.begin() + i);
				continue;
			}
			////高度不夠長一般支撐,垂直往平台長圓錐
			if (NeedGenBootomSup(*ppoint, i) == true)
				continue;
			//檢查支撐點與物件距離
			bool IsMiniSup = false;
			if (GetTouchPerpendicularRHVector3forCheckPoinDiatance(*ppoint, /*in*/ cubeSpaceforNewsup, /*out*/ IsMiniSup) == true)
			{
				Symbols.erase(Symbols.begin() + i);
				continue;
			}
			else if (IsMiniSup == true)
			{
				ppoint->MiniSupport = true;
				unique_ptr<SupportData> minisup(new SupportData());
				if (AddMiniSup(ppoint->GetPosition(), *ppoint, minisup) == true)
				{
					_items2.push_back(move(minisup));
				}
				else // no minisup
				{
					Symbols.erase(Symbols.begin() + i);	// C# AP: Symbols.Remove(Symbols[i]);
				}
				continue;
			}
			else
			{
				SupHeadStartEndList.push_back(CalSupportHeadPositionforNewsup(ppoint->GetTouchRadius(), ppoint->GetPosition(), ppoint->GetDirection(), cubeSpaceforNewsup));//newsup  
				cnt++;
				ListCount++;
				//內部支撐
				if (SupHeadStartEndList[ListCount].First() == RHVector3())
				{
					ppoint->MiniSupport = true;
					unique_ptr<SupportData> minisup(new SupportData());
					if (AddMiniSup(ppoint->GetPosition(), *ppoint, minisup) == true)
					{
						_items2.push_back(move(minisup));
						//minisupNum++;
					}
					else //no minisup
					{
						Symbols.erase(Symbols.begin() + i);	//Symbols.Remove(Symbols[i]);
						// i--;
					}
					SupHeadStartEndList.erase(SupHeadStartEndList.begin()+ListCount);
					ListCount--;
					continue;
				}
			}
			if (ppoint) 
			{
				ppoint->HeadStart = SupHeadStartEndList[ListCount].First();			//newsup
				ppoint->HeadEnd = SupHeadStartEndList[ListCount].Second();			//newsup
				ppoint->HeadCubeNum = SupHeadStartEndList[ListCount].Third();
			}
	
			for (int supheight = ppoint->HeadCubeNum.z; supheight > 0; supheight--)//支撐頭下方到碰到物件前都是可以被Branch Merge的區域
			{
				if (cubeSpaceforNewsup.GetCubeByIndex(SupHeadStartEndList[HeadNum].Third().x, SupHeadStartEndList[HeadNum].Third().y, supheight - 1).EmptyCube == false)
				{
					break;
				}
				CubeUpdate::UpdateCubeForUnderSupportPoint(SupHeadStartEndList[HeadNum].Third().x, SupHeadStartEndList[HeadNum].Third().y, supheight, cubeSpaceforNewsup, true);
			}
			HeadNum++;
			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
		}
		DbgMsg(__FUNCTION__ "[Needle + Head] ===> _items2.Count: %d", _items2.size());
	
		/////////////////////////////////////////////////////////////////////////////////生長子支撐點的斜向支柱
		//一階支撐
		for (int i = static_cast<int>(Symbols.size()) - 1; i >= 0; i--)
		{
			SupportPoint point = *(dynamic_cast<SupportPoint *>(Symbols[i].get()));

			if (point.HeadEnd != RHVector3())
			{
				vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> GenerateResult;
				GenerateResult = genBody.GenSupportBody(point.HeadCubeNum, point.HeadEnd, cubeSpaceforNewsup, ThinBodyList, HaveTopGenTailList, HaveTopGenInnerSupportList, NewMergePointList);
				//GenerateResult = (GenSupportBody(point.HeadCubeNum, point.HeadEnd, BelongModel.cubeSpaceforNewsup));

				cnt++;
				for (int Sup = 0; Sup < GenerateResult.size(); Sup++)
				{
					if (GenerateResult[Sup].First() != GenerateResult[Sup].Third())
					{
						SupSubBodyStartEndList.push_back(GenerateResult[Sup]);
					}
				}
			}
			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
		}
		//二階支撐
		for (int j = 0; j < NewMergePointList.size(); j++)
		{
			vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> SecondGenerateResult;
			SecondGenerateResult = genBody.GenSupportBody(	NewMergePointList[j].Second(), 
															NewMergePointList[j].First(), 
															cubeSpaceforNewsup, 
															ThinBodyList, 
															HaveTopGenTailList, 
															HaveTopGenInnerSupportList, 
															NewMergePointList);
			//SecondGenerateResult = (GenSupportBody(NewMergePointList[j].Second, NewMergePointList[j].First, BelongModel.cubeSpaceforNewsup));
			for (int Sup = 0; Sup < SecondGenerateResult.size(); Sup++)
			{
				if (SecondGenerateResult[Sup].First() != SecondGenerateResult[Sup].Third())
				{
					SupSubBodyStartEndList.push_back(SecondGenerateResult[Sup]);
					cnt++;
				}
			}
			NewMergePointList.erase(NewMergePointList.begin() + j);	//C#: NewMergePointList.Remove(NewMergePointList[j]);
			j -= 1;
			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
		}
		vector<vector<Coord3D>> OverhangPointList;	// C# AP: unordered_set<vector<Coord3D>> OverhangPointList; C++: Because "unordered_set" needs implement Hash, ....
		vector<int> OriginalList;
		vector<Tuple<RHVector3, RHVector3>> checkTable;
		//粗圓錐支柱
		for (int i = static_cast<int>(Symbols.size()) - 1; i >= 0; i--)
		{
			SupportPoint point = *(dynamic_cast<SupportPoint *>(Symbols[i].get()));
			if (point.HeadEnd != RHVector3())
			{
				Triple<RHVector3, RHVector3, bool> BackboneList(RHVector3(), RHVector3(), false);
		
				bool isoverhang = GenBackbone(BackboneList, point, cubeSpaceforNewsup, OverhangPointList, checkTable);
				if (isoverhang == true)
					OriginalList.push_back(i);
				cnt++;
				//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
			}
		}
		DbgMsg(__FUNCTION__ "[Needle + Head + Body]===> _items2.size(): %d", _items2.size());         ///粗圓錐支柱
		//***************************************************************************************************************************************************************	  
		//主支撐Body的FirmBranch
		FirmBranchPointList.clear();
		for (int i = static_cast<int>(Symbols.size()) - 1; i >= 0; i--)
		{
			SupportPoint *ppoint = dynamic_cast<SupportPoint *>(Symbols[i].get());
			if (ppoint->HeadEnd != RHVector3())
			{
				vector<int> FirmBranchPoint = genBranch.CheckVerticalSupportLength(ppoint->HeadCubeNum, cubeSpaceforNewsup);
				cnt++;

				if (FirmBranchPoint.size() > 0)
				{
					for (int j = (int)FirmBranchPoint.size() - 1; j >= 0; j--)
					{
						Tuple<RHVector3, CubeNum> BufferList = genBranch.AddingFirmBranchPoint(ppoint->HeadCubeNum, cubeSpaceforNewsup, FirmBranchPoint[j]);
						if (BufferList.Second().z > 0)
						{
							FirmBranchPointList.push_back(BufferList);
							cnt++;
						}
						//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
					}
					for (int j = 0; j < FirmBranchPointList.size(); j++)
					{
						vector<Triple<RHVector3, RHVector3, CubeNum>> AddingFirmBranchResult = genBranch.AddingFirmBranch(FirmBranchPointList[j].Second(), cubeSpaceforNewsup);
						for (int Sup = 0; Sup < genBranch.BranchList.size(); Sup++)
						{
							if (genBranch.BranchList[Sup].First() != genBranch.BranchList[Sup].Second())
								MergeBranchList.push_back(genBranch.BranchList[Sup]);//newsup   
							cnt++;
						}
						//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));

						FirmBranchPointList.erase(FirmBranchPointList.begin()+j);	//remove FirmBranchPointList[j]
						cnt--;
						j -= 1;
					}
					//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
				}
			}
			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
		}
		///////////////////////////////////////////////////////////////////////子支撐Body段的FirmBranch
		DbgMsg(__FUNCTION__"===> ThinBodyList.size(): %d", ThinBodyList.size());
		//找子支撐的FirmBranch
		for (int i = (int)ThinBodyList.size() - 1; i >= 0; i--)	// Don't use size_t
		{
			vector<int> FirmBranchPoint = genBranch.CheckVerticalSupportLength(ThinBodyList[i].Second(), cubeSpaceforNewsup);
			cnt++;
			if (FirmBranchPoint.size() > 0)
			{
				for (int j = (int)FirmBranchPoint.size() - 1; j >= 0; j--)
				{
					FirmBranchPointList.push_back(genBranch.AddingFirmBranchPoint(ThinBodyList[i].Second(), cubeSpaceforNewsup, FirmBranchPoint[j]));
				}

				for (int j = 0; j < FirmBranchPointList.size(); j++)
				{
					vector<Triple<RHVector3, RHVector3, CubeNum>> AddingFirmBranchResult = genBranch.AddingFirmBranch(FirmBranchPointList[j].Second(), cubeSpaceforNewsup);
					for (int Sup = 0; Sup < genBranch.BranchList.size(); Sup++)
					{
						if (genBranch.BranchList[Sup].First() != genBranch.BranchList[Sup].Second())
							MergeBranchList.push_back(genBranch.BranchList[Sup]);//newsup   
						cnt++;
					}
					//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
					FirmBranchPointList.erase(FirmBranchPointList.begin() + j);	//remove FirmBranchPointList[j]
					cnt--;
					j -= 1;
				}
			}
		}
		//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
		DbgMsg(__FUNCTION__"===> MergeBranchList.size(): %d", MergeBranchList.size());	// two points ==> MergeBranchList: 3
		checkTable.clear();
		for (int i = (int)MergeBranchList.size() - 1; i >= 0; i--)
		{
			bool FT = false;
			for(const auto &mergeBranch : checkTable)
			{
				if (mergeBranch.First().x == MergeBranchList[i].First().x
					&& mergeBranch.First().y == MergeBranchList[i].First().y
					&& mergeBranch.First().z == MergeBranchList[i].First().z
					&& mergeBranch.Second().x == MergeBranchList[i].Second().x
					&& mergeBranch.Second().y == MergeBranchList[i].Second().y
					&& mergeBranch.Second().z == MergeBranchList[i].Second().z)
				{
					FT = true;
					break;
				}
			}
			if (FT == false)
			{
				Tuple<RHVector3, RHVector3> mergeBranch(MergeBranchList[i].First(), MergeBranchList[i].Second());
				checkTable.push_back(mergeBranch);

				unique_ptr<SupportData> supBranch(new SupportData());
				//supBranch->originalModel = ReferenceMesh.Body();
				supBranch->supType = (int)ModelType::BRANCH,
				supBranch->SetPosition(Coord3D((float)MergeBranchList[i].First().x, (float)MergeBranchList[i].First().y, (float)MergeBranchList[i].First().z));
				supBranch->orientation = MergeBranchList[i].Second().Subtract(MergeBranchList[i].First());
				
				RHVector3 BranchOrientation = MergeBranchList[i].Second().Subtract(MergeBranchList[i].First());
				RHVector3 InitialPosition(0, 0, 0);
				RHVector3 BodyTop(InitialPosition.x, InitialPosition.y, InitialPosition.z);
				RHVector3 BodyBottom(InitialPosition.x, InitialPosition.y, InitialPosition.z - BranchOrientation.Length());
				//GenerateCylinder(BodyBottom, BodyTop, (float)SCALE_FACTOR_DEFAULT * 5, (float)SCALE_FACTOR_DEFAULT * 5, /*out*/ supBranch->originalModel);
				supBranch->length = BranchOrientation.Length();
				supBranch->radius1 = SCALE_FACTOR_DEFAULT * 5.0;

				//supBranch->SetScale(Coord3D((float)SCALE_FACTOR_DEFAULT, (float)SCALE_FACTOR_DEFAULT, (float)supBranch->orientation.Length())); // use same size    Steven
				supBranch->RotateModel();
				_items2.push_back(move(supBranch));
				RHVector3 vertical = RHVector3(0, 0, -1);
			}
		}

		DbgMsg(__FUNCTION__"===> HaveTopGenInnerSupportList.size(): %d", HaveTopGenInnerSupportList.size());	//**********************************
		//加內部支撐tail;	// Charles: Need to debug........
		for (int n = (int)HaveTopGenInnerSupportList.size() - 1; n >= 0; n--)
		{
			bool Maintail = true;
			int x = HaveTopGenInnerSupportList[n].Second().x;
			int y = HaveTopGenInnerSupportList[n].Second().y;
			int z = HaveTopGenInnerSupportList[n].Second().z;
			CalCandidateTailPoint(x, y, z, HaveTopGenInnerSupportList[n].First());	// update CandidateInnerSupportPos
			cnt++;
			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
			if (CandidateInnerSupportPos.size() > 0)
			{
				SupTailStartEndList.push_back(GenTailConeforNewsup(HaveTopGenInnerSupportList[n].First(), 0.5, CandidateInnerSupportPos, cubeSpaceforNewsup));
			}
			AddGrabTail(HaveTopGenInnerSupportList[n].First(), HaveTopGenInnerSupportList[n].Second(), HaveTopGenInnerSupportList[n].Third(), cubeSpaceforNewsup);
			cnt--;
			//OnGenerateMeshsProcessRateUpdate(this, new ProcessRateEventArgs((uint)(cnt * 50 / totalCountOfProgressBar)));
		}

		DbgMsg(__FUNCTION__"===> InnerSupBodyList.size(): %d", InnerSupBodyList.size());
		for (int i = (int)InnerSupBodyList.size() - 1; i >= 0; i--)
		{
			RHVector3 tree;
			RHVector3 branchSupOrigin;
			unique_ptr<SupportData> InnresupTail(new SupportData());
			GenInnersupTail(i, /*out*/ tree, /*out*/ branchSupOrigin, /*out*/ InnresupTail);
			double Thickradius = 0.5;
			double Thinkradius = 0.3;
			//AddTopSphere(InnresupTail->originalModel, num_phi, num_theta, tree, Thickradius);
			InnresupTail->length = tree.z;
			InnresupTail->radius1 = Thickradius;
			//bool isNeedToMakeTopEndCap = false;
			//AddTree(InnresupTail->originalModel, num_sides, tree, Thickradius, Thinkradius, isNeedToMakeTopEndCap);
			InnresupTail->isNeedToMakeTopEndCap = false;
			InnresupTail->radius2 = Thinkradius;
			//AddBtmSphere(InnresupTail->originalModel, num_phi, num_theta, branchSupOrigin, Thinkradius);
			_items2.push_back(move(InnresupTail));

			GenInnresupBody(i);

			RHVector3 origin;
			unique_ptr<SupportData> ConnectBall(new SupportData());
			GenConnectBall(i, /*out*/ origin, /*out*/ConnectBall);
			//AddTopSphere(ConnectBall->originalModel, num_phi, num_theta, origin, Thickradius);
			//AddBtmSphere(ConnectBall->originalModel, num_phi, num_theta, origin, Thickradius);
			ConnectBall->radius1 = Thickradius;
			_items2.push_back(move(ConnectBall));
		}

		DbgMsg(__FUNCTION__"===> HaveTopGenTailList.size(): %d", HaveTopGenTailList.size());
		for (int n = (int)HaveTopGenTailList.size() - 1; n >= 0; n--)	
		{
			GenADDsupTail(n);
		}

		DbgMsg(__FUNCTION__"===> SupTailStartEndList.size(): %d", SupTailStartEndList.size());
		for (int i = (int)SupTailStartEndList.size() - 1; i >= 0; i--)
		{
			//foreach(SupportData supPart in supParts1)
			{
#if !DEBUG_ADD_SUPPORT_MODEL_STEP
				_items2.push_back(move(SupTailStartEndList[i].Third()));
				// for debug, insert model whatever success or failure
#endif
			}
		}
		unordered_map<int, int> CheckTable;
		DbgMsg(__FUNCTION__"[**********************************]===> _items2.size(): %d", _items2.size());
		DbgMsg(__FUNCTION__"===> SupSubBodyStartEndList.size(): %d", SupSubBodyStartEndList.size());
		///////////////////////////////////////////////////////////////////////
		for (int i = (int)SupSubBodyStartEndList.size() - 1; i >= 0; i--)
		{
			if (SupSubBodyStartEndList[i].Second() == RHVector3(0,0,0) && SupSubBodyStartEndList[i].Third() != RHVector3(0, 0, 0))
			{
				unique_ptr<SupportData> supBranch(new SupportData());
				//supBranch->originalModel = ReferenceMesh.Body();
				RHVector3 BranchOrientation = SupSubBodyStartEndList[i].Third().Subtract(SupSubBodyStartEndList[i].First());
				RHVector3 InitialPosition(0, 0, 0);
				RHVector3 BodyTop(InitialPosition.x, InitialPosition.y, InitialPosition.z);
				RHVector3 BodyBottom(InitialPosition.x, InitialPosition.y, InitialPosition.z - BranchOrientation.Length());
				//GenerateCylinder(BodyBottom, BodyTop, (float)SCALE_FACTOR_DEFAULT * 5, (float)SCALE_FACTOR_DEFAULT * 5, /*out*/ supBranch->originalModel);
				supBranch->length = BranchOrientation.Length();
				supBranch->radius1 = SCALE_FACTOR_DEFAULT * 5.0;

				supBranch->supType = (int)ModelType::BRANCH;
				supBranch->SetPosition(Coord3D((float)SupSubBodyStartEndList[i].First().x, (float)SupSubBodyStartEndList[i].First().y, (float)SupSubBodyStartEndList[i].First().z));
				supBranch->orientation = BranchOrientation;
				//supBranch->SetScale(Coord3D((float)SCALE_FACTOR_DEFAULT, (float)SCALE_FACTOR_DEFAULT, (float)supBranch->orientation.Length())); // use same size    Steven
				supBranch->RotateModel();

				Tuple<RHVector3, RHVector3> SupbranchInfo(SupSubBodyStartEndList[i].First(), SupSubBodyStartEndList[i].Third());
				bool FT = false;
				for(const auto &mergeBranch : checkTable)
				{
					if (mergeBranch.First().x == SupbranchInfo.First().x
						&& mergeBranch.First().y == SupbranchInfo.First().y
						&& mergeBranch.First().z == SupbranchInfo.First().z
						&& mergeBranch.Second().x == SupbranchInfo.Second().x
						&& mergeBranch.Second().y == SupbranchInfo.Second().y
						&& mergeBranch.Second().z == SupbranchInfo.Second().z)
					{
						FT = true;
						break;
					}
				}
				if (FT == false)
				{
					checkTable.push_back(SupbranchInfo);
					_items2.push_back(move(supBranch));
				}
				CheckTable.insert(std::make_pair(i, (int)_items2.size() - 1));
			}
			if (SupSubBodyStartEndList[i].Second() != RHVector3(0, 0, 0))
			{
				unique_ptr<SupportData> supMainBody(new SupportData());
				//supMainBody->originalModel = ReferenceMesh.Body();
				RHVector3 BranchOrientation = SupSubBodyStartEndList[i].Third().Subtract(SupSubBodyStartEndList[i].Second());
				RHVector3 InitialPosition(0, 0, 0);
				RHVector3 BodyTop(InitialPosition.x, InitialPosition.y, InitialPosition.z);
				RHVector3 BodyBottom(InitialPosition.x, InitialPosition.y, InitialPosition.z - BranchOrientation.Length());
				//GenerateCylinder(BodyBottom, BodyTop, (float)SCALE_FACTOR_DEFAULT * 5, (float)SCALE_FACTOR_DEFAULT * 5, /*out*/ supMainBody->originalModel);
				supMainBody->length = BranchOrientation.Length();
				supMainBody->radius1 = SCALE_FACTOR_DEFAULT * 5.0;

				supMainBody->supType = (int)ModelType::BRANCH;
				supMainBody->SetPosition(Coord3D((float)SupSubBodyStartEndList[i].Second().x, (float)SupSubBodyStartEndList[i].Second().y, (float)SupSubBodyStartEndList[i].Second().z));
				supMainBody->orientation = BranchOrientation;
				//supMainBody->SetScale(Coord3D((float)SCALE_FACTOR_DEFAULT, (float)SCALE_FACTOR_DEFAULT, (float)supMainBody->orientation.Length())); // use same size    Steven
				supMainBody->RotateModel();
				_items2.push_back(move(supMainBody));

				unique_ptr<SupportData> supBranch(new SupportData());
				//supBranch->originalModel = ReferenceMesh.Body();
				BranchOrientation = SupSubBodyStartEndList[i].Second().Subtract(SupSubBodyStartEndList[i].First());
				BodyTop = RHVector3(InitialPosition.x, InitialPosition.y, InitialPosition.z);
				BodyBottom = RHVector3(InitialPosition.x, InitialPosition.y, InitialPosition.z - BranchOrientation.Length());
				//GenerateCylinder(BodyBottom, BodyTop, (float)SCALE_FACTOR_DEFAULT * 5, (float)SCALE_FACTOR_DEFAULT * 5, /*out*/ supBranch->originalModel);
				supBranch->length = BranchOrientation.Length();
				supBranch->radius1 = SCALE_FACTOR_DEFAULT * 5.0;

				supBranch->supType = (int)ModelType::BRANCH;
				supBranch->SetPosition(Coord3D((float)SupSubBodyStartEndList[i].First().x, (float)SupSubBodyStartEndList[i].First().y, (float)SupSubBodyStartEndList[i].First().z));
				supBranch->orientation = BranchOrientation;
				//supBranch->SetScale(Coord3D((float)SCALE_FACTOR_DEFAULT, (float)SCALE_FACTOR_DEFAULT, (float)supBranch->orientation.Length())); // use same size    Steven
				supBranch->RotateModel();
				_items2.push_back(move(supBranch));
				//一次長後不用連接球
				//增加sup圓球填補空隙
				unique_ptr<SupportData> ConnectBall(new SupportData());
				ConnectBall->supType = (int)ModelType::MARK;
				ConnectBall->SetPosition(Coord3D((float)SupSubBodyStartEndList[i].Second().x, (float)SupSubBodyStartEndList[i].Second().y, (float)SupSubBodyStartEndList[i].Second().z));
				RHVector3 origin = RHVector3(0, 0, 0);
				ConnectBall->orientation = origin;

				//AddTopSphere(ConnectBall->originalModel, num_phi, num_theta, origin, 0.66);
				//AddBtmSphere(ConnectBall->originalModel, num_phi, num_theta, origin, 0.66);
				ConnectBall->radius1 = 0.66;
				_items2.push_back(move(ConnectBall));
			}
		}
		DbgMsg(__FUNCTION__"[**********************************]===> _items2.size(): %d", _items2.size());
		DbgMsg(__FUNCTION__"===> OverhangPointList.size(): %d", OverhangPointList.size());
		//PointList[0] first
		//PointList[1] second
		int count = 0;
		for(int i=0; i< OverhangPointList.size(); i++)	//C# AP: foreach(List<Coord3D> PointList in OverhangPointList)
		{
			vector<Coord3D> PointList = OverhangPointList[i];
			bool connectbranchpoint = false;
			for (int i = (int)SupSubBodyStartEndList.size() - 1; i >= 0; i--)
			{	// Note: PointList[1].x is float while )SupSubBodyStartEndList[i].First().x is double. It needs to cast the "double" value to "float" first; otherwise, the if-condition may fail.
				if (PointList[1].x == (float)SupSubBodyStartEndList[i].First().x && 
					PointList[1].y == (float)SupSubBodyStartEndList[i].First().y &&
					PointList[1].z == (float)SupSubBodyStartEndList[i].First().z)
				{
					connectbranchpoint = true;
					break;
				}
			}
			for (int n = (int)HaveTopGenInnerSupportList.size() - 1; n >= 0; n--)
			{
				if (PointList[1].x == (float)HaveTopGenInnerSupportList[n].First().x &&
					PointList[1].y == (float)HaveTopGenInnerSupportList[n].First().y &&
					PointList[1].z == (float)HaveTopGenInnerSupportList[n].First().z)
				{
					connectbranchpoint = true;
					break;
				}
			}
			if (connectbranchpoint == false)
			{
				for (int n = (int)SupSubBodyStartEndList.size() - 1; n >= 0; n--)
				{
					if (SupSubBodyStartEndList[n].Second() != RHVector3(0,0,0))
						if (PointList[0].x == (float)SupSubBodyStartEndList[n].Second().x &&
							PointList[0].y == (float)SupSubBodyStartEndList[n].Second().y &&
							PointList[0].z == (float)SupSubBodyStartEndList[n].Second().z)
						{
							std::unordered_map<int, int>::const_iterator got = CheckTable.find(n);		//CheckTable.TryGetValue(n, /*out*/value);
							if (got != CheckTable.end())
							{				
								int value = CheckTable[n];
								_items2.erase(_items2.begin() + value);
							}
						}
				}
				int secondpointnumber = -1;
				for (int n = 0; n < _items2.size(); n++)
				{
					if (_items2[n]->GetPosition().x == PointList[0].x && 
						_items2[n]->GetPosition().y == PointList[0].y && 
						_items2[n]->GetPosition().z == PointList[0].z)
						secondpointnumber = n;
					if (_items2[n]->GetPosition().x == PointList[1].x && 
						_items2[n]->GetPosition().y == PointList[1].y && 
						_items2[n]->GetPosition().z == PointList[1].z)
						_items2.erase(_items2.begin() + n); //_items2.Remove(_items2[n]);
				}
				_items2.erase(_items2.begin() + secondpointnumber);
				_items2.erase(_items2.begin() + secondpointnumber - 1);
				Symbols.erase(Symbols.begin() + OriginalList[count]);	// C#: Symbols.Remove(Symbols[OriginalList[count]]); // Charles: needs to check......
			}
			count += 1;
		}
		///////////////////////////////////////////////////////////////////////New Support 
		Status = STATUS::Done;
		DbgMsg(__FUNCTION__"[Needle + head + Body + Tail]===> _items2.size(): %d", _items2.size());
		DbgMsg(__FUNCTION__"[OUT]");
	}
	
	bool SupportMeshsCylinder::NeedGenBootomSup(SupportPoint &point, int i)
	{
		int ShortLengthToGen = 5;
		double MinimaLengthToGen = 1;
		//double SupportDiameter = (point.TouchDiameter / 10) * 2;
		if (point.GetPosition().z <= ShortLengthToGen && point.GetPosition().z > MinimaLengthToGen)
		{
			RHVector3 LowertoPoint = RHVector3(point.GetPosition());
			LowertoPoint.z -= 0.0;//for切層bug:支撐點會打在物件內
			point.MiniSupport = true;
			unique_ptr<SupportData> Bottomsup(new SupportData());
			//檢查往下長有沒有打到物件

			if (AddMiniSup(LowertoPoint, point, /*out*/ Bottomsup) == true)
			{
				_items2.push_back(move(Bottomsup));
				return true;
			}
			else
			{
				//if (CheckeCubeEmpty(point.Position, BelongModel.cubeSpaceforNewsup, Largespan) == true)
				if (GetTouchPerpendicularRHVector3forLowsupport(point, cubeSpaceforNewsup, Cubedata2))
				{
					AddConnectPlateformSup(point.GetPosition(), point.GetTouchRadius(), cubeSpaceforNewsup, Largespan);
					return true;
				}
				else
				{
					Symbols.erase(Symbols.begin() + i);	//C# AP: Symbols.Remove(Symbols[i]);
					return true;
				}
			}
		}
		return false;
	}

	void SupportMeshsCylinder::AddConnectPlateformSup(RHVector3 Starpoint, double supRadius, CubeMatrix &modelcube, float span)
	{
		unique_ptr<SupportData> FistHead(new SupportData());
		FistHead->supType = (int)ModelType::HEAD;
		FistHead->orientation = RHVector3(0, 0, -1);
		//FistHead->orientation.NormalizeSafe();
		RHVector3 FirstHeadLength = RHVector3(FistHead->orientation);
		FirstHeadLength.Scale(Starpoint.z);
		RHVector3 FirstHeadPosition = Starpoint.Add(FirstHeadLength);
		FistHead->SetPosition(Coord3D((float)FirstHeadPosition.x, (float)FirstHeadPosition.y, (float)FirstHeadPosition.z));

		FistHead->curPos.transpose();
		FistHead->curPos2.transpose();

		double BottomRadius = supRadius;
		if (BottomRadius < 0.35)
		{
			BottomRadius = 0.35;
		}

		RHVector3 StarPoint1;
		RHVector3 EndPoint = RHVector3(0.0, 0.0, Starpoint.z);
		//AddTopSphere(FistHead->originalModel, num_phi, num_theta, EndPoint, supRadius);
		//AddTree(FistHead->originalModel, num_sides, EndPoint, supRadius, BottomRadius, true); // bool isNeedToMakeTopEndCap = true;
		FistHead->isNeedToMakeTopEndCap = true;
		FistHead->length = Starpoint.z;
		FistHead->radius1 = supRadius;
		FistHead->radius2 = BottomRadius;
		
		RHVector3 TailPos = RHVector3(FirstHeadPosition.x, FirstHeadPosition.y, FirstHeadPosition.z + 1);

		int TailPosCubeX, TailPosCubeY, TailPosCubeZ;
		TailPosCubeX = lrint(floor((TailPos.x - modelcube.MinPoint.x) / span));//changespan
		TailPosCubeY = lrint(floor((TailPos.y - modelcube.MinPoint.y) / span));
		TailPosCubeZ = lrint(floor((TailPos.z - modelcube.MinPoint.z) / span));
		if (modelcube.GetCubeByIndex(TailPosCubeX, TailPosCubeY, TailPosCubeZ).ConnectPlatform == true)
		{
			bool iscubeempty = CheckPosition(TailPosCubeX, TailPosCubeY, modelcube);
			if (iscubeempty == true)
			{
				if (Starpoint.z > 2)
					HaveTopGenTailList.push_back(TailPos);
			}
		}
		_items2.push_back(move(FistHead));
	}
	//MiniSup
	bool SupportMeshsCylinder::AddMiniSup(RHVector3 StartPos, SupportPoint &point, /*out*/unique_ptr<SupportData> &supBody)
	{
		double diamScale;
		if (point.GetTouchRadius() > 0.15)
			diamScale = 0.045;
		else
			diamScale = 0.033;
		RHVector3 vertical = RHVector3(0, 0, -1);
		RHVector3 direct;
		bool isTouch;

		//supBody->originalModel = ReferenceMesh.Body();

		if (editMode == PointEditModeCode::Normal && internalSupport == InternalSupportMode::Disable)
			return false;

		supBody->supType = (int)ModelType::BODY;

		RHVector3 tailTouchWor;
		RHVector3 tailNormal;

		direct = point.GetDirection();

		for (int i = 0; i < 2; i++)
		{
			isTouch = RayCollisionDetect::getPickTouchNearest2(cubeSpaceforNewsup, Cubedata2, StartPos, direct, 0.01, /*out*/ tailTouchWor,  /*out*/ tailNormal); /* && tailNormal.z > 0.6); */

			// if mini support > 4mm, discard this mini support
			// if (touchModel != BelongModel)
			{
				if (isTouch && StartPos.Distance(tailTouchWor) <= 4 /*&& StartPos.Distance(tailTouchWor) > 0.20*/)
				{
					break;
				}
			}

			if (i == 0)
			{
				direct = vertical;
				continue;
			}
			// touch nothing or distance to touch point > 4
			return false;
		}
		RHVector3 BranchOrientation = direct;
		RHVector3 InitialPosition(0, 0, 0);
		RHVector3 BodyTop(InitialPosition.x, InitialPosition.y, InitialPosition.z);
		RHVector3 BodyBottom(InitialPosition.x, InitialPosition.y, InitialPosition.z - BranchOrientation.Length());
		//GenerateCylinder(BodyBottom, BodyTop, diamScale * 5, diamScale * 5, /*out*/ supBody->originalModel);
		supBody->length = BranchOrientation.Length();
		supBody->radius1 = diamScale * 5;

		supBody->SetPosition(Coord3D((float)StartPos.x, (float)StartPos.y, (float)StartPos.z));
		//supBody->SetScale(Coord3D((float)diamScale, (float)diamScale, (float)StartPos.Distance(tailTouchWor)));

		if (direct != vertical)
		{
			supBody->orientation = direct;
			supBody->orientation.NormalizeSafe();

			supBody->RotateModel();
		}
		return true;
	}
	//BackBone
	bool SupportMeshsCylinder::GenBackbone(Triple<RHVector3, RHVector3, bool> &BackboneList, const SupportPoint &Points, CubeMatrix &modelcube, vector<vector<Coord3D>> &OverhangPointList, vector<Tuple<RHVector3, RHVector3>> &checkTable)
	{
		bool isoverhang = false;
		BackboneList = genBody.RecordBackbone(Points.HeadCubeNum, cubeSpaceforNewsup);
		bool FT = false;

		for (const auto &backbone : checkTable)  //	for (const auto &one : modifiedUpperHole)
		{
			if (backbone.First().x == BackboneList.First().x
				&& backbone.First().y == BackboneList.First().y
				&& backbone.First().z == BackboneList.First().z
				&& backbone.Second().x == BackboneList.Second().x
				&& backbone.Second().y == BackboneList.Second().y
				&& backbone.Second().z == BackboneList.Second().z)
			{
				FT = true;
				break;
			}

		}
		if (FT == false)
		{
			Tuple<RHVector3, RHVector3> backbone(BackboneList.First(), BackboneList.Second());
			checkTable.push_back(backbone);

			RHVector3 Vector;
			Vector = BackboneList.First().Subtract(BackboneList.Second());
			double Length = Vector.Length();
			double Top = BackboneList.First().z;
			double End = BackboneList.Second().z;
			double Slope = 0.5 / Top;
			double RadiusofBackboneEnd = 0.66 + (Length * Slope);

			unique_ptr<SupportData> Backbone(new SupportData());
			Backbone->SetPosition(Coord3D((float)BackboneList.Second().x, (float)BackboneList.Second().y, (float)BackboneList.Second().z));
			Vector.NormalizeSafe();
			Backbone->orientation = Vector;
			Backbone->supType = (int)ModelType::HEAD;

			RHVector3 vertical(0, 0, -1);
			RHVector3 EndPoint(0, 0, Length);
			RHVector3 StarPoint(0, 0, 0);

			if (Length > 0)
			{
				if (modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform == true)
				{
					//AddTopSphere(Backbone->originalModel, num_phi, num_theta, EndPoint, 0.66);
					Backbone->length = Length;
					Backbone->radius1 = 0.66;

					if ((BackboneList.Second().z >= 3) && (BackboneList.Third() == true))
					{
						//bool isNeedToMakeTopEndCap = false;
						//AddTree(Backbone->originalModel, num_sides, EndPoint, 0.66, RadiusofBackboneEnd, isNeedToMakeTopEndCap);
						Backbone->isNeedToMakeTopEndCap = false;
						Backbone->radius2 = RadiusofBackboneEnd;
					}
					else
					{
						//bool isNeedToMakeTopEndCap = true;
						//AddTree(Backbone->originalModel, num_sides, EndPoint, 0.66, RadiusofBackboneEnd, isNeedToMakeTopEndCap);
						Backbone->isNeedToMakeTopEndCap = true;
						Backbone->radius2 = RadiusofBackboneEnd;
					}
				}
				if (modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform == false)
				{
					//AddTopSphere(Backbone->originalModel, num_phi, num_theta, EndPoint, 0.66);
					Backbone->length = Length;
					Backbone->radius1 = 0.66;

					if (BackboneList.Third() == true)
					{
						//bool isNeedToMakeTopEndCap = false;
						//AddTree(Backbone->originalModel, num_sides, EndPoint, 0.66, 0.66, isNeedToMakeTopEndCap);
						Backbone->isNeedToMakeTopEndCap = false;
						Backbone->radius2 = 0.66;
					}
					else
					{
						//bool isNeedToMakeTopEndCap = true;
						//AddTree(Backbone->originalModel, num_sides, EndPoint, 0.66, 0.66, isNeedToMakeTopEndCap);
						Backbone->isNeedToMakeTopEndCap = true;
						Backbone->radius2 = 0.66;
					}
				}
				//if (BackboneList.Second().z >= 3 && modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform == true && BackboneList.Third() == true)
				//	AddBtmSphere(Backbone->originalModel, num_phi, num_theta, StarPoint, RadiusofBackboneEnd);
				//if (modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform != true && BackboneList.Third() == true)
				//	AddBtmSphere(Backbone->originalModel, num_phi, num_theta, StarPoint, 0.66);
				if (Backbone->GetPosition().z > 1)
				{
					vector<Coord3D> PointList;
					PointList.push_back(Coord3D((float)BackboneList.First().x, (float)BackboneList.First().y, (float)BackboneList.First().z));
					PointList.push_back(Coord3D((float)BackboneList.Second().x, (float)BackboneList.Second().y, (float)BackboneList.Second().z));
					OverhangPointList.push_back(PointList);
					isoverhang = true;
				}
				_items2.push_back(move(Backbone));	// Note: afer "move(Backbone)", the Backbone poiner is no longer valid.
				CheckAddBackboneTail(BackboneList, modelcube, Points);
			}
		}
		return isoverhang;
#if 0
		bool isoverhang = false;
		BackboneList = genBody.RecordBackbone(Points.HeadCubeNum, cubeSpaceforNewsup);

		RHVector3 Vector;
		Vector = BackboneList.First().Subtract(BackboneList.Second());
		double Length = Vector.Length();
		double Top = BackboneList.First().z;
		double End = BackboneList.Second().z;
		double Slope = 0.5 / Top;
		double RadiusofBackboneEnd = 0.66 + (Length * Slope);

		unique_ptr<SupportData> Backbone(new SupportData());
		Backbone->SetPosition(Coord3D((float)BackboneList.Second().x, (float)BackboneList.Second().y, (float)BackboneList.Second().z));
		Vector.NormalizeSafe();
		Backbone->orientation = Vector;
		Backbone->supType = (int)ModelType::HEAD;

		RHVector3 vertical(0, 0, -1);
		RHVector3 EndPoint(0, 0, Length);
		RHVector3 StarPoint(0, 0, 0);

		if (Length > 0)
		{
			if (modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform == true)
			{
				AddTopSphere(Backbone->originalModel, num_phi, num_theta, EndPoint, 0.66);
				AddTree(Backbone->originalModel, num_sides, EndPoint, 0.66, RadiusofBackboneEnd);
			}
			if (modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform == false)
			{
				AddTopSphere(Backbone->originalModel, num_phi, num_theta, EndPoint, 0.66);
				AddTree(Backbone->originalModel, num_sides, EndPoint, 0.66, 0.66);
			}

			if (BackboneList.Second().z >= 3 && modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform == true && BackboneList.Third() == true)
				AddBtmSphere(Backbone->originalModel, num_phi, num_theta, StarPoint, RadiusofBackboneEnd);
			if (modelcube.GetCubeByIndex(Points.HeadCubeNum.x, Points.HeadCubeNum.y, Points.HeadCubeNum.z).ConnectPlatform != true && BackboneList.Third() == true)
				AddBtmSphere(Backbone->originalModel, num_phi, num_theta, StarPoint, 0.66);

			if (Backbone->GetPosition().z > 1)
			{
				vector<Coord3D> PointList;
				PointList.push_back(Coord3D((float)BackboneList.First().x, (float)BackboneList.First().y, (float)BackboneList.First().z));
				PointList.push_back(Coord3D((float)BackboneList.Second().x, (float)BackboneList.Second().y, (float)BackboneList.Second().z));
				OverhangPointList.push_back(PointList);
				isoverhang = true;
			}
			_items2.push_back(move(Backbone));	// Note: afer "move(Backbone)", the Backbone poiner is no longer valid.
			CheckAddBackboneTail(BackboneList, modelcube, Points);
		}
		return isoverhang;
#endif
	}

	void SupportMeshsCylinder::CheckAddBackboneTail(Triple<RHVector3, RHVector3, bool> BackboneList, CubeMatrix &modelcube, SupportPoint Points)
	{
		bool iscubeempty = CheckPosition(Points.HeadCubeNum.x, Points.HeadCubeNum.y, modelcube);
		if (iscubeempty == true)
		{
			if (BackboneList.Second().z == 1 || BackboneList.Second().z == 0)
			{
				RHVector3 Tailpos(BackboneList.Second());
				Tailpos.z = 1;
				HaveTopGenTailList.push_back(Tailpos);
			}
		}
	}
	//CollisionDetect
	bool SupportMeshsCylinder::GetTouchPerpendicularRHVector3forCheckPoinDiatance(SupportPoint &point, const CubeMatrix &modelcube, /*out*/ bool &IsMiniSup)
	{
		DbgMsg(__FUNCTION__"[IN]");
		vector<Tuple<RHVector3, RHVector3>> SupHeadtouchNorWorList;
		RHVector3 touchWor;
		RHVector3 touchNormal;
		point.DirectionNormalizeSafe();
		RHVector3 NormalCheck = RHVector3(point.GetDirection());
		double normThreshold = 0.01;
		IsMiniSup = false;
		if (point.edge == true)
		{
			DbgMsg(__FUNCTION__"[OUT][IsMiniSup: false][return false]");
			return false;
		}
		///與物件距離太近長內部支撐
		if (RayCollisionDetect::getPickTouchNearest2(cubeSpaceforNewsup, Cubedata2, point.GetPosition(), /*out*/ point.Direction(), normThreshold, /*out*/ touchWor, /*out*/ touchNormal))
		{
			SupHeadtouchNorWorList.push_back(Tuple<RHVector3, RHVector3>(touchWor, touchNormal));
		}
		if (SupHeadtouchNorWorList.size() > 0)
		{
			RHVector3 Length;
			int MinSupportLength = 4;	// 4mm
			Length = SupHeadtouchNorWorList[0].First().Subtract(point.GetPosition());
			if (Length.Length() < MinSupportLength)
			{
				IsMiniSup = true;
			}
			DbgMsg(__FUNCTION__"[OUT][IsMiniSup: %s][return false]", IsMiniSup?"true":"false");
			return false;
		}
		DbgMsg(__FUNCTION__"[OUT][IsMiniSup: false][return false]");
		return false;
	}
	bool SupportMeshsCylinder::GetTouchPerpendicularRHVector3forLowsupport(SupportPoint point, CubeMatrix &cubeSpace, CubeData &modelData)
	{
		vector<Tuple<RHVector3, RHVector3>> SupHeadtouchNorWorList;
		RHVector3 touchWor;
		RHVector3 touchNormal;
		point.DirectionNormalizeSafe();
		//RHVector3 touchWorCheck = null;
		RHVector3 NormalCheck = RHVector3(point.GetDirection());
		double normThreshold = 0.01;
		//double tolerance = 0.1;
		RHVector3 LowertoPoint = RHVector3(point.GetPosition());
		LowertoPoint.z -= 0.1;//subtract the radius of Mark
		RHVector3 Vector = RHVector3(0, 0, -1);
		if (RayCollisionDetect::getPickTouchNearest2(cubeSpace, modelData, LowertoPoint, Vector, normThreshold, /*out*/ touchWor, /*out*/ touchNormal))
			SupHeadtouchNorWorList.push_back(Tuple<RHVector3, RHVector3>(touchWor, touchNormal));
		if (SupHeadtouchNorWorList.size() > 0)
		{
			return false;
		}
		return true;
	}
	bool SupportMeshsCylinder::GetTouchPerpendicularRHVector3forHead2(CubeNum Startcube, const CubeMatrix &modelcube, RHVector3 rotatedHeadCenterWor, RHVector3 Normal)
	{
		vector<Tuple<RHVector3, RHVector3>> SupHeadtouchNorWorList;
		RHVector3 touchWor;
		RHVector3 touchNormal;
		Normal.NormalizeSafe();
		RHVector3 NormalCheck = RHVector3(Normal);
		double normThreshold = 0.5;
		if (RayCollisionDetect::GetPickTouchNearestforHead2(this->cubeSpaceforNewsup, this->Cubedata2, rotatedHeadCenterWor, Normal, normThreshold, Startcube, /*out*/ touchWor, /*out*/ touchNormal))
		{
			return true;
		}
		return false;
	}

	// Checked OK. 2019/7/3.
	// Test Model: Robot, Z: 20, Rotate Y: 60, Inner support
	//魚叉內部支撐
	void SupportMeshsCylinder::AddGrabTail(RHVector3 Headend, CubeNum cubenum, bool Trident, CubeMatrix &modelcube)//newsup
	{
		vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> MainBodyStarEnd;
		SingleInnerSupBodyList.clear(); //newsup     
		int x = cubenum.x;
		int y = cubenum.y;
		int z;

		if (Trident == true)
			z = cubenum.z + 1;
		else
			z = cubenum.z;

		int MaxDistance = 1;
		vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> AllGrabTailBodyList;

		int Wright = 1; int Up = 1; int Down = -1;
		RHVector3 MainBodyHead = modelcube.GetCubeByIndex(x, y, z).Centerpoint;

		AllGrabTailBodyList = FindDirecToGenGrabTailBody(x, y, z, Wright, Down, modelcube);
		if (AllGrabTailBodyList.size() < 2)
			AllGrabTailBodyList = FindDirecToGenGrabTailBody(x, y, z, Wright, Up, modelcube);
		if (AllGrabTailBodyList.size() < 2)
			AllGrabTailBodyList = FindDirecToGenGrabTailBody(x, y, z, Wright, 0, modelcube);
		if (AllGrabTailBodyList.size() < 2)
			AllGrabTailBodyList = FindDirecToGenGrabTailBody(x, y, z, 0, Up, modelcube);
		if (AllGrabTailBodyList.size() < 2)
		{
			int TailNum = 0;
			for (int i = 0; i < SingleInnerSupBodyList.size(); i++)			//foreach(Quadruple<RHVector3, RHVector3, RHVector3, int*> SinglePoint in SingleInnerSupBodyList)
			{
				Quadruple<RHVector3, RHVector3, RHVector3, CubeNum> SinglePoint = SingleInnerSupBodyList[i];
				TailNum++;
				if (TailNum > 2)
					break;
				InnerSupBodyList.push_back(SinglePoint);
				//(new Quadruple<RHVector3, RHVector3, RHVector3, int[]>(MainBodyHead, SinglePoint.Second, SinglePoint.Third, CubeNum));
			}
		}

		for (int i = 0; i < AllGrabTailBodyList.size(); i++)//foreach(Quadruple<RHVector3, RHVector3, int[], bool> AllTail in AllGrabTailBodyList)
		{
			Quadruple<RHVector3, RHVector3, CubeNum, bool> AllTail = AllGrabTailBodyList[i];
			MainBodyStarEnd.push_back(AllTail);
		}

		for (int t = 0; t < MainBodyStarEnd.size(); t++)
		{
			ChecktoAddTail(MainBodyStarEnd, t);
		}
	}
	void SupportMeshsCylinder::ChecktoAddTail(vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> list, int t)
	{
		if (list[t].Second().z <= 1 && list[t].Second().z > 0)
		{
			RHVector3 tailpos = list[t].Second();
			tailpos.z = 1;
			HaveTopGenTailList.push_back(tailpos);
		}
	}
	vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> SupportMeshsCylinder::FindDirecToGenGrabTailBody(int x, int y, int z, int a, int b, CubeMatrix &modelcube)
	{
		CubeNum cubeNum;	 //{ 0, 0, 0 };
		//bool DownToPlate = false;
		vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> GrabTailStartEnd;
		bool PathCollision = false;
		RHVector3 MainBodyHead = modelcube.GetCubeByIndex(x, y, z).Centerpoint;
		vector<Triple<RHVector3, RHVector3, shared_ptr<SupportData>>> TailTouchPoint;
		bool Maintail = false;

		//if (TailTouchPoint.Count < 2)
		{
			TestDirc(x, y, z, a, b, PathCollision, modelcube, GrabTailStartEnd);
			//PathCollision = false;
			TestDirc(x, y, z, -a, -b, PathCollision, modelcube, GrabTailStartEnd);

			// if (GrabTailStartEnd.Count >= 2)
			{
				for (int i = 0; i < GrabTailStartEnd.size(); i++)	// C#: foreach(Quadruple<RHVector3, RHVector3, int*, bool> AllTail in GrabTailStartEnd)
				{
					Quadruple<RHVector3, RHVector3, CubeNum, bool> AllTail = GrabTailStartEnd[i];
					CalCandidateTailPoint(x, y, z, AllTail.Second());//找碰撞點
					if (CandidateInnerSupportPos.size() > 0)
					{
						TailTouchPoint.push_back(GenTailConeforNewsup(AllTail.Second(), 0.5, CandidateInnerSupportPos, cubeSpaceforNewsup));
					}
				}
				if (TailTouchPoint.size() >= 2)
				{
					for (int i = 0; i < TailTouchPoint.size(); i++)	//foreach(Triple<RHVector3, RHVector3, SupportData> GrabTail in TailTouchPoint)
					{
						InnerSupBodyList.push_back(Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>(MainBodyHead, TailTouchPoint[i].First(), TailTouchPoint[i].Second(), cubeNum));
					}
					return GrabTailStartEnd;
				}
				if (TailTouchPoint.size() < 2)
				{
					for (int i = 0; i < TailTouchPoint.size(); i++)	//foreach(Triple<RHVector3, RHVector3, SupportData> GrabTail in TailTouchPoint)
					{
						SingleInnerSupBodyList.push_back(Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>(MainBodyHead, TailTouchPoint[i].First(), TailTouchPoint[i].Second(), cubeNum));
					}
				}
			}
		}
		return GrabTailStartEnd;
	}


	void SupportMeshsCylinder::TestDirc(int x, int y, int z, int a, int b, bool PathCollision, CubeMatrix &modelcube, /*out*/vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> &GrabTailStartEnd)
	{
		RHVector3 MainBodyRelayPoint;
		CubeNum cubeNum;
		if (x + a < modelcube.XNum() && y + b > 0 && z - 1 >= 1)
		{
			if (modelcube.GetCubeByIndex(x + a, y + b, z - 1).EmptyCube == true)
			{
				{
					double XDirect = a;
					double YDirect = b;

					double XSpan = XDirect;//每0.5格的變化量
					double YSpan = YDirect;//每0.5格的變化量

					int XNum = lrint(floor(x + XSpan));

					int YNum = lrint(floor(y + YSpan));

					if (modelcube.GetCubeByIndex(XNum, YNum, z - 1).EmptyCube != true)
					{
						PathCollision = true;
					}

					if (PathCollision == false)
					{
						double X = modelcube.GetCubeByIndex(x, y, z - 1).Centerpoint.x;
						double Y = modelcube.GetCubeByIndex(x, y, z - 1).Centerpoint.y;
						double Z = modelcube.GetCubeByIndex(x, y, z - 1).Centerpoint.z;

						MainBodyRelayPoint = RHVector3(X + a, Y + b, Z);
						cubeNum.x = x + a;
						cubeNum.y = y + b;
						cubeNum.z = z - 1;

						// MergePoint = (new Triple<RHVector3, int[], bool>(modelcube.CubeMat[x - i + j, y + i, z - i].Centerpoint, CubeNum, DownToPlate));
						for (int k = (z - 1) - 1; k >= 0; k--)//垂直段的Body可以被連接
						{
							CubeUpdate::UpdateCubeForUnderSupportPoint(x + a, y + b, k, cubeSpaceforNewsup, true);
							CubeUpdate::UpdateCubeforBakebone(x + a, y + b, k, cubeSpaceforNewsup, true);
						}
						GrabTailStartEnd.push_back(Quadruple<RHVector3, RHVector3, CubeNum, bool>(MainBodyRelayPoint, MainBodyRelayPoint, cubeNum, true));
					}
				}
			}
		}
		PathCollision = false;
	}
	//找內部支撐接觸點
	void SupportMeshsCylinder::CalCandidateTailPoint(int x, int y, int z, RHVector3 StarPoint)
	{
		RHVector3 StartPoint = StarPoint;
		CubeNum Startcube(x, y, z);
		GetPickTouchListforTail(Startcube, StartPoint, 0, /*out*/ CandidateInnerSupportPos);
	}

	// Checked OK. 2019/6/18
	void SupportMeshsCylinder::GetPickTouchListforTail(CubeNum Startcube, RHVector3 pickStartWor, double normZThreshold, /*out*/vector<Tuple<RHVector3, RHVector3>> &InnerSuptouchNorWorList)
	{
		vector<Tuple<double, RHVector3>> deltaNorModelList;
		InnerSuptouchNorWorList.clear();
		RHVector3 pickVector;
		RHVector3 supTouchWor;
		double Length = std::numeric_limits<double>::infinity();	// double.MaxValue;

		for (double x = -1; x <= 1; x += 0.5)
		{
			for (double y = -1; y <= 1; y += 0.5)
			{
				pickVector = RHVector3(x, y, -1.0f);
				bool rtnBool = RayCollisionDetect::getPickTouchListforTailforNewsup(
					pickStartWor,
					pickVector,
					this->cubeSpaceforNewsup,	// C# AP: this
					this->Cubedata2,
					normZThreshold,
					Startcube,
					/*out*/ deltaNorModelList);

				vector<Tuple<RHVector3, RHVector3>> PathList;
				PathList = vector<Tuple<RHVector3, RHVector3>>();
				if (deltaNorModelList.size() > 0)
				{
					GenTailpath(pickStartWor, pickVector, deltaNorModelList, /*out*/ supTouchWor, /*out*/ PathList);
				}
				if (PathList.size() > 0)
				{
					for (int i = 0; i < PathList.size(); i++)
					{
						RHVector3 Touch;
						double angleBetween;
						CalDistanceandAngle(i, PathList, pickStartWor, /*out*/ Touch, /*out*/ angleBetween);

						if (Touch.Length() < Length && supTouchWor.z > 0  /*&&angleBetween >= 135*/)
						{
							Length = Touch.Length();
							//if (Touch.Length < 4)
							{
								InnerSuptouchNorWorList.clear();
								InnerSuptouchNorWorList.push_back(Tuple<RHVector3, RHVector3>(PathList[i].First(), PathList[i].Second()));
							}
						}
					}
				}
			}
		}
	}

	void SupportMeshsCylinder::GenTailpath(RHVector3 StartWor, RHVector3 Vector, vector<Tuple<double, RHVector3>> deltaNorModelList,
		/*out*/RHVector3 &supTouchWor,
		/*out*/vector<Tuple<RHVector3, RHVector3>> &TempList)
	{
		RHVector3 supTouchNorWor;
		TempList.clear();
		Vector.NormalizeSafe();
		Vector.Scale(deltaNorModelList[0].First());
		supTouchWor = StartWor.Add(Vector);
		if (supTouchWor.z < 0.3)
		{
			double newScale = StartWor.z - 0.3;
			Vector.Scale(newScale / abs(Vector.z));
			supTouchWor = StartWor.Add(Vector);
		}
		supTouchNorWor = deltaNorModelList[0].Second();
		TempList.push_back(Tuple<RHVector3, RHVector3>(supTouchWor, supTouchNorWor));
	}

	void SupportMeshsCylinder::CalDistanceandAngle(int i, vector<Tuple<RHVector3, RHVector3>> PathList, RHVector3 pickStartWor, /*out*/RHVector3 &Touch, /*out*/double &angleBetween)
	{
		Touch = PathList[i].First().Subtract(pickStartWor);
		double innerProduct;
		double cosine = 0;
		RHVector3 testvector = RHVector3(Touch);
		testvector.NormalizeSafe();
		RHVector3 supTouchNor = RHVector3(PathList[i].Second());
		supTouchNor.NormalizeSafe();
		innerProduct = supTouchNor.ScalarProduct(testvector);
		cosine = innerProduct / (supTouchNor.Length() * testvector.Length());
		angleBetween = acos(cosine) * (180 / M_PI); // unit: radian         
	}

	Triple<RHVector3, RHVector3, shared_ptr<SupportData>> SupportMeshsCylinder::GenTailConeforNewsup(RHVector3 StarPoint, double supRadius, vector<Tuple<RHVector3, RHVector3>> &List, CubeMatrix &modelcube)
	{
		shared_ptr<SupportData> treesupport(new SupportData());
		RHVector3 branchSupOrigin;
		double cosThetaMax = std::numeric_limits<double>::infinity();	//Double.PositiveInfinity;
		RHVector3 touchCosMax;
		RHVector3 touchNormal;
		RHVector3 tailVec;
		RHVector3 FinalNoraml;

		for (int i = 0; i < List.size(); i++)	//foreach(Tuple<RHVector3, RHVector3> touchNor in List)
		{
			Tuple<RHVector3, RHVector3> touchNor = List[i];
			touchNormal = touchNor.Second();
			touchNormal.NormalizeSafe();
			//touchNormal.Scale(-1);
			tailVec = touchNor.First().Subtract(StarPoint);
			tailVec.NormalizeSafe();
			double cosTheta = touchNormal.ScalarProduct(tailVec);

			if (cosTheta < cosThetaMax)
			{
				cosThetaMax = cosTheta;
				touchCosMax = touchNor.First();
				FinalNoraml = tailVec;
			}
		}

		RHVector3 a;
		a = touchCosMax.Subtract(StarPoint);
		double b = a.Length();

		RHVector3 Original = RHVector3(0.0, 0.0, -b);
		RHVector3 tree = RHVector3(0.0, 0.0, b);
		treesupport->SetPosition(Coord3D((float)touchCosMax.x, (float)touchCosMax.y, (float)touchCosMax.z));
		treesupport->orientation = FinalNoraml;
		RHVector3 vertical = RHVector3(0, 0, -1);
		RHVector3 cross = vertical.CrossProduct(treesupport->orientation);
		double innerProduct = vertical.ScalarProduct(treesupport->orientation);
		double cosine = innerProduct / (vertical.Length() * treesupport->orientation.Length());
		double angleBetween = acos(cosine); // unit: radian
		if (angleBetween != 0 && angleBetween != 3.1415926535897931) // OpenGL library bug: when angle is zero, CreateFromAxisAngle will be NaN (Not a Number)
		{
			Matrix4 m4;
			treesupport->curPos2 = treesupport->curPos = m4.rotate_unit_radian((float)angleBetween, Vector3((float)cross.x, (float)cross.y, (float)cross.z));// the same as GL.Rotate!
		}
		treesupport->curPos.transpose();
		treesupport->curPos2.transpose();

		treesupport->supType = (int)ModelType::HEAD;
		//AddTopSphere(treesupport->originalModel, num_phi, num_theta, tree, 0.66);
		treesupport->length = b;
		treesupport->radius1 = 0.66;
		//AddTree(treesupport->originalModel, num_sides, tree, 0.66, 0.3, false);
		treesupport->isNeedToMakeTopEndCap = false;
		treesupport->radius2 = 0.3;
		//AddBtmSphere(treesupport->originalModel, num_phi, num_theta, branchSupOrigin, 0.3);
		return Triple<RHVector3, RHVector3, shared_ptr<SupportData>>(StarPoint, touchCosMax, treesupport);//輸出起點,終點,cube num	//newsup
	}
	// Checked OK. 2019/6/24.
	void SupportMeshsCylinder::GenNeedleSup(double supRadius, unique_ptr<SupportData> &FistHead)
	{
		RHVector3 vertical1(0, 0, -1);
		RHVector3 cross1 = vertical1.CrossProduct(FistHead->orientation);

		// calculate the angle between (0,0,-1) and supTouchNor
		double innerProduct1 = vertical1.ScalarProduct(FistHead->orientation);
		double cosine1 = innerProduct1 / (vertical1.Length() * FistHead->orientation.Length());
		double angleBetween1 = acos(cosine1); // unit: radian
		bool Isnan = false;
		Isnan = isnan(angleBetween1);
		if (Isnan == true)
		{
			angleBetween1 = 0;
		}
		if (angleBetween1 != 0) // C# AP: OpenGL library bug: when angle is zero, CreateFromAxisAngle will be NaN (Not a Number)
		{						// Cc++: m4.rotate_unit_radian() will return identity matrix when angle is zero.
			Matrix4 m4;
			FistHead->curPos2 = FistHead->curPos = m4.rotate_unit_radian((float)angleBetween1, Vector3((float)cross1.x, (float)cross1.y, (float)cross1.z));// C# AP: the same as GL.Rotate!
		}
		FistHead->curPos.transpose();
		FistHead->curPos2.transpose();

		RHVector3 StarPoint1 = RHVector3(0, 0, 0);
		RHVector3 EndPoint = RHVector3(0, 0, 0.3);
		//AddTopSphere(FistHead->originalModel, num_phi, num_theta, EndPoint, supRadius);
		FistHead->length = 0.3;
		FistHead->radius1 = supRadius;
		//AddBtmSphere(FistHead->originalModel, num_phi, num_theta, StarPoint1, supRadius);
		//AddTree(FistHead->originalModel, num_sides, EndPoint, supRadius, supRadius, false);	// It should be TopSphere first, then Tree, and then BtmSphere. But, in order to thave same STL file as the AP. We keep the order.
		FistHead->isNeedToMakeTopEndCap = false;
		FistHead->radius2 = supRadius;
	}
	void SupportMeshsCylinder::GenSupHead(Tuple<RHVector3, CubeNum> SupportHeadEndandCubeNum, double supRadius, RHVector3 FirstHeadPosition)
	{
		unique_ptr<SupportData> NewsupHead(new SupportData());	// clear Newssuphead
		NewsupHead->supType = (int)ModelType::HEAD;
		NewsupHead->SetPosition(Coord3D((float)SupportHeadEndandCubeNum.First().x, (float)SupportHeadEndandCubeNum.First().y, (float)SupportHeadEndandCubeNum.First().z)); // supTouchWorCor is on triangle surface
		RHVector3 HeadOrientation = SupportHeadEndandCubeNum.First().Subtract(FirstHeadPosition);

		double Length = HeadOrientation.Length();
		HeadOrientation.NormalizeSafe();
		NewsupHead->orientation = RHVector3(HeadOrientation);
		RHVector3 vertical = RHVector3(0, 0, -1);
		NewsupHead->orientation.NormalizeSafe();

		RHVector3 cross = vertical.CrossProduct(NewsupHead->orientation);						// Charles: checked okay	
		// calculate the angle between (0,0,-1) and supTouchNor
		double innerProduct = vertical.ScalarProduct(NewsupHead->orientation);					// Charles: checked okay
		double cosine = innerProduct / (vertical.Length() * NewsupHead->orientation.Length());	// Charles: checked okay
		double angleBetween = acos(cosine); // unit: radian										// Charles: checked okay
		if (angleBetween != 0) // OpenGL library bug: when angle is zero, CreateFromAxisAngle will be NaN (Not a Number)
		{
			Matrix4 m4;
			NewsupHead->curPos2 = NewsupHead->curPos = m4.rotate_unit_radian((float)angleBetween, Vector3((float)cross.x, (float)cross.y, (float)cross.z));// the same as GL.Rotate!
		}
		NewsupHead->curPos.transpose();
		NewsupHead->curPos2.transpose();

		RHVector3 StarPoint = RHVector3(0, 0, 0);
		RHVector3 EEndPoint = RHVector3(0.0, 0.0, Length);

		/*	SupportData HeadConnectBall;
			HeadConnectBall->supType = (int)ModelType::HEAD;
			HeadConnectBall->SetPosition(Coord3D((float)FirstHeadPosition.x, (float)FirstHeadPosition.y, (float)FirstHeadPosition.z));
			HeadConnectBall->orientation = StarPoint;*/

		//AddTopSphere(NewsupHead->originalModel, num_phi, num_theta, EEndPoint, supRadius);
		NewsupHead->length = Length;
		NewsupHead->radius1 = supRadius;
		//AddTree(NewsupHead->originalModel, num_sides, EEndPoint, supRadius, 0.67, false);
		NewsupHead->isNeedToMakeTopEndCap = false;
		NewsupHead->radius2 = 0.67;
		//AddBtmSphere(NewsupHead->originalModel, num_phi, num_theta, StarPoint, 0.67);
		_items2.push_back(move(NewsupHead));
	}
	void SupportMeshsCylinder::GenerateCylinder(RHVector3 EndPoint, RHVector3 StartPoint, double TopRadius, double BottomRadius, /*out*/ TopoModel &model)
	{
		RHVector3 Points[4];
		TopoVertex Vertices[3];
		TopoTriangle Triangle;

		double Theta = 0;
		double dTheta = 2 * M_PI / num_sides;

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

	void SupportMeshsCylinder::GenInnersupTail(int i, /*out*/ RHVector3 &tree, /*out*/ RHVector3 &branchSupOrigin, /*out*/ unique_ptr<SupportData> &supMainBody)
	{
		supMainBody->SetPosition(Coord3D((float)InnerSupBodyList[i].Third().x, (float)InnerSupBodyList[i].Third().y, (float)InnerSupBodyList[i].Third().z));
		supMainBody->orientation = InnerSupBodyList[i].Third().Subtract(InnerSupBodyList[i].Second());

		supMainBody->supType = (int)ModelType::HEAD;

		RHVector3 a = RHVector3(0, 0, 0);
		a = InnerSupBodyList[i].Second().Subtract(InnerSupBodyList[i].Third());
		double b = a.Length();
		branchSupOrigin = RHVector3(0, 0, 0);
		RHVector3 Original = RHVector3(0, 0, -b);

		tree = RHVector3(0, 0, b);
		RHVector3 vertical = RHVector3(0, 0, -1);
		RHVector3 cross = vertical.CrossProduct(supMainBody->orientation);
		double innerProduct = vertical.ScalarProduct(supMainBody->orientation);
		double cosine = innerProduct / (vertical.Length()*supMainBody->orientation.Length());
		double angleBetween = acos(cosine); // unit: radian
		if (angleBetween != 0) // OpenGL library bug: when angle is zero, CreateFromAxisAngle will be NaN (Not a Number)
		{
			Matrix4 m4;
			supMainBody->curPos2 = supMainBody->curPos = m4.rotate_unit_radian((float)angleBetween, Vector3((float)cross.x, (float)cross.y, (float)cross.z));// the same as GL.Rotate!
		}
		supMainBody->curPos.transpose();
		supMainBody->curPos2.transpose();
		return;
	}

	void SupportMeshsCylinder::GenInnresupBody(int i)
	{
		unique_ptr<SupportData> InnresupBody(new SupportData());
		//InnresupBody->originalModel = ReferenceMesh.Body();
		RHVector3 BranchOrientation = InnerSupBodyList[i].Second().Subtract(InnerSupBodyList[i].First());
		RHVector3 InitialPosition(0, 0, 0);
		RHVector3 BodyTop(InitialPosition.x, InitialPosition.y, InitialPosition.z);
		RHVector3 BodyBottom(InitialPosition.x, InitialPosition.y, InitialPosition.z - BranchOrientation.Length());
		//GenerateCylinder(BodyBottom, BodyTop, (float)0.1 * 5, (float)0.1 * 5, /*out*/ InnresupBody->originalModel);
		InnresupBody->length = BranchOrientation.Length();
		InnresupBody->radius1 = 0.1 * 5.0;

		InnresupBody->supType = (int)ModelType::BRANCH;
		InnresupBody->SetPosition(Coord3D((float)InnerSupBodyList[i].First().x, (float)InnerSupBodyList[i].First().y, (float)InnerSupBodyList[i].First().z));
		InnresupBody->orientation = BranchOrientation;
		//InnresupBody->SetScale(Coord3D((float)0.1, (float)0.1, (float)InnresupBody->orientation.Length())); // use same size    Steven
		InnresupBody->RotateModel();
		_items2.push_back(move(InnresupBody));
		return;
	}

	void SupportMeshsCylinder::GenConnectBall(int i, /*out*/ RHVector3 &origin, /*in*//*out*/ unique_ptr<SupportData> &ConnectBall)
	{
		ConnectBall->supType = (int)ModelType::MARK;
		ConnectBall->SetPosition(Coord3D((float)InnerSupBodyList[i].Second().x, (float)InnerSupBodyList[i].Second().y, (float)InnerSupBodyList[i].Second().z));
		origin = RHVector3(0, 0, 0);
		ConnectBall->orientation = origin;
		return;
	}

	void SupportMeshsCylinder::GenADDsupTail(int n)	//Checked OK. 2019/5/31.
	{
		unique_ptr<SupportData> supTail(new SupportData());

		//supTail->originalModel = ReferenceMesh.Foot();
		//RHVector3 BodyTop = new RHVector3(0, 0, 0);
		//RHVector3 BodyBottom = new RHVector3(0, 0, -1);
		//GenerateCylinder(BodyBottom, BodyTop, (float)2.5, (float)2.5, out supTail.originalModel);
		supTail->length = 1.0;
		supTail->radius1 = 2.5;

		supTail->SetPosition(Coord3D((float)HaveTopGenTailList[n].x, (float)HaveTopGenTailList[n].y, (float)HaveTopGenTailList[n].z));
		//supTail->SetScale(Coord3D((float)SUP_DIAMETER_DEFAULT / 4.0f, (float)SUP_DIAMETER_DEFAULT / 4.0f, 1.0f));
		//supTail.Scale = new Coord3D((float)SCALE_FACTOR_DEFAULT, (float)SCALE_FACTOR_DEFAULT, 1f);
		supTail->supType = (int)ModelType::TAIL;
		_items2.push_back(move(supTail));
	}

	void SupportMeshsCylinder::GenADDsupBed(int n)
	{
		unique_ptr<SupportData> supTail(new SupportData());
		//supTail.originalModel = SupportModel.ReferenceMesh.Foot;
		supTail->SetPosition(Coord3D((float)HaveTopGenTailList[n].x, (float)HaveTopGenTailList[n].y, (float)HaveTopGenTailList[n].z));

		if (bbox.zMin() < 2.5)
		{
			GenADDsupTail(n);
		}
		else
		{
			supTail->SetScale(Coord3D(0.15f, 0.15f, 2.5f)); // JOSHUA 04-30-2018 | Resize Foot to Smaller Scale for Raft Support Requirement
			supTail->supType = (int)ModelType::CUBOID_FOOT; // JOSHUA 04-30-2018 | Use CUBOID_FOOT instead to remove opacity and adopt raft support color.
		}
		//Tuple<int, TopoTriangle> mIdxTriTuple = new Tuple<int, TopoTriangle>(0, null);
		//if (supTail.BBintersectModels(_modelsList))
		//{
		//    //supTail.Scale = new Coord3D((float)supRadius / 5, (float)supRadius / 5, 1f);    
		//    supTail.Scale = new Coord3D((float)SCALE_FACTOR_DEFAULT, (float)SCALE_FACTOR_DEFAULT, 1f);    // use same size    Steven
		//}
		_items2.push_back(move(supTail));
	}

	bool SupportMeshsCylinder::CheckPosition(int i, int j, CubeMatrix &modelcube)
	{
		bool output = true;
		for (int x = -1; x < 2; x++)
		{
			if (output == true)
				for (int y = -1; y < 2; y++)
				{
					if (i + x >= 0 && j + y >= 0)
					{
						if (modelcube.GetCubeByIndex(i + x, j + y, 0).EmptyCube == false)
						{
							output = false;
							break;
						}
					}
				}
		}
		return output;
	}

	void SupportMeshsCylinder::GenMesh(int indexSupportData, double &length, double &radius1, double &radius2, bool &isNeedToMakeTopEndCap)
	{
		SupportData *supportData = _items2[indexSupportData].get();

		/*DbgMsg(__FUNCTION__"===> supportData[%d]->supType: %d", indexSupportData, supportData->supType);
		DbgMsg(__FUNCTION__"===> length: %f", supportData->length);
		DbgMsg(__FUNCTION__"===> radius1: %f", supportData->radius1);
		DbgMsg(__FUNCTION__"===> radius2: %f", supportData->radius2);
		DbgMsg(__FUNCTION__"===> isNeedToMakeTopEndCap: %s", supportData->isNeedToMakeTopEndCap?"true":"false");*/

		RHVector3 origin(0, 0, 0);
		RHVector3 EndPoint(0, 0, supportData->length);
		if (supportData->supType == (int)ModelType::MARK)
		{
			AddTopSphere(supportData->originalModel, num_phi, num_theta, origin, 0.66);
			AddBtmSphere(supportData->originalModel, num_phi, num_theta, origin, 0.66);
		}
		else if (supportData->supType == (int)ModelType::TAIL
			|| supportData->supType == (int)ModelType::BODY
			|| supportData->supType == (int)ModelType::BRANCH)
		{
			RHVector3 BodyBottom(0, 0, -supportData->length);
			GenerateCylinder(BodyBottom, origin, supportData->radius1, supportData->radius1, /*out*/ supportData->originalModel);
		}
		else if (supportData->supType == (int)ModelType::HEAD)	//3
		{
			if (supportData->isNeedToMakeTopEndCap)
			{
				AddTopSphere(supportData->originalModel, num_phi, num_theta, EndPoint, supportData->radius1);
				AddTree(supportData->originalModel, num_sides, EndPoint, supportData->radius1, supportData->radius2, supportData->isNeedToMakeTopEndCap);
			}
			else
			{
				AddTopSphere(supportData->originalModel, num_phi, num_theta, EndPoint, supportData->radius1);
				AddTree(supportData->originalModel, num_sides, EndPoint, supportData->radius1, supportData->radius2, supportData->isNeedToMakeTopEndCap);
				AddBtmSphere(supportData->originalModel, num_phi, num_theta, origin, supportData->radius2);
			}
		}
		length = supportData->length;
		radius1 = supportData->radius1;
		radius2 = supportData->radius2;
		isNeedToMakeTopEndCap = supportData->isNeedToMakeTopEndCap;
		return;
	}
}