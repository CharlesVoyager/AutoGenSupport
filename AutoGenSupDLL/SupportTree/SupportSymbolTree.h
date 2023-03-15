#pragma once
#include "SupportSymbol.h"

namespace XYZSupport
{
	/*
		Trunk
		-------
		double DepthAdjust;
		RHVector3 DepthAdjustNor;
		bool DepthAdjustSelected;
		RHVector3 _position;			// Not used
		RHVector3 _direction;			// Not used
		RHVector3 _rootposition;		// 位置 with z = 0
		RHVector3 _topposition;			// 位置 with z = _length
		double _radius_top;
		double _radius_base;
		double _length;

		Branch
		-------
		double DepthAdjust;
		RHVector3 DepthAdjustNor;       // 通常(0,0,-1). 用來調整_rootposition的z. _rootposition.z = _radius_top * DepthAdjust.
		bool DepthAdjustSelected;
		RHVector3 _position;			// Trunk上面頂點位置
		RHVector3 _direction;			// Branch一端在模型的位置連到 Trunk上面頂點位置的方向
		RHVector3 _rootposition;		// Branch一端在模型的位置
		RHVector3 _topposition;			// 與_rootposition一樣但 z + _length
		double _radius_top;
		double _radius_base;
		double _length;
	*/
	class SupportSymbolTree : public SupportSymbol
	{
	public:
		double DepthAdjust;				// Collision Depth Adjust slider value
		RHVector3 DepthAdjustNor;       // to be used for depth adjust of branch support mesh
		bool DepthAdjustSelected;       // symbol selected for depth adjust indicator
	
	private:
		RHVector3 _position;
		RHVector3 _direction;
		RHVector3 _rootposition;
		RHVector3 _topposition;
		double _radius_top;
		double _radius_base;
		double _length;

	public:
		void ToDbgMsg() const
		{
			DbgMsg(__FUNCTION__"===> DepthAdjust: %f", DepthAdjust);
			DbgMsg(__FUNCTION__"===> DepthAdjustNor: %s", DepthAdjustNor.ToString().c_str());
			DbgMsg(__FUNCTION__"===> DepthAdjustSelected: %s", DepthAdjustSelected ? "true":"false");
			DbgMsg(__FUNCTION__"===> _position: %s", _position.ToString().c_str());
			DbgMsg(__FUNCTION__"===> _direction: %s", _direction.ToString().c_str());
			DbgMsg(__FUNCTION__"===> _rootposition: %s", _rootposition.ToString().c_str());
			DbgMsg(__FUNCTION__"===> _topposition: %s", _topposition.ToString().c_str());
			DbgMsg(__FUNCTION__"===> _radius_top: %f", _radius_top);
			DbgMsg(__FUNCTION__"===> _radius_base: %f", _radius_base);
			DbgMsg(__FUNCTION__"===> _length: %f", _length);
			DbgMsg(__FUNCTION__"===> Type: %d (3: Trunk, 4: Branch)", Type);
		}

	public:
		RHVector3 GetPosition() const { return _position; }
		void SetPosition(RHVector3 value) { _position = value; }

		RHVector3 GetDirection() const { return _direction; }
		void SetDirection(RHVector3 value) { _direction = value; }

		RHVector3 GetRootPosition() const { return _rootposition; }
		void SetRootPosition(RHVector3 value) { _rootposition = value; }

		RHVector3 GetTopPosition() const { return _topposition; }
		void SetTopPosition(RHVector3 value) { _topposition = value; }

		double GetRadiusTop() const { return _radius_top; }
		void SetRadiusTop(double value) { _radius_top = value; }
		
		double GetRadiusBase() const { return _radius_base; }
		void SetRadiusBase(double value) { _radius_base = value; }

		double GetLength() const { return _length; }
		void SetLength(double value) { _length = value; }

		SupportSymbolTree()
		{
			_position = RHVector3(0, 0, 0);
			_rootposition = RHVector3(0, 0, 0);
			_topposition = RHVector3(0, 0, 0);
			_direction = RHVector3(0, 0, 0);
			DepthAdjustNor = RHVector3(0, 0, 0);
			DepthAdjust = 0;
			_radius_top = 0;
			_radius_base = 0;
			_length = 0;
			Type = SymbolType::TreeTrunk;
			SelectSymbol = false;
			SymbolError = false;
		}

		SupportSymbolTree(const SupportSymbolTree &symbol)
		{
			_position = symbol._position;
			_rootposition = symbol._rootposition;
			_topposition = symbol._topposition;
			_direction = symbol._direction;
			DepthAdjustNor = symbol.DepthAdjustNor;
			DepthAdjust = symbol.DepthAdjust;
			_radius_top = symbol._radius_top;
			_radius_base = symbol._radius_base;
			_length = symbol._length;
			Type = symbol.Type;
			SelectSymbol = symbol.SelectSymbol;
			SymbolError = symbol.SymbolError;
		}

		SupportSymbolTree(RHVector3 root, RHVector3 top, RHVector3 pos, RHVector3 dir, double radius_top, double radius_base, double length)
		{
			_rootposition = root;
			_topposition = top;
			_position = pos;
			_direction = dir;
			_radius_top = radius_top;
			_radius_base = radius_base;
			_length = length;
		}

		SupportSymbolTree(RHVector3 root, RHVector3 top, RHVector3 pos, RHVector3 dir, double radius_top, double radius_base)
		{
			_rootposition = root;
			_topposition = top;
			_position = pos;
			_direction = dir;
			_radius_top = radius_top;
			_radius_base = radius_base;
		}
		SupportSymbolTree(Vector3 root, Vector3 top, Vector3 pos, Vector3 dir, double radius_top, double radius_base, double length)
		{
			_rootposition = RHVector3(root);
			_topposition = RHVector3(top);
			_direction = RHVector3(dir);
			_position = RHVector3(pos);
			_radius_top = radius_top;
			_radius_base = radius_base;
			_length = length;
		}

		SupportSymbolTree(Vector3 root, Vector3 top, Vector3 pos, Vector3 dir, double radius_top, double radius_base)
		{
			_rootposition = RHVector3(root);
			_topposition = RHVector3(top);
			_direction = RHVector3(dir);
			_position = RHVector3(pos);
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
			if (_rootposition == ((SupportSymbolTree *)&item)->GetRootPosition() &&
				_topposition == ((SupportSymbolTree *)&item)->GetTopPosition() &&
				_direction == ((SupportSymbolTree *)&item)->GetDirection() &&
				_radius_base == ((SupportSymbolTree *)&item)->GetRadiusBase() &&
				_radius_top == ((SupportSymbolTree *)&item)->GetRadiusTop())
			{
				if (((SupportSymbolTree *)&item)->Type == SymbolType::TreeTrunk)
				{
					if (_length == ((SupportSymbolTree *)&item)->_length)
						return true;
					else
						return false;
				}

				return true;
			}
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
			_rootposition.x += moveX;
			_rootposition.y += moveY;
			_rootposition.z += moveZ;

			_topposition.x += moveX;
			_topposition.y += moveY;
			_topposition.z += moveZ;
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
			///         REAL32[3] – Contact position
			///         REAL32[3] – Normal vector of contact point
			///         UINT16 – Attribute byte count
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
