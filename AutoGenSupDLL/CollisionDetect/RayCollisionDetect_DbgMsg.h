#pragma once
#include "RHVector3.h"
#include "TopoTriangle.h"
#include "CubeMatrix.h"
#include "CubeNum.h"
#include "DataCombination.h"

namespace XYZSupport
{
	class RayCollisionDetect
	{
		//--- MODEL_SLA // milton
		// 碰撞測試,給定射線的起始點(pickStartWor)及方向(pickDirWor),回傳碰到的點及normal.
	public:
		static bool getPickTouchNearest2(	CubeMatrix &cubeSpace,
											CubeData &modelData,
											/*in*/ RHVector3 pickStartWor,
											/*out*/ RHVector3 &pickDirWor,
											/*in*/ double normZThreshold,		// default: 0.01
											/*out*/ RHVector3 &supTouchWor,
											/*out*/ RHVector3 &supTouchNorWor)
		{
			DbgMsg(__FUNCTION__"[IN]");

			DbgMsg(__FUNCTION__"===> pickStartWor: %s", pickStartWor.ToString().c_str());
			DbgMsg(__FUNCTION__"===> pickDirWor: %s", pickDirWor.ToString().c_str());

			vector<Tuple<double, RHVector3>> deltaNorModelList;	// <length, direction>
			supTouchWor = RHVector3(0, 0, 0);
			supTouchNorWor = RHVector3(0, 0, 0);
			bool rtnBool = getPickTouchList(cubeSpace,
											modelData,
											pickStartWor,
											pickDirWor,
											normZThreshold,
											/*out*/ deltaNorModelList);

			if (rtnBool && deltaNorModelList.size() > 0)
			{		
				// Charles: TEST TEST TEST
				DbgMsg(__FUNCTION__"===>  deltaNorModelList.size(): %d", deltaNorModelList.size());
				for (int i = 0; i < deltaNorModelList.size(); i++)
				{
					DbgMsg(__FUNCTION__"===> deltaNorModelList[%d]: %f, %s", i, deltaNorModelList[i].First(), deltaNorModelList[i].Second().ToString().c_str());
				}


				pickDirWor.NormalizeSafe();	// Charles: Added. In C# AP, pickDirWor will be normalized in getPickTouchList().
				pickDirWor.Scale(deltaNorModelList[0].First());
				supTouchWor = pickStartWor.Add(pickDirWor);
				supTouchNorWor = deltaNorModelList[0].Second();
			}
			else
			{
				DbgMsg(__FUNCTION__"===>  rtnBool: %s", rtnBool ?"true":"false");
				DbgMsg(__FUNCTION__"===>  deltaNorModelList.size(): %d", deltaNorModelList.size());
			}

		
			DbgMsg(__FUNCTION__"===> supTouchWor: %s", supTouchWor.ToString().c_str());
			DbgMsg(__FUNCTION__"===> supTouchNorWor: %s", supTouchNorWor.ToString().c_str());
			DbgMsg(__FUNCTION__"===>  rtnBool: %s", rtnBool ? "true" : "false");
			DbgMsg(__FUNCTION__"[OUT]");

	
			return rtnBool;
		}

		/// <summary>
		/// detect the model's cube which  has collision with ray tracing
		/// </summary>
		/// <param name="model">model that block the ray</param>
		/// <param name="cube">cube to test</param>
		/// <param name="pickStartWor">ray start point with world coordinate</param>
		/// <param name="pickDirWor">ray directory</param>
		/// <param name="normZThreshold">threshold of Z unit normal vector</param>
		/// <returns></returns>
		private:
		static Triple<double, RHVector3, CubeData *> getPickTouchFromCube(	CubeData &modelData,
																			bool forhead,			// default: false
																			const Cube &cube,
																			const RHVector3 &pickStartWor,
																			RHVector3 &pickDirWor,
																			double normZThreshold)	// default: 0.01
		{
			if (0 == cube.verIdxTriIdxList.size())
				return Triple<double, RHVector3, CubeData *>(0, RHVector3(), nullptr);

			double nearestPointDelta = std::numeric_limits<double>::infinity(); // C# AP: double.PositiveInfinity;
			RHVector3 nearestPointNormal;
			double delta = 0;
			double u = 0;
			double v = 0;
			RHBoundingBox cubeBox;
			cubeBox.Add(cube.XLowBound(), cube.YLowBound(), cube.ZLowBound());
			cubeBox.Add(cube.XLowBound() + cube.Span(), cube.YLowBound() + cube.Span(), cube.ZLowBound() + cube.Span());
			// We try ray intersection with cube bounding triangles first
			// we need to define an api that getting an model and ray as arguments
			if (true == intersectRayAABox1(cubeBox, pickStartWor, pickDirWor) || cubeBox.ContainsPoint(pickStartWor))
			{
				for(const auto &verIdxTriIdx : cube.verIdxTriIdxList) // C# AP: foreach(Tuple<int, int> verIdxTriIdx in cube.verIdxTriIdxList)
				{
					TopoTriangle triWor = getTriWorByMesh(verIdxTriIdx.Second(), modelData.Model, modelData.Trans);
					if (forhead == false)
					{
						if (normZThreshold > 0 && triWor.normal.z < normZThreshold) continue; // direction of ray and triangle normal is different
						if (normZThreshold < 0 && triWor.normal.z > normZThreshold) continue; // direction of ray and triangle normal is different
					}
					if (triWor.IntersectsLineTest(pickStartWor, pickDirWor, /*out*/ delta, /*out*/ u, /*out*/ v))
					{
						if (delta > 0 && delta < nearestPointDelta)
						{
							nearestPointDelta = delta;
							nearestPointNormal = RHVector3(triWor.normal);
						}
					} // for foreach (Tuple<int, int> verIdxTriIdx in cube.verIdxTriIdxList)
				}
			}
			if (nearestPointNormal != RHVector3())
				return Triple<double, RHVector3, CubeData *>(nearestPointDelta, nearestPointNormal, &modelData);
			else
				return Triple<double, RHVector3, CubeData *>(0, RHVector3(), nullptr);
		}//

		static Tuple<double, RHVector3> getPickTouchFromCube2(	CubeData &modelData,
																bool forhead,
																Cube cube,
																RHVector3 pickStartWor,
																RHVector3 &pickDirWor,	/*out*/
																double normZThreshold)
		{
			if (0 == cube.verIdxTriIdxList.size())
				return Tuple<double, RHVector3>(0, RHVector3());

			double nearestPointDelta = std::numeric_limits<double>::infinity(); // C# AP: double.PositiveInfinity;
			RHVector3 nearestPointNormal;
			double delta = 0;
			double u = 0;
			double v = 0;
			RHBoundingBox cubeBox;
			cubeBox.Add(cube.XLowBound(), cube.YLowBound(), cube.ZLowBound());
			cubeBox.Add(cube.XLowBound() + cube.Span(), cube.YLowBound() + cube.Span(), cube.ZLowBound() + cube.Span());

			// We try ray intersection with cube bounding triangles first
			// we need to define an api that getting an model and ray as arguments
			if (true == intersectRayAABox1(cubeBox, pickStartWor, pickDirWor) || cubeBox.ContainsPoint(pickStartWor))
			{
				for(int i = 0; i< cube.verIdxTriIdxList.size(); i++)	//foreach(Tuple<int, int> verIdxTriIdx in cube.verIdxTriIdxList)
				{
					Tuple<int, int> verIdxTriIdx = cube.verIdxTriIdxList[i];
					TopoTriangle triWor = getTriWorByMesh(verIdxTriIdx.Second(), modelData.Model, modelData.Trans);	// C# AP: TopoTriangle triWor = supportcylinder.getTriWorByMesh(verIdxTriIdx.Second());	//
					if (forhead == false)
					{
						if (normZThreshold > 0 && triWor.normal.z < normZThreshold) continue; // direction of ray and triangle normal is different
						if (normZThreshold < 0 && triWor.normal.z > normZThreshold) continue; // direction of ray and triangle normal is different
					}
					if (triWor.IntersectsLineTest(pickStartWor, pickDirWor, /*out*/ delta, /*out*/ u, /*out*/ v))
					{
						if (delta > 0 && delta < nearestPointDelta)
						{
							nearestPointDelta = delta;
							nearestPointNormal = RHVector3(triWor.normal);
						}
					} // for foreach (Tuple<int, int> verIdxTriIdx in cube.verIdxTriIdxList)
				}
			}
			if (nearestPointNormal != RHVector3())
				return Tuple<double, RHVector3>(nearestPointDelta, nearestPointNormal);
			else
				return Tuple<double, RHVector3>(0, RHVector3());
		}
		//check cube that only ray through by.

		public:
		static bool intersectRayAABox1(RHBoundingBox box, RHVector3 pos, /*out*/RHVector3 &dir)
		{
			// dir is unit direction vector of ray
			dir.NormalizeSafe();
			RHVector3 dirfrac;
			double t;
			dirfrac.x = 1.0 / dir.x;
			dirfrac.y = 1.0 / dir.y;
			dirfrac.z = 1.0 / dir.z;
			// lb is the corner of AABB with minimal coordinates - left bottom, rt is maximal corner
			// r.org is origin of ray
			double t1 = (box.minPoint.x - pos.x) * dirfrac.x;
			double t2 = (box.maxPoint.x - pos.x) * dirfrac.x;
			double t3 = (box.minPoint.y - pos.y) * dirfrac.y;
			double t4 = (box.maxPoint.y - pos.y) * dirfrac.y;
			double t5 = (box.minPoint.z - pos.z) * dirfrac.z;
			double t6 = (box.maxPoint.z - pos.z) * dirfrac.z;

			double tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
			double tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

			// if tmax < 0, ray (line) is intersecting AABB, but whole AABB is behing us
			if (tmax < 0)
			{
				t = tmax;
				return false;
			}

			// if tmin > tmax, ray doesn't intersect AABB
			if (tmin > tmax)
			{
				t = tmax;
				return false;
			}

			t = tmin;
			return true;
		}

		/// <summary>
		/// detect the model's point which  has collision with ray tracing
		/// </summary>
		/// <param name="modelType">model type</param>
		/// <param name="pickStartWor">ray start point with world coordinate</param>
		/// <param name="pickDirWor">ray directory</param>
		/// <param name="normZThreshold">threshold of Z unit normal vector</param>
		/// <param name="deltaNorModelList">output list of intersection models </param>
		/// <returns></returns>
		static bool getPickTouchList(	CubeMatrix &cubeSpace,
										CubeData &modelData,
										RHVector3 pickStartWor,
										RHVector3 pickDirWor,
										double normZThreshold,
										/*out*/ vector<Tuple<double, RHVector3>> &deltaNorModelList)
		{
			double delta = 0;
			double u = 0;
			double v = 0;
			bool forhead = false;
			TopoTriangle tInWorld;

#if 0	// 這段code在找AP所有intersction model. As for C++ Support library, we only check the model stored in the CubeData 
			if (GetAllViewModels == nullptr)
				return false;

			// We need to find the intersection model, assuming there are no supports 
			foreach (ThreeDModel m in GetAllViewModels())
			{

				if (m.GetType() != modelType) continue;
				if (typeof(SupportModel) == m.GetType())
				{
					SupportModel supModel = (SupportModel)m;
					if ((int)SupportModel.ModelType.HEAD == supModel.supType ||
						(int)SupportModel.ModelType.BODY == supModel.supType ||
						(int)SupportModel.ModelType.BRANCH == supModel.supType ||
						(int)SupportModel.ModelType.TAIL == supModel.supType ||
						(int)SupportModel.ModelType.FOOT == supModel.supType ||
						//Modified to fix unable to create tree support after porting Surface to base function.
						//(int)SupportModel.ModelType.F2BARROW != supModel.supType
						(int)SupportModel.ModelType.F2BARROW == supModel.supType
						)
						continue;
				}
				PrintModel model = (PrintModel)m;
				if (nullptr == model.originalModel) continue;
				if (!model.BoundingBoxWOSupport.ContainsPoint(pickStartWor) &&
					 !isPickModelBB(model.BoundingBoxWOSupport, pickStartWor, pickDirWor))
					continue;
				
				if (nullptr != model.cubeSpace && model.cubeSpace.Count() > 0)
				{
					// we have to generate 8 face of the cube here
					//foreach (Layer layer in model.layerList)
					//{
					foreach(Cube cube in model.cubeSpace.ToArray())
					{
						Triple<Double, RHVector3, CubeData *> deltaNorModel = getPickTouchFromCube(ref model, forhead, cube, pickStartWor, pickDirWor, normZThreshold);
						if (nullptr != deltaNorModel.Second)
							deltaNorModelList.Add(deltaNorModel);
					}// foreach (Cube cube in layer.cubeList)
					//}// foreach (Layer layer in model.layerList)
				}
				else
				{
					foreach(TopoTriangle t in model.Model.triangles.triangles)
					{
						model.Model.getTriInWorld(model.trans, t, out tInWorld);
						// Debug.WriteLine("The normal is " + t.normal );
						if (0 != normZThreshold)
						{
							if (normZThreshold > 0 && tInWorld.normal.z < normZThreshold) continue; // direction of ray and triangle normal is different
							if (normZThreshold < 0 && tInWorld.normal.z > normZThreshold) continue; // direction of ray and triangle normal is different
						}

						if (tInWorld.IntersectsLineTest(pickStartWor, pickDirWor, out delta, out u, out v))
						{
							//Debug.WriteLine("InterSect! The u, v  is " + u + " " + v);
							if (delta > 0)
							{
								deltaNorModelList.Add(new Triple<Double, RHVector3, PrintModel>(delta, tInWorld.normal, model));
							}
							/*if ( delta>0 && delta < minDelta)
							{
								minDelta = delta;
								//minU = u;
								//minV = v;
								nearModel = model;
								//nearTriangle = tInWorld;
							}*/
						}
					}// end foreach triangle
				}
			}// for each model
#endif
			/* Test Result
			Model: Hedgehog.stl
			Generate2() time: 10.6s
			The following loop totally takes 7.7s during whole Gerate2() process.
			*/
			//g_sw2.Start();
#if 1
			for(int i=0; i< cubeSpace.Count(); i++)			// C# AP: foreach(Cube cube in model.cubeSpace.ToArray())
			{
				Cube& cube = cubeSpace.GetCubeByIndex(i);	// use "&" to reduce time.	// 12s --> 7s (Hedgehog.stl)	
				Triple<double, RHVector3, CubeData *> deltaNorModel = getPickTouchFromCube(modelData, forhead, cube, pickStartWor, pickDirWor, normZThreshold);
				if (RHVector3() != deltaNorModel.Second())
				{
					deltaNorModelList.push_back(Tuple<double, RHVector3>(deltaNorModel.First(), deltaNorModel.Second()));
				}
			}// foreach (Cube cube in layer.cubeList)
#else
			vector<Cube> &cubeVector = cubeSpace.ToArray();
			for (const auto &cube: cubeVector)			// C# AP: foreach(Cube cube in model.cubeSpace.ToArray())
			{
				Triple<double, RHVector3, CubeData *> deltaNorModel = getPickTouchFromCube(modelData, forhead, cube, pickStartWor, pickDirWor, normZThreshold);
				if (RHVector3() != deltaNorModel.Second())
				{
					deltaNorModelList.push_back(Tuple<double, RHVector3>(deltaNorModel.First(), deltaNorModel.Second()));
				}
			}// foreach (Cube cube in layer.cubeList)

#endif
			//g_sw2.Stop();

			if (deltaNorModelList.size() > 0)
			{
				// T(u, v) = (1-u-v)V0 + uV1 + vV2  
				/*supTouchWor = new RHVector3(nearTriangle.vertices[0].pos.x * (1 - minU - minV) + nearTriangle.vertices[1].pos.x * minU + nearTriangle.vertices[2].pos.x * minV,
												   nearTriangle.vertices[0].pos.y * (1 - minU - minV) + nearTriangle.vertices[1].pos.y * minU + nearTriangle.vertices[2].pos.y * minV,
												   nearTriangle.vertices[0].pos.z * (1 - minU - minV) + nearTriangle.vertices[1].pos.z * minU + nearTriangle.vertices[2].pos.z * minV);*/
				sort(deltaNorModelList.begin(), deltaNorModelList.end());	// C# AP: deltaNorModelList.Sort(delegate(Triple<Double, RHVector3, PrintModel> t1, Triple<Double, RHVector3, PrintModel> t2)	{return t1.First.CompareTo(t2.First);}	);

				/*Debug.WriteLine(" minDelta is " + minDelta);
				foreach (Triple<Double, RHVector3, PrintModel> deltaNorModel in deltaNorModelList)
				{
					Debug.WriteLine(" Delta is " + deltaNorMid.First + " normal " + deltaNorMid.Second + " mid " + deltaNorModel.Third);
				}*/
				return true;
				// Debug.WriteLine("The center of intersection at world-space is " + supTouch + " normal " + supTouchNor.ToString());
			}
			return false;
		}

		private:
		static bool isPickModelBB(RHBoundingBox modelBB, RHVector3 pickStartWor, RHVector3 pickDirWor)
		{
			double delta = 0, u = 0, v = 0;
			vector<TopoTriangle> BBTriangles = modelBB.getBoundingTri();
			for (int i = 0; i < BBTriangles.size(); i++)	// C# AP: foreach(TopoTriangle boundingTriWor in BBTriangles)
			{
				TopoTriangle boundingTriWor = BBTriangles[i];
				if (boundingTriWor.normal.ScalarProduct(pickDirWor) < 0 &&
					 boundingTriWor.IntersectsLineTest(pickStartWor, pickDirWor, /*out*/ delta, /*out*/ u, /*out*/ v) &&
					 delta > 0)  // this is THE intersection cube!
					return true;
			}
			return false;
		}

		public:
		static bool getPickTouchListforTailforNewsup(
										RHVector3 pickStartWor,
										RHVector3 &pickDirWor,
										CubeMatrix &cubeSpace, // C# AP: SupportCylinder.SupportMeshsCylinder supportcylinder,
										CubeData &modelData,
										double normZThreshold,
										CubeNum StartCube,
										/*out*/ vector<Tuple<double, RHVector3>> &deltaNorModelList)
		{
			double delta = 0;
			double u = 0;
			double v = 0;
			TopoTriangle tInWorld;
			deltaNorModelList.clear();

			//if (GetAllViewModels == nullptr)
			//	return false;

			// We need to find the intersection model, assuming there are no supports 
			{
				if (cubeSpace.Count() > 0)
				{
					// we have to generate 8 face of the cube here
					//foreach (Layer layer in model.layerList)
					//{
					//Cube cube = model.cubeSpaceforNewsup.CubeMat[x, y, z];
					//foreach (Cube cube in model.cubeSpaceforNewsup.CubeMat)
					{
						bool forhead = false;
						//for (int d = 1; d <= 5; d++)
						{
							for (int L = 0; L <= 2; L++)
							{
								for (int D = 0; D <= 1; D++)
								{
									for (int i = D; i <= D; i++)
									{
										for (int j = i; j >= i; j--)
										{
											if (CheckTailPath2(forhead, StartCube, StartCube.x - i + j, StartCube.y + i, StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x + i - j, StartCube.y - i, StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x + i, StartCube.y + i - j, StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x - i, StartCube.y - i + j, StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x - i, StartCube.y - i , StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x + i, StartCube.y + i , StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x + i, StartCube.y - i, StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
											if (CheckTailPath2(forhead, StartCube, StartCube.x - i, StartCube.y + i, StartCube.z - L, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
											{
												if (deltaNorModelList.size() > 0)
												{
													sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
													return true;
												}
											}
										}
										// foreach (Cube cube in layer.cubeList)
									}// foreach (Layer layer in model.layerList)
								}
							}
						}
					}
				}
				else
				{
					for(int i=0; i<modelData.Model.triangles.Count(); i++)	// C# AP: foreach(TopoTriangle t in supportcylinder.Cubedata2.Model.triangles.triangles)
					{
						//model.Model.getTriInWorld(model.trans, t, out tInWorld);
						// Debug.WriteLine("The normal is " + t.normal );
					
						TopoTriangle t = modelData.Model.triangles._t[i];
						if (0 != normZThreshold)
						{
							if (normZThreshold > 0 && tInWorld.normal.z < normZThreshold) continue; // direction of ray and triangle normal is different
							if (normZThreshold < 0 && tInWorld.normal.z > normZThreshold) continue; // direction of ray and triangle normal is different
						}

						if (tInWorld.IntersectsLineTest(pickStartWor, pickDirWor, /*out*/ delta, /*out*/ u, /*out*/ v))
						{
							//Debug.WriteLine("InterSect! The u, v  is " + u + " " + v);
							if (delta > 0)
							{
								deltaNorModelList.push_back(Tuple<double, RHVector3>(delta, tInWorld.normal));
							}

						}
					}// end foreach triangle
				}
			}// for each model

			if (deltaNorModelList.size() > 0)
			{
				sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
				/*Debug.WriteLine(" minDelta is " + minDelta);
				foreach (Triple<Double, RHVector3, PrintModel> deltaNorModel in deltaNorModelList)
				{
					Debug.WriteLine(" Delta is " + deltaNorMid.First + " normal " + deltaNorMid.Second + " mid " + deltaNorModel.Third);
				}*/
				return true;
				// Debug.WriteLine("The center of intersection at world-space is " + supTouch + " normal " + supTouchNor.ToString());
			}
			return false;
		}
public:		
		static bool GetPickTouchNearestforHead2(	CubeMatrix &cubeSpace,
													CubeData &modelData,
													RHVector3 pickStartWor,
													RHVector3 pickDirWor,
													double normZThreshold,
													CubeNum StartCube,
													/*out*/ RHVector3 supTouchWor,
													/*out*/ RHVector3 supTouchNorWor)
		{
			vector<Tuple<double, RHVector3>> deltaNorModelList;
			supTouchWor = RHVector3(0, 0, 0);
			supTouchNorWor = RHVector3(0, 0, 0);
			bool rtnBool = getPickTouchListforHead2(cubeSpace,
													modelData,
													pickStartWor,
													pickDirWor,
													normZThreshold,
													StartCube,
													/*out*/ deltaNorModelList);

			if (rtnBool && deltaNorModelList.size() > 0)
			{
				pickDirWor.Scale(deltaNorModelList[0].First());
				supTouchWor = pickStartWor.Add(pickDirWor);
				supTouchNorWor = deltaNorModelList[0].Second();
			}

			return rtnBool;
		}

private: 
		// Checked OK. 2019/6/18
		static bool getPickTouchListforHead2(	CubeMatrix &cubeSpace,
												CubeData &modelData,
												RHVector3 pickStartWor,
												RHVector3 pickDirWor,
												double normZThreshold,
												CubeNum StartCube,
												/*out*/ vector<Tuple<double, RHVector3>> &deltaNorModelList)
		{
			double delta = 0;
			double u = 0;
			double v = 0;
			TopoTriangle tInWorld;

		/*	if (GetAllViewModels == nullptr)
				return false;*/
			{
				if (cubeSpace.Count() > 0)
				{
					bool forhead = true;
					{
						for (int z = 0; z <= 1; z++)
						{
							for (int i = -1; i <= 1; i++)
							{
								for (int j = -1; j <= 1; j++)
								{
									if (CheckTailPath2(forhead, StartCube, StartCube.x + i, StartCube.y + j, StartCube.z - z, cubeSpace, modelData, pickStartWor, pickDirWor, normZThreshold, /*out*/ deltaNorModelList) == true)
									{
										if (deltaNorModelList.size() > 0)
										{
											sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
											return true;
										}
									}
								}
							}
						}
					}
				}
				else
				{
					for (int i = 0; i < modelData.Model.triangles.Count(); i++)	// C# AP: foreach(TopoTriangle t in supportcylinder.Cubedata2.Model.triangles.triangles)
					{
						TopoTriangle t = modelData.Model.triangles._t[i];
						Vector4 *pver1 = t.vertices[0].pos.asVector4();
						Vector4 *pver2 = t.vertices[1].pos.asVector4();
						Vector4 *pver3 = t.vertices[2].pos.asVector4();
						Vector4 ver1 = modelData.Trans*(*pver1);	// C# AP: Vector4.Transform(ver1, modelData.Trans);
						Vector4 ver2 = modelData.Trans*(*pver2);	// C# AP: Vector4.Transform(ver2, modelData.Trans);
						Vector4 ver3 = modelData.Trans*(*pver3);	// C# AP: Vector4.Transform(ver3, modelData.Trans);
						delete pver1;
						delete pver2;
						delete pver3;
						TopoVertex v1 = TopoVertex(0, RHVector3(ver1.x, ver1.y, ver1.z));
						TopoVertex v2 = TopoVertex(1, RHVector3(ver2.x, ver2.y, ver2.z));
						TopoVertex v3 = TopoVertex(2, RHVector3(ver3.x, ver3.y, ver3.z));
						tInWorld = TopoTriangle(v1, v2, v3);
						if (0 != normZThreshold)
						{
							if (normZThreshold > 0 && tInWorld.normal.z < normZThreshold) continue; // direction of ray and triangle normal is different
							if (normZThreshold < 0 && tInWorld.normal.z > normZThreshold) continue; // direction of ray and triangle normal is different
						}

						if (tInWorld.IntersectsLineTest(pickStartWor, pickDirWor, /*out*/ delta, /*out*/ u, /*out*/ v))
						{
							if (delta > 0)
							{
								deltaNorModelList.push_back(Tuple<double, RHVector3>(delta, tInWorld.normal));
							}

						}
					}// end foreach triangle
				}
			}// for each model
			if (deltaNorModelList.size() > 0)
			{
				sort(deltaNorModelList.begin(), deltaNorModelList.end());// C# AP: deltaNorModelList.Sort(delegate(Tuple<double, RHVector3> t1, Tuple<Double, RHVector3> t2){return t1.First.CompareTo(t2.First);});
				return true;
			}
			return false;
		}

		private:
		static bool CheckTailPath2(	bool forhead, CubeNum StartCube, 
									int x, int y, int z, 
									CubeMatrix &cubeSpace,
									CubeData &modelData,
									RHVector3 pickStartWor,
									RHVector3 &pickDirWor,
									double normZThreshold,
									/*out*/ vector<Tuple<double, RHVector3>> &deltaNorModelList)
		 {
			//int X = StartCube.y + (x);
			if (x > cubeSpace.XNum() || x < 0)
				return false;
			//int Y = StartCube.y + (y);
			if (y > cubeSpace.YNum() || y < 0)
				 return false;
			// int Z = StartCube.z + (z);
			if (z < 0)
				 return false;

			Cube &cube = cubeSpace.GetCubeByIndex(x, y, z);

			Tuple<double, RHVector3> deltaNorModel = getPickTouchFromCube2(modelData, forhead, cube, pickStartWor, pickDirWor, normZThreshold);

			if (RHVector3() != deltaNorModel.Second())
			{
				deltaNorModelList.push_back(deltaNorModel);
				return true;
			}
			return false;
		 }
	 };
}
