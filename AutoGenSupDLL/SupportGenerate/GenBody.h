#pragma once

#include <vector>
#include "DataCombination.h"
#include "RHVector3.h"
#include "CubeMatrix.h"
#include "CubeUpdate.h"
#include "CubeNum.h"

using namespace std;

namespace XYZSupport
{
	class GenBody
	{
	private:
		vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> TestList;
		//CubeUpdate CubeUpdate = new CubeUpdate();	// Don't need. Use CubeUpdate staic function instead.

	public:
		vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> GenSupportBody(	CubeNum cubeNum,
																					RHVector3 HeadEnd,
																					CubeMatrix &modelcube,		// Update CubeMatrix information
																					vector<Tuple<RHVector3, CubeNum>> &ThinBodyList,
																					vector<Tuple<RHVector3, double>> &HaveTopGenTailList,
																					vector<Triple<RHVector3, CubeNum, bool>> &HaveTopGenInnerSupportList,
																					vector<Tuple<RHVector3, CubeNum>> &NewMergePointList)//newsup
		{
			vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> SupportBodyList;
			Quadruple<RHVector3, RHVector3, RHVector3, CubeNum> SubSupport;

			RHVector3 MergePoint;
			RHVector3 MergePointLand;
			CubeNum MergePointCube;	// { 0, 0, 0 };
			int x = cubeNum.x;
			int y = cubeNum.y;
			int z = cubeNum.z;

			bool HaveMergePointDownword = false;
			int MergepointZ = z;
			//if (z - 1 >= 0)//離平台一定距離
			{
				//檢查底下cube
				for (int checkunderpoint = z - 1; checkunderpoint > 0; checkunderpoint--)
				{
					if (modelcube.GetCubeByIndex(x, y, checkunderpoint).backbone == true)
					{
						MergepointZ = checkunderpoint;
						HaveMergePointDownword = true;
						continue;
					}
					if (modelcube.GetCubeByIndex(x, y, checkunderpoint).EmptyCube == false)
					{
						break;
					}
				}
				if (HaveMergePointDownword == true)
				{
					MergePoint = modelcube.GetCubeByIndex(x, y, MergepointZ).Centerpoint;
					MergePointCube.x = x;
					MergePointCube.y = y;
					MergePointCube.z = MergepointZ;

					for (int k = cubeNum.z; k >= MergepointZ; k--)
					{
						CubeUpdate::UpdateCubeforBakebone(x, y, k, modelcube, true);
					}
					return SupportBodyList;
				}

				if (HaveMergePointDownword != true)
				{
					Quadruple<RHVector3, RHVector3, CubeNum, bool> MergeResult;
					//從modelcube.CubeMat[x, y, height+1]開始長
					int MaxDiatanceforSup = 4;
					FindPathtoMerge(x, y, z, MaxDiatanceforSup, modelcube, ThinBodyList, HaveTopGenTailList, HaveTopGenInnerSupportList, NewMergePointList);

					for (int BranchNum = 0; BranchNum < TestList.size(); BranchNum++)
					{
						MergeResult = TestList[BranchNum];
						MergePoint = MergeResult.First();
						MergePointLand = MergeResult.Second();
		
						MergePointCube.x = MergeResult.Third().x;
						MergePointCube.y = MergeResult.Third().y;
						MergePointCube.z = MergeResult.Third().z;
				
						//UpdateBranchNumber(MergePointCube.x, MergePointCube.y, MergePointCube.z, BelongModel.cubeSpaceforNewsup);
						SubSupport = Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>(HeadEnd, MergePoint, MergePointLand, MergePointCube);
						//if (MergeResult.Fourth!=true)
						{
							SupportBodyList.push_back(SubSupport);
						}
					}

					if (TestList.size() >= 2 && TestList[1].Fourth() == false)
					{
						for (int SupportHeight = cubeNum.z - 1; SupportHeight > 0; SupportHeight--)
						{
							CubeUpdate::UpdateCubeForUnderSupportPoint(cubeNum.x, cubeNum.y, SupportHeight, modelcube, false);
							//modelcube.CubeMat[cubeNum.x, cubeNum.y, SupportHeight].UnderSupportPoint = false;
							if (modelcube.GetCubeByIndex(cubeNum.x, cubeNum.y, SupportHeight).EmptyCube != true)
							{
								break;
							}
						}
					}
					return SupportBodyList;
				}
			}

			/////////////////////////////////長Head已經篩選過Z-2>=0///////////////////////////////////////////////////////////         
			////接地
			MergePoint = modelcube.GetCubeByIndex(x, y, 0).Centerpoint;

			MergePointCube.x = x;
			MergePointCube.y = y;
			MergePointCube.z = 0;

			for (int checkpath = z; checkpath >= 0; checkpath--)
			{
				if (modelcube.GetCubeByIndex(x, y, checkpath).EmptyCube != true)
				{
					MergePointCube.z = checkpath + 1;
					break;
				}
			}
			for (int k = z; k >= MergePointCube.z; k--)
			{
				CubeUpdate::UpdateCubeForUnderSupportPoint(cubeNum.x, cubeNum.y, k, modelcube, true);
				//modelcube.CubeMat[cubeNum.x, cubeNum.y, k].UnderSupportPoint = true;
				CubeUpdate::UpdateCubeforBakebone(x, y, k, modelcube, true);
				//modelcube.CubeMat[cubeNum.x, cubeNum.y, k].backbone = true;
			}

			MergePoint.z = modelcube.GetCubeByIndex(x, y, 0).Centerpoint.z;

			if (SupportBodyList[0].Third().z <= 1 && SupportBodyList[0].Third().z > 0)
			{
				RHVector3 tailpos = SupportBodyList[0].Third();
				tailpos.z = 1;
				Tuple<RHVector3, double> tailPosition = Tuple<RHVector3, double>(tailpos, 0.0);
				HaveTopGenTailList.push_back(tailPosition);
			}
			return SupportBodyList;
		}
public:
		Quadruple<RHVector3, RHVector3, CubeNum, bool> FindPathtoMerge(	int x, int y, int z, int Threshold, 
																			CubeMatrix &modelcube,
																			vector<Tuple<RHVector3, CubeNum>> &ThinBodyList,
																			vector<Tuple<RHVector3, double>> &HaveTopGenTailList,
																			vector<Triple<RHVector3, CubeNum, bool>> &HaveTopGenInnerSupportList,
																			vector<Tuple<RHVector3, CubeNum>> &NewMergePointList)//newsup
		{
			TestList.clear();
			bool DownToPlate = false;
			Quadruple<RHVector3, RHVector3, CubeNum, bool> MergePoint;
			bool PathCollision = false;
			RHVector3 FirstSupvector;

			//int n = 5;//長度限制
			for (int D = 1; D <= Threshold; D++)
			{
				for (int i = D; i <= D; i++)
				{
					for (int j = i; j >= i; j--)
					{
						if (TestList.size() < 2)
							CheckPath(x - i + j, y + i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
						if (TestList.size() < 2)
							CheckPath(x + i - j, y - i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
						if (TestList.size() < 2)
							CheckPath(x + i, y + i - j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
						if (TestList.size() < 2)
							CheckPath(x - i, y - i + j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
						else if (TestList.size() >= 2)
							return MergePoint;
					}
					//}

					//for (int i = 1; i <= D; i++)
					//{
					for (int next = 0; next < i * 2; next++)
					{
						if (i + next < i * 2)
						{
							int j = i + next;
							if (TestList.size() < 2)
								CheckPath(x - i + j, y + i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							if (TestList.size() < 2)
								CheckPath(x + i - j, y - i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							if (TestList.size() < 2)
								CheckPath(x + i, y + i - j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							if (TestList.size() < 2)
								CheckPath(x - i, y - i + j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							else if (TestList.size() >= 2)
								return MergePoint;
						}

						if (i - next >= 0)
						{
							int j = i - next;
							if (TestList.size() < 2)
								CheckPath(x - i + j, y + i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							if (TestList.size() < 2)
								CheckPath(x + i - j, y - i, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							if (TestList.size() < 2)
								CheckPath(x + i, y + i - j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							if (TestList.size() < 2)
								CheckPath(x - i, y - i + j, z - i, x, y, z, i, FirstSupvector, modelcube, PathCollision, DownToPlate, MergePoint, NewMergePointList, /*out*/FirstSupvector);
							else if (TestList.size() >= 2)
								return MergePoint;
						}
					}
					//}
				}
			}

			//////////////////////////沒得長
			CubeNum CubeNumLand; //{ 0, 0, 0 };
			CubeNumLand.x = x;
			CubeNumLand.y = y;
			CubeNumLand.z = 0;

			if (modelcube.GetCubeByIndex(x, y, z).ConnectPlatform != true)
			{
				if (modelcube.GetCubeByIndex(x, y, z).ObjectBottomEdge == true)
				{
					CubeNumLand.x = x;
					CubeNumLand.y = y;
					CubeNumLand.z = 0;

					MergePoint = FindPlaceToLanding(x, y, z, Threshold, modelcube);
					Tuple<RHVector3, CubeNum> ThinBody = Tuple<RHVector3, CubeNum>(MergePoint.First(), MergePoint.Third());
					ThinBodyList.push_back(ThinBody);
				/*	if (MergePoint.Second() != NULL)
					{*/
						if (MergePoint.Second().z <= 1 && MergePoint.Second().z > 0)
						{
							Tuple<RHVector3, double> tailPosition = Tuple<RHVector3, double>(MergePoint.Second(), 0.0);
							HaveTopGenTailList.push_back(tailPosition);
						}
				/*	}*/

					if (MergePoint.Fourth() == false)
					{
						RHVector3 TailPos = modelcube.GetCubeByIndex(CubeNumLand.x, CubeNumLand.y, CubeNumLand.z).Centerpoint;
						TailPos.z = 0;//在物件邊緣的Tail直接長到平台,不長圓盤
						for (int k = z; k >= CubeNumLand.z; k--)
						{
							CubeUpdate::UpdateCubeForUnderSupportPoint(CubeNumLand.x, CubeNumLand.y, k, modelcube, true);
							CubeUpdate::UpdateCubeforBakebone(CubeNumLand.x, CubeNumLand.y, k, modelcube, true);
						}
						DownToPlate = true;
						//MergePoint = (new Quadruple<RHVector3, RHVector3, int[], bool>(null, TailPos, CubeNumLand, DownToPlate));
						MergePoint = Quadruple<RHVector3, RHVector3, CubeNum, bool>(RHVector3(), RHVector3(), CubeNum(), DownToPlate);
					}
				}
				if (modelcube.GetCubeByIndex(x, y, z).ObjectBottomEdge == false)
				{
					for (int checkpath = z - 1; checkpath >= 0; checkpath--)
					{
						if (modelcube.GetCubeByIndex(x, y, checkpath).EmptyCube != true)
						{
							if (z - checkpath >= 2)
							{
								CubeNumLand.z = checkpath + 1;//往上預留空間長tail
								Triple<RHVector3, CubeNum, bool> b = Triple<RHVector3, CubeNum, bool>(modelcube.GetCubeByIndex(x, y, CubeNumLand.z).Centerpoint, CubeNumLand, true);
								HaveTopGenInnerSupportList.push_back(b);//未長到平台須長內部支撐     
								//CubeNumLand.z = checkpath + 1;//往上預留空間長tail
							}
							else
							{
								CubeNumLand.z = checkpath + 1;
								Triple<RHVector3, CubeNum, bool> a = Triple<RHVector3, CubeNum, bool>(modelcube.GetCubeByIndex(x, y, CubeNumLand.z).Centerpoint, CubeNumLand, false);
								HaveTopGenInnerSupportList.push_back(a);//未長到平台須長內部支撐            
							}
							break;
						}
					}
					for (int k = z; k >= CubeNumLand.z; k--)
					{
						CubeUpdate::UpdateCubeForUnderSupportPoint(CubeNumLand.x, CubeNumLand.y, k, modelcube, true);
						CubeUpdate::UpdateCubeforBakebone(CubeNumLand.x, CubeNumLand.y, k, modelcube, true);
					}
					DownToPlate = true;
					MergePoint = Quadruple<RHVector3, RHVector3, CubeNum, bool>(RHVector3(), RHVector3(), CubeNum(), DownToPlate);
				}
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			if (modelcube.GetCubeByIndex(x, y, z).ConnectPlatform == true)
			{
				CubeNumLand.x = x;
				CubeNumLand.y = y;
				CubeNumLand.z = 0;

				for (int k = z; k >= CubeNumLand.z; k--)
				{
					CubeUpdate::UpdateCubeForUnderSupportPoint(CubeNumLand.x, CubeNumLand.y, k, modelcube, true);
					CubeUpdate::UpdateCubeforBakebone(CubeNumLand.x, CubeNumLand.y, k, modelcube, true);
				}
				DownToPlate = true;
				RHVector3 pos = RHVector3(modelcube.GetCubeByIndex(CubeNumLand.x, CubeNumLand.y, CubeNumLand.z).Centerpoint);
				pos.z = 0;
		
				//MergePoint = (new Quadruple<RHVector3, RHVector3, int[], bool>(null, pos, CubeNumLand, DownToPlate));
				MergePoint = Quadruple<RHVector3, RHVector3, CubeNum, bool>(RHVector3(), RHVector3(), CubeNum(), DownToPlate);
			}
			///////////////////////////////////////////////////////////////////////////////////////////////////////////////
			//not
			TestList.push_back(MergePoint);
			return MergePoint;
		}

		// Checked OK. 2019/6/3
		void CheckPath(int X, int Y, int Z, int x, int y, int z, int CandidatePos,
							   RHVector3 FirstSupvector,
							   CubeMatrix &modelcube,
							   bool PathCollision,
							   bool DownToPlate,
							   Quadruple<RHVector3, RHVector3, CubeNum, bool> MergePoint,
							   vector<Tuple<RHVector3, CubeNum>> &NewMergePointList,
							   RHVector3 &Supvector)
		{
			Supvector = FirstSupvector;

			if (X < modelcube.XNum() && X > 0 && Y < modelcube.YNum() && Y > 0 && Z > 0)
			{
				if (modelcube.GetCubeByIndex(X, Y, Z).UnderSupportPoint == true)
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
							if (modelcube.GetCubeByIndex(XNum, YNum, z - Distance / 2).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum, YNum2, z - Distance / 2).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum2, YNum, z - Distance / 2).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance / 2).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum, YNum, z - (Distance / 2 + 1)).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum, YNum2, z - (Distance / 2 + 1)).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum2, YNum, z - (Distance / 2 + 1)).EmptyCube != true ||
								modelcube.GetCubeByIndex(XNum2, YNum2, z - (Distance / 2 + 1)).EmptyCube != true)
							{
								PathCollision = true;
								break;
							}
						}
					}

					RHVector3 vector3 = modelcube.GetCubeByIndex(X, Y, Z).Centerpoint.Subtract(modelcube.GetCubeByIndex(x, y, z).Centerpoint);
					vector3.NormalizeSafe();
					if (TestList.size() != 0)
					{
						double TwovectroDistance = vector3.Distance(FirstSupvector);
						if (TwovectroDistance <= 0.01)
						{
							PathCollision = true;
						}
					}

					if (PathCollision == false)
					{
						CubeNum CubeNum1; //{ 0, 0, 0 };
						CubeNum1.x = X;
						CubeNum1.y = Y;
						CubeNum1.z = Z;
						MergePoint = Quadruple<RHVector3, RHVector3, CubeNum, bool>(RHVector3(), modelcube.GetCubeByIndex(X, Y, Z).Centerpoint, CubeNum1, DownToPlate);
						if (modelcube.GetCubeByIndex(X, Y, Z).backbone != true)
						{
							Tuple<RHVector3, CubeNum> NewMergePoint(modelcube.GetCubeByIndex(X, Y, Z).Centerpoint, CubeNum1);
							NewMergePointList.push_back(NewMergePoint);
						}

						CubeUpdate::UpdateCubeforBakebone(MergePoint.Third().x, MergePoint.Third().y, MergePoint.Third().z, modelcube, true);
						CubeUpdate::UpdateCubeForBelongSupportPoint(MergePoint.Third().x, MergePoint.Third().y, MergePoint.Third().z, MergePoint.Second(), modelcube);
						TestList.push_back(MergePoint);
						Supvector = vector3;
						Supvector.NormalizeSafe();
					}
				}
			}
			//DbgMsg(__FUNCTION__"[OUT]");
		}

		Quadruple<RHVector3, RHVector3, CubeNum, bool> FindPlaceToLanding(int x, int y, int z, int MaxDistance, CubeMatrix &modelcube)
		{
			CubeNum cubeNum;	//0, 0, 0;
			//bool DownToPlate = false;
			Quadruple<RHVector3, RHVector3, CubeNum, bool> MainBodyStarEnd;
			bool PathCollision = false;
			RHVector3 MainBodyRelayPoint;
			RHVector3 MainBodyEndPoint;
			for (int i = 1; i <= MaxDistance; i++)
			{
				for (int j = 0; j < i * 2; j++)
				{
					if (x - i + j < modelcube.XNum() && x - i + j > 0 && y + i < modelcube.YNum() && z - i >= 1)
					{
						if (modelcube.GetCubeByIndex(x - i + j, y + i, z - i).EmptyCube == true && modelcube.GetCubeByIndex(x - i + j, y + i, z - i).ConnectPlatform == true)
						{
							// if (modelcube.CubeMat[x - i + j, y + i, z - i].BranchNum < 5)
							{
								double XDirect = (x - i + j) - x;
								double YDirect = (y + i) - y;

								double XSpan = XDirect / (i);//每0.5格的變化量
								double YSpan = YDirect / (i);//每0.5格的變化量
								for (int Distance = 1; Distance < i; Distance += 1)
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

										if (modelcube.GetCubeByIndex(XNum, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance + 1).EmptyCube != true)
										{
											PathCollision = true;
											break;
										}
									}
								}

								if (PathCollision == false)
								{
									//Points.MainBodyRelayPoint = modelcube.CubeMat[x - i + j, y + i, z - i].Centerpoint;
									MainBodyRelayPoint = modelcube.GetCubeByIndex(x - i + j, y + i, z - i).Centerpoint;
									cubeNum.x = x - i + j;
									cubeNum.y = y + i;
									cubeNum.z = z - i;
									//Points.MainBodyEndPoint = modelcube.CubeMat[x - i + j, y + i, 0].Centerpoint;
									MainBodyEndPoint = modelcube.GetCubeByIndex(x - i + j, y + i, 0).Centerpoint;

									// MergePoint = (new Triple<RHVector3, int[], bool>(modelcube.CubeMat[x - i + j, y + i, z - i].Centerpoint, CubeNum, DownToPlate));
									for (int k = (z - i) - 1; k >= 0; k--)//垂直段的Body可以被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x - i + j, y + i, k, modelcube, true);
										CubeUpdate::UpdateCubeforBakebone(x - i + j, y + i, k, modelcube, true);
									}
									for (int k = z; k >= 0; k--)//原本接觸點垂直以下的空間不能被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x, y, z - k, modelcube, false);
									}
									for (int Distance = 0; Distance <= i; Distance += 1)
									{

									}
									return MainBodyStarEnd = Quadruple<RHVector3, RHVector3, CubeNum, bool>(MainBodyRelayPoint, MainBodyEndPoint, cubeNum, true);
								}
							}
						}
					}
					PathCollision = false;

					if (x + i - j > 0 && x + i - j < modelcube.XNum() && y - i > 0 && z - i >= 1)
					{
						if (modelcube.GetCubeByIndex(x + i - j, y - i, z - i).EmptyCube == true && modelcube.GetCubeByIndex(x + i - j, y - i, z - i).ConnectPlatform == true)
						{
							// if (modelcube.CubeMat[x - i + j, y + i, z - i].BranchNum < 5)
							{
								double XDirect = (x + i - j) - x;
								double YDirect = (y - i) - y;

								double XSpan = XDirect / (i);//每0.5格的變化量
								double YSpan = YDirect / (i);//每0.5格的變化量
								for (int Distance = 1; Distance < i; Distance += 1)
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
										if (modelcube.GetCubeByIndex(XNum, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance + 1).EmptyCube != true)
										{
											PathCollision = true;
											break;
										}
									}
								}

								if (PathCollision == false)
								{
									//Points.MainBodyRelayPoint = modelcube.CubeMat[x + i - j, y - i, z - i].Centerpoint;
									MainBodyRelayPoint = modelcube.GetCubeByIndex(x + i - j, y - i, z - i).Centerpoint;
									cubeNum.x = x + i - j;
									cubeNum.y = y - i;
									cubeNum.z = z - i;
									//Points.MainBodyEndPoint = modelcube.CubeMat[x + i - j, y - i, 0].Centerpoint;
									MainBodyEndPoint = modelcube.GetCubeByIndex(x + i - j, y - i, 0).Centerpoint;

									// MergePoint = (new Triple<RHVector3, int[], bool>(modelcube.CubeMat[x - i + j, y + i, z - i].Centerpoint, cubeNum, DownToPlate));

									for (int k = (z - i) - 1; k >= 0; k--)//垂直段的Body可以被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x + i - j, y - i, k, modelcube, true);
										CubeUpdate::UpdateCubeforBakebone(x + i - j, y - i, k, modelcube, true);
									}
									for (int k = z; k >= 0; k--)//原本接觸點垂直以下的空間不能被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x, y, z - k, modelcube, false);
									}
									return MainBodyStarEnd = Quadruple<RHVector3, RHVector3, CubeNum, bool>(MainBodyRelayPoint, MainBodyEndPoint, cubeNum, true);
								}
							}
						}
					}
					PathCollision = false;
					if (x + i < modelcube.XNum() && y + i - j > 0 && y + i - j < modelcube.YNum() && z - i >= 1)
					{
						if (modelcube.GetCubeByIndex(x + i, y + i - j, z - i).EmptyCube == true && modelcube.GetCubeByIndex(x + i, y + i - j, z - i).ConnectPlatform == true)
						{
							// if (modelcube.CubeMat[x - i + j, y + i, z - i].BranchNum < 5)
							{
								double XDirect = (x + i) - x;
								double YDirect = (y + i - j) - y;

								double XSpan = XDirect / (i);//每0.5格的變化量
								double YSpan = YDirect / (i);//每0.5格的變化量
								for (int Distance = 1; Distance < i; Distance += 1)
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

										if (modelcube.GetCubeByIndex(XNum, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance + 1).EmptyCube != true)
										{
											PathCollision = true;
											break;
										}
									}
								}

								if (PathCollision == false)
								{
									//Points.MainBodyRelayPoint = modelcube.CubeMat[x + i, y + i - j, z - i].Centerpoint;
									MainBodyRelayPoint = modelcube.GetCubeByIndex(x + i, y + i - j, z - i).Centerpoint;
									cubeNum.x = x + i;
									cubeNum.y = y + i - j;
									cubeNum.z = z - i;
									//Points.MainBodyEndPoint = modelcube.CubeMat[x + i, y + i - j, 0].Centerpoint;
									MainBodyEndPoint = modelcube.GetCubeByIndex(x + i, y + i - j, 0).Centerpoint;

									// MergePoint = (new Triple<RHVector3, int[], bool>(modelcube.CubeMat[x - i + j, y + i, z - i].Centerpoint, cubeNum, DownToPlate));

									for (int k = (z - i); k >= 0; k--)//垂直段的Body可以被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x + i, y + i - j, k, modelcube, true);
										CubeUpdate::UpdateCubeforBakebone(x + i, y + i - j, k, modelcube, true);
									}
									for (int k = z; k >= 0; k--)//原本接觸點垂直以下的空間不能被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x, y, z - k, modelcube, false);
									}
									return MainBodyStarEnd = Quadruple<RHVector3, RHVector3, CubeNum, bool>(MainBodyRelayPoint, MainBodyEndPoint, cubeNum, true);
								}
							}
						}
					}
					PathCollision = false;

					if (x - i > 0 && y - i + j < modelcube.YNum() && y - i + j > 0 && z - i >= 1)
					{

						if (modelcube.GetCubeByIndex(x - i, y - i + j, z - i).EmptyCube == true && modelcube.GetCubeByIndex(x - i, y - i + j, z - i).ConnectPlatform == true)
						{
							// if (modelcube.CubeMat[x - i + j, y + i, z - i].BranchNum < 5)
							{
								double XDirect = (x - i) - x;
								double YDirect = (y - i + j) - y;

								double XSpan = XDirect / (i);//每0.5格的變化量
								double YSpan = YDirect / (i);//每0.5格的變化量
								for (int Distance = 1; Distance < i; Distance += 1)
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
										if (modelcube.GetCubeByIndex(XNum, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum, YNum2, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum, z - Distance + 1).EmptyCube != true ||
											 modelcube.GetCubeByIndex(XNum2, YNum2, z - Distance + 1).EmptyCube != true)
										{
											PathCollision = true;
											break;
										}
									}
								}

								if (PathCollision == false)
								{
									//Points.MainBodyRelayPoint = modelcube.CubeMat[x - i, y - i + j, z - i].Centerpoint;
									MainBodyRelayPoint = modelcube.GetCubeByIndex(x - i, y - i + j, z - i).Centerpoint;
									cubeNum.x = x - i;
									cubeNum.y = y - i + j;
									cubeNum.z = z - i;
									//Points.MainBodyEndPoint = modelcube.CubeMat[x - i, y - i + j, 0].Centerpoint;
									MainBodyEndPoint = modelcube.GetCubeByIndex(x - i, y - i + j, 0).Centerpoint;

									// MergePoint = (new Triple<RHVector3, int[], bool>(modelcube.CubeMat[x - i + j, y + i, z - i].Centerpoint, cubeNum, DownToPlate));

									for (int k = (z - i) - 1; k >= 0; k--)//垂直段的Body可以被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x - i, y - i + j, k, modelcube, true);
										CubeUpdate::UpdateCubeforBakebone(x - i, y - i + j, k, modelcube, true);
									}
									for (int k = z; k >= 0; k--)//原本接觸點垂直以下的空間不能被連接
									{
										CubeUpdate::UpdateCubeForUnderSupportPoint(x, y, z - k, modelcube, false);
									}
									return MainBodyStarEnd = Quadruple<RHVector3, RHVector3, CubeNum, bool>(MainBodyRelayPoint, MainBodyEndPoint, cubeNum, true);
								}
							}
						}
					}
				}
			}
			return MainBodyStarEnd = Quadruple<RHVector3, RHVector3, CubeNum, bool>(RHVector3(), RHVector3(), cubeNum, false);
		}

		Triple<RHVector3, RHVector3, bool> RecordBackbone(CubeNum cubeNum, CubeMatrix &modelcube)
		{
			int x = cubeNum.x;
			int y = cubeNum.y;
			int z = cubeNum.z;
			
			int length = 0;
			int LowestHeight = z;
			bool NeedConnectBall = false;

			for (int j = z; j >= 0; j--)
			{
				// if (modelcube.CubeMat[x, y, j].EmptyCube == true)
				{
					if (modelcube.GetCubeByIndex(x, y, j).backbone == true)
					{
						length++;
						LowestHeight = j;
					}
					if ((modelcube.GetCubeByIndex(x, y, j).backbone != true))
					{
						NeedConnectBall = true;
						break;
					}
				}

				if (modelcube.GetCubeByIndex(x, y, j).EmptyCube != true)
				{
					break;
				}
			}
			Triple<RHVector3, RHVector3, bool> BackboneList = Triple<RHVector3, RHVector3, bool>(modelcube.GetCubeByIndex(x, y, z).Centerpoint, modelcube.GetCubeByIndex(x, y, LowestHeight).Centerpoint, NeedConnectBall);
			return BackboneList;
		}
	};
}
