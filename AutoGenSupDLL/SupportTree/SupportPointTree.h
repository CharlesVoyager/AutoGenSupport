#pragma once
#include "SupportSymbol.h"
#include "RHVector3.h"

namespace XYZSupport
{
	class SupportPointTree : public SupportSymbol
	{
	private:
		RHVector3 _position;
		RHVector3 _direction;
		double _diameter;

	public:
		RHVector3 GetPosition() const { return _position;  }
		void SetPosition(RHVector3 value) { _position = value; }

		RHVector3 GetDirection() const { return _direction; }
		void SetDirection(RHVector3 value) { _direction = value; }

		double GetTouchDiameter() const { return _diameter; }
		void SetTouchDiameter(double value) { _diameter = value; }

	public:
		SupportPointTree() 
		{
			Type = SymbolType::TreePoint;
		}
		SupportPointTree(const SupportPointTree &pt)
		{
			_position = pt.GetPosition();
			_direction = pt.GetDirection();
			_diameter = pt.GetTouchDiameter();
			Type = pt.Type;
		}

	public:
		SupportPointTree(RHVector3 pos, RHVector3 dir, double diameter)
		{
			_position = pos;
			_direction = dir;
			_diameter = diameter;
		}

	public:
		SupportPointTree(Vector3 pos, Vector3 dir, double diameter)
		{
			_position = RHVector3(pos);
			_direction = RHVector3(dir);
			_diameter = diameter;
		}

	public:
		bool IsNearby(RHVector3 pointIn)
		{
			double epsilon = 0.01;

			if (_position.Subtract(pointIn).Length() < epsilon)
				return true;

			return false;
		}

		const double MIN_V_DISTANCE_BETWEEN_SUP = 0.2; // minimum vertical distance between support points
		const double MIN_H_DISTANCE_BETWEEN_SUP = 1.0; // minimum horizontal distance between support points

	public:
		bool IsCloseTo(const SupportSymbol &item)
		{
			SupportPointTree *ptToCompare = (SupportPointTree *)&item;

			if ((_position.Subtract(ptToCompare->GetPosition()).Length() < MIN_H_DISTANCE_BETWEEN_SUP)
				&& abs(_position.z - ptToCompare->GetPosition().z) < MIN_V_DISTANCE_BETWEEN_SUP)
			{
				return true;
			}
			return false;
		}

	public:
		bool IsIdentical(const SupportSymbol &item)
		{
			if (_position == ((SupportPointTree *)&item)->GetPosition())
				return true;
			return false;
		}

		/// <summary>
		/// Calculate the new position of support points when the object translates. 
		/// </summary>
		/// <param name="moveX">distance to translate along x-axis direction</param>
		/// <param name="moveY">distance to translate along x-axis direction</param>
		/// <param name="moveZ">distance to translate along x-axis direction</param>
	public:
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
	public:
		/*void RotateZ(RHVector3 point, Double zRot)
		{
			Matrix4 rotMatZ = Matrix4.CreateRotationZ(((float)-zRot) * (float)Math.PI / 180.0f);
			Vector4 relativePos = new Vector4((float)(this.Position.x - point.x), (float)(this.Position.y - point.y), (float)(this.Position.z - point.z), 0);
			Vector4 rotRelativePos = Vector4.Transform(relativePos, rotMatZ);
			this.Position.x = rotRelativePos.X + point.x;
			this.Position.y = rotRelativePos.Y + point.y;
			this.Position.z = rotRelativePos.Z + point.z;
		}*/

	};

}
