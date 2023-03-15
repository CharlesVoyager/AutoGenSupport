#pragma once

#include "SupportMeshsBase.h"
#include "DataCombination.h"
#include "CubeMatrix.h"
#include "GenBody.h"
#include "GenBranch.h"
#include "SupportReferenceMesh.h"
#include "SupportPoint.h"
#include "SupportSymbolCone.h"

namespace XYZSupport
{
	class SupportMeshsCylinder : public SupportMeshsBase
	{
		GenBody genBody;
		//CubeUpdate cubeUpdate;	// C++ AP: Don't need. Direct call static CubeUpdate function.
		GenBranch genBranch; 
		//public InternalSupportMode InternalSupport{ get; set; }

		SupportReferenceMesh ReferenceMesh;	// BIG CLASS!!! 放在這裡. SupportMeshsCylinder class建立時後, 此物件建立一次. 不要放在function裡當local object, 造成每次使用此SupportReferenceMesh物件, 都要再一次生成!!!

	private:
		const int MODEL_SCALE = 15; // value 15, if support diameter is 2.0mm, then scale to 2.0mm/15 = 0.1333mm
		const int SUP_DIAMETER_DEFAULT = (int)Diameter::SMALL;
		const double SCALE_FACTOR_DEFAULT = (double)SUP_DIAMETER_DEFAULT / (double)MODEL_SCALE;  // value 0.1333, if support diameter is 2.0, then scale factor is 2.0/15 = 0.1333    

		////////////////////////////////////newsup ///////////////////////////////////////
		RHVector3 supHeadend;
		Triple<RHVector3, RHVector3, CubeNum> SupHeadStartEnd;
		vector<Triple<RHVector3, RHVector3, CubeNum>> SupHeadStartEndList;
		vector<Triple<RHVector3, RHVector3, CubeNum>> BranchList;
		//public List<Quadruple<RHVector3, RHVector3, RHVector3, int[]>> SupMainBodyStartEndList;
		vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> SupSubBodyStartEndList;
		vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> InnerSupBodyList;
		vector<Quadruple<RHVector3, RHVector3, RHVector3, CubeNum>> SingleInnerSupBodyList;

	public:
		vector<Tuple<RHVector3, CubeNum>> ThinBodyList;

	private:
		vector<Triple<RHVector3, RHVector3, CubeNum>> MergeBranchList;
		vector<Triple<RHVector3, RHVector3, shared_ptr<SupportData>>> SupTailStartEndList;

	public:
		vector<Triple<RHVector3, CubeNum, bool>> HaveTopGenInnerSupportList;
		vector<Tuple<RHVector3, double>> HaveTopGenTailList;
	private:
		vector<Tuple<RHVector3, RHVector3>> CandidateInnerSupportPos;
		vector<Tuple<RHVector3, CubeNum>> NewMergePointList;
		vector<Tuple<RHVector3, CubeNum>> FirmBranchPointList;	// Don't use vector<Tuple<RHVector3, int*>>. Becuase a local array point in a function cannot pass to out of function, ...
		//public int minisupNum;

	public:
		float Largespan = 2;//cube邊長//change span 
		float Smallspan = 1;//cube邊長//change span 
		////////////////////////////////////newsup ///////////////////////////////////////
		CubeMatrix cubeSpaceforNewsup;			// = null;//newsup			//************ CubeMatrix for generating support *******************
		//CubeMatrix cubeSpaceforNewsupOrigrnal;// = null;//newsup			//C# AP: cubeSpaceforNewsupOrigrnal = cubeSpaceforNewsup, C++: Don't need this.
		vector<RHVector3> vtxPosWorldCor;
		vector<RHVector3> triNormalWorldCor;
		bool dirtySpaceInfo = true;

	protected:
		RHBoundingBox bbox;					// Bounding box of the model

	public:
		RHVector3 OriginalBboxMax;
		RHVector3 OriginalBboxMin;
		CubeMatrix cubeSpace;				// C# AP: = null;

	private:
		const int num_phi = 12;				// 12
		const int num_theta = 12;			// 12
		const int num_sides = 12;			// 12
		const double refAngle45 = 0.7854;	// = 0.7854; // minimum reference angle for a good branch support
		int baseType = 0;
		float newBaseZScale = 0;
		bool enableEasyReleaseTab = true;
		int haveBrim = 0;

#if DEBUG_SUPPORT_ADD_FAILURE
		private List<RHVector3> infoSupHeadDropList = new List<RHVector3>();// for debug
		private List<RHVector3> infoSupBodyDropList = new List<RHVector3>();// for debug
		private List<RHVector3> infoSupFailList = new List<RHVector3>();	// for debug
		private List<RHVector3> infoSupPtAdjList = new List<RHVector3>();	// for debug
#endif

	private:
		void includePoint(RHVector3 v);
		void calVtxWorldCoordinate();
		void ExtraBboxforGenSupport(double ExtraSpace, RHBoundingBox &bbox, const RHBoundingBox &BoundingBoxWOSupport);
	
		Triple<RHVector3, RHVector3, CubeNum> CalSupportHeadPositionforNewsup(double supRadius, RHVector3 supTouchWor, RHVector3 supTouchNor, CubeMatrix &modelcube);	//newsup
		void GenNeedleSup(double supRadius, unique_ptr<SupportData> &FistHead);
		void GenSupHead(Tuple<RHVector3, CubeNum> SupportHeadEndandCubeNum, double supRadius, RHVector3 FirstHeadPosition);
		
		void SetCubeCenterpoint(float nspan);			//newsup
		void UpdateInfoforConnectPlateform(float nspan);//newsup

		vector<Triple<RHVector3, double, CubeNum>> CalSupportHead(RHVector3 supTouchWorCor, RHVector3 supTouchNor, CubeMatrix &modelcube, float span, CubeNum num);//newsup
		vector<Triple<RHVector3, double, CubeNum>> CalHeadAngle(	CubeMatrix &modelcube, RHVector3 testvector, RHVector3 supTouchNor, RHVector3 supTouchWorCor, 
																	int a, int b, int c, 
																	vector<Triple<RHVector3, double, CubeNum>> &list);	//newsup
		Tuple<RHVector3, CubeNum> ChooseHeadCube(vector<Triple<RHVector3, double, CubeNum>> SupHeadList, CubeMatrix &modelcube, CubeNum num, RHVector3 supTouchWorCor);

		void CalCandidateTailPoint(int x, int y, int z, RHVector3 StarPoint);	// C# AP: has more two arguments - "CubeMatrix modelcube, bool Main", but it was never used.
		void GetPickTouchListforTail(CubeNum Startcube, RHVector3 pickStartWor, double normZThreshold, /*out*/vector<Tuple<RHVector3, RHVector3>> &InnerSuptouchNorWorList);
		void GenTailpath(RHVector3 StartWor, RHVector3 Vector, vector<Tuple<double, RHVector3>> deltaNorModelList, /*out*/RHVector3 &supTouchWor, /*out*/vector<Tuple<RHVector3, RHVector3>> &TempList);
		void CalDistanceandAngle(int i, vector<Tuple<RHVector3, RHVector3>> PathList, RHVector3 pickStartWor, /*out*/RHVector3 &Touch, /*out*/double& angleBetween);
		void AddGrabTail(RHVector3 Headend, CubeNum cubenum, bool Trident, CubeMatrix &modelcube);//newsup
		void ChecktoAddTail(vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> list, int t);

		void GenInnersupTail(int i, /*out*/RHVector3 &tree, /*out*/RHVector3 &branchSupOrigin, /*out*/unique_ptr<SupportData> &supMainBody);
		void GenInnresupBody(int i);
		void GenConnectBall(int i, /*out*/RHVector3 &origin, /*out*/unique_ptr<SupportData> &ConnectBall);
		void GenADDsupTail(int n);
		void GenADDsupBed(int n);
		void GenADDsupBrim();

		Triple<RHVector3, RHVector3, shared_ptr<SupportData>> GenTailConeforNewsup(RHVector3 StarPoint, double supRadius, vector<Tuple<RHVector3, RHVector3>> &List, CubeMatrix &modelcube);
		vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> FindDirecToGenGrabTailBody(int x, int y, int z, int a, int b, CubeMatrix& modelcube);
		void TestDirc(int x, int y, int z, int a, int b, bool PathCollision, CubeMatrix &modelcube, /*out*/vector<Quadruple<RHVector3, RHVector3, CubeNum, bool>> &GrabTailStartEnd);

		bool GenBackbone(Triple<RHVector3, RHVector3, bool> &BackboneList, const SupportPoint &Points, CubeMatrix &modelcube, vector<vector<Coord3D>> &OverhangPointList, vector<Tuple<RHVector3, RHVector3>> &checkTable);

		void CheckAddBackboneTail(Triple<RHVector3, RHVector3, bool> BackboneList, CubeMatrix &modelcube, SupportPoint Points, double bottomRadius);
		bool CheckPosition(int i, int j, CubeMatrix &modelcube);

		bool NeedGenBootomSup(SupportPoint &point, int i);
		bool AddMiniSup(RHVector3 StartPos, SupportPoint &point, unique_ptr<SupportData> &supBody);
		bool GetTouchPerpendicularRHVector3forLowsupport(SupportPoint point, CubeMatrix &cubeSpace, CubeData &modelData);
		void AddConnectPlateformSup(RHVector3 Starpoint, double supRadius, CubeMatrix &modelcube, float span);
		bool GetTouchPerpendicularRHVector3forCheckPoinDiatance(SupportPoint &point, const CubeMatrix &modelcube, /*out*/ bool &IsMiniSup);

		void UpdateSpacePartition();
		void BuildUniformGridSpaceStructureForCD(float span, double upperBound, CubeMatrix &cubeMatrix);	// ForCD: for cube data.
		void BuildUniformGridSpaceStructureForNewsup(/*in*/TopoModel &model, float span, double upperBound, RHBoundingBox &bbox, RHBoundingBox &BoundingBoxWOSupport, /*out*/CubeMatrix &cubeMatrixforNewsup);//newsup

		CubeNum CalCubeNumforPoint(RHVector3 FirstHeadPosition, const CubeMatrix &modelcube);
		bool GetTouchPerpendicularRHVector3forHead2(CubeNum Startcube, const CubeMatrix &modelcube, RHVector3 rotatedHeadCenterWor, RHVector3 Normal);

		void genSupFromList(const vector<unique_ptr<SupportSymbol>> &ptsCones);

		void UpdateConeSupportCubeInfo(const vector<unique_ptr<SupportSymbol>> &symbols);
		bool CheckConeDistance(SupportPoint &point, const vector<unique_ptr<SupportSymbol>> &symbols);

	private:
		RHBoundingBox& BoundingBoxWOSupport() { return bbox; }

	private:
		vector<unique_ptr<SupportSymbol>> coneSymbols;							// C++: Added for storing cone symbols.
		InternalSupportMode internalSupport = InternalSupportMode::Disable;		// added for c++ module
	
	public:
		SupportMeshsCylinder(PointEditModeCode _EditMode, bool bEnbale, float _NewBaseZScale, bool _EnableEasyReleaseTab, int BaseType, int HaveBrim, float PrinterWidth, float PrinterDepth)
		{
			editMode = _EditMode;
			internalSupport = static_cast<InternalSupportMode>(bEnbale);
			newBaseZScale = _NewBaseZScale;
			enableEasyReleaseTab = _EnableEasyReleaseTab;
			baseType = BaseType;
			haveBrim = HaveBrim;
			printerWidth = PrinterWidth;
			printerDepth = PrinterDepth;
		}
	public:
		void Generate2();									// C#: public override List<SupportData> Generate2(List<SupportSymbol> symbols, CubeMatrix cubeInfo)
		void GenMesh(shared_ptr<SupportData> &supportData);	// C++: Added for generating bed base.
		void RemoveTailSupportData();
		void AddSupportSymbolCone(unique_ptr<SupportSymbolCone> p)
		{
			coneSymbols.push_back(move(p));
		}
	};
}