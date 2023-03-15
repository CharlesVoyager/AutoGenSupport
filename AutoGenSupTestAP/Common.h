#pragma once
#include "../AutoGenSupDLL/Helper/MyTools.h"
using namespace MyTools;

// Enumeration PointEditModeCode below is copied from SupportCommon.h.
enum class PointEditModeCode {
	Normal = 1, User, Cone, Tree, Depth, FacetToBase, Auto, Bed,// support points editor mode: normal (auto generation support points), User (manual edit support points)
};
enum class MeshTypeCode { Normal, Mark };

enum class ModelType
{
	MODEL = 1,															                    // NOTE: Tim renmed BRANCH to BODY_NOCUP, TAIL to CYLINDER.
	MARK, HEAD, BODY, BODY_NOCUP, CYLINDER, FOOT,						// normal support	// XYZware Trunk: BRANCH (5), TAIL (6), FOOT (7) ===> C++: BODY_NOCUP (5), CYLINDER (6), FOOT (7)
	MARK_CONE, CONE,													// cone support
	MARK_BRANCH, SYMBOL_TRUNK, TREE_TRUNK, SYMBOL_BRANCH, TREE_BRANCH,	// tree support
	BED_CELL, // bed support 
	CUBOID_CELL, CUBOID_FOOT, // cuboid support
	TAB,
	//F2BARROW// facet to base	
};

struct SupportPoint
{
	double pos[3];
	double dir[3];
	double radius;

	SupportPoint(const double inPos[3], const double inDir[3], const double inRadius)
	{
		memcpy(pos, inPos, sizeof(pos));
		memcpy(dir, inDir, sizeof(dir));
		radius = inRadius;
	}
};

struct SupportDataWOMesh
{
	float bufferPosition[3]; float bufferScale[3]; float bufferRotation[3];
	int supType;
	double bufferOrientation[3];
	float bufferCurPos[16]; float bufferCurPos2[16]; float bufferTrans[16];
	double bufferBoundingBox[6];
	double depthAdjust;
	double length; double radius1; double radius2; float bufferMatrix[16];

	void ToPrint()
	{
		printf("Position: %f %f %f\n", bufferPosition[0], bufferPosition[1], bufferPosition[2]);
		printf("Scale: %f %f %f\n", bufferScale[0], bufferScale[1], bufferScale[2]);
		printf("Rotation: %f %f %f\n", bufferRotation[0], bufferRotation[1], bufferRotation[2]);
		printf("supType: %d\n", supType);

		printf("BoundingBox(min): %f %f %f\n", bufferBoundingBox[0], bufferBoundingBox[1], bufferBoundingBox[2]);
		printf("BoundingBox(max): %f %f %f\n", bufferBoundingBox[3], bufferBoundingBox[4], bufferBoundingBox[5]);

		printf("depthAdjust: %f\n", depthAdjust);
		printf("length: %f\n", length);
		printf("radius1: %f\n", radius1);
		printf("radius2: %f\n", radius2);
		PrintMatrix4(bufferMatrix);
	}
};