// 下列 ifdef 區塊是建立巨集以協助從 DLL 匯出的標準方式。
// 這個 DLL 中的所有檔案都是使用命令列中所定義 SLICER20DLL_EXPORTS 符號編譯的。
// 在命令列定義的符號。任何專案都不應定義這個符號
// 這樣一來，原始程式檔中包含這檔案的任何其他專案
// 會將 SLICER20DLL_API 函式視為從 DLL 匯入的，而這個 DLL 則會將這些符號視為
// 匯出的。
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
