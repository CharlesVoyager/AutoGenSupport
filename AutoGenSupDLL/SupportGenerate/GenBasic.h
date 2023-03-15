#pragma once

#include "SupportCommon.h"
#include "TopoTriangle.h"
#include "TopoModel.h"

namespace XYZSupport
{
	void AddTopSphere(/*out*/TopoModel &model, const int num_phi, const int num_theta, RHVector3 center, double radius);
	void AddTree(/*out*/TopoModel &model, const int num_sides, RHVector3 end_point, double radius1, double radius2, bool isNeedToMakeTopEndCap = false);
	void AddBtmSphere(/*out*/TopoModel &model, const int num_phi, const int num_theta, RHVector3 center, double radius);
	void AddTreeSymbol(TopoModel &treesupportmodel, const int num_sides, ModelType supType, RHVector3 end_point, double radius1, double radius2);
	void GenerateCylinder(int num_sides, RHVector3 EndPoint, RHVector3 StartPoint, double TopRadius, double BottomRadius, /*out*/ TopoModel &model);
	void AddBrim(TopoModel &model, RHBoundingBox bbox, RHVector3 center, float printerWidth, float printerDepth);
}