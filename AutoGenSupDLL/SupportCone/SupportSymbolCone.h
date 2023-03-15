#pragma once
#include "SupportSymbol.h"

namespace XYZSupport
{
	class SupportSymbolCone : public SupportSymbol
	{
	public:
		double DepthAdjust;                 //Collision Depth Adjust slider value
		bool DepthAdjustSelected;           //symbol selected for depth adjust indicator   
	
	private:
		RHVector3 _position;
		RHVector3 _direction;
		double _radius_top;
		double _radius_base;

	public:
		RHVector3 GetPosition() const { return _position; }
		void SetPosition(RHVector3 value) { _position = value; }

		RHVector3 GetDirection() const { return _direction; }
		void SetDirection(RHVector3 value) { _direction = value; }

		double GetRadiusTop() const { return _radius_top; }
		void SetRadiusTop(double value) { _radius_top = value; }
		
		double GetRadiusBase() const { return _radius_base; }
		void SetRadiusBase(double value) { _radius_base = value; }

		SupportSymbolCone()
		{
			_position = RHVector3(0, 0, 0);
			_direction = RHVector3(0, 0, 0);
			_radius_top = 0;
			_radius_base = 0;
			DepthAdjust = 0;

			Type = SymbolType::ConePoint;
		}

		SupportSymbolCone(const SupportSymbolCone &symbol)
		{
			_position = symbol._position;
			_direction = symbol._direction;
			_radius_top = symbol._radius_top;
			_radius_base = symbol._radius_base;
			DepthAdjust = symbol.DepthAdjust;
		}

		SupportSymbolCone(RHVector3 pos, RHVector3 dir, double radius_top, double radius_base)
		{
			_position = pos;
			_direction = dir;
			_radius_top = radius_top;
			_radius_base = radius_base;
		}

		SupportSymbolCone(Vector3 pos, Vector3 dir, double radius_top, double radius_base)
		{
			_position = RHVector3(pos);
			_direction = RHVector3(dir);
			_radius_top = radius_top;
			_radius_base = radius_base;
		}

		bool IsNearby(RHVector3 pointIn)
		{
			return false;
		}

		bool IsCloseTo(const SupportSymbol &item)
		{
			return false;
		}

		bool IsIdentical(const SupportSymbol &item)
		{
			if (_position == item.GetPosition() &&
				_direction == item.GetDirection() &&
				_radius_base == item.GetRadiusBase() &&
				_radius_top == item.GetRadiusTop())
				return true;
			return false;
		}

		/// <summary>
		 /// Calculate the new position of support points when the object translates. 
		 /// </summary>
		 /// <param name="moveX">distance to translate along x-axis direction</param>
		 /// <param name="moveY">distance to translate along x-axis direction</param>
		 /// <param name="moveZ">distance to translate along x-axis direction</param>
		void Translate(double moveX, double moveY, double moveZ)
		{
			_position.x += moveX;
			_position.y += moveY;
			_position.z += moveZ;
		}

		/// <summary>
		/// Calculate the new position of support points when the object rotates. 
		/// </summary>
		/// <param name="zRot">angle in degree to rotate counter-clockwise around z-axis</param>
		//void RotateZ(RHVector3 point, double zRot)
		//{
		//	Matrix4 rotMatZ = Matrix4.CreateRotationZ(((float)-zRot) * (float)Math.PI / 180.0f);
		//	Vector4 relativePos = Vector4((float)(_position.x - point.x), (float)(_position.y - point.y), (float)(_position.z - point.z), 0);
		//	Vector4 rotRelativePos = Vector4.Transform(relativePos, rotMatZ);
		//	_position.x = rotRelativePos.x + point.x;
		//	_position.y = rotRelativePos.y + point.y;
		//	_position.z = rotRelativePos.z + point.z;
		//}

		//UInt32 SavedEntrySize
		//{
		//	get { return MeshInOut.Format3ws.SupportSection.UnitSize.SupCone; }
		//}

			/// <summary>
			/// save cone support position
			/// </summary>
			/// <param name="w">output binary stream</param>
			/// <param name="center">center of model</param>
			/// <remarks>
			///         foreach cone support
			///         REAL64 - Top diameter size
			///         REAL64 - Bottom diameter size
			///         REAL64 - Embedded depth
			///         REAL32[3] ¡V Contact position
			///         REAL32[3] ¡V Normal vector of contact point
			///         UINT16 ¡V Attribute byte count
			///         end
			/// </remarks>
		//void Write(BinaryWriter w, RHVector3 center)
		//{
		//	w.Write((double)RadiusTop);
		//	w.Write((double)RadiusBase);
		//	w.Write((double)DepthAdjust);
		//	w.Write((float)(_position.x - center.x));
		//	w.Write((float)(_position.y - center.y));
		//	w.Write((float)(_position.z - center.z));
		//	w.Write((float)_direction.x);
		//	w.Write((float)_direction.y);
		//	w.Write((float)_direction.z);
		//	w.Write((short)0);
		//}

		//void Read(BinaryReader r, RHVector3 center)
		//{
		//	RadiusTop = r.ReadDouble();
		//	RadiusBase = r.ReadDouble();
		//	DepthAdjust = r.ReadDouble();
		//	_position.x = r.ReadSingle() + center.x;
		//	_position.y = r.ReadSingle() + center.y;
		//	_position.z = r.ReadSingle() + center.z;
		//	_direction.x = r.ReadSingle();
		//	_direction.y = r.ReadSingle();
		//	_direction.z = r.ReadSingle();
		//	r.ReadUInt16();
		//}
	};
}
