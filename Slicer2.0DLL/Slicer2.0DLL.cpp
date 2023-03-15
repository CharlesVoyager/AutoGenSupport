// Slicer2.0DLL.cpp : 定義 DLL 應用程式的匯出函式。
//

#include "stdafx.h"
#include "Slicer2.0DLL.h"
//#include <iostream>
//#include <stdio.h>
//#include <cmath>//#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#pragma warning( disable : 4996 )

using namespace std;
using namespace cura;

// 這是匯出變數的範例

SLICER20DLL_API int nSlicer20DLL=0;

bool loadMeshSTL_ascii(Mesh* mesh, const TCHAR* filename, const FMatrix3x3& matrix)
{
	FILE* stlFile;
    
#ifdef _WINDOWS
	_wfopen_s(&stlFile, filename, L"rb");
#elif __APPLE__
    wstring wstr(filename);
    string strFilename(wstr.begin(), wstr.end());
    stlFile = fopen(strFilename.c_str(), "rb");
#endif
    if (stlFile == NULL)
        return false;
    
	TCHAR line[1024];
	FPoint3 vertex;
	Point3 v0(0, 0, 0), v1(0, 0, 0), v2(0, 0, 0);
	int count = 0, facecount = 1;
	// size 直接使用 1024
	while (fgetws(line, 1024, stlFile))
	{
		if (swscanf(line, L" vertex %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3)
		{
			count++;
			switch (count)
			{
			case 1:
				//v0 = matrix.apply(vertex);
				//printf("face %d \n", facecount);
				//printf("v0 : x = %f y = %f z = %f\n", vertex.x, vertex.y, vertex.z);
				//v0 = Point3(MM2INT(vertex.x), MM2INT(vertex.y), MM2INT(vertex.z)); 
				v0 = matrix.apply(vertex);
				//printf("v0 : x = %d y = %d z = %d\n", v0.x, v0.y, v0.z);
				break;
			case 2:
				//v1 = matrix.apply(vertex);
				//printf("v1 : x = %f y = %f z = %f\n", vertex.x, vertex.y, vertex.z);
				//v1 = Point3(MM2INT(vertex.x), MM2INT(vertex.y), MM2INT(vertex.z));
				v1 = matrix.apply(vertex);
				//printf("v1 : x = %d y = %d z = %d\n", v1.x, v1.y, v1.z);
				break;
			case 3:
				//v2 = matrix.apply(vertex);
				//mesh->addFace(v0, v1, v2);				
				//printf("v2 : x = %f y = %f z = %f\n", vertex.x, vertex.y, vertex.z);
				//v2 = Point3(MM2INT(vertex.x), MM2INT(vertex.y), MM2INT(vertex.z));
				v2 = matrix.apply(vertex);
				//printf("v2 : x = %d y = %d z = %d\n", v2.x, v2.y, v2.z);
				mesh->addFace(v0, v1, v2);
				facecount++;
				count = 0;
				break;
			}
		}
	}
	fclose(stlFile);
	mesh->connectFace();
	return true;
}

bool loadMeshSTL_binary(Mesh* mesh, const TCHAR* filename, const FMatrix3x3& matrix)
{
	FILE* stlFile;
    
#ifdef _WINDOWS
    _wfopen_s(&stlFile, filename, L"rb");
#elif __APPLE__
    wstring wstr(filename);
    string strFilename(wstr.begin(), wstr.end());
    stlFile = fopen(strFilename.c_str(), "rb");
#endif
    
    
	fseek(stlFile, 0L, SEEK_END);
	long long file_size = ftell(stlFile); //The file size is the position of the cursor after seeking to the end.
	rewind(stlFile); //Seek back to start.
	size_t face_count = (file_size - 80 - sizeof(uint32_t)) / 50; //Subtract the size of the header. Every face uses exactly 50 bytes.

	TCHAR buffer[80];
	//Skip the header
	if (fread(buffer, 80, 1, stlFile) != 1)
	{
		fclose(stlFile);
		return false;
	}

	uint32_t reported_face_count;
	//Read the face count. We'll use it as a sort of redundancy code to check for file corruption.
	if (fread(&reported_face_count, sizeof(uint32_t), 1, stlFile) != 1)
	{
		fclose(stlFile);
		return false;
	}
	if (reported_face_count != face_count)
	{
		printf("Face count reported by file (%s) is not equal to actual face count (%s). File could be corrupt!\n", std::to_string(reported_face_count).c_str(), std::to_string(face_count).c_str());
	}

	//For each face read:
	//float(x,y,z) = normal, float(X,Y,Z)*3 = vertexes, uint16_t = flags
	// Every Face is 50 Bytes: Normal(3*float), Vertices(9*float), 2 Bytes Spacer
	mesh->faces.reserve(face_count);
	mesh->vertices.reserve(face_count);
	//printf("face count : %d\n", face_count);
	for (unsigned int i = 0; i < face_count; i++)
	{
		if (fread(buffer, 50, 1, stlFile) != 1)
		{
			fclose(stlFile);
			return false;
		}
		float *v = ((float*)buffer) + 3;
		Point3 v0 = matrix.apply(FPoint3(v[0], v[1], v[2]));
		Point3 v1 = matrix.apply(FPoint3(v[3], v[4], v[5]));
		Point3 v2 = matrix.apply(FPoint3(v[6], v[7], v[8]));
		mesh->addFace(v0, v1, v2);
	}
	fclose(stlFile);
	mesh->connectFace();
	return true;
}

bool loadMeshSTL(Mesh* mesh, LPCWSTR filename, const FMatrix3x3& matrix)
{
	FILE* stlFile;    
#ifdef _WINDOWS
    _wfopen_s(&stlFile, filename, L"rb");
#elif __APPLE__
    wstring wstr(filename);
    string strFilename(wstr.begin(), wstr.end());
    stlFile = fopen(strFilename.c_str(), "rb");
#endif
    
	if (stlFile == nullptr)
	{
#ifdef _WINDOWS
		MessageBoxW(0, L"failed to open STL file!", L"_wfopen_s", MB_OK);
#elif __APPLE__
        
#endif
		return false;
	}

	//Skip any whitespace at the beginning of the file.
	unsigned long long num_whitespace = 0; //Number of whitespace characters.
	//unsigned char whitespace;
	TCHAR whitespace;
	if (fread(&whitespace, 1, 1, stlFile) != 1)
	{

		fclose(stlFile);
		return false;
	}

	while(iswspace(whitespace))
	{
		num_whitespace++;
		if (fread(&whitespace, 1, 1, stlFile) != 1)
		{
			fclose(stlFile);
			return false;
		}
	}

	fseek(stlFile, num_whitespace, SEEK_SET); //Seek to the place after all whitespace (we may have just read too far).
	TCHAR buffer[6];
	if (fread(buffer, 5, 1, stlFile) != 1)
	{
		fclose(stlFile);
		return false;
	}
	fclose(stlFile);

	buffer[5] = L'\0';
	if (!wcscmp(buffer, L"solid"))
	{
		bool load_success = loadMeshSTL_ascii(mesh, filename, matrix);
		if (!load_success)
			return false;
		// This logic is used to handle the case where the file starts with
		// "solid" but is a binary file.
		if (mesh->faces.size() < 1)
		{
			mesh->clear();
			return loadMeshSTL_binary(mesh, filename, matrix);
		}
		return true;
	}
	return loadMeshSTL_binary(mesh, filename, matrix);
}

void WriteMeshToFile(const Mesh* mesh)
{
    ofstream meshFile;
    meshFile.open("mesh.txt");
    std::vector<MeshFace> faces = mesh->faces;
    std::vector<MeshVertex> vertices = mesh->vertices;
	printf("face %lu\n", faces.size());
	//std::cout << "mesh size : " << faces.size() << std::endl;
    //std::for_each(begin(faces), end(faces), [&meshFile](MeshFace face){
    //    meshFile << face.vertex_index[0] << endl;
    //});

    for (auto face : faces)
    {
        for (auto vertex_index : face.vertex_index)
        {
            Point3 p = vertices[vertex_index].p;
            //meshFile << "x : " << p.x  << "y : " << p.y << "z : " << p.z << endl;
			//meshFile << p.x << ", " << p.y << ", " << p.z << "," << endl;
			meshFile << INT2MM2(p.x) << ", " << INT2MM2(p.y) << ", " << INT2MM2(p.z) << "," << endl;
        }
    }
    meshFile.close();
}

SLICER20DLL_API int fnSlicer20DLL(LPCWSTR fileName, double layerHeight)
{
	std::vector<Point> srcPoints, disPoints;

	CSlicer20DLL slicer20DLL = CSlicer20DLL(fileName, layerHeight, srcPoints, disPoints, nullptr);
	return 0;
}

SLICER20DLL_API int fnSlicer20DLL2(LPCWSTR fileName, double layerHeight, PROGESS_CALLBACK callback)
{
	//Mesh mesh = Mesh();
	//FMatrix3x3 matrix;
	//if (!loadMeshSTL(&mesh, fileName, matrix))
	//{
	//	printf("failed to load STL file!!!\n");
	//	return 1;
	//}

	//int initial_layer_thickness = MM2INT(layerHeight);
	//int layer_thickness = MM2INT(layerHeight);MM2INT

	//int initial_slice_z = initial_layer_thickness - layer_thickness / 2;
	//int slice_layer_count = (mesh.max().z - initial_slice_z) / layer_thickness + 1;

	std::vector<Point> srcPoints, disPoints;

	CSlicer20DLL slicer20DLL = CSlicer20DLL(fileName, layerHeight, srcPoints, disPoints,  callback);
	return 0;
}

SLICER20DLL_API int fnSlicer20DLL_Calibrate(LPCWSTR fileName, double layerHeight, double src_x0, double src_y0, double src_x1, double src_y1, double src_x2, double src_y2, double src_x3, double src_y3, double dis_x0, double dis_y0, double dis_x1, double dis_y1, double dis_x2, double dis_y2, double dis_x3, double dis_y3)
{
	//Mesh mesh = Mesh();
	//FMatrix3x3 matrix;
	//if (!loadMeshSTL(&mesh, fileName, matrix))
	//{
	//	printf("failed to load STL file!!!\n");
	//	return 1;
	//}

	//int initial_layer_thickness = MM2INT(layerHeight);
	//int layer_thickness = MM2INT(layerHeight);
	//int initial_slice_z = initial_layer_thickness - layer_thickness / 2;
	//int slice_layer_count = (mesh.max().z - initial_slice_z) / layer_thickness + 1;
	
	std::vector<Point> srcPoints(4, Point(0, 0)), disPoints(4, Point(0, 0));
	srcPoints[0] = Point(MM2INT(src_x0), MM2INT(src_y0));
	srcPoints[1] = Point(MM2INT(src_x1), MM2INT(src_y1));
	srcPoints[2] = Point(MM2INT(src_x2), MM2INT(src_y2));
	srcPoints[3] = Point(MM2INT(src_x3), MM2INT(src_y3));
	disPoints[0] = Point(MM2INT(dis_x0), MM2INT(dis_y0));
	disPoints[1] = Point(MM2INT(dis_x1), MM2INT(dis_y1));
	disPoints[2] = Point(MM2INT(dis_x2), MM2INT(dis_y2));
	disPoints[3] = Point(MM2INT(dis_x3), MM2INT(dis_y3));

	//std::vector<Point> bbPoint;
	//bbPoint.reserve(4);
	//int bb_Height, bb_Width;
	//Slicer* slicer = new Slicer(&mesh, initial_slice_z, layer_thickness, slice_layer_count, fileName, srcPoints, disPoints);
	//delete slicer;
	//return 0;
	//std::cout << "test" << std::endl;
	CSlicer20DLL slicer20DLL = CSlicer20DLL(fileName, layerHeight, srcPoints, disPoints, NULL);
	return 0;
}

// 這是已匯出的類別建構函式。
// 請參閱 Slicer2.0DLL.h 中的類別定義
//CSlicer20DLL::CSlicer20DLL(LPCWSTR fileName, double layerHeight)
//{
//	Mesh mesh = Mesh();
//	FMatrix3x3 matrix;
//	if (!loadMeshSTL(&mesh, fileName, matrix))
//	{
//		printf("failed to load STL file!!!\n");
//	}
//
//	int initial_layer_thickness = MM2INT(layerHeight);
//	int layer_thickness = MM2INT(layerHeight);
//	int initial_slice_z = initial_layer_thickness - layer_thickness / 2;
//	int slice_layer_count = (mesh.max().z - initial_slice_z) / layer_thickness + 1;
//
//	std::vector<Point> srcPoints, disPoints;
//	Slicer* slicer = new Slicer(&mesh, initial_slice_z, layer_thickness, slice_layer_count, fileName, srcPoints, disPoints);
//	delete slicer;
//	return;
//}

CSlicer20DLL::CSlicer20DLL(LPCWSTR fileName, double layerHeight, std::vector<Point> srcPoints, std::vector<Point> disPoints, PROGESS_CALLBACK callback)
{
	//printf("layerHeight = %f \n", layerHeight);

	//printf("fileName = %S \n", fileName);
	//MessageBoxW(0, fileName, L"MessageBox caption", MB_OK);
	Mesh mesh = Mesh();
	FMatrix3x3 matrix;
	if (!loadMeshSTL(&mesh, fileName, matrix))
	{
#ifdef _WINDOWS
		MessageBoxW(0, L"failed to load STL file!", L"loadMeshSTL", MB_OK);
#elif __APPLE__
        
#endif
		printf("failed to load STL file!!!\n");
		return;
	}

    //WriteMeshToFile(&mesh);

	//int initial_layer_thickness = MM2INT(layerHeight);
	//int layer_thickness = MM2INT(layerHeight);
	int initial_layer_thickness = MM2_2INT(layerHeight);
	int layer_thickness = MM2_2INT(layerHeight);
	 
	int initial_slice_z = initial_layer_thickness - layer_thickness / 2;
	int slice_layer_count = (mesh.max().z - initial_slice_z) / layer_thickness + 1;

	Slicer* slicer = new Slicer(&mesh, initial_slice_z, layer_thickness, slice_layer_count, fileName, srcPoints, disPoints, callback);
	delete slicer;
	return;
}
