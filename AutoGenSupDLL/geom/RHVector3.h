#pragma once
#include <string>
#include <cmath>
#include <algorithm>    // std::min
#include "Vectors.h"

using namespace std;

namespace XYZSupport
{
#if 0	// Vector2/Vector3/Vector4 are defined in the Vectors.h
	class Vector2
	{
	public:
		float x;
		float y;

		Vector2(float _x, float _y)
		{
			x = _x;
			y = _y;
		}
	};

	class Vector3
	{	
	public:
		float x;
		float y;
		float z;

		Vector3(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
	};

	class Vector4
	{
	public:
		float x;
		float y;
		float z;
		float w;

		Vector4(float _x, float _y, float _z, float _w)
		{
			x = _x;
			y = _y;
			z = _z;
			w = _w;
		}
		string ToString() { return "(" + to_string(x) + " " + to_string(y) + " " + to_string(z) + " " + to_string(w) + ")"; }
	};
#endif
	class Coord3D
	{
	public:
		float x, y, z;
		float inix, iniy, iniz;		//Edward Add new value
		Coord3D()
		{
			x = 0; y = 0; z = 0;
			inix = 0; iniy = 0; iniz = 0;
		}
		Coord3D(float _x, float _y, float _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		string ToString() const
		{
			return to_string(x) + " " + to_string(y) + " " + to_string(z);
		}
	};

	class RHVector3
	{
#if PRECISION_SINGLE
	public:
		float x = 0, y = 0, z = 0;
#else
	public:
		double x = 0, y = 0, z = 0;
#endif

	public:
		RHVector3() {}
		RHVector3(double _x, double _y, double _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		//RHVector3(float _x, float _y, float _z)	// Marked this constructor to avoid ambiguous declartion with RHVector3(double _x, double _y, double _z).
		//{
		//	x = _x;
		//	y = _y;
		//	z = _z;
		//}

		RHVector3(long long _x, long long _y, long long _z)
		{
			x = (double)_x;
			y = (double)_y;
			z = (double)_z;
		}
		RHVector3(int _x, int _y, int _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		RHVector3(int _x, int _y, double _z)
		{
			x = _x;
			y = _y;
			z = _z;
		}
		RHVector3(const RHVector3 &orig)
		{
			x = orig.x;
			y = orig.y;
			z = orig.z;
		}

		RHVector3(Vector3 orig)
		{
			x = orig.x;
			y = orig.y;
			z = orig.z;
		}

		RHVector3(Vector4 orig)
		{
			x = orig.x / orig.w;
			y = orig.y / orig.w;
			z = orig.z / orig.w;
		}

		void operator= (const RHVector3 &other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
		}

		bool operator== (const RHVector3 &compare) const
		{
#if 1
			double local = floor(x * 100000) * 5915587277 + floor(y * 100000) * 1500450271 + floor(z * 100000) * 3267000013;
			double temp = floor(compare.x * 100000) * 5915587277 + floor(compare.y * 100000) * 1500450271 + floor(compare.z * 100000) * 3267000013;

			if (local == temp)
				return true;
			else
				return false;
#else
			if (x == compare.x && y == compare.y && z == compare.z)
				return true;
			else
				return false;
#endif
		}

		bool operator!=(const RHVector3 &comp) const
		{
			return !(*this == comp);
		}
		bool operator<(const RHVector3 &comp) const
		{
			if (x == comp.x &&
				y == comp.y &&
				z == comp.z)
				return false;

			if (x == comp.x &&
				y == comp.y )
				return z < comp.z;

			if (x == comp.x )
				return y < comp.y;

			return x < comp.x;
		}
#if PRECISION_SINGLE
		Vector4 asVector4()
		{
			return new Vector4(x, y, z, 1);
		}

		Vector3 asVector3()
		{
			return new Vector3(x, y, z);
		}

		float Length() { return (float)sqrt(x*x + y * y + z * z); }	// 模長 => 向量的大小
#else
		Vector4* asVector4()
		{
			return new Vector4((float)x, (float)y, (float)z, 1);
		}

		Vector3* asVector3()
		{
			return new Vector3((float)x, (float)y, (float)z);
		}
		Vector4 toVector4() const
		{
			return Vector4((float)x, (float)y, (float)z, 1);
		}
		double Length()	const{ return sqrt( x * x + y * y + z * z);	}	 // 模長 => 向量的大小
#endif

		/// <summary>
		/// Scales the Vector3 to unit length.
		/// </summary>
		void NormalizeSafe()
		{
#if PRECISION_SINGLE
			float l = Length();
#else
			double l = Length();
#endif
			if (l == 0)
			{
				x = y = 0;
				z = 0;
			}
			else
			{
				x /= l;
				y /= l;
				z /= l;
			}
		}

		void StoreMinimum(const RHVector3 &vec)
		{
			x = min(x, vec.x);
			y = min(y, vec.y);
			z = min(z, vec.z);
		}

		void StoreMaximum(const RHVector3 &vec)
		{
			x = max(x, vec.x);
			y = max(y, vec.y);
			z = max(z, vec.z);
		}

#if PRECISION_SINGLE
		float Distance(RHVector3 point)
		{
			double dx = point.x - x;
			double dy = point.y - y;
			double dz = point.z - z;
			return (float)Math.Sqrt(dx * dx + dy * dy + dz * dz);
		}
#else
		double Distance(RHVector3 point)
		{
			double dx = point.x - x;
			double dy = point.y - y;
			double dz = point.z - z;
			return sqrt(dx * dx + dy * dy + dz * dz);
		}
#endif
		void Scale(double factor)
		{
			x *= (float)factor;
			y *= (float)factor;
			z *= (float)factor;
		}
		void Scale(float factor)
		{
			x *= factor;
			y *= factor;
			z *= factor;
		}

		double ScalarProduct(RHVector3 vector)
		{
			return x * vector.x + y * vector.y + z * vector.z;
		}

		double AngleForNormalizedVectors(RHVector3 direction)
		{
			return cos(ScalarProduct(direction));
		}

		double Angle(RHVector3 direction)
		{
			return cos(ScalarProduct(direction) / (Length()*direction.Length()));
		}

		RHVector3 Subtract(RHVector3 vector)
		{
			return RHVector3(x - vector.x, y - vector.y, z - vector.z);
		}

		RHVector3 Add(RHVector3 vector)
		{
			return RHVector3(x + vector.x, y + vector.y, z + vector.z);
		}

		void SubtractInternal(RHVector3 vector)
		{
			x -= vector.x;
			y -= vector.y;
			z -= vector.z;
		}

		void AddInternal(RHVector3 vector)
		{
			x += vector.x;
			y += vector.y;
			z += vector.z;
		}

		RHVector3 CrossProduct(RHVector3 vector)
		{
			return RHVector3(
				y*vector.z - z * vector.y,
				z*vector.x - x * vector.z,
				x*vector.y - y * vector.x);
		}

#if PRECISION_SINGLE
		float &operator[](int dimension)	// get and set
		{
			if (dimension == 0) return x;
			else if (dimension == 1) return y;
			else return z;
		}
#else
		double &operator[](int dimension)	// get and set
		{
			if (dimension == 0) return x;
			else if (dimension == 1) return y;
			else return z;
		}
#endif

		//--- MODEL_SLA
		bool Equals(RHVector3 compare)
		{
			if (x == compare.x && y == compare.y && z == compare.z)
				return true;
			else
				return false;
		}
		//---

		string ToString() const
		{
			return "(" + std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z) + ")";
		}
	};
}