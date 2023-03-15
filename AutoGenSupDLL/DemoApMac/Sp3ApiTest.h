#pragma once
#include "SupportCommon.h"

using namespace XYZSupport;
using namespace std;

struct SupportPoint
{
    double pos[3];
    double dir[3];
    double radius;
};

struct SupportDataWOMesh
{
	int supType;
	double depthAdjust;
	double length; double radius1; double radius2; float bufferMatrix[16];
};

class Sp3ApiTest
{
private:
    string strEditMode;  //Normal, User, Cone, Tree
    string strTypeCode;  //Normal, Mark
    string stlFilename;
    float position[3];
    float scale[3];
    float rotation[3];
    bool innerSupport;
    string printerName;
    wstring density;
    double contactSize;
    float newBaseZScale;
    
private:
	unsigned char* getStlBufferFromStlFile(float trans[16], /*out*/ float uiPosition[3]);
	int getThreshold(int threshold[4]);	// get thredshold parameter for automatically support points
	float getNewBaseZScale();
	int addSupportPointAuto();
	
	int GenSupportCylinder(PointEditModeCode mode, MeshTypeCode typeCode);
	int GenSupportCone(MeshTypeCode typeCode);

	int AddSupportDataWOMeshTest(PointEditModeCode mode, MeshTypeCode typeCode);

	vector<SupportDataWOMesh> vecSupportDataWOMesh;

public:
    Sp3ApiTest(string _editMode,
               string _typeCode,
               string _stlFilename,
               string _position,
               string _scale,
               string _rotation,
               string _innerSupport,
               string _printerName,
               string _density,
               string _contactSize,
               string _newBaseZScale);
	int Run();
};
