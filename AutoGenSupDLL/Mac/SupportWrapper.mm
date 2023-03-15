#import <Foundation/Foundation.h>
#import "SupportWrapper.h"
#import "Sp3ApiForQt.h"
#import "XYZSupport.h"
#import "MyTools.h"

using namespace std;
using namespace MyTools;

@implementation SupportModule

+ (int) Create:(int) editMode
      MeshType:(int) typeCode
InternalSupport:(bool) internalSupport
 NewBaseZScale:(float) newBaseZScale
{
    NSLog(@"[NKG][SupportModule Create]");
    
    return SP3_Create((PointEditModeCode)editMode,
                      (MeshTypeCode)typeCode,
                      internalSupport,
                      newBaseZScale,
                      true,         // enable easy release tab. (To Do: No easy release tab for PartPro120 xP.)
                      nullptr);
}

+ (int) SetModelAndTrans:(const unsigned char *) stlBuffer
          Transformation:(const float *) trans
{
    NSLog(@"[NKG][SupportModule SetModelAndTransByBuffer]");
    return SP3_SetModelAndTrans(stlBuffer, trans);
}

+ (int) AddSupportPointAuto:(double) radius_contactSize
                 Thresholds:(int *) thresholds
{
    NSLog(@"[NKG][SupportModule AddSupportPointAuto]");
    
    NSError *error;
    /*
     NSDownloadsDirectory: 			/Users/cp23dsw/Downloads/
     NSApplicationSupportDirectory: /Users/cp23dsw/Library/Application Support/
     NSDocumentationDirectory: 		/Users/cp23dsw/Library/Documentation/
     */
    NSURL *paths = [[NSFileManager defaultManager] URLForDirectory:NSDownloadsDirectory
                                                          inDomain:NSUserDomainMask
                                                 appropriateForURL:nil
                                                            create:false
                                                             error:&error];
    NSString *nsTemp = [paths absoluteString]; 
    NSString *nsTemp1 = [nsTemp stringByReplacingOccurrencesOfString:@"%20" withString:@" "];
    NSString *nsAppDataFolder = [nsTemp1 substringFromIndex:7]; // remove "file://". If the path includes "file://", then Support Modile fails to save pictureCpp.stl!

    string str = [nsAppDataFolder UTF8String];
    wstring wstrAppDataFolder(str.begin(), str.end());

    return SP3_AddSupportPointAuto(L"MED",   // Not used in the support module
                                   radius_contactSize,
                                   thresholds,
                                   wstrAppDataFolder.c_str(),    // temp folder
                                   L"",     // path for slicer DLL  NOTE: Don't need to provide path in Mac
                                   L"",     // path for XYZmesh DLL NOTE: Don't need to provide path in Mac
                                   NULL);   // callback
}

+ (int) AddSupportPointManual:(const double *) position
					Direction:(const double *) direction
                       Radius:(double) radius
{
    //NSLog(@"[NKG][SupportModule AddSupportPointManual]");

    return SP3_AddSupportPointManual(position, direction, radius);
}

+ (int) GenSupportDataWOMesh:(int *) pSupportDataCount
{
    int nCount = 0;
    int ret = SP3_GenSupportDataWOMesh(nCount);
    *pSupportDataCount = nCount;

    return ret;
}

+ (int) GetSupportDataWOMesh:(int) indexSupportData
                    Position:(float *) bufferPosition
                       Scale:(float *) bufferScale
                    Rotation:(float *) bufferRotation           // X Y Z
                 SupportType:(int *) pSupType
                 Orientation:(double *) bufferOrientation       // X Y Z
                      curPos:(float *) bufferCurPos             // Matrix4
                     curPos2:(float *) bufferCurPos2            // Matrix4
                       Trans:(float *) bufferTrans;             // Matrix4
{
    int supType = 0;
    double bufferBoundingBox[6];
    double depthAdjust = 0;
    double length = 0; double radius1 = 0; double radius2 = 0; float bufferMatrix[16];
    int ret = SUP_GetSupportDataWOMesh(indexSupportData,
                                        bufferPosition, bufferScale, bufferRotation,
                                        supType,
                                        bufferOrientation,
                                        bufferCurPos, bufferCurPos2, bufferTrans,
                                        bufferBoundingBox,
                                        depthAdjust,
                                        length, radius1, radius2, bufferMatrix);
    *pSupType = supType;
    return ret;
}

+ (unsigned char*) GenSupportDataMesh:(int) indexSupportData
                      /*out*/MeshSize:(int *) pMeshSize
                /*out*/Transformation:(float *) trans
{
    int meshSize = 0;
    unsigned char *pStlBuffer = SP3_GenSupportDataMesh(indexSupportData, meshSize, trans);
    *pMeshSize = meshSize;
    return pStlBuffer;
}

+ (int) Release
{
    NSLog(@"[NKG][SupportModule Release]");
    
    return SP3_Release();
}

@end
