#pragma once

#include <vector>
#include "Helper.h"

using namespace std;

namespace XYZSupport
{
	class Cube
	{
	private:
#if PRECISION_SINGLE
		Single xLowBound = 0;// inclusive
		Single yLowBound = 0;// inclusive
		Single zLowBound = 0;// exclusive
		Single span = 5;
#else
		double xLowBound = 0;// inclusive
		double yLowBound = 0;// inclusive
		double zLowBound = 0;// exclusive
		double span = 5;
#endif
		int groupIdx = -1;

	public:
		vector<Tuple<int, int>> verIdxTriIdxList;	// <lowest z, triangle index>
		bool ConnectPlatform = false;				//newsup cube�O�_�q�쥭�x
		bool EmptyCube = true;						//newsup cube�O�_������       
		bool ObjectBottomEdge = true;
		bool UnderSupportPoint = false; 			//newsup cube�O�_�b�伵�I�U��
		RHVector3 Centerpoint;						//newsup cube�������I
		vector<RHVector3> BelongSupportPoint;		//newsup cube�Q���Ӥ伵�q�L
		bool backbone = false;						//newsup

		Cube() {}

		Cube(double xLow, double yLow, double zLow, double span)
		{
#if PRECISION_SINGLE
			this.xLowBound = (float)xLow;
			this.yLowBound = (float)yLow;
			this.zLowBound = (float)zLow;
			this.span = (float)span;
#else
			xLowBound = xLow;
			yLowBound = yLow;
			zLowBound = zLow;
			this->span = span;
#endif
		}
		void Clean()
		{
			if (verIdxTriIdxList.size() > 0)
				verIdxTriIdxList.clear();
			//verIdxTriIdxList = null;
		}
		int GetGroupIdx() const { return groupIdx; }
		void SetGroupIdx(int value) { groupIdx = value; }

#if PRECISION_SINGLE
		public Single XLowBound{ get { return xLowBound; } }
		public Single YLowBound{ get { return yLowBound; } }
		public Single ZLowBound{ get { return zLowBound; } }
		public Single Span{ get { return span; } }
#else
		double XLowBound() const { return xLowBound; }
		double YLowBound() const { return yLowBound; }
		double ZLowBound() const { return zLowBound; }
		double Span() const { return span; }
#endif
	};
}