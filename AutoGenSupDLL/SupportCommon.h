#pragma once
// Reference: XYZware_Nobel_Series SVN13801

namespace XYZSupport
{
	enum class STATUS { Idle, Busy, Done, UserAbort };
	enum class COMMAND { None, Abort };
	enum class MeshTypeCode { Normal, Mark };

	enum class Diameter { SMALL = 2, MED = 4, LARGE = 5 };	// support contact size: small (diameter: 2.2, scale factor: (2.0mm/15) = 0.1333mm), contact size: 2.2 *0.1333 = 0.29326mm
	enum class PointEditModeCode {	Normal = 1, User, Cone, Tree, Depth, FacetToBase, Auto, Bed,// support points editor mode: normal (auto generation support points), User (manual edit support points)
									}; 
	enum class InternalSupportMode { Disable, Enable };
	enum class SymbolType { UnKnown = 0, NormalPoint, ConePoint, TreeTrunk, TreeBranch, 
							TreePoint,						// Charles: added in 1.7.0.14.
							};

	enum class ModelType
	{
		MODEL = 1,															                    // NOTE: Tim renmed BRANCH to BODY_NOCUP, TAIL to CYLINDER.
		MARK, HEAD, BODY, BODY_NOCUP, CYLINDER, FOOT,						// normal support	// XYZware Trunk: BRANCH (5), TAIL (6), FOOT (7) ===> C++: BODY_NOCUP (5), CYLINDER (6), FOOT (7)
		MARK_CONE, CONE,													// cone support
		MARK_BRANCH, SYMBOL_TRUNK, TREE_TRUNK, SYMBOL_BRANCH, TREE_BRANCH,	// tree support
		BED_CELL, // bed support 
		CUBOID_CELL, CUBOID_FOOT, // cuboid support
		TAB, BRIM
		//F2BARROW// facet to base	
	};

	enum ERROR_CODE
	{
		E_SUCCESS = 0,
		E_NO_INTIAL = -1,
		E_INVALID_INPUT = -2,
		E_FILE_ERROR = -3,
		E_NULL_PTR = -4,
		E_REENTRY = -5,
		E_ILLEGAL_CALL = -6,
		E_OTHER = -9, /* consider some way of returning the OS error too */
	};
}