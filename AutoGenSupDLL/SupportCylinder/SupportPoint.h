#pragma once
#include "SupportSymbol.h"
#include "CubeNum.h"

namespace XYZSupport
{
	class SupportPoint : public SupportSymbol
	{
	private:
		RHVector3 position;
		RHVector3 direction;
		double radius;
	
	public:
		bool edge;						//newsup        
		int level = 0;					//newsup
		RHVector3 HeadStart;			//newsup
		RHVector3 HeadEnd;				//newsup
		CubeNum HeadCubeNum;			//newsup
		RHVector3 MainBodyRelayPoint;	//newsup
		//public int[] MainBodyCubeNum = new int[3];//newsup
		RHVector3 MainBodyEndPoint;//newsup      
		//public RHVector3 MainTailPos;//newsup
		bool MiniSupport;//newsup       
		int TailCubeNum[3];//newsup
		
	public:
		RHVector3 GetPosition() const { return position; }
		void SetPosition(RHVector3 value) { position = value; }

		RHVector3 GetDirection() const { return direction; }
		RHVector3 &Direction() { return direction; }
		void SetDirection(RHVector3 value) { direction = value; }

		double GetTouchRadius() const { return radius; }
		void SetTouchRadius(double value) { radius = value; }

		string ToString() const
		{
			return position.ToString() + " " + direction.ToString() + " " + to_string(radius);
		}

		SupportPoint() 
		{
			Type = SymbolType::NormalPoint;
		}

		SupportPoint(const SupportPoint &pt)
		{
			position = pt.position;
			direction = pt.direction;
			radius = pt.radius;

			edge = pt.edge;//newsup
			HeadStart = pt.HeadStart;//newsup
			HeadEnd = pt.HeadEnd;//newsup
			level = pt.level;//newsup
			HeadCubeNum = pt.HeadCubeNum;

			MainBodyRelayPoint = pt.MainBodyRelayPoint;//newsup
		}

		SupportPoint(const RHVector3 &_pos, const RHVector3 &_dir, double _radius)
		{
			position = _pos;
			direction = _dir;
			radius = _radius;
		}

		SupportPoint(const Vector3 &_pos, const Vector3 &_dir, double _radius)
		{
			position = _pos;
			direction = _dir;
			radius = _radius;
		}
		void DirectionNormalizeSafe() { direction.NormalizeSafe(); }

		bool IsNearby(RHVector3 pointIn)
		{
			double epsilon = 0.01;

			if (position.Subtract(pointIn).Length() < epsilon)
				return true;

			return false;
		}

		void operator= (const SupportPoint &pt)
		{
			position = pt.position;
			direction = pt.direction;
			radius = pt.radius;

			edge = pt.edge;
			HeadStart = pt.HeadStart;
			HeadEnd = pt.HeadEnd;
			level = pt.level;
			HeadCubeNum = pt.HeadCubeNum;

			MainBodyRelayPoint = pt.MainBodyRelayPoint;
		}

	private:
		const double MIN_V_DISTANCE_BETWEEN_SUP = 0.2; // minimum vertical distance between support points
		const double MIN_H_DISTANCE_BETWEEN_SUP = 1.0; // minimum horizontal distance between support points

	public:
		bool IsCloseTo(const SupportSymbol& item)
		{
			SupportPoint *ptToCompare = (SupportPoint *)&item;

			if ((position.Subtract(ptToCompare->GetPosition()).Length() < MIN_H_DISTANCE_BETWEEN_SUP)
				&& abs(position.z - ptToCompare->GetPosition().z) < MIN_V_DISTANCE_BETWEEN_SUP)
			{
				return true;
			}
			return false;
		}

		bool IsIdentical(const SupportSymbol& item)
		{
			if ( position == ((SupportPoint *)&item)->GetPosition() )
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
			position.x += moveX;
			position.y += moveY;
			position.z += moveZ;
		}

		/// <summary>
		/// Calculate the new position of support points when the object rotates. 
		/// </summary>
		/// <param name="zRot">angle in degree to rotate counter-clockwise around z-axis</param>
		//void RotateZ(RHVector3 point, double zRot)
		//{
		//	Matrix4 rotMatZ = Matrix4.CreateRotationZ(((float)-zRot) * (float)Math.PI / 180.0f);
		//	Vector4 relativePos = new Vector4((float)(this.Position.x - point.x), (float)(this.Position.y - point.y), (float)(this.Position.z - point.z), 0);
		//	Vector4 rotRelativePos = Vector4.Transform(relativePos, rotMatZ);
		//	position.x = rotRelativePos.X + point.x;
		//	position.y = rotRelativePos.Y + point.y;
		//	position.z = rotRelativePos.Z + point.z;
		//	//Tim---20190314---Update the direction info(3ws generated support error)
		//	Vector4 orivector4 = new Vector4((float)this.Direction.x, (float)this.Direction.y, (float)this.Direction.z, 0);
		//	Vector4 newvector4 = Vector4.Transform(orivector4, rotMatZ);
		//	direction.x = newvector4.X;
		//	direction.y = newvector4.Y;
		//	direction.z = newvector4.Z;
		//}

		unsigned int SavedEntrySize()
		{
			//get { return MeshInOut.Format3ws.SupportSection.UnitSize.SupCyld; }
			return 0;
		}

		/// <summary>
		/// save cylinder support position
		/// </summary>
		/// <param name="w">output binary stream</param>
		/// <param name="center">center of model</param>
		/// <remarks>
		///         foreach support
		///         REAL32 - Contact point size
		///         REAL32[3] ¡V Vertex 1
		///         REAL32[3] ¡V Normal vector
		///         UINT16 ¡V Attribute byte count
		///         end
		/// </remarks>
	/*	void Write(BinaryWriter w, RHVector3 center)
		{
			w.Write((float)TouchRadius);
			w.Write((float)(Position.x - center.x));
			w.Write((float)(Position.y - center.y));
			w.Write((float)(Position.z - center.z));
			w.Write((float)Direction.x);
			w.Write((float)Direction.y);
			w.Write((float)Direction.z);
			w.Write((short)0);
		}

		void Read(BinaryReader r, RHVector3 center)
		{
			radius = r.ReadSingle();
			position.x = r.ReadSingle() + center.x;
			position.y = r.ReadSingle() + center.y;
			position.z = r.ReadSingle() + center.z;
			direction.x = r.ReadSingle();
			direction.y = r.ReadSingle();
			direction.z = r.ReadSingle();
			r.ReadUInt16();
		}*/
	};
}
