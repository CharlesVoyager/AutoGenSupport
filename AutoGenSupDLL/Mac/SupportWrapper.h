#pragma once


@interface SupportModule : NSObject

+ (int) Create:(int) editMode
      MeshType:(int) typeCode
InternalSupport:(bool) internalSupport
 NewBaseZScale:(float) newBaseZScale;

+ (int) SetModelAndTrans:(const unsigned char *) stlBuffer
          Transformation:(const float *) trans;

+ (int) AddSupportPointAuto:(double) radius_contactSize // 0.15mm ~ 0.65mm
                 Thresholds:(int *) thresholds;

+ (int) AddSupportPointManual:(const double *) position
					Direction:(const double *) direction
                       Radius:(double) radius;

+ (int) GenSupportDataWOMesh:(int *) pSupportDataCount;

+ (int) GetSupportDataWOMesh:(int) indexSupportData
                    Position:(float *) bufferPosition   // X Y Z
                       Scale:(float *) bufferScale      // X Y Z
                    Rotation:(float *) bufferRotation   // X Y Z
                 SupportType:(int *) pSupType
                 Orientation:(double *) bufferOrientation    // X Y Z
                      curPos:(float *) bufferCurPos             // Matrix4
                     curPos2:(float *) bufferCurPos2            // Matrix4
                       Trans:(float *) bufferTrans;             // Matrix4
    
+ (unsigned char*) GenSupportDataMesh:(int) indexSupportData
                      /*out*/MeshSize:(int *) pMeshSize
                /*out*/Transformation:(float *) trans;

+ (int) Release;

@end
