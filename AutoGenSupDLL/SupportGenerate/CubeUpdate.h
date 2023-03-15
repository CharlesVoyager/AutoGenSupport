#pragma once
#include "RHVector3.h"
#include "CubeMatrix.h"

namespace XYZSupport
{
	class CubeUpdate
	{
	public:
		static void UpdateCubeForBelongSupportPoint(int x, int y, int z, RHVector3 BelongPoint, CubeMatrix &modelcube)
		{
			modelcube.GetCubeByIndex(x, y, z).BelongSupportPoint.push_back(BelongPoint);
		}
		static void UpdateCubeForUnderSupportPoint(int x, int y, int z, CubeMatrix &modelcube, bool status)
		{
			modelcube.GetCubeByIndex(x, y, z).UnderSupportPoint = status;
		}
		static void UpdateCubeforBakebone(int x, int y, int z, CubeMatrix &modelcube, bool status)
		{
			modelcube.GetCubeByIndex(x, y, z).backbone = status;
		}
	};
}