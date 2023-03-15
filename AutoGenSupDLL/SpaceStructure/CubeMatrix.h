#pragma once

#include "Cube.h"
#include "Helper.h"

namespace XYZSupport
{
	class CubeMatrix	// C#: public class CubeMatrix : IEnumerable
	{
	public:
		enum STATUS { Idle, Busy, UserAbort, Done };
		enum COMMAND { None, Abort };

		STATUS Status;
		COMMAND Command;

		 //delegate void ProcessRateEventHandler(object sender, ProcessRateEventArgs e);
		 //event ProcessRateEventHandler ProcessRateUpdate;
	
	private:
		vector<Cube> _cubes;
		int _xNum;
		int _yNum;
		int _zNum;

	public:
		RHVector3 MinPoint;
		RHVector3 MaxPoint;
		double Span;
		int XNum() const { return _xNum; }
		int YNum() const { return _yNum; }
		int ZNum() const { return _zNum; }
		int Count() const { return _xNum * _yNum * _zNum; }
		bool IsEmpty() const { return Count() == 0 ? true:false; } 

	private:
		int getIndex(int x, int y, int z)
		{
			return z * _xNum * _yNum + x * _yNum + y;
		}

		void serialIndexToThreeIndex(int serialIndex, int &x, int &y, int &z)
		{
			if (serialIndex == 0)
			{
				x = 0;
				y = 0;
				z = 0;
			}
			else
			{
				z = serialIndex / (_xNum*_yNum);
				int remainZ = serialIndex % (_xNum*_yNum);
				if (remainZ == 0)
				{
					z--;
					x = _xNum - 1;
					y = _yNum - 1;
				}
				else
				{
					x = remainZ / _yNum;
					if ((remainZ % _yNum) == 0)
					{
						x--;
						y = _yNum - 1;
					}
					else
						y = remainZ % _yNum;
				}
			}
		}

	public:
		string ToString() const
		{
			return "MinPoint: " + MinPoint.ToString() + " MaxPoint: " + MaxPoint.ToString() + " Count: " + to_string(Count()) + " (" + to_string(_xNum) + " " + to_string(_yNum) + " " + to_string(_zNum) + ")";
		}
		vector<Cube>& ToArray() { return _cubes; }

		//Cube this[int x, int y, int z]
		//{
		//	get { return _cubes[getIndex(x, y, z)]; }
		//	private set { _cubes[getIndex(x, y, z)] = value; }
		//}
		Cube& GetCubeByIndex(int x, int y, int z) { return _cubes[getIndex(x, y, z)]; }
		Cube& GetCubeByIndex(int index) { return _cubes[index]; }
		void SetCubeByIndex(int x, int y, int z, Cube value) { _cubes[getIndex(x, y, z)] = value; }
	
		CubeMatrix()
		{
			_xNum = _yNum = _zNum = 0;
			Status = STATUS::Idle;
			Command = COMMAND::None;
		}

		/// <summary>
		/// generate empty cube matrix
		/// </summary>
		/// <param name="minPoint">minimum point of bounding box of the specified space</param>
		/// <param name="maxPoint">maximum point of bounding box of the specified space</param>
		/// <param name="span">step unit of cube matrix</param>
		CubeMatrix(RHVector3 minPoint, RHVector3 maxPoint, double span)
		{
			MinPoint = minPoint;
			MaxPoint = maxPoint;
			Span = span;

			_xNum = (int)((MaxPoint.x - MinPoint.x) / span) + 1;
			_yNum = (int)((MaxPoint.y - MinPoint.y) / span) + 1;
			_zNum = (int)((MaxPoint.z - MinPoint.z) / span) + 1;

			_cubes.resize(_xNum * _yNum * _zNum);
			// From min. z-pos to Max. z-pos by increment (span/10)mm
			//for (Double zStep = MinPoint.z; zStep <= MaxPoint.z; zStep += Span)
			double zStep, yStep, xStep;
			zStep = MinPoint.z;
			for (int k = 0; k < _zNum; k++)
			{
				//Debug.Assert(zStep <= MaxPoint.z);
				xStep = MinPoint.x;
				for (int i = 0; i < _xNum; i++)
				{
					//Debug.Assert(xStep <= MaxPoint.x);
					yStep = MinPoint.y;
					for (int j = 0; j < _yNum; j++)
					//for (Double yStep = MinPoint.y; yStep <= MaxPoint.y; yStep += Span)
					{
						//Debug.Assert(yStep <= MaxPoint.y);
						SetCubeByIndex(i, j, k, Cube(xStep, yStep, zStep, Span));		// C# AP: this[i, j, k] = new Cube(xStep, yStep, zStep, Span);    // layer.cubeList紀錄在同一個X位置下,Y的起始與結束位置
						yStep += Span;
					}
					xStep += Span;
				}
				zStep += Span;
			}
			Status = STATUS::Done;
		}

		void Clean()
		{
			if (_cubes.size()>0)
			{
				_cubes.clear();
			}
			_xNum = _yNum = _zNum = 0;
		}

		/// <summary>
		/// Set triangle and its lowest vertex to cube matrix
		/// </summary>
		/// <param name="pos"></param>
		/// <param name="posIdx"></param>
		/// <param name="triIdx"></param>
		void SetTriangle(RHVector3 pos, int posIdx, int triIdx, CubeMatrix &CubetoSet)
		{
			int xIdx = (int)((pos.x - MinPoint.x) / Span);
			int yIdx = (int)((pos.y - MinPoint.y) / Span);
			int zIdx = (int)((pos.z - MinPoint.z) / Span);
			if (zIdx < ZNum() && xIdx < XNum() && yIdx < YNum())
			{
				// record lowest Z-pos in verIdxTriIdxList, i: index of triangle vertex, triIdx: triangle index 
				CubetoSet.GetCubeByIndex(xIdx, yIdx, zIdx).verIdxTriIdxList.push_back(Tuple<int, int>(posIdx, triIdx));
				//CubeMat[xIdx, yIdx, zIdx].verIdxTriIdxList.Add(new Tuple<int, int>(posIdx, triIdx));//newsup
			}
		}

#if 0	//un-used functions
		/// <summary>
		/// Build cube matrix. Cubes store lowest vertex of triangle.
		/// </summary>
		/// <param name="zAxisNormalThresh">threshold of z-axis component of normal vector</param>
		/// <param name="submesh">triangle mesh</param>
		int Build(PrintModel model, Submesh submesh, double zAxisNormalThresh, CubeMatrix CubetoBuild);	// Charles: Never used. Note: The first argument is PrintModel.

		/// <summary>
		/// Build cube matrix. Cubes store contained triangles.
		/// </summary>
		/// <param name="zAxisNormalThresh">threshold of z-axis component of normal vector</param>
		/// <param name="submesh">triangle mesh</param>
		int BuildForCD(PrintModel model, Submesh submesh, double zAxisNormalThresh);	// Charles: Never used. Note: The first argument is PrintModel.
#endif

		int BuildForCD(const TopoModel &model, vector<RHVector3> vtxPosWorldCor, double zAxisNormalThresh)
		{
			int totalTriangle;
			int processing_step;

			if (0 == model.triangles.Count()) return -1;

			Status = STATUS::Busy;

			// for each triangle, store information of the lower z position of vertex in cube of layer
			//
			int yEleNum = (int)((MaxPoint.y - MinPoint.y) / Span) + 1; // y element number of one column of cube
			//model.UpdateMatrix();

			totalTriangle = model.triangles.Count();
			processing_step = (totalTriangle >= 32) ? (totalTriangle / 32) : 1;
			for (int triIdx = 0; triIdx < totalTriangle; triIdx++)    // total triangles
			{
				double lowsetZ = std::numeric_limits<double>::infinity(); // lowest z
				RHVector3 pos;
				int vId;
				int vtxIdxLowZ = 0; // vertex index of lowest z
				int triVId[3];
				RHBoundingBox triBBox;

				// get the lowest vertex in the triangle
				for (int i = 0; i < 3; i++)
				{
					vId = model.triangles._t[triIdx].vertices[i].id;
					pos = vtxPosWorldCor[vId];
					if (pos.z < lowsetZ)
					{
						lowsetZ = pos.z;
						vtxIdxLowZ = i;
					}
					triVId[i] = vId;
					triBBox.Add(pos);
				}

				// upperBound: z-axis normal vector, support will be added when normal vector large than -1.01 (all triangles)
				//if ((model.triNormalWorldCor[triIdx].z / model.triNormalWorldCor[triIdx].Length) > zAxisNormalThresh) continue;

				// record triangle index to every cube which is intersection with
				int xIdxS, yIdxS, zIdxS; // start cube index of triangle bounding box
				int xIdxE, yIdxE, zIdxE; // end cube index of triangle bounding box
				xIdxS = (int)((triBBox.xMin() - MinPoint.x) / Span);
				yIdxS = (int)((triBBox.yMin() - MinPoint.y) / Span);
				zIdxS = (int)((triBBox.zMin() - MinPoint.z) / Span);
				xIdxE = (int)((triBBox.xMax() - MinPoint.x) / Span);
				yIdxE = (int)((triBBox.yMax() - MinPoint.y) / Span);
				zIdxE = (int)((triBBox.zMax() - MinPoint.z) / Span);
				for (int k = zIdxS; k <= zIdxE; k++)
				{
					for (int j = yIdxS; j <= yIdxE; j++)
					{
						for (int i = xIdxS; i <= xIdxE; i++)
						{
							if (k < ZNum() && i < XNum() && j < YNum())
							{
								// record lowest Z-pos in verIdxTriIdxList, i: index of triangle vertex, triIdx: triangle index 
								GetCubeByIndex(i, j, k).verIdxTriIdxList.push_back(Tuple<int, int>(vtxIdxLowZ, triIdx));
								//CubeMat[i, j, k].verIdxTriIdxList.Add(new Tuple<int, int>(vtxIdxLowZ, triIdx));
							}
						}
					}
				}

				//if (ProcessRateUpdate != null && triIdx % processing_step == 0)
				//{
				//	ProcessRateUpdate(this, new ProcessRateEventArgs((uint)(((double)triIdx / totalTriangle) * 50.0 + 50.0)));
				//}

				if (Command == COMMAND::Abort)
					break;
			}
			if (Command == COMMAND::Abort)
			{
				Status = STATUS::UserAbort;
			}
			else
			{
				Status = STATUS::Done;
			}
			Command = COMMAND::None;
			return 0;
		}

		void BuildForNewsup(const TopoModel &model, vector<RHVector3> vtxPosWorldCor, double zAxisNormalThresh)//newsup
		{
			int totalTriangle;
			int processing_step;

			if (0 != model.triangles.Count())
			{
				// for each triangle, store information of the lower z position of vertex in cube of layer
				//
				int yEleNum = (int)((MaxPoint.y - MinPoint.y) / Span) + 1;	// y element number of one column of cube
																			//model.UpdateMatrix();
				totalTriangle = model.triangles.Count();
				processing_step = (totalTriangle >= 32) ? (totalTriangle / 32) : 1;
				for (int triIdx = 0; triIdx < totalTriangle; triIdx++)		// total triangles
				{
					double lowsetZ = std::numeric_limits<double>::infinity(); // lowest z
					RHVector3 pos;
					int vId;
					int vtxIdxLowZ = 0; // vertex index of lowest z
					int triVId[3];
					RHBoundingBox triBBox;

					// get the lowest vertex in the triangle
					for (int i = 0; i < 3; i++)
					{
						vId = model.triangles._t[triIdx].vertices[i].id;
						pos = vtxPosWorldCor[vId];
						if (pos.z < lowsetZ)
						{
							lowsetZ = pos.z;
							vtxIdxLowZ = i;
						}
						triVId[i] = vId;
						triBBox.Add(pos);
					}

					int xIdxS, yIdxS, zIdxS; // start cube index of triangle bounding box
					int xIdxE, yIdxE, zIdxE; // end cube index of triangle bounding box
					xIdxS = (int)((triBBox.xMin() - MinPoint.x) / Span);
					yIdxS = (int)((triBBox.yMin() - MinPoint.y) / Span);
					zIdxS = (int)((triBBox.zMin() - MinPoint.z) / Span);
					xIdxE = (int)((triBBox.xMax() - MinPoint.x) / Span);
					yIdxE = (int)((triBBox.yMax() - MinPoint.y) / Span);
					zIdxE = (int)((triBBox.zMax() - MinPoint.z) / Span);

					for (int k = zIdxS; k <= zIdxE; k++)
					{
						for (int j = yIdxS; j <= yIdxE; j++)
						{
							for (int i = xIdxS; i <= xIdxE; i++)
							{
								if (k < ZNum() && i < XNum() && j < YNum())
								{
									// record lowest Z-pos in verIdxTriIdxList, i: index of triangle vertex, triIdx: triangle index 
									GetCubeByIndex(i, j, k).verIdxTriIdxList.push_back( Tuple<int, int>(vtxIdxLowZ, triIdx));
								}
							}
						}
					}

				}
			}
		}
	
		void CheckCubeEmptyandObjectBottom(bool atBBObjCor, TopoModel &model, Matrix4 trans)	//  equivelent to C# AP:  public void CheckCubeEmptyandObjectBottom(SupportCylinder.SupportMeshsCylinder models, bool atBBObjCor)//newsup
		{
			if (Count() > 0)
			{
				for(int i=0; i < Count(); i++) //foreach(Cube cube in models.cubeSpaceforNewsup.ToArray())
				{
					Cube& cube = GetCubeByIndex(i);	//NOTE: GetCubeByIndex() is returning reference. DON'T use "Cube cube = GetCubeByIndex(i)" without "&"!!!
					RHBoundingBox cubeBox;
					cubeBox.Add(cube.XLowBound(), cube.YLowBound(), cube.ZLowBound());
					cubeBox.Add(cube.XLowBound() + cube.Span(), cube.YLowBound() + cube.Span(), cube.ZLowBound() + cube.Span());

					if (cube.verIdxTriIdxList.size() > 0)
					{
						for(int j=0; j < cube.verIdxTriIdxList.size(); j++)	//foreach(Tuple<int, int> verIdxTriIdx in cube.verIdxTriIdxList)
						{
							TopoTriangle triWor = getTriWorByMesh(cube.verIdxTriIdxList[j].Second(), model, trans);
							bool intersect = true;

							intersect = BBOverlapTriWor(cubeBox, Matrix4(), triWor);
							if (intersect)
							{
								cube.EmptyCube = false;
								cube.ObjectBottomEdge = false;
								break;
							}
						}
					}
				}
			}
		}

		bool BBOverlapTriWor(RHBoundingBox box, Matrix4 toWorMx, TopoTriangle modelTri)//newsup
		{
			TopoVertex verWor[3];
			for (int i = 0; i < 3; i++)
			{
				Vector4 *pverV4Wor = modelTri.vertices[i].pos.asVector4();	// C# AP: Vector4.Transform(modelTri.vertices[i].pos.asVector4(), toWorMx); NOTE NOTE NOE: Because toWorMx is identity mtrix, why do we need to multiply identity matrix???
				//Debug.WriteLine("Model Vertex in object space is " + modelVertex);
				verWor[i] = TopoVertex(i, RHVector3(pverV4Wor->x, pverV4Wor->y, pverV4Wor->z));
				delete pverV4Wor;
			}
			TopoTriangle modelTriWor = TopoTriangle(verWor[0], verWor[1], verWor[2]);
			if (box.overlapTri(modelTriWor))// At world coordinate
				return true;
			else
				return false;
		}

		//public void UpdateConeSupportCubeInfo(PrintModel models);	//never used
	};
}