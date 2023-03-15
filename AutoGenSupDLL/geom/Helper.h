#pragma once

#include <algorithm>
#include "TopoTriangle.h"
#include "TopoModel.h"
#include "Matrix4.h"
#include "LayerModel.h"

namespace XYZSupport
{	
	// Model trainge to world coordinate. From C# AP: SupportMeshsCylinder.getTriWorByMesh(). 原來AP只有傳三角形index參數, 固定將Cubedata2的model裡指定的三角形轉成世界座標.
	TopoTriangle getTriWorByMesh(int triIdx, /*in*/const TopoModel &model, Matrix4 trans);

	// From C# AP: SupportMeshsCylinder.TransformPoint()
	void TransformPoint(const Vector3 &v3, Vector3 &outv, const Matrix4 &trans);
	void TransformPoint(const Vector4 &v4, Vector3 &outv, const Matrix4 &trans);

	// from C# AP: importByteArray()
	void StlBinaryToModel(	unsigned char *stlBinary,		// model coordination	
							int binarySize, 
					/*out*/ TopoModel *pmodel);				// have to use pointer; otherwise, SupportReferenceMesh.h will fail to cmopile.

	void StlBinaryToModel2(const unsigned char *stlBinary,	// model coordination
					/*out*/ TopoModel &model);				// have to use pointer; otherwise, SupportReferenceMesh.h will fail to cmopile.

	void ResetVertexPosToBBox(const string &filename, Vector3 bboxCenter, /*out*/ TopoModel &model);
	//void ResetVertexPosToBBox(unsigned char *stlBinary, Vector3 bboxCenter);	// This function is in MyTools.h.

	/*
	[Purpose] Convert a TopoModel class into the STL binary.
	[Input] model: ToopModel class
	        buffer: a buffer for storing the result. The AP side should allocate a buffer with size (84 + triangle number * 50) for storing the meshes.
			size: the size of the buffer
	[Output] buffer: The STL binary buffer is model coordination.
	*/
	int ModelToStlBinary(/*in*/const TopoModel &model, /*in*//*out*/unsigned char *buffer, /*in*/int bufferSize);

	int ModelToStlFile(/*in*/const TopoModel &model, string filename);

	RHBoundingBox GetBoundingBox(TopoModel &model, Matrix4 trans);

	/****************************************************************************/
	unsigned int GetStlFileSize(const unsigned char *stlBinary);
	PolygonPoint GetMinLocation(unsigned char *stlBinary);

	template <class T, class U>
	int IndexOf(T container, U vlaue)
	{
		return (int)std::distance(container.begin(), std::find(container.begin(), container.end(), vlaue));
	}
	/******************************************************************************/
}