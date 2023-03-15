#pragma once

#include "TopoModel.h"

namespace XYZSupport
{
	class CubeData
	{
	public:
		TopoModel Model;
		Matrix4 Trans;

		CubeData() {}

		CubeData(const TopoModel &model)
		{
			Model = model;
		}
	};
}
