#ifndef _PROJECTIVEMAPPING_H_
#define _PROJECTIVEMAPPING_H_

#include "intpoint.h"
#include "clipper.hpp"
#include <vector>  

using namespace cura;

class ProjectiveMapping
{
public:
	ProjectiveMapping(std::vector<cura::Point> srcPoints, std::vector<cura::Point> disPoints) : srcCorner(srcPoints), disCorner(disPoints)
	{
		matrix_Done = getMatrix();
	}
	virtual ~ProjectiveMapping();

	BOOL isMatrixDone() { return matrix_Done; }
	virtual BOOL transformPoints(std::vector<cura::Point> srcPoints, std::vector<cura::Point>& disPoints);

protected:
	std::vector<cura::Point> srcCorner, disCorner;
	BOOL matrix_Done = FALSE;
	std::vector<double> matrix_SquareToDis, matrix_SquareToSrc_Inverse;

	void findInverseHomography(std::vector<Point>disPoints, std::vector<double>& matrix, BOOL inverse);
	Point transformByMatrix(Point srcPoint, std::vector<double> matrix_1, std::vector<double> matrix_2_Inverse);
	int64_t interpolate(int64_t x, int64_t x0, int64_t x1, int64_t y0, int64_t y1) const
	{
		int64_t dx_01 = x1 - x0;
		int64_t num = (y1 - y0) * (x - x0);
		num += num > 0 ? dx_01 / 2 : -dx_01 / 2; // add in offset to round result
		int64_t y = y0 + num / dx_01;
		return y;
	}

private:
	virtual BOOL getMatrix();
	BOOL isSingleQuad(std::vector<cura::Point> srcPoints);
};

#endif //_PROJECTIVEMAPPING_H_