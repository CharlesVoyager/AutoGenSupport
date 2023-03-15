#pragma once

using namespace std;

namespace XYZSupport
{
	class CubeNum
	{
	public:
		int x = 0;
		int y = 0;
		int z = 0;
		CubeNum() {}
		CubeNum(int _x, int _y, int _z) 
		{
			this->x = _x;
			this->y = _y;
			this->z = _z;
		}
		string ToString() const
		{
			return to_string(x) + " " + to_string(y) + " " + to_string(z);
		}
	};
}
