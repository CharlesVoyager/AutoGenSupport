#pragma once

namespace XYZSupport
{
	class StlChecker
	{
	private:
		unsigned int STLErrCode;

	public:
		bool isNoData;
		bool isNotFollowV2V;
		int dupCnt;
		int shrEdgCnt;
		int colinearCnt;
		int degenerateCnt;

		enum STL_ERR_CODE
		{
			STL_NO_DATA = 0x01,
			STL_ERR_DUPLICATE = 0x02,
			STL_NOT_V2V = 0x04,
			STL_ERR_SHARE_EDGE = 0x08,
			STL_COLINEAR = 0x10,
			STL_DEGENERATE = 0x20
		};

		StlChecker()
		{
			isNoData = isNotFollowV2V = false;
			STLErrCode = 0;
			dupCnt = shrEdgCnt = colinearCnt = degenerateCnt = 0;
		}

		unsigned int getSTLErr()
		{
			STLErrCode = 0;

			if (isNoData)
				STLErrCode |= STL_ERR_CODE::STL_NO_DATA;
			if (dupCnt > 0)
				STLErrCode |= STL_ERR_CODE::STL_ERR_DUPLICATE;
			if (isNotFollowV2V)
				STLErrCode |= STL_ERR_CODE::STL_NOT_V2V;
			if (shrEdgCnt > 0)
				STLErrCode |= STL_ERR_CODE::STL_ERR_SHARE_EDGE;
			if (colinearCnt > 0)
				STLErrCode |= STL_ERR_CODE::STL_COLINEAR;
			if (degenerateCnt > 0)
				STLErrCode |= STL_ERR_CODE::STL_DEGENERATE;

			return STLErrCode;
		}
	};
}
