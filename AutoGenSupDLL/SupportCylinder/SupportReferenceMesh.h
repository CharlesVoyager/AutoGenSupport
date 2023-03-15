#pragma once

#include "TopoModel.h"
#include "SupportStructure.h"

namespace XYZSupport
{
	class TailSet
	{
	private:
		TopoModel _l;
		TopoModel _m;
		TopoModel _s;;

	public:
		TopoModel& L() { return _l; }
		TopoModel& M() { return _m; }
		TopoModel& S() { return _s; }
	};

	class SupportReferenceMesh
	{
	public:
		TailSet Tail;

	private:
		TopoModel _body;
		TopoModel _foot;
		TopoModel _mark;

		TopoModel _bedcellstyle1;
		TopoModel _bedcellstyle2;
		TopoModel _bedcellstyle3;
		TopoModel _bedcellstyle4;
		TopoModel _cuboidcell;
		TopoModel _tabcell;

	public:
		TopoModel& Body() { return _body; }
		TopoModel& Foot() { return _foot; }
		TopoModel& Mark() { return _mark; }

		TopoModel& BedCellStyle1() { return _bedcellstyle1; }
		TopoModel& BedCellStyle2() { return _bedcellstyle2; }
		TopoModel& BedCellStyle3() { return _bedcellstyle3; }
		TopoModel& BedCellStyle4() { return _bedcellstyle4; }
		TopoModel& CuboidCell() { return _cuboidcell; }
		TopoModel& TabCell() { return _tabcell; }

	public:
		SupportReferenceMesh()
		{
			//try
			//{
				// build support model from "stl" file
#if READ_SUPPORT_MODEL_FROM_STL
				transSTL2Arr();
#endif
				// make BBintersectWithModels less sensitive    ¿À¨d∏Iº≤
				////Mark.importArr(ref SupportStructure.support_mark_stl);
				StlBinaryToModel(SupportStructure::support_mark_stl, sizeof(SupportStructure::support_mark_stl), &_mark);
				// TODO: fix support head bounding box check
				// reduce size of bounding box for less sensitive

				//supCrossBodyTopo.importArr(ref SupportStructure.support_foot_stl);
				//Body.importArr(ref SupportStructure.support_foot_stl);
				StlBinaryToModel(SupportStructure::support_foot_stl, sizeof(SupportStructure::support_foot_stl), &_body);
				//Foot.importArr(ref SupportStructure.support_foot_stl);
				StlBinaryToModel(SupportStructure::support_foot_stl, sizeof(SupportStructure::support_foot_stl), &_foot);
				//Tail.L.importArr(ref SupportStructure.l_touch_sup_tail_stl);
				StlBinaryToModel(SupportStructure::l_touch_sup_tail_stl, sizeof(SupportStructure::l_touch_sup_tail_stl), &Tail.L());
				//Tail.M.importArr(ref SupportStructure.m_touch_sup_tail_stl);
				StlBinaryToModel(SupportStructure::m_touch_sup_tail_stl, sizeof(SupportStructure::m_touch_sup_tail_stl), &Tail.M());
				//Tail.S.importArr(ref SupportStructure.s_touch_sup_tail_stl);
				StlBinaryToModel(SupportStructure::s_touch_sup_tail_stl, sizeof(SupportStructure::s_touch_sup_tail_stl), &Tail.S());
				//---
				// --start-- JOSHUA 01-12-2018 | import bed and cuboid cell byte array
				StlBinaryToModel(SupportStructure::support_bed_cell_style1_stl, sizeof(SupportStructure::support_bed_cell_style1_stl), &_bedcellstyle1);
				StlBinaryToModel(SupportStructure::support_bed_cell_style2_stl, sizeof(SupportStructure::support_bed_cell_style2_stl), &_bedcellstyle2);
				StlBinaryToModel(SupportStructure::support_bed_cell_style3_stl, sizeof(SupportStructure::support_bed_cell_style3_stl), &_bedcellstyle3);
				StlBinaryToModel(SupportStructure::support_cuboid_cell_stl, sizeof(SupportStructure::support_cuboid_cell_stl), &_cuboidcell);
				// --end--
				//Added by Ervin 02-14-2019 New Base Cell
				//stlMesh.importByteArray(ref SupportStructure.support_bed_cell_style4_stl, BedCellStyle4);

				// --start-- JOSHUA 04-16-2018 | import tab cell byte array
				StlBinaryToModel(SupportStructure::support_bed_tab, sizeof(SupportStructure::support_bed_tab), &_tabcell);
			//}
	/*		catch { }*/
		}
	};
}
