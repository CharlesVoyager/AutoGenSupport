#include "stdafx.h"
#include "SparsePointGridInclusive.h"
#include "slicertest.h"
#include <limits>
#include <algorithm>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

namespace cura {

//int largest_neglected_gap_first_phase = MM2INT(0.01); //!< distance between two line segments regarded as connected
//int largest_neglected_gap_second_phase = MM2INT(0.02); //!< distance between two line segments regarded as connected
//int max_stitch1 = MM2INT(10.0); //!< maximal distance stitched between open polylines to form polygons
int largest_neglected_gap_first_phase = MM2_2INT(0.01); //!< distance between two line segments regarded as connected
int largest_neglected_gap_second_phase = MM2_2INT(0.02); //!< distance between two line segments regarded as connected
int max_stitch1 = MM2_2INT(10.0); //!< maximal distance stitched between open polylines to form polygons
	

/*!
* returns a hash for the location, but first divides by the vertex_meld_distance,
* so that any point within a box of vertex_meld_distance by vertex_meld_distance would get mapped to the same hash.
*/

void SlicerLayer::makeBasicPolygonLoops(Polygons& open_polylines)
{
	for (unsigned int start_segment_idx = 0; start_segment_idx < segments.size(); start_segment_idx++)
	{
		if (!segments[start_segment_idx].addedToPolygon)
		{
			makeBasicPolygonLoop(open_polylines, start_segment_idx);
		}
	}
	//Clear the segmentList to save memory, it is no longer needed after this point.
	//segments.clear();
}

void SlicerLayer::makeBasicPolygonLoop(Polygons& open_polylines, unsigned int start_segment_idx)
{
	//std::vector<SlicerSegment> segment_test;
	//Polygon poly;
	PolygonSingle poly;
	poly.add(segments[start_segment_idx].start);
	//poly.pointIdx_to_faceIdx.push_back(segments[start_segment_idx].faceIndex);

	for (int segment_idx = start_segment_idx; segment_idx != -1;)
	{
		//segment_test.push_back(segments[segment_idx]);
		SlicerSegment& segment = segments[segment_idx];
		poly.add(segment.end);
        poly.pointIdx_to_faceIdx.push_back(segment.faceIndex);
		segment.addedToPolygon = true;
		segment_idx = getNextSegmentIdx(segment, start_segment_idx);

		if (segment_idx != -1)
			segments[segment_idx].start = segment.end; // Jason_20180828

		if (segment_idx == static_cast<int>(start_segment_idx))
		{ // polyon is closed
			polygons.add(poly);
			polygons.allFaceIdx.push_back(poly.pointIdx_to_faceIdx);
			return;
		}
	}
	
	//MessageBoxW(0, L"polygon couldn't be closed!!", L"MessageBox caption", MB_OK);
	// polygon couldn't be closed
	open_polylines.add(poly);
	open_polylines.allFaceIdx.push_back(poly.pointIdx_to_faceIdx);
    //printf("open_polylines.add : %d \n", open_polylines.size());
	//printf("open_polylines:: %d\n", open_polylines.size());
	//for (auto segment_p : segment_test)
	//{
	//	printf("start : %d, ", segment_p.start.X);
	//	printf("start : %d, ", segment_p.start.Y);

	//	printf("end : %d", segment_p.end.X);
	//	printf("end : %d\n", segment_p.end.Y);
	//}


	//printf("polygon couldn't be closed!! open_polyline %d\n", open_polylines.size());
}

int SlicerLayer::tryFaceNextSegmentIdx(const SlicerSegment& segment, int face_idx, unsigned int start_segment_idx) const
{
	decltype(face_idx_to_segment_idx.begin()) it;
	auto it_end = face_idx_to_segment_idx.end();
	it = face_idx_to_segment_idx.find(face_idx);
	if (it != it_end)
	{
		int segment_idx = (*it).second;
		cura::Point p1 = segments[segment_idx].start;
		cura::Point diff = segment.end - p1;
		if (shorterThen(diff, largest_neglected_gap_first_phase))
		{
			if (segment_idx == static_cast<int>(start_segment_idx))
			{
				return start_segment_idx;
			}
			if (segments[segment_idx].addedToPolygon)
			{
				return -1;
			}
			return segment_idx;
		}
	}

	return -1;
}

int SlicerLayer::getNextSegmentIdx(const SlicerSegment& segment, unsigned int start_segment_idx)
{
	int next_segment_idx = -1;

	bool segment_ended_at_edge = segment.endVertex == nullptr;
	if (segment_ended_at_edge)
	{
		int face_to_try = segment.endOtherFaceIdx;
		if (face_to_try == -1)
		{
			return -1;
		}
		return tryFaceNextSegmentIdx(segment, face_to_try, start_segment_idx);
	}
	else
	{
		// segment ended at vertex

		const std::vector<uint32_t> &faces_to_try = segment.endVertex->connected_faces;
		for (int face_to_try : faces_to_try)
		{
			int result_segment_idx =
				tryFaceNextSegmentIdx(segment, face_to_try, start_segment_idx);
			if (result_segment_idx == static_cast<int>(start_segment_idx))
			{
				return start_segment_idx;
			}
			else if (result_segment_idx != -1)
			{
				// not immediately returned since we might still encounter the start_segment_idx
				next_segment_idx = result_segment_idx;
			}
		}
	}

	return next_segment_idx;
}

void SlicerLayer::connectOpenPolylines(Polygons& open_polylines)
{
	bool allow_reverse = false;
	// Search a bit fewer cells but at cost of covering more area.
	// Since acceptance area is small to start with, the extra is unlikely to hurt much.
	coord_t cell_size = largest_neglected_gap_first_phase * 2;
	//coord_t cell_size = largest_neglected_gap_first_phase;
	connectOpenPolylinesImpl(open_polylines, largest_neglected_gap_second_phase, cell_size, allow_reverse);
}

void SlicerLayer::stitch(Polygons& open_polylines)
{
	bool allow_reverse = true;
	connectOpenPolylinesImpl(open_polylines, max_stitch1, max_stitch1, allow_reverse);
}

const SlicerLayer::Terminus SlicerLayer::Terminus::INVALID_TERMINUS{ ~static_cast<Index>(0U) };

bool SlicerLayer::PossibleStitch::operator<(const PossibleStitch& other) const
{
	// better if lower distance
	if (dist2 > other.dist2)
	{
		return true;
	}
	else if (dist2 < other.dist2)
	{
		return false;
	}

	// better if in order instead of reversed
	if (!in_order() && other.in_order())
	{
		return true;
	}

	// better if lower Terminus::Index for terminus_0
	// This just defines a more total order and isn't strictly necessary.
	if (terminus_0.asIndex() > other.terminus_0.asIndex())
	{
		return true;
	}
	else if (terminus_0.asIndex() < other.terminus_0.asIndex())
	{
		return false;
	}

	// better if lower Terminus::Index for terminus_1
	// This just defines a more total order and isn't strictly necessary.
	if (terminus_1.asIndex() > other.terminus_1.asIndex())
	{
		return true;
	}
	else if (terminus_1.asIndex() < other.terminus_1.asIndex())
	{
		return false;
	}

	// The stitches have equal goodness
	return false;
}

std::priority_queue<SlicerLayer::PossibleStitch>
SlicerLayer::findPossibleStitches(
const Polygons& open_polylines,
coord_t max_dist, coord_t cell_size,
bool allow_reverse) const
{
	std::priority_queue<PossibleStitch> stitch_queue;

	// maximum distance squared
	int64_t max_dist2 = max_dist * max_dist;

	// Represents a terminal point of a polyline in open_polylines.
	struct StitchGridVal
	{
		unsigned int polyline_idx;
		// Depending on the SparsePointGridInclusive, either the start point or the
		// end point of the polyline
		Point polyline_term_pt;
	};

	struct StitchGridValLocator
	{
		Point operator()(const StitchGridVal& val) const
		{
			return val.polyline_term_pt;
		}
	};

	// Used to find nearby end points within a fixed maximum sparse grid 
	SparsePointGrid<StitchGridVal, StitchGridValLocator> grid_ends(cell_size);
	// Used to find nearby start points within a fixed maximum radius
	SparsePointGrid<StitchGridVal, StitchGridValLocator> grid_starts(cell_size);

	// populate grids

	// Inserts the ends of all polylines into the grid (does not
	//   insert the starts of the polylines).
	for (unsigned int polyline_0_idx = 0; polyline_0_idx < open_polylines.size(); polyline_0_idx++)
	{
		ConstPolygonRef polyline_0 = open_polylines[polyline_0_idx];

		if (polyline_0.size() < 1) continue;

		StitchGridVal grid_val;
		grid_val.polyline_idx = polyline_0_idx;
		grid_val.polyline_term_pt = polyline_0.back();
		grid_ends.insert(grid_val);
	}
	//printf("grid_ends size  : %d\n", grid_ends.size());
	// Inserts the start of all polylines into the grid.
	if (allow_reverse)
	{
		for (unsigned int polyline_0_idx = 0; polyline_0_idx < open_polylines.size(); polyline_0_idx++)
		{
			ConstPolygonRef polyline_0 = open_polylines[polyline_0_idx];

			if (polyline_0.size() < 1) continue;

			StitchGridVal grid_val;
			grid_val.polyline_idx = polyline_0_idx;
			grid_val.polyline_term_pt = polyline_0[0];
			grid_starts.insert(grid_val);
		}
	}

	// search for nearby end points
	for (unsigned int polyline_1_idx = 0; polyline_1_idx < open_polylines.size(); polyline_1_idx++)
	{
		ConstPolygonRef polyline_1 = open_polylines[polyline_1_idx];
		if (polyline_1.size() < 1) continue;

		std::vector<StitchGridVal> nearby_ends;

		 //Check for stitches that append polyline_1 onto polyline_0
		 //in natural order.  These are stitches that use the end of
		 //polyline_0 and the start of polyline_1.
		nearby_ends = grid_ends.getNearby(polyline_1[0], max_dist);
		for (const auto& nearby_end : nearby_ends)
		{
			Point diff = nearby_end.polyline_term_pt - polyline_1[0];
			int64_t dist2 = vSize2(diff);
			if (dist2 < max_dist2)
			{
				PossibleStitch poss_stitch;
				poss_stitch.dist2 = dist2;
				poss_stitch.terminus_0 = Terminus{ nearby_end.polyline_idx, true };
				poss_stitch.terminus_1 = Terminus{ polyline_1_idx, false };
				stitch_queue.push(poss_stitch);
			}
		}

		if (allow_reverse)
		{
			// Check for stitches that append polyline_1 onto polyline_0
			// by reversing order of polyline_1.  These are stitches that
			// use the end of polyline_0 and the end of polyline_1.
			nearby_ends = grid_ends.getNearby(polyline_1.back(), max_dist);
			for (const auto& nearby_end : nearby_ends)
			{
				// Disallow stitching with self with same end point
				if (nearby_end.polyline_idx == polyline_1_idx)
				{
					continue;
				}

				Point diff = nearby_end.polyline_term_pt - polyline_1.back();

				int64_t dist2 = vSize2(diff);
				if (dist2 < max_dist2)
				{
					PossibleStitch poss_stitch;
					poss_stitch.dist2 = dist2;
					poss_stitch.terminus_0 = Terminus{ nearby_end.polyline_idx, true };
					poss_stitch.terminus_1 = Terminus{ polyline_1_idx, true };
					stitch_queue.push(poss_stitch);
				}
			}

			// Check for stitches that append polyline_1 onto polyline_0
			// by reversing order of polyline_0.  These are stitches that
			// use the start of polyline_0 and the start of polyline_1.
			std::vector<StitchGridVal> nearby_starts =
				grid_starts.getNearby(polyline_1[0], max_dist);
			for (const auto& nearby_start : nearby_starts)
			{
				// Disallow stitching with self with same end point
				if (nearby_start.polyline_idx == polyline_1_idx)
				{
					continue;
				}

				Point diff = nearby_start.polyline_term_pt - polyline_1[0];
				int64_t dist2 = vSize2(diff);
				if (dist2 < max_dist2)
				{
					PossibleStitch poss_stitch;
					poss_stitch.dist2 = dist2;
					poss_stitch.terminus_0 = Terminus{ nearby_start.polyline_idx, false };
					poss_stitch.terminus_1 = Terminus{ polyline_1_idx, false };
					stitch_queue.push(poss_stitch);
				}
			}
		}
	}

	return stitch_queue;
}

void SlicerLayer::planPolylineStitch(
	const Polygons& open_polylines,
	Terminus& terminus_0, Terminus& terminus_1, bool reverse[2]) const
{
	size_t polyline_0_idx = terminus_0.getPolylineIdx();
	size_t polyline_1_idx = terminus_1.getPolylineIdx();
	bool back_0 = terminus_0.isEnd();
	bool back_1 = terminus_1.isEnd();
	reverse[0] = false;
	reverse[1] = false;
	if (back_0)
	{
		if (back_1)
		{
			// back of both polylines
			// we can reverse either one and then append onto the other
			// reverse the smaller polyline
			if (open_polylines[polyline_0_idx].size() <
				open_polylines[polyline_1_idx].size())
			{
				std::swap(terminus_0, terminus_1);
			}
			reverse[1] = true;
		}
		else {
			// back of 0, front of 1
			// already in order, nothing to do
		}
	}
	else
	{
		if (back_1)
		{
			// front of 0, back of 1
			// in order if we swap 0 and 1
			std::swap(terminus_0, terminus_1);
		}
		else
		{
			// front of both polylines
			// we can reverse either one and then prepend to the other
			// reverse the smaller polyline
			if (open_polylines[polyline_0_idx].size() >
				open_polylines[polyline_1_idx].size())
			{
				std::swap(terminus_0, terminus_1);
			}
			reverse[0] = true;
		}
	}
}

void SlicerLayer::joinPolylines(PolygonRef& polyline_0, PolygonRef& polyline_1,
	const bool reverse[2]) const
{
	if (reverse[0])
	{
		// reverse polyline_0
		size_t size_0 = polyline_0.size();
		for (size_t idx = 0U; idx != size_0 / 2; ++idx)
		{
			std::swap(polyline_0[idx], polyline_0[size_0 - 1 - idx]);
		}
	}
	if (reverse[1])
	{
		// reverse polyline_1 by adding in reverse order
		for (int poly_idx = polyline_1.size() - 1; poly_idx >= 0; poly_idx--)
			polyline_0.add(polyline_1[poly_idx]);
	}
	else
	{
		// append polyline_1 onto polyline_0
		for (Point& p : polyline_1)
		{
			polyline_0.add(p);
			//polyline_0.pointIdx_to_faceIdx
		}
			
	}
	polyline_1.clear();
}

SlicerLayer::TerminusTrackingMap::TerminusTrackingMap(Terminus::Index end_idx) :
m_terminus_old_to_cur_map(end_idx)
{
	// Initialize map to everything points to itself since nothing has moved yet.
	for (size_t idx = 0U; idx != end_idx; ++idx)
	{
		m_terminus_old_to_cur_map[idx] = Terminus{ idx };
	}
	m_terminus_cur_to_old_map = m_terminus_old_to_cur_map;
}

void SlicerLayer::TerminusTrackingMap::updateMap(
	size_t num_terms,
	const Terminus *cur_terms, const Terminus *next_terms,
	size_t num_removed_terms,
	const Terminus *removed_cur_terms)
{
	// save old locations
	std::vector<Terminus> old_terms(num_terms);
	for (size_t idx = 0U; idx != num_terms; ++idx)
	{
		old_terms[idx] = getOldFromCur(cur_terms[idx]);
	}
	// update using maps old <-> cur and cur <-> next
	for (size_t idx = 0U; idx != num_terms; ++idx)
	{
		m_terminus_old_to_cur_map[old_terms[idx].asIndex()] = next_terms[idx];
		Terminus next_term = next_terms[idx];
		if (next_term != Terminus::INVALID_TERMINUS)
		{
			m_terminus_cur_to_old_map[next_term.asIndex()] = old_terms[idx];
		}
	}
	// remove next locations that no longer exist
	for (size_t rem_idx = 0U; rem_idx != num_removed_terms; ++rem_idx)
	{
		m_terminus_cur_to_old_map[removed_cur_terms[rem_idx].asIndex()] =
			Terminus::INVALID_TERMINUS;
	}
}

void SlicerLayer::connectOpenPolylinesImpl(Polygons& open_polylines, coord_t max_dist, coord_t cell_size, bool allow_reverse)
{
	// below code closes smallest gaps first

	std::priority_queue<PossibleStitch> stitch_queue =
		findPossibleStitches(open_polylines, max_dist, cell_size, allow_reverse);
    //if (stitch_queue.size() != 0)
        //printf("stitch_queue size is : %d \n", stitch_queue.size());
	static const Terminus INVALID_TERMINUS = Terminus::INVALID_TERMINUS;
	Terminus::Index terminus_end_idx = Terminus::endIndexFromPolylineEndIndex(open_polylines.size());
	 //Keeps track of how polyline end point locations move around
	TerminusTrackingMap terminus_tracking_map(terminus_end_idx);

	while (!stitch_queue.empty())
	{
		// Get the next best stitch
		PossibleStitch next_stitch;
		next_stitch = stitch_queue.top();
		stitch_queue.pop();
		Terminus old_terminus_0 = next_stitch.terminus_0;
		Terminus terminus_0 = terminus_tracking_map.getCurFromOld(old_terminus_0);
		if (terminus_0 == INVALID_TERMINUS)
		{
			// if we already used this terminus, then this stitch is no longer usable
			continue;
		}
		Terminus old_terminus_1 = next_stitch.terminus_1;
		Terminus terminus_1 = terminus_tracking_map.getCurFromOld(old_terminus_1);
		if (terminus_1 == INVALID_TERMINUS)
		{
			// if we already used this terminus, then this stitch is no longer usable
			continue;
		}

		size_t best_polyline_0_idx = terminus_0.getPolylineIdx();
		size_t best_polyline_1_idx = terminus_1.getPolylineIdx();

		// check to see if this completes a polygon
		bool completed_poly = best_polyline_0_idx == best_polyline_1_idx;
		if (completed_poly)
		{
			// finished polygon
			PolygonRef polyline_0 = open_polylines[best_polyline_0_idx];
			polygons.add(polyline_0);
			polyline_0.clear();
			Terminus cur_terms[2] = { { best_polyline_0_idx, false },
			{ best_polyline_0_idx, true } };
			for (size_t idx = 0U; idx != 2U; ++idx)
			{
				terminus_tracking_map.markRemoved(cur_terms[idx]);
			}
			continue;
		}

		// we need to join these polylines

		// plan how to join polylines
		bool reverse[2];
		planPolylineStitch(open_polylines, terminus_0, terminus_1, reverse);

		// need to reread since planPolylineStitch can swap terminus_0/1
		best_polyline_0_idx = terminus_0.getPolylineIdx();
		best_polyline_1_idx = terminus_1.getPolylineIdx();
		PolygonRef polyline_0 = open_polylines[best_polyline_0_idx];
		PolygonRef polyline_1 = open_polylines[best_polyline_1_idx];

		// join polylines according to plan
		joinPolylines(polyline_0, polyline_1, reverse);

		// update terminus_tracking_map
		Terminus cur_terms[4] = { { best_polyline_0_idx, false },
		{ best_polyline_0_idx, true },
		{ best_polyline_1_idx, false },
		{ best_polyline_1_idx, true } };
		Terminus next_terms[4] = { { best_polyline_0_idx, false },
			INVALID_TERMINUS,
			INVALID_TERMINUS,
			{ best_polyline_0_idx, true } };
		if (reverse[0])
		{
			std::swap(next_terms[0], next_terms[1]);
		}
		if (reverse[1])
		{
			std::swap(next_terms[2], next_terms[3]);
		}
		// cur_terms -> next_terms has movement map
		// best_polyline_1 is always removed
		terminus_tracking_map.updateMap(4U, cur_terms, next_terms,
			2U, &cur_terms[2]);
	}
    //printf("stitch_queue size is : %d \n", stitch_queue.size());
}

void SlicerLayer::stitch_extensive(Polygons& open_polylines)
{
	//For extensive stitching find 2 open polygons that are touching 2 closed polygons.
	// Then find the shortest path over this polygon that can be used to connect the open polygons,
	// And generate a path over this shortest bit to link up the 2 open polygons.
	// (If these 2 open polygons are the same polygon, then the final result is a closed polyon)

	while (1)
	{
		unsigned int best_polyline_1_idx = -1;
		unsigned int best_polyline_2_idx = -1;
		GapCloserResult best_result;
		best_result.len = POINT_MAX;
		best_result.polygonIdx = -1;
		best_result.pointIdxA = -1;
		best_result.pointIdxB = -1;

		for (unsigned int polyline_1_idx = 0; polyline_1_idx < open_polylines.size(); polyline_1_idx++)
		{
			PolygonRef polyline_1 = open_polylines[polyline_1_idx];
			if (polyline_1.size() < 1) continue;

			{
				GapCloserResult res = findPolygonGapCloser(polyline_1[0], polyline_1.back());
				if (res.len > 0 && res.len < best_result.len)
				{
					best_polyline_1_idx = polyline_1_idx;
					best_polyline_2_idx = polyline_1_idx;
					best_result = res;
				}
			}

			for (unsigned int polyline_2_idx = 0; polyline_2_idx < open_polylines.size(); polyline_2_idx++)
			{
				PolygonRef polyline_2 = open_polylines[polyline_2_idx];
				if (polyline_2.size() < 1 || polyline_1_idx == polyline_2_idx) continue;

				GapCloserResult res = findPolygonGapCloser(polyline_1[0], polyline_2.back());
				if (res.len > 0 && res.len < best_result.len)
				{
					best_polyline_1_idx = polyline_1_idx;
					best_polyline_2_idx = polyline_2_idx;
					best_result = res;
				}
			}
		}

		if (best_result.len < POINT_MAX)
		{
			if (best_polyline_1_idx == best_polyline_2_idx)
			{
				if (best_result.pointIdxA == best_result.pointIdxB)
				{
					polygons.add(open_polylines[best_polyline_1_idx]);
					open_polylines[best_polyline_1_idx].clear();
				}
				else if (best_result.AtoB)
				{
					PolygonRef poly = polygons.newPoly();
					for (unsigned int j = best_result.pointIdxA; j != best_result.pointIdxB; j = (j + 1) % polygons[best_result.polygonIdx].size())
						poly.add(polygons[best_result.polygonIdx][j]);
					for (unsigned int j = open_polylines[best_polyline_1_idx].size() - 1; int(j) >= 0; j--)
						poly.add(open_polylines[best_polyline_1_idx][j]);
					open_polylines[best_polyline_1_idx].clear();
				}
				else
				{
					unsigned int n = polygons.size();
					polygons.add(open_polylines[best_polyline_1_idx]);
					for (unsigned int j = best_result.pointIdxB; j != best_result.pointIdxA; j = (j + 1) % polygons[best_result.polygonIdx].size())
						polygons[n].add(polygons[best_result.polygonIdx][j]);
					open_polylines[best_polyline_1_idx].clear();
				}
			}
			else
			{
				if (best_result.pointIdxA == best_result.pointIdxB)
				{
					for (unsigned int n = 0; n<open_polylines[best_polyline_1_idx].size(); n++)
						open_polylines[best_polyline_2_idx].add(open_polylines[best_polyline_1_idx][n]);
					open_polylines[best_polyline_1_idx].clear();
				}
				else if (best_result.AtoB)
				{
					PolygonSingle poly;
					for (unsigned int n = best_result.pointIdxA; n != best_result.pointIdxB; n = (n + 1) % polygons[best_result.polygonIdx].size())
						poly.add(polygons[best_result.polygonIdx][n]);
					for (unsigned int n = poly.size() - 1; int(n) >= 0; n--)
						open_polylines[best_polyline_2_idx].add(poly[n]);
					for (unsigned int n = 0; n<open_polylines[best_polyline_1_idx].size(); n++)
						open_polylines[best_polyline_2_idx].add(open_polylines[best_polyline_1_idx][n]);
					open_polylines[best_polyline_1_idx].clear();
				}
				else
				{
					for (unsigned int n = best_result.pointIdxB; n != best_result.pointIdxA; n = (n + 1) % polygons[best_result.polygonIdx].size())
						open_polylines[best_polyline_2_idx].add(polygons[best_result.polygonIdx][n]);
					for (unsigned int n = open_polylines[best_polyline_1_idx].size() - 1; int(n) >= 0; n--)
						open_polylines[best_polyline_2_idx].add(open_polylines[best_polyline_1_idx][n]);
					open_polylines[best_polyline_1_idx].clear();
				}
			}
		}
		else
		{
			break;
		}
	}
}

GapCloserResult SlicerLayer::findPolygonGapCloser(Point ip0, Point ip1)
{
	GapCloserResult ret;
	ClosePolygonResult c1 = findPolygonPointClosestTo(ip0);
	ClosePolygonResult c2 = findPolygonPointClosestTo(ip1);
	if (c1.polygonIdx < 0 || c1.polygonIdx != c2.polygonIdx)
	{
		ret.len = -1;
		return ret;
	}
	ret.polygonIdx = c1.polygonIdx;
	ret.pointIdxA = c1.pointIdx;
	ret.pointIdxB = c2.pointIdx;
	ret.AtoB = true;

	if (ret.pointIdxA == ret.pointIdxB)
	{
		//Connection points are on the same line segment.
		ret.len = vSize(ip0 - ip1);
	}
	else{
		//Find out if we have should go from A to B or the other way around.
		Point p0 = polygons[ret.polygonIdx][ret.pointIdxA];
		int64_t lenA = vSize(p0 - ip0);
		for (unsigned int i = ret.pointIdxA; i != ret.pointIdxB; i = (i + 1) % polygons[ret.polygonIdx].size())
		{
			Point p1 = polygons[ret.polygonIdx][i];
			lenA += vSize(p0 - p1);
			p0 = p1;
		}
		lenA += vSize(p0 - ip1);

		p0 = polygons[ret.polygonIdx][ret.pointIdxB];
		int64_t lenB = vSize(p0 - ip1);
		for (unsigned int i = ret.pointIdxB; i != ret.pointIdxA; i = (i + 1) % polygons[ret.polygonIdx].size())
		{
			Point p1 = polygons[ret.polygonIdx][i];
			lenB += vSize(p0 - p1);
			p0 = p1;
		}
		lenB += vSize(p0 - ip0);

		if (lenA < lenB)
		{
			ret.AtoB = true;
			ret.len = lenA;
		}
		else{
			ret.AtoB = false;
			ret.len = lenB;
		}
	}
	return ret;
}

ClosePolygonResult SlicerLayer::findPolygonPointClosestTo(Point input)
{
	ClosePolygonResult ret;
	for (unsigned int n = 0; n<polygons.size(); n++)
	{
		Point p0 = polygons[n][polygons[n].size() - 1];
		for (unsigned int i = 0; i<polygons[n].size(); i++)
		{
			Point p1 = polygons[n][i];

			//Q = A + Normal( B - A ) * ((( B - A ) dot ( P - A )) / VSize( A - B ));
			Point pDiff = p1 - p0;
			int64_t lineLength = vSize(pDiff);
			if (lineLength > 1)
			{
				int64_t distOnLine = dot(pDiff, input - p0) / lineLength;
				if (distOnLine >= 0 && distOnLine <= lineLength)
				{
					Point q = p0 + pDiff * distOnLine / lineLength;
					if (shorterThen(q - input, 100))
					{
						ret.intersectionPoint = q;
						ret.polygonIdx = n;
						ret.pointIdx = i;
						return ret;
					}
				}
			}
			p0 = p1;
		}
	}
	ret.polygonIdx = -1;
	return ret;
}

void SlicerLayer::makePolygons(const Mesh* mesh)
{
	Polygons open_polylines;

	makeBasicPolygonLoops(open_polylines);
    //printf("polygons size : %d \n", polygons.size());
    //printf("open_polylines size : %d \n", open_polylines.size());
	connectOpenPolylines(open_polylines);
    //printf("polygons connectOpenPolylines size : %d \n", polygons.size());
	// TODO: (?) for mesh surface mode: connect open polygons. Maybe the above algorithm can create two open polygons which are actually connected when the starting segment is in the middle between the two open polygons.

	//if (mesh->getSettingAsSurfaceMode("magic_mesh_surface_mode") == ESurfaceMode::NORMAL)
	//{ // don't stitch when using (any) mesh surface mode, i.e. also don't stitch when using mixed mesh surface and closed polygons, because then polylines which are supposed to be open will be closed
		//stitch(open_polylines);
	//}

	//if (extensive_stitching)
	//{
		//stitch_extensive(open_polylines);
	//}

	//if (keep_none_closed)
	//{
		//for (PolygonRef polyline : open_polylines)
		//{
		//	if (polyline.size() > 0)
		//		polygons.add(polyline);
		//}
	//}

	//for (PolygonRef polyline : open_polylines)
	//{
	//	if (polyline.size() > 0)
	//		openPolylines.add(polyline);
	//}
	//if (open_polylines.size() > 0)
    //printf("open_polylines left : %d \n", open_polylines.size());
    
	//Remove all the tiny polygons, or polygons that are not closed. As they do not contribute to the actual print.
    int snapDistance = MM2_2INT(0.3); // TODO: hardcoded value

 //   //int snapDistance = MM2_2INT(1.0); // TODO: hardcoded value
	//auto it = std::remove_if(polygons.begin(), polygons.end(), [snapDistance](PolygonRef poly) { return poly.shorterThan(snapDistance); });
	//polygons.erase(it, polygons.end());

	//Finally optimize all the polygons. Every point removed saves time in the long run.
    //printf("polygons before simplfy: %d \n", polygons.size());

	//polygons.simplify();

	//polygons.removeDegenerateVerts(); // remove verts connected to overlapping line segments

    //std::cout << "polygons size " << polygons.size() << std::endl;
}

void SlicerLayer::offsetPolygons(const Mesh* mesh)
{
	Point offSet = Point(0, 0) - Point(mesh->min().x, mesh->min().y);

	for (unsigned int i = 0; i < polygons.size(); i++)
	{
		for (unsigned p = 0; p < polygons.paths[i].size(); p++)
		{
			polygons.paths[i][p].X = polygons.paths[i][p].X + offSet.X;
			polygons.paths[i][p].Y = polygons.paths[i][p].Y + offSet.Y;
		}
	}

    for (unsigned int i = 0; i < layerparts.size(); i++)
    {
        for (unsigned int j = 0; j < layerparts[i].size(); j++)
        {
            for (unsigned p = 0; p < layerparts[i].paths[j].size(); p++)
            {
                layerparts[i].paths[j][p].X = layerparts[i].paths[j][p].X + offSet.X;
                layerparts[i].paths[j][p].Y = layerparts[i].paths[j][p].Y + offSet.Y;
            }
        }
    }

	for (auto &segment : segments)
	{
		segment.start.X = segment.start.X + offSet.X;
		segment.start.Y = segment.start.Y + offSet.Y;
		segment.end.X = segment.end.X + offSet.X;
		segment.end.Y = segment.end.Y + offSet.Y;
	}
}

void SlicerLayer::transformPolygons(ProjectiveMapping* projectiveMapping)
{
	std::vector<Point> tempPoints;

	for (unsigned int i = 0; i < polygons.size(); i++)
	{
		tempPoints.reserve(polygons.paths[i].size());
		projectiveMapping->transformPoints(polygons.paths[i], tempPoints);
		polygons.paths[i] = tempPoints;
		tempPoints.clear();
	}
}

void SlicerLayer::makeLayerpars(const int layerIdx)
{
    layerparts = polygons.splitIntoParts(true);
    std::reverse(layerparts.begin(), layerparts.end());

    //if (layerIdx == 289)
    //{
    //    MessageBox(NULL, L"289", L"Msg title", MB_OK | MB_ICONQUESTION);

    //    int layerPartSize = layerparts.size();
    //    wchar_t text[128];
    //    swprintf(text, 128, L"The layerPartSize size is %d", layerPartSize);

    //    MessageBox(NULL, text, L"Msg title", MB_OK | MB_ICONQUESTION);
    //}

    //for (vector<PolygonsPart>::iterator it = layerparts.begin(); it != layerparts.end(); ++it)
    //{
    //    if ((*it).size() == 1)
    //    {
    //        ConstPolygonRef polygon = (*it).paths[0];
    //        if (polygon.orientation() == false)
    //        {
    //            layerparts.erase(it);
    //            --it;
    //        }
    //    }
    //}

    //std::cout << "polygons.splitIntoParts() " << layerparts.size() << std::endl;

    //for (int i = 0; i < layerparts.size(); i++)
    //{
    //    
    //    for (int j = 0; j < layerparts[i].size(); j++)
    //    {
    //        std::cout << "layerparts : " << i << " paths : " << j << " : size : " << layerparts[i].paths[j].size() << std::endl;
    //    }
    //}

    //std::cout << ::endl;
    for (int i = 0; i < layerparts.size();i++)
    {
    //    //if (layerparts[i].size() == 1)
    //    //{
    //    //    ConstPolygonRef polygon = layerparts[i].paths[0];
    //    //    if (polygon.orientation() == false)
    //    //    {
    //    //        layerparts.erase(layerparts.begin() + i);
    //    //        i--;
    //    //    }
    //    //}
    //    //std::cout << "layerparts[i].paths.size()" << layerparts[i].paths.size() << std::endl;

        // 消去polygon聯集運算後產生的多的polygon
        //int snapDistance = MM2_2INT(0.3); // TODO: hardcoded value
		int snapDistance = MM2_2INT(0.1); // TODO: hardcoded value
        int holePathsidx = 0;
        auto it = std::remove_if(layerparts[i].paths.begin() + holePathsidx, layerparts[i].paths.end(), [snapDistance](PolygonRef poly) { return poly.shorterThan(snapDistance); });
        layerparts[i].paths.erase(it, layerparts[i].paths.end());
    }

    for (int i = 0; i < layerparts.size(); i++)
    {
        //for (int j = 0; j < layerparts[i].paths.size(); j++)
        //{
        //    if (layerparts[i].paths[j].size() < 1)
        //        layerparts[i].erase(layerparts[i].begin() + j);
        //}

        if (layerparts[i].size() < 1)
            layerparts.erase(layerparts.begin() + i);
    }
}

//Slicer::Slicer(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName, std::vector<Point> srcPoints, std::vector<Point> disPoints) : mesh(mesh)
Slicer::Slicer(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName, std::vector<Point> srcPoints, std::vector<Point> disPoints, PERCENTAGE_CALLBACK callback)
	: mesh(mesh)
{
	layers.resize(slice_layer_count);

	for (int32_t layer_nr = 0; layer_nr < slice_layer_count; layer_nr++)
	{
		layers[layer_nr].z = initial + thickness * layer_nr;
	}

	for (unsigned int mesh_idx = 0; mesh_idx < mesh->faces.size(); mesh_idx++)
	{
		const MeshFace& face = mesh->faces[mesh_idx];
		const MeshVertex& v0 = mesh->vertices[face.vertex_index[0]];
		const MeshVertex& v1 = mesh->vertices[face.vertex_index[1]];
		const MeshVertex& v2 = mesh->vertices[face.vertex_index[2]];
		Point3 p0 = v0.p;
		Point3 p1 = v1.p;
		Point3 p2 = v2.p;
		int32_t minZ = p0.z;
		int32_t maxZ = p0.z;
		if (p1.z < minZ) minZ = p1.z;
		if (p2.z < minZ) minZ = p2.z;
		if (p1.z > maxZ) maxZ = p1.z;
		if (p2.z > maxZ) maxZ = p2.z;
		int32_t layer_max = (maxZ - initial) / thickness;
		for (int32_t layer_nr = (minZ - initial) / thickness; layer_nr <= layer_max; layer_nr++)
		{
			int32_t z = layer_nr * thickness + initial;
			if (z < minZ) continue;
			if (layer_nr < 0) continue;
			SlicerSegment s;
			s.endVertex = nullptr;
			int end_edge_idx = -1;
			if (p0.z < z && p1.z >= z && p2.z >= z)
			{
				s = project2D(p0, p2, p1, z);
				end_edge_idx = 0;
				if (p1.z == z)
				{
					s.endVertex = &v1;
				}
			}
			else if (p0.z > z && p1.z < z && p2.z < z)
			{
				s = project2D(p0, p1, p2, z);
				end_edge_idx = 2;
			}

			else if (p1.z < z && p0.z >= z && p2.z >= z)
			{
				s = project2D(p1, p0, p2, z);
				end_edge_idx = 1;
				if (p2.z == z)
				{
					s.endVertex = &v2;
				}
			}
			else if (p1.z > z && p0.z < z && p2.z < z)
			{
				s = project2D(p1, p2, p0, z);
				end_edge_idx = 0;
			}
			else if (p2.z < z && p1.z >= z && p0.z >= z)
			{
				s = project2D(p2, p1, p0, z);
				end_edge_idx = 2;
				if (p0.z == z)
				{
					s.endVertex = &v0;
				}
			}
			else if (p2.z > z && p1.z < z && p0.z < z)
			{
				s = project2D(p2, p0, p1, z);
				end_edge_idx = 1;
			}
			else
			{
				//Not all cases create a segment, because a point of a face could create just a dot, and two touching faces
				//  on the slice would create two segments
				continue;
			}
			layers[layer_nr].face_idx_to_segment_idx.insert(std::make_pair(mesh_idx, layers[layer_nr].segments.size()));
			s.faceIndex = mesh_idx;
			s.endOtherFaceIdx = face.connected_face_index[end_edge_idx];
			s.addedToPolygon = false;
			layers[layer_nr].segments.push_back(s);
		}

		if (callback != NULL) callback(((((float)mesh_idx + 1)) / mesh->faces.size()) * 100.0f);
	}
	std::vector<SlicerLayer>& layers_ref = layers; // force layers not to be copied into the threads
#pragma omp parallel for default(none) shared(mesh,layers_ref) firstprivate(keep_none_closed, extensive_stitching)
    //std::cout << "layers :" << layers_ref.size() << std::endl;
	for (unsigned int layer_nr = 0; layer_nr<layers_ref.size(); layer_nr++)
	{
        //if ((layer_nr != 253) || (layer_nr != 254) || (layer_nr != 255)) continue;
        //if (layer_nr != 481 && layer_nr != 482) continue;
        //if (layer_nr != 0) continue;
        //printf("layer : %d\n", layer_nr);
		layers_ref[layer_nr].makePolygons(mesh);
        layers_ref[layer_nr].makeLayerpars(layer_nr);
		layers_ref[layer_nr].polygons.setPolygonsHoleIdx();	
		layers_ref[layer_nr].offsetPolygons(mesh);

		if (callback != NULL) callback(((((float)layer_nr + 1)) / layers_ref.size()) * 100.0f);
	}

    //int layer289size = layers_ref[289].polygons.size();
    //wchar_t text[128];
    //swprintf(text, 128, L"The size is %d", layer289size);
    //MessageBox(NULL, text, L"Msg title", MB_OK | MB_ICONQUESTION);

	//std::cout << "check projectiveMapping ." << std::endl;
	//if (!srcPoints.empty() && !disPoints.empty())
	//{
	//	std::cout << "projectiveMapping ." << std::endl;
	//	if (!projectiveMapping(layers_ref, srcPoints, disPoints))
	//		std::cout << "projectiveMapping failed." << std::endl;
	//}

	savePartsSVG(mesh, initial, thickness, slice_layer_count, fileName);
	//saveMeshIdxSVG(mesh, initial, thickness, slice_layer_count, fileName); // not use
	savePolygonsSVG(mesh, initial, thickness, slice_layer_count, fileName);
	saveMeshesSVG(mesh, initial, thickness, slice_layer_count, fileName);
	saveSegmentsSVG(mesh, initial, thickness, slice_layer_count, fileName);
}

BOOL Slicer::projectiveMapping(std::vector<SlicerLayer>& layers_ref, std::vector<Point> srcPoints, std::vector<Point> disPoints)
{
	BOOL ret = FALSE;
    
#ifdef _WINDOWS
	typedef ProjectiveMapping* (__stdcall *TRANSIMAGE_API)(std::vector<Point>, std::vector<Point>);
	typedef int(__stdcall *BB_API)(std::vector<Point>, int &, int &);

	HINSTANCE TRANSIMAGE = LoadLibrary(L"TransImage.dll");
	
	if (TRANSIMAGE != NULL) 
	{
		TRANSIMAGE_API fnTransImage = (TRANSIMAGE_API)GetProcAddress(TRANSIMAGE, "fnTransImage");
		if (fnTransImage != NULL)
		{
			ProjectiveMapping* projectiveMapping = fnTransImage(srcPoints, disPoints);
			if (projectiveMapping->isMatrixDone())
			{
				for (unsigned int layer_nr = 0; layer_nr < layers_ref.size(); layer_nr++)
				{
					layers_ref[layer_nr].transformPolygons(projectiveMapping);
				}
				delete projectiveMapping;
				ret = TRUE;
			}
		}
		else 
			std::cout << "could not locate the function" << std::endl;

		FreeLibrary(TRANSIMAGE);
		return ret;
	}
	std::cout << "could not load the TransImage.dll" << std::endl;
#elif __APPLE__
    std::cout << "Slicer::projectiveMapping() ===> Not Implement!!!" << std::endl;
#endif
	return ret;
}

void Slicer::savePartsSVG(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName)
{
	wstring svgFile = fileName;
	//svgFile = svgFile.replace(svgFile.find(L".") + 1, 3, L"svg");
	svgFile = svgFile.replace(svgFile.find_last_of(L".") + 1, 3, L"svg");
	/*MessageBoxW(0, svgFile, L"MessageBox caption", MB_OK);*/
	//wcout << svgFile << endl;
    
	wfstream fp;
	//MessageBoxW(0, L"save to SVG", L"savePoygonsSVG", MB_OK);

#ifdef _WINDOWS
	fp.open(svgFile, ios::out);//開啟檔案
#elif __APPLE__
    string strSvgFile(svgFile.begin(), svgFile.end());
    fp.open(strSvgFile, ios::out);
#endif
    
	if (!fp){
		wcout << L"Fail to open file: " << svgFile << endl;
#ifdef _WINDOWS
		MessageBoxW(0, L"Fail to open file", L"savePoygonsSVG", MB_OK);
#elif __APPLE__
#endif
		return;
	}

	fp << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	fp << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;
	Point3 widthHeight = mesh->max() - mesh->min();
	//fp << "<svg width=\"" << INT2MM(widthHeight.x) << "\" height=\"" << INT2MM(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl; INT2MM2
	fp << "<svg width=\"" << INT2MM2(widthHeight.x) << "\" height=\"" << INT2MM2(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl;
	fp << "  <!--" << endl;
	fp << "  Generated using Slic3r XYZprinting Slicer v1.1.30 - Added epsilon for input object size checking - (20170504 14:08)" << endl;
	fp << "  http://slic3r.org/" << endl;
	fp << "   -->" << endl;

	for (int layer_nr = 0; layer_nr < slice_layer_count; layer_nr++)
	{
		fp << "  <g id=\"layer" << layer_nr << "\" slic3r:z=\"" << INT2MM2(layers[layer_nr].z) << "\">" << endl;

        for (unsigned int i = 0; i < layers[layer_nr].layerparts.size(); i++)
        {		
            //if (layer_nr == 289) break;

            for (unsigned int j = 0; j < layers[layer_nr].layerparts[i].size(); j++)
            {
                bool isFirstPath = j == 0;
                wstring isHole = isFirstPath ? L"contour" : L"hole";
                wstring fillType = isFirstPath ? L"fill: white" : L"fill: black";
                fp << "    <polygon slic3r:type=\"" << isHole << "\" points=\"";
                for (unsigned q = 0; q < layers[layer_nr].layerparts[i].paths[j].size(); q++)
                {
                    fp << INT2MM2(layers[layer_nr].layerparts[i].paths[j][q].X) << "," << INT2MM2(layers[layer_nr].layerparts[i].paths[j][q].Y);
                    if (q != layers[layer_nr].layerparts[i].paths[j].size() - 1)
                        fp << " ";
                }
                fp << "\" style=\"" << fillType << "\" />" << endl;
            }
        }

		fp << "  </g>" << endl;
	}
	fp << "</svg>" << endl;

	fp.close();
}

void Slicer::saveMeshIdxSVG(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName)
{
    wstring svgFile = fileName;
    svgFile = svgFile.replace(svgFile.find_last_of(L"."), 11, L"_meshIdx.svg");

    wfstream fp;

#ifdef _WINDOWS
    fp.open(svgFile, ios::out);//開啟檔案
#elif __APPLE__
    fp.open(string(svgFile.begin(), svgFile.end()), ios::out);
#endif
    
    if (!fp){
        wcout << L"Fail to open file: " << svgFile << endl;
#ifdef _WINDOWS
        MessageBoxW(0, L"Fail to open file", L"saveNormalsSVG", MB_OK);
#elif __APPLE__
#endif
        return;
    }

	fp << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	fp << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;
	Point3 widthHeight = mesh->max() - mesh->min();
	//fp << "<svg width=\"" << INT2MM(widthHeight.x) << "\" height=\"" << INT2MM(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl; INT2MM2
	fp << "<svg width=\"" << INT2MM2(widthHeight.x) << "\" height=\"" << INT2MM2(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl;
	fp << "  <!--" << endl;
	fp << "  Generated using Slic3r XYZprinting Slicer v1.1.30 - Added epsilon for input object size checking - (20170504 14:08)" << endl;
	fp << "  http://slic3r.org/" << endl;
	fp << "   -->" << endl;

    for (int layer_nr = 0; layer_nr < slice_layer_count; layer_nr++)
    {
        fp << "  <g id=\"layer" << layer_nr << "\" slic3r:z=\"" << INT2MM2(layers[layer_nr].z) << "\">" << endl;

        for (unsigned int i = 0; i < layers[layer_nr].polygons.allFaceIdx.size(); i++)
        {
			fp << "    <polygon meshidx=\"";

            std::vector<int>& faceidce = layers[layer_nr].polygons.allFaceIdx[i];
            for (auto faceidx : faceidce)
            {
				fp << faceidx << "," << " ";
                //FPoint3& normal = mesh->normals[faceidx];
                //fp << INT2MM2(normal.x) << "," << INT2MM2(normal.y) << "," << INT2MM2(normal.z) << " ";
            }
            fp << "\" />" << endl;
        }
        fp << "  </g>" << endl;
    }
    fp << "</svg>" << endl;

    fp.close();
}

void Slicer::savePolygonsSVG(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName)
{
	wstring svgFile = fileName;
	svgFile = svgFile.replace(svgFile.find_last_of(L"."), 11, L"_polygons.svg");

	wfstream fp;

#ifdef _WINDOWS
	fp.open(svgFile, ios::out);//開啟檔案
#elif __APPLE__
    fp.open(string(svgFile.begin(), svgFile.end()), ios::out);
#endif
	if (!fp) {
		wcout << L"Fail to open file: " << svgFile << endl;
#ifdef _WINDOWS
		MessageBoxW(0, L"Fail to open file", L"saveNormalsSVG", MB_OK);
#elif __APPLE__
#endif
		return;
	}

	fp << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	fp << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;
	Point3 widthHeight = mesh->max() - mesh->min();
	//fp << "<svg width=\"" << INT2MM(widthHeight.x) << "\" height=\"" << INT2MM(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl; INT2MM2
	fp << "<svg width=\"" << INT2MM2(widthHeight.x) << "\" height=\"" << INT2MM2(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl;
	fp << "  <!--" << endl;
	fp << "  Generated using Slic3r XYZprinting Slicer v1.1.30 - Added epsilon for input object size checking - (20170504 14:08)" << endl;
	fp << "  http://slic3r.org/" << endl;
	fp << "   -->" << endl;

	for (int layer_nr = 0; layer_nr < slice_layer_count; layer_nr++)
	{
		fp << "  <g id=\"layer" << layer_nr << "\" slic3r:z=\"" << INT2MM2(layers[layer_nr].z) << "\">" << endl;

		for (unsigned int i = 0; i < layers[layer_nr].polygons.size(); i++)
		{
			bool ishole = layers[layer_nr].polygons.polyIsHole[i];
			wstring contourHole = ishole ? L" hole" : L"contour";
			wstring fillType = ishole ? L"fill: black" : L"fill: white";
			fp << "    <polygon slic3r:type=\"" << contourHole << "\" points=\"";

			for (unsigned int j = 0; j < layers[layer_nr].polygons.paths[i].size(); j++)
			{
				fp << INT2MM2(layers[layer_nr].polygons.paths[i][j].X) << "," << INT2MM2(layers[layer_nr].polygons.paths[i][j].Y);
				if (j != layers[layer_nr].polygons.paths[i].size() - 1)
					fp << " ";
			}

			fp << "\" style=\"" << fillType << "\" />" << endl;
		}

		fp << "  </g>" << endl;
	}
	fp << "</svg>" << endl;

	fp.close();
}

void Slicer::saveMeshesSVG(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName)
{
	wstring svgFile = fileName;
	svgFile = svgFile.replace(svgFile.find_last_of(L"."), 11, L"_mesh.svg");

	wfstream fp;

#ifdef _WINDOWS
	fp.open(svgFile, ios::out);//開啟檔案
#elif __APPLE__
    fp.open(string(svgFile.begin(), svgFile.end()), ios::out);
#endif
	if (!fp) {
		wcout << L"Fail to open file: " << svgFile << endl;
#ifdef _WINDOWS
		MessageBoxW(0, L"Fail to open file", L"saveNormalsSVG", MB_OK);
#elif __APPLE__
#endif
		return;
	}

	fp << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	fp << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;
	Point3 widthHeight = mesh->max() - mesh->min();
	//fp << "<svg width=\"" << INT2MM(widthHeight.x) << "\" height=\"" << INT2MM(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl; INT2MM2
	fp << "<svg width=\"" << INT2MM2(widthHeight.x) << "\" height=\"" << INT2MM2(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl;
	fp << "  <!--" << endl;
	fp << "  Generated using Slic3r XYZprinting Slicer v1.1.30 - Added epsilon for input object size checking - (20170504 14:08)" << endl;
	fp << "  http://slic3r.org/" << endl;
	fp << "   -->" << endl;

	for (unsigned int i = 0; i < mesh->faces.size(); i++)
	{
		fp << "  <g id=\"mesh" << i << "\">" << endl;
		fp << "    <polygon slic3r:type=\"contour\" points=\"";
		//mesh &face = layers[layer_nr].polygons.allFaceIdx[i];

		for (unsigned int j = 0; j < 3; j++)
		{
			fp << INT2MM2(mesh->vertices[mesh->faces[i].vertex_index[j]].p.x) << "," << " ";
			fp << INT2MM2(mesh->vertices[mesh->faces[i].vertex_index[j]].p.y) << "," << " ";
			fp << INT2MM2(mesh->vertices[mesh->faces[i].vertex_index[j]].p.z) << "," << " ";
		}
		fp << "\" />" << endl;
		fp << "  </g>" << endl;
	}

	fp << "</svg>" << endl;

	fp.close();
}

void Slicer::saveSegmentsSVG(Mesh* mesh, int initial, int thickness, int slice_layer_count, LPCWSTR fileName)
{
	wstring svgFile = fileName;
	svgFile = svgFile.replace(svgFile.find_last_of(L"."), 11, L"_segments.svg");

	wfstream fp;

#ifdef _WINDOWS
	fp.open(svgFile, ios::out);//開啟檔案
#elif __APPLE__
    fp.open(string(svgFile.begin(), svgFile.end()), ios::out);
#endif
	if (!fp) {
		wcout << L"Fail to open file: " << svgFile << endl;
#ifdef _WINDOWS
		MessageBoxW(0, L"Fail to open file", L"saveNormalsSVG", MB_OK);
#elif __APPLE__
#endif
		return;
	}

	fp << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>" << endl;
	fp << "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/2001/REC-SVG-20010904/DTD/svg10.dtd\">" << endl;
	Point3 widthHeight = mesh->max() - mesh->min();
	//fp << "<svg width=\"" << INT2MM(widthHeight.x) << "\" height=\"" << INT2MM(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl; INT2MM2
	fp << "<svg width=\"" << INT2MM2(widthHeight.x) << "\" height=\"" << INT2MM2(widthHeight.y) << "\" xmlns=\"http://www.w3.org/2000/svg\" xmlns:svg=\"http://www.w3.org/2000/svg\" xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns:slic3r=\"http://slic3r.org/namespaces/slic3r\">" << endl;
	fp << "  <!--" << endl;
	fp << "  Generated using Slic3r XYZprinting Slicer v1.1.30 - Added epsilon for input object size checking - (20170504 14:08)" << endl;
	fp << "  http://slic3r.org/" << endl;
	fp << "   -->" << endl;

	for (int layer_nr = 0; layer_nr < slice_layer_count; layer_nr++)
	{
		fp << "  <g id=\"layer" << layer_nr << "\" slic3r:z=\"" << INT2MM2(layers[layer_nr].z) << "\">" << endl;

		for (auto segment : layers[layer_nr].segments)
		{
			fp << "    <segment points=\"";

			fp << INT2MM2(segment.start.X) << "," << INT2MM2(segment.start.Y) << " ";
			fp << INT2MM2(segment.end.X) << "," << INT2MM2(segment.end.Y) << " ";
			fp << segment.faceIndex << " ";

			fp << "\" />" << endl;
		}
		fp << "  </g>" << endl;
	}
	fp << "</svg>" << endl;

	fp.close();
}

}//namespace cura
