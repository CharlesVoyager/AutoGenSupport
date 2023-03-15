#include "stdafx.h"
#include "Helper.h"
#include "SupportCommon.h"

using namespace MyTools;
namespace XYZSupport
{
	// Model trainge to world coordinate. From C# AP: SupportMeshsCylinder.getTriWorByMesh()
	TopoTriangle getTriWorByMesh(int triIdx, /*in*/const TopoModel &model, Matrix4 trans)
	{
		if (triIdx > (model.triangles.Count() - 1) || triIdx < 0)
		{
			DbgMsg(__FUNCTION__"[ERROR] ===> triIdx: %d", triIdx);
			return TopoTriangle();
		}

		TopoVertex verWorArr[3]; // variable for world coordinate

		// triangles紀錄XYZ的值(vertex1~3)
		//int n1 = 3 * submesh.triangles[triIdx].vertex1;
		//int n2 = 3 * submesh.triangles[triIdx].vertex2;
		//int n3 = 3 * submesh.triangles[triIdx].vertex3;

		// glVettices紀錄個別X/Y/Z座標的值
		verWorArr[0] = TopoVertex(0, RHVector3(model.triangles._t[triIdx].vertices[0].pos));
		verWorArr[1] = TopoVertex(1, RHVector3(model.triangles._t[triIdx].vertices[1].pos));
		verWorArr[2] = TopoVertex(2, RHVector3(model.triangles._t[triIdx].vertices[2].pos));

		for (int i = 0; i < 3; i++)
		{
			Vector3 verWor;
			TransformPoint(verWorArr[i].pos.toVector4(), verWor, trans);		// 轉為實際座標 World coordination
			verWorArr[i] = TopoVertex(i, verWor);								// 此時verWor放的是實際座標
		}
		return TopoTriangle(verWorArr[0], verWorArr[1], verWorArr[2]);
	}

	// From C# AP: SupportMeshsCylinder.TransformPoint()
	void TransformPoint(const Vector3 &v3, Vector3 &outv, const Matrix4 &trans)
	{
		Vector4 v4(v3.x, v3.y, v3.z, 1);
		outv.x = trans.Column0().dot(v4);		//Vector4.Dot(Cubedata2.Trans.Column0, v4);
		outv.y = trans.Column1().dot(v4);		//Vector4.Dot(Cubedata2.Trans.Column1, v4);
		outv.z = trans.Column2().dot(v4);		//Vector4.Dot(Cubedata2.Trans.Column2, v4);
	}
	void TransformPoint(const Vector4 &v4, Vector3 &outv, const Matrix4 &trans)
	{
		outv.x = trans.Column0().dot(v4);		//Vector4.Dot(Cubedata2.Trans.Column0, v4);
		outv.y = trans.Column1().dot(v4);		//Vector4.Dot(Cubedata2.Trans.Column1, v4);
		outv.z = trans.Column2().dot(v4);		//Vector4.Dot(Cubedata2.Trans.Column2, v4);
	}

	// Reference: importSTL from C# AP.
	/*
	Test Model: Ring3.stl
	Triangles count: 270,102
	StlBinaryToModel() time spent: 1 second.
	*/
	void StlBinaryToModel(unsigned char *stlBinary,	int binarySize, /*out*/TopoModel *pmodel)	// C# AP: void importByteArray(unsigned char *stlArr, int arraySize, /*out*/TopoModel *pmodel)
	{
		unsigned char *r = stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);	// number of triangles

		//try
		//{
		for (unsigned int i = 0; i < nTri; i++)
		{
			float x, y, z;

			// normal: 12 bytes
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 normal = RHVector3(x, y, z);	// NOTE: DON'T use "RHVector3(ReadSingle(r), ReadSingle(r), ReadSingle(r))". It will cause the pointer move in wrong order!!!

			// p1: 12 bytes
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 p1 = RHVector3(x, y, z);		// NOTE: DON'T use "RHVector3(ReadSingle(r), ReadSingle(r), ReadSingle(r))". It will cause the pointer move in wrong order!!!

			// p2: 12 bytes
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 p2 = RHVector3(x, y, z);		// NOTE: DON'T use "RHVector3(ReadSingle(r), ReadSingle(r), ReadSingle(r))". It will cause the pointer move in wrong order!!!

			// p3: 12 bytes
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 p3 = RHVector3(x, y, z);		// NOTE: DON'T use "RHVector3(ReadSingle(r), ReadSingle(r), ReadSingle(r))". It will cause the pointer move in wrong order!!!
			normal.NormalizeSafe();
			pmodel->addTriangle(p1, p2, p3, normal);
			r++;	//skip two bytes
			r++;
		}
		//}
		//catch
		//{
		//	throw;
		//	////MessageBox.Show(Trans.T("M_LOAD_STL_FILE_ERROR"), Trans.T("W_LOAD_STL_FILE_ERROR"), MessageBoxButtons.OK, MessageBoxIcon.Error);
		//}
	}

	// Optimized the function StlBinaryToModel() and the excution time should be better.
	/*
		Test Model: Ring3.stl
		Triangles count: 270,102
		StlBinaryToModel2() time spent: 0.6 second.
	*/
	void StlBinaryToModel2(const unsigned char *stlBinary, /*out*/ TopoModel &model)
	{
		if (stlBinary == nullptr) return;

		unsigned char *r = (unsigned char *)stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);	// number of triangles

		model.triangles._t.clear();

		float x, y, z;
		for (unsigned int i = 0; i < nTri; i++)
		{
			r += 12;	//skip normal, 12 bytes
			
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 v1(x, y, z);
			TopoVertex p1 = model.addVertex(v1);

			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 v2(x, y, z);
			TopoVertex p2 = model.addVertex(v2);
	
			x = ReadSingle(r);
			y = ReadSingle(r);
			z = ReadSingle(r);
			RHVector3 v3(x, y, z);
			TopoVertex p3 = model.addVertex(v3);

			model.triangles.AddWithoutCheck(p1, p2, p3);
			r+=2;	//skip two bytes
		}
	}

	void ResetVertexPosToBBox(const string &filename, Vector3 bboxCenter, /*out*/ TopoModel &model)
	{
		vector<unsigned char> meshes;
		ReadAllBytes(filename, meshes);
		if (meshes.size() == 0) return;

		unsigned char *stlBinary = &meshes[0];
		unsigned char *r = stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);	// number of triangles

		model.triangles._t.clear();

		float x, y, z;
		for (unsigned int i = 0; i < nTri; i++)
		{
			r += 12;	//skip normal, 12 bytes

			x = ReadSingle(r) - bboxCenter.x;
			y = ReadSingle(r) - bboxCenter.y;
			z = ReadSingle(r) - bboxCenter.z;
			RHVector3 v1(x, y, z);
			TopoVertex p1 = model.addVertex(v1);

			x = ReadSingle(r) - bboxCenter.x;
			y = ReadSingle(r) - bboxCenter.y;
			z = ReadSingle(r) - bboxCenter.z;
			RHVector3 v2(x, y, z);
			TopoVertex p2 = model.addVertex(v2);

			x = ReadSingle(r) - bboxCenter.x;
			y = ReadSingle(r) - bboxCenter.y;
			z = ReadSingle(r) - bboxCenter.z;
			RHVector3 v3(x, y, z);
			TopoVertex p3 = model.addVertex(v3);

			model.triangles.AddWithoutCheck(p1, p2, p3);
			r += 2;	//skip two bytes
		}
	}

	int ModelToStlBinary(/*in*/ const TopoModel &model, /*out*/ unsigned char *buffer, int bufferSize)
	{
		unsigned int trianglesCount = model.triangles.Count();

		if (trianglesCount == 0) return -1;

		//UINT8[80] – Header										// 80 bytes
		memset(buffer, 0, 80);
		
		unsigned char *w = buffer + 80;

		//UINT32 – Number of triangles								// 4 bytes
		WriteUInt32(w, trianglesCount);

		for (unsigned int j = 0; j < trianglesCount; j++)
		{
			float f = 0;

			//REAL32[3] – Normal vector								// 12 bytes	
			f = (float)model.triangles._t[j].normal.x;
			WriteSingle(w, f);

			f = (float)model.triangles._t[j].normal.y;
			WriteSingle(w, f);

			f = (float)model.triangles._t[j].normal.z;
			WriteSingle(w, f);

			//REAL32[3] – Vertex 1									// 12 bytes
			//REAL32[3] – Vertex 2									// 12 bytes
			//REAL32[3] – Vertex 3									// 12 bytes
			for (unsigned int k = 0; k < 3; k++)
			{
				f = (float)model.triangles._t[j].vertices[k].pos.x;
				WriteSingle(w, f);

				f = (float)model.triangles._t[j].vertices[k].pos.y;
				WriteSingle(w, f);

				f = (float)model.triangles._t[j].vertices[k].pos.z;
				WriteSingle(w, f);
			}

			//UINT16 – Attribute byte count							// 2 bytes
			w[0] = 0x00; w++;
			w[0] = 0x00; w++;
		}
		return 0;
	}

	int ModelToStlFile(/*in*/ const TopoModel &model, string filename)
	{
		int size = model.Size();
		vector<unsigned char> vecBuffer(size, 0);
		ModelToStlBinary(model, &vecBuffer[0], size);
		BinFile binFile(filename);
		binFile.Write(vecBuffer);
		binFile.Close();
		return 0;
	}

	
	// Note 1. position.z: UI's Z mean bottom of the model, while the position argument's Z means center of the model.
	//                 EX: Model ===> Cube (Dimension: 10mm x 10mm x 10mm)
	//                     UI position: 64, 64, 20 ===> for ToTransMatrix's position argumenet should be 64, 64, 25.
	//      2. This function is equivalent to C# AP PrintModel.UpdateMatrix().
	//		3. The XYZware C# app computes transformation matrix by changing only one rotate coordinate - X, Y, or Z at one time. Therefore, the C# app requires the
	//         variables "preRX2, preRY2, preRZ2" to record the previous transformation's rotate state.
	//         The Cpp function does NOT use "preRX2, preRY2, preRZ2" but the method as below. It will have the same result as C# application.
	//       
	Matrix4 ToTransAndRotateMatrix(Coord3D position, Coord3D scale, Coord3D rotation, /*out*/ Matrix4 &rotateMatrix)
	{
		Matrix4 trans;
		rotateMatrix.identity();

		// rotate x
		float x = rotation.x;
		float y = 0;
		float z = 0;

		Matrix4 transl = Matrix4::CreateTranslation(position.x, position.y, position.z);	

		Matrix4 identity;
		Matrix4 scaleM4 = identity.scale(scale.x != 0 ? scale.x : 1, scale.y != 0 ? scale.y : 1, scale.z != 0 ? scale.z : 1);
		identity.identity();

		Matrix4 rotx = identity.rotateX(x);	//unit: degree
		rotx.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationX().
		identity.identity();
		trans = rotx;

		Matrix4 roty = identity.rotateY(y);
		roty.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationY().
		identity.identity();
		trans *= roty;

		Matrix4 rotz = identity.rotateZ(z);
		rotz.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationZ().
		identity.identity();
		trans *= rotz;

		rotateMatrix = trans * rotateMatrix;
		Matrix4 cT = rotateMatrix;
		cT.transpose();
		trans = scaleM4 * cT;
		trans = trans * transl;

		// rotate y
		x = 0;
		y = rotation.y;
		z = 0;

		transl = Matrix4::CreateTranslation(position.x, position.y, position.z);

		identity.identity();
		scaleM4 = identity.scale(scale.x != 0 ? scale.x : 1, scale.y != 0 ? scale.y : 1, scale.z != 0 ? scale.z : 1);
		identity.identity();

		rotx = identity.rotateX(x);			//unit: degree
		rotx.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationX().
		identity.identity();
		trans = rotx;

		roty = identity.rotateY(y);
		roty.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationY().
		identity.identity();
		trans *= roty;

		rotz = identity.rotateZ(z);
		rotz.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationZ().
		identity.identity();
		trans *= rotz;

		rotateMatrix = trans * rotateMatrix;
		cT = rotateMatrix;
		cT.transpose();
		trans = scaleM4 * cT;
		trans = trans * transl;

		// rotate z
		x = 0;
		y = 0;
		z = rotation.z;

		transl = Matrix4::CreateTranslation(position.x, position.y, position.z);

		identity.identity();
		scaleM4 = identity.scale(scale.x != 0 ? scale.x : 1, scale.y != 0 ? scale.y : 1, scale.z != 0 ? scale.z : 1);
		identity.identity();

		rotx = identity.rotateX(x);	//unit: degree
		rotx.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationX().
		identity.identity();
		trans = rotx;

		roty = identity.rotateY(y);
		roty.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationY().
		identity.identity();
		trans *= roty;

		rotz = identity.rotateZ(z);
		rotz.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationZ().
		identity.identity();
		trans *= rotz;

		rotateMatrix = trans * rotateMatrix;
		cT = rotateMatrix;
		cT.transpose();
		trans = scaleM4 * cT;
		trans = trans * transl;
		return trans;
	}

	RHBoundingBox GetBoundingBox(TopoModel &model, Matrix4 trans)
	{
		RHBoundingBox bbox;
		
		for (auto &v : model.vertices._v)
		{
			float x, y, z;
			Vector4 *pv4 = v.pos.asVector4();

			x = trans.Column0().dot(*pv4);
			y = trans.Column1().dot(*pv4);
			z = trans.Column2().dot(*pv4);
			bbox.Add(RHVector3(x, y, z));
			delete pv4;
		}
		return bbox;
	}

	vector<string> StringSplit(string strWhole, const char *separator)
	{
		vector<string> vecStr;
		string strSingle;
		string printerName;

		std::size_t found;
		do {
			found = strWhole.find_first_of(separator);
			strSingle = strWhole.substr(0, found);
			strWhole = strWhole.substr(found + 1);

			if (!strSingle.empty())
				vecStr.push_back(strSingle);

		} while (!strWhole.empty() && found != std::string::npos);
		return vecStr;
	}

	unsigned int GetTrianglesCount(const unsigned char *stlBinary) {
		const unsigned char *r = stlBinary + 80;	// pointer to start parse address

		unsigned int output;

		*((unsigned char*)(&output) + 0) = *r;
		*((unsigned char*)(&output) + 1) = *(r + 1);
		*((unsigned char*)(&output) + 2) = *(r + 2);
		*((unsigned char*)(&output) + 3) = *(r + 3);

		return output;				// number of triangles
	}

	unsigned int GetStlFileSize(const unsigned char *stlBinary) {
		return GetTrianglesCount(stlBinary) * 50 + 84;
	}

	PolygonPoint GetMinLocation(unsigned char *stlBinary)
	{
		PolygonPoint xyMinLaction;
		xyMinLaction.x = std::numeric_limits<double>::max();
		xyMinLaction.y = std::numeric_limits<double>::max();

		unsigned char *r = stlBinary + 80;	// pointer to start parse address
		unsigned int nTri = ReadUInt32(r);	// number of triangles

		float x, y, z;
		for (unsigned int i = 0; i < nTri; i++)
		{
			r += 12;	//skip normal 12 bytes

			for (int j = 0; j < 3; j++)	// 3 vertices
			{		
				x = ReadSingle(r);
				y = ReadSingle(r);
				z = ReadSingle(r);
				if (xyMinLaction.x > x) xyMinLaction.x = x;
				if (xyMinLaction.y > y) xyMinLaction.y = y;
			}
			r += 2;		//skip two bytes
		}
		return xyMinLaction;
	}
}