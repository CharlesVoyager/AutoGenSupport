#pragma once

#include "SupportCommon.h"
#include "RHVector3.h"

namespace XYZSupport
{
    class SupportSymbol
	{
	public:
		SymbolType Type;
		bool SelectSymbol;		// inital value: false
		bool SymbolError;		// inital value: false

		// SupportPoint, SupportSymbolCone
		virtual RHVector3 GetPosition() const { return RHVector3(-1, -1, -1); }

		// SupportPoint, SupportSymbolCone
		virtual RHVector3 GetDirection() const { return RHVector3(-1, -1, -1); }
		
		// SupportPoint
		virtual double GetTouchRadius() const { return -1; }

		// SupportSymbolCone
		virtual double GetRadiusTop() const { return -1; }

		// SupportSymbolCone
		virtual double GetRadiusBase() const { return -1; }

		// SupportSymbolTree
		virtual RHVector3 GetRootPosition() const { return RHVector3(-1, -1, -1); }
		
		// SupportSymbolTree
		virtual RHVector3 GetTopPosition() const { return RHVector3(-1, -1, -1); }

		virtual ~SupportSymbol() {}

		virtual bool IsIdentical(const SupportSymbol &item) = 0;

		virtual bool IsCloseTo(const SupportSymbol &item) = 0;

		virtual bool IsNearby(RHVector3 pointIn) = 0;

		virtual void Translate(double moveX, double moveY, double moveZ) = 0;

		/* The followinh functions are never used.
		virtual void RotateZ(RHVector3 point, double zRot) = 0;
		virtual unsigned int SavedEntrySize();	
		virtual void Write(BinaryWriter w, RHVector3 center) = 0;
		virtual void Read(BinaryReader r, RHVector3 center) = 0;
		*/
	};
}