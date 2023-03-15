// �U�C ifdef �϶��O�إߥ����H��U�q DLL �ץX���зǤ覡�C
// �o�� DLL �����Ҧ��ɮ׳��O�ϥΩR�O�C���ҩw�q SLICER20DLL_EXPORTS �Ÿ��sĶ���C
// �b�R�O�C�w�q���Ÿ��C����M�׳������w�q�o�ӲŸ�
// �o�ˤ@�ӡA��l�{���ɤ��]�t�o�ɮת������L�M��
// �|�N SLICER20DLL_API �禡�����q DLL �פJ���A�ӳo�� DLL �h�|�N�o�ǲŸ�����
// �ץX���C
#ifdef SLICER20DLL_EXPORTS
#define SLICER20DLL_API __declspec(dllexport)
#define SLICER20DLL_Calibrate_API __declspec(dllexport)
#define SLICER20DLL_Calibrate_API __declspec(dllexport)
#elif __APPLE__
#define SLICER20DLL_API
#define SLICER20DLL_Calibrate_API
#endif

#include "slicertest.h"
#include "mesh.h"
#include "intpoint.h"
#include "floatpoint.h"
//#include "ProjectiveMapping.h"
#include <fstream>  
#include <iostream>  
#include <string>  
#include <stdint.h>
#include <cmath>

typedef void(*PROGESS_CALLBACK)(float);

class SLICER20DLL_API CSlicer20DLL {
public:
	CSlicer20DLL(LPCWSTR fileName, double layerHeight, std::vector<Point> srcPoints, std::vector<Point> disPoints, PROGESS_CALLBACK callback);
	
};

extern SLICER20DLL_API int nSlicer20DLL;

extern "C" SLICER20DLL_API int fnSlicer20DLL(LPCWSTR fileName, double layerHeight);
extern "C" SLICER20DLL_API int fnSlicer20DLL2(LPCWSTR fileName, double layerHeight, PROGESS_CALLBACK callback);
extern "C" SLICER20DLL_Calibrate_API int fnSlicer20DLL_Calibrate(LPCWSTR fileName, double layerHeight, double src_x0, double src_y0, double src_x1, double src_y1, double src_x2, double src_y2, double src_x3, double src_y3, double dis_x0, double dis_y0, double dis_x1, double dis_y1, double dis_x2, double dis_y2, double dis_x3, double dis_y3);

//extern "C" SLICER20DLL_API int fnSlicer20DLL(LPCWSTR fileName, double layerHeight);
