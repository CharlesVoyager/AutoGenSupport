// slicertest.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include "slicertest.h"
#include "mesh.h"
#include "intpoint.h"
#include "floatpoint.h"
#include <fstream>  
#include <iostream>  
#include <string>  
#include <stdint.h>
#include <cmath>
//#define _CRT_SECURE_NO_WARNINGS
//#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#pragma warning( disable : 4996 )
using namespace std;


bool loadMeshSTL_ascii(Mesh* mesh, const TCHAR* filename, const FMatrix3x3& matrix)
{
	//FILE* stlFile = _tfsopen(filename, L"rt", _SH_DENYNO); 
	FILE* stlFile;
	_tfopen_s(&stlFile, filename, L"rb");
	if (stlFile == NULL)
		return false;

	TCHAR line[1024];
	FPoint3 vertex;
	Point3 v0(0, 0, 0), v1(0, 0, 0), v2(0, 0, 0);
	int count = 0, facecount = 1;
	// size 直接使用 1024，而不用sizeof(line)，這寫法會當掉
	while (_fgetts(line, 1024, stlFile))
	{
		if (_stscanf(line, L" vertex %f %f %f", &vertex.x, &vertex.y, &vertex.z) == 3)
		{
			count++;
			switch (count)
			{
			case 1:
				//v0 = matrix.apply(vertex);
				printf("face %d \n", facecount);
				printf("v0 : x = %f y = %f z = %f\n", vertex.x, vertex.y, vertex.z);
 			    //v0 = Point3(MM2INT(vertex.x), MM2INT(vertex.y), MM2INT(vertex.z)); 
				v0 = matrix.apply(vertex);
				printf("v0 : x = %d y = %d z = %d\n", v0.x, v0.y, v0.z);
				break;
			case 2:
				//v1 = matrix.apply(vertex);
				printf("v1 : x = %f y = %f z = %f\n", vertex.x, vertex.y, vertex.z);
				//v1 = Point3(MM2INT(vertex.x), MM2INT(vertex.y), MM2INT(vertex.z));
				v1 = matrix.apply(vertex);
				printf("v1 : x = %d y = %d z = %d\n", v1.x, v1.y, v1.z);
				break;
			case 3:
				//v2 = matrix.apply(vertex);
				//mesh->addFace(v0, v1, v2);				
				printf("v2 : x = %f y = %f z = %f\n", vertex.x, vertex.y, vertex.z);
				//v2 = Point3(MM2INT(vertex.x), MM2INT(vertex.y), MM2INT(vertex.z));
				v2 = matrix.apply(vertex);
				printf("v2 : x = %d y = %d z = %d\n", v2.x, v2.y, v2.z);
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
	//printf("loadMeshSTL_binary!!!\n\n");
	//FILE* stlFile = _tfsopen(filename, L"rt", _SH_DENYNO);
	FILE* stlFile;
	_tfopen_s(&stlFile, filename, L"rb");

	fseek(stlFile, 0L, SEEK_END);
	long long file_size = ftell(stlFile); //The file size is the position of the cursor after seeking to the end.
	rewind(stlFile); //Seek back to start.
	size_t face_count = (file_size - 80 - sizeof(uint32_t)) / 50; //Subtract the size of the header. Every face uses exactly 50 bytes.

	TCHAR buffer[80];
	//Skip the header
	if (fread(buffer, 80, 1, stlFile) != 1)
	{
		fclose(stlFile);
		printf("44444\n");
		return false;
	}

	uint32_t reported_face_count;
	//Read the face count. We'll use it as a sort of redundancy code to check for file corruption.
	if (fread(&reported_face_count, sizeof(uint32_t), 1, stlFile) != 1)
	{
		fclose(stlFile);
		printf("55555\n");
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
	printf("face count : %d\n", face_count);
	for (unsigned int i = 0; i < face_count; i++)
	{
		printf("face : %d\n", i);
		if (fread(buffer, 50, 1, stlFile) != 1)
		{
			fclose(stlFile);
			printf("66666\n");
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

bool loadMeshSTL(Mesh* mesh, const TCHAR* filename, const FMatrix3x3& matrix)
{
	FILE* stlFile = _tfsopen(filename, L"rt", _SH_DENYNO);
	if (stlFile == nullptr)
	{		
		return false;
	}
	
	//Skip any whitespace at the beginning of the file.
	unsigned long long num_whitespace = 0; //Number of whitespace characters.
	//unsigned char whitespace;
	TCHAR whitespace;
	if (fread(&whitespace, 1, 1, stlFile) != 1)
	{
		
		fclose(stlFile);
		printf("1111\n");
		return false;
	}

	while (_istspace(whitespace))
	{
		num_whitespace++;
		if (fread(&whitespace, 1, 1, stlFile) != 1)
		{
			fclose(stlFile);
			printf("22222\n");
			return false;
		}
	}

	fseek(stlFile, num_whitespace, SEEK_SET); //Seek to the place after all whitespace (we may have just read too far).
	TCHAR buffer[6];
	if (fread(buffer, 5, 1, stlFile) != 1)
	{
		fclose(stlFile);
		printf("33333\n");
		return false;
	}
	fclose(stlFile);

	buffer[5] = L'\0';
	// 寫 if (!_tcsncmp(buffer, L"solid", 5)) 會判斷失敗
	if (_tcsncmp(buffer, L"solid", 5) != 0)
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

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc != 2) // no STL file
	{
		printf("Please enter STL file!\n");
		return 1;
	}

	printf("test start!!!\n\n");

	Mesh mesh =  Mesh();
	FMatrix3x3 matrix;
	//if (!loadMeshfromSTL(&mesh, "Box.stl"))
	if (!loadMeshSTL(&mesh, argv[1], matrix))
		printf("failed to load STL file!!!\n");

	for (unsigned int i = 0; i < mesh.vertices.size(); i++)
	{
		printf("vertetx %d : x=%d y=%d z=%d \n", i, mesh.vertices[i].p.x, mesh.vertices[i].p.y, mesh.vertices[i].p.z);
		//for (unsigned int f : mesh.vertices[i].connected_faces) // search through all faces connected to the first vertex and find those that are also connected to the second
		//{
		//	printf("connected face : %d\n", f);
		//}
	}

	for (unsigned int p = 0; p < mesh.faces.size(); p++)
	{
		printf("face : %d \n", p);
		for (int j = 0; j < 3; j++)		
		{
			printf("vertex index %d : %d   connect face index %d : %d\n", j, mesh.faces[p].vertex_index[j], j, mesh.faces[p].connected_face_index[j]);
		}
	}
	//printf("mesh max.z = %d\nmesh min.z = %d", mesh.max.z, mesh.min.z);
	printf("\nslicing start!!!\n\n");

	//int initial_layer_thickness = 10;
	int initial_layer_thickness = 100;
	//int initial_layer_thickness = 200;
	int layer_thickness = 100;
	int initial_slice_z = initial_layer_thickness - layer_thickness / 2;
	int slice_layer_count = (mesh.max().z - initial_slice_z) / layer_thickness + 1;

	Slicer* slicer = new Slicer(&mesh, initial_slice_z, layer_thickness, slice_layer_count);
	printf("\nslicing finished!!!\n\n");


	return 0;
}
