#pragma once
#include <vector>
#include "DataCombination.h"
#include "RHVector3.h"
#include "CubeMatrix.h"
#include "CubeUpdate.h"

using namespace std;

namespace XYZSupport
{
	class GenBranch
	{
		CubeUpdate CubeUpdate;
	public:
		vector<Triple<RHVector3, RHVector3, CubeNum>> BranchList;
		//public List<Tuple<RHVector3, int[]>> NewMergePointListBranch;

		// Checked ok, 209/6/20
		vector<int> CheckVerticalSupportLength(CubeNum cubeNum, CubeMatrix &modelcube)
		{
			int x = cubeNum.x;
			int y = cubeNum.y;
			int z = cubeNum.z;
			vector<int> NewPoint;
			int length = 0;
			double VerticalSupportLength = 0;
			int TopHeight = z;
			int LowestHeight = 0;
			for (int i = z; i > 0; i--)
			{
				if (modelcube.GetCubeByIndex(x, y, i - 1).EmptyCube != true)
				{
					LowestHeight = i;
				}
			}
			for (int j = z; j >= LowestHeight; j--)
			{
				if (modelcube.GetCubeByIndex(x, y, j).backbone == true || (j <= 1)/*最後一段連接到平台 || modelcube.CubeMat[x, y, j-1].EmptyCube!=true*/)
				{
					// if (modelcube.CubeMat[x, y, j].BelongSupportPoint == null)
					{
						length++;
					}
					if ((modelcube.GetCubeByIndex(x, y, j).BelongSupportPoint.size() > 0)/* || (modelcube.CubeMat[x, y, j].EmptyCube != true)*/ || 
						(j <= 1 || 
						modelcube.GetCubeByIndex(x, y, j - 1).EmptyCube != true))
					{
						VerticalSupportLength = length;

						if (VerticalSupportLength > 3.0)
						{
							if (VerticalSupportLength <= 6.0)
							{
								NewPoint.push_back(TopHeight - 3);
							}
							else if (VerticalSupportLength > 6)
							{
								//長Branch
								int space = lrint(ceil(VerticalSupportLength / 6.0));
								int pos = lrint(VerticalSupportLength / space);
								for (int scale = pos; scale <= VerticalSupportLength; scale += pos)
								{
									if (TopHeight - scale > (j))
									{
										NewPoint.push_back(TopHeight - scale);
										//break;//只加最高點,剩下的讓算法搜尋,會長X型 若不要則長平行
									}
								}
							}
						}
						length = 0;
						TopHeight = j;
						continue;
					}
				}
			}
			return NewPoint;
		}

		Tuple<RHVector3, CubeNum> AddingFirmBranchPoint(CubeNum cubeNum, CubeMatrix &modelcube, int Height)
		{
			int x = cubeNum.x;
			int y = cubeNum.y;
			int z = Height;
			CubeNum newCubeNum(x, y, z);
		
			return Tuple<RHVector3, CubeNum>(modelcube.GetCubeByIndex(x, y, Height).Centerpoint, newCubeNum);
		}

		vector<Triple<RHVector3, RHVector3, CubeNum>> AddingFirmBranch(CubeNum firmBranchPointCube, CubeMatrix modelcube)
		{
			//找最近的支柱
			int x = firmBranchPointCube.x;
			int y = firmBranchPointCube.y;
			int z = firmBranchPointCube.z;

			vector<Triple<RHVector3, RHVector3, CubeNum>> SubSupportList;
			RHVector3 MergePoint;
			int MergePointCube[] = { 0, 0, 0, };

			BranchList.clear(); //newsup      
			bool PathCollision = false;
			RHVector3 FirstSupvector;

			//int n = 5;//長度限制
			for (int D = 1; D <= 10; D++)
			{
				for (int i = D; i <= D; i++)
				{
					for (int j = i; j >= i; j--)//X=Y CheckEvery45degreeFirst
					{
						if (BranchList.size() < 2)
							CheckBranchPath(x - i + j, y + i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
						if (BranchList.size() < 2)
							CheckBranchPath(x + i - j, y - i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
						if (BranchList.size() < 2)
							CheckBranchPath(x + i, y + i - j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
						if (BranchList.size() < 2)
							CheckBranchPath(x - i, y - i + j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
						else if (BranchList.size() >= 2)
						{
							SubSupportList = BranchList;
							return SubSupportList;
						}
					}

					for (int next = 0; next < i * 2; next++)
					{
						if (i + next < i * 2)
						{
							int j = i + next;
							if (BranchList.size() < 2)
								CheckBranchPath(x - i + j, y + i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							if (BranchList.size() < 2)
								CheckBranchPath(x + i - j, y - i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							if (BranchList.size() < 2)
								CheckBranchPath(x + i, y + i - j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							if (BranchList.size() < 2)
								CheckBranchPath(x - i, y - i + j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							else if (BranchList.size() >= 2)
							{
								SubSupportList = BranchList;
								return SubSupportList;
							}
						}
						if (i - next >= 0)
						{
							int j = i - next;
							if (BranchList.size() < 2)
								CheckBranchPath(x - i + j, y + i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							if (BranchList.size() < 2)
								CheckBranchPath(x + i - j, y - i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							if (BranchList.size() < 2)
								CheckBranchPath(x + i, y + i - j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							if (BranchList.size() < 2)
								CheckBranchPath(x - i, y - i + j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, /*out*/ FirstSupvector);
							else if (BranchList.size() >= 2)
							{
								SubSupportList = (BranchList);
								return SubSupportList;
							}
						}
					}
				}
			}
			return SubSupportList;
		}

		void CheckBranchPath(int X, int Y, int Z, int x, int y, int z,
									int CandidatePos,
									RHVector3 FirstSupvector,
									CubeMatrix &modelcube,
									bool PathCollision,
									/*out*/ RHVector3 &Supvector)
		{

			Supvector = FirstSupvector;
			Triple<RHVector3, RHVector3, CubeNum> SubSupport = Triple<RHVector3, RHVector3, CubeNum>(RHVector3(), RHVector3(), CubeNum());

			if (X < modelcube.XNum() && X > 0 && Y < modelcube.YNum() && Y > 0 && Z > 0)
			{

				if (modelcube.GetCubeByIndex(X, Y, Z).backbone == true)
				{
					double XDirect = (X)-x;
					double YDirect = (Y)-y;

					double XSpan = XDirect / (CandidatePos * 2);//每1格的變化量
					double YSpan = YDirect / (CandidatePos * 2);//每1格的變化量
					for (int Distance = 1; Distance <= CandidatePos * 2; Distance += 1)
					{
						int XNum = lrint(floor(x + XSpan * Distance));
						int XNum2 = lrint(floor(x + XSpan * (Distance + 1)));
						int YNum = lrint(floor(y + YSpan * Distance));
						int YNum2 = lrint(floor(y + YSpan * (Distance + 1)));
						if (XNum2 >= modelcube.XNum())
						{
							XNum2 = XNum;
						}
						if (YNum2 >= modelcube.YNum())
						{
							YNum2 = YNum;
						}
						if (XNum2 != XNum || YNum2 != YNum)
						{
							if (	modelcube.GetCubeByIndex(XNum, YNum, z - Distance / 2).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum, YNum2, z - Distance / 2).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum2, YNum, z - Distance / 2).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance / 2).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum, YNum, z - Distance / 2 + 1).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum, YNum2, z - Distance / 2 + 1).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum2, YNum, z - Distance / 2 + 1).EmptyCube != true ||
									modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance / 2 + 1).EmptyCube != true)
							{
								PathCollision = true;
								break;
							}
						}
					}

					RHVector3 vector3 = modelcube.GetCubeByIndex(X, Y, Z).Centerpoint.Subtract(modelcube.GetCubeByIndex(x, y, z).Centerpoint);
					vector3.NormalizeSafe();
					if (BranchList.size() != 0)
					{
						double TwovectroDistance = vector3.Distance(FirstSupvector);
						if (TwovectroDistance <= 0.01)
						{
							PathCollision = true;
						}
					}

					if (PathCollision == false)
					{
						CubeNum cubeNum1;	// { 0, 0, 0 };
						cubeNum1.x = X;
						cubeNum1.y = Y;
						cubeNum1.z = Z;
						SubSupport = Triple<RHVector3, RHVector3, CubeNum>(modelcube.GetCubeByIndex(x, y, z).Centerpoint, modelcube.GetCubeByIndex(X, Y, Z).Centerpoint, cubeNum1);

						//NewMergePointListBranch = new List<Tuple<RHVector3, int[]>>();

						//if (modelcube.GetCubeByIndexX, Y, Z].backbone != true)
						//{
						//    Tuple<RHVector3, int[]> NewMergePoint;
						//    NewMergePoint = new Tuple<RHVector3, int[]>(modelcube.GetCubeByIndexX, Y, Z].Centerpoint, CubeNum1);
						//    NewMergePointListBranch.Add(NewMergePoint);
						//}
						Supvector = vector3;
						Supvector.NormalizeSafe();
						CubeUpdate.UpdateCubeforBakebone(SubSupport.Third().x, SubSupport.Third().y, SubSupport.Third().z, modelcube, true);
						BranchList.push_back(SubSupport);
					}
				}
			}
		}
	};
}
