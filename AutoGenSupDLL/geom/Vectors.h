#pragma once

///////////////////////////////////////////////////////////////////////////////
// Vectors.h
// =========
// 2D/3D/4D vectors
//
//  AUTHOR: Song Ho Ahn (song.ahn@gmail.com)
// CREATED: 2007-02-14
// UPDATED: 2016-04-04
//
// Copyright (C) 2007-2016 Song Ho Ahn
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <iostream>

using namespace std;

namespace XYZSupport
{
	///////////////////////////////////////////////////////////////////////////////
	// 2D vector 
	///////////////////////////////////////////////////////////////////////////////
	struct Vector2
	{
		float x;
		float y;

		// ctors
		Vector2() : x(0), y(0) {};
		Vector2(float x, float y) : x(x), y(y) {};

		// utils functions
		void        set(float x, float y);
		float       length() const;                         //
		float       distance(const Vector2& vec) const;     // distance between two vectors
		Vector2&    normalize();                            //
		float       dot(const Vector2& vec) const;          // dot product
		bool        equal(const Vector2& vec, float e) const; // compare with epsilon

		// operators
		Vector2     operator-() const;                      // unary operator (negate)
		Vector2     operator+(const Vector2& rhs) const;    // add rhs
		Vector2     operator-(const Vector2& rhs) const;    // subtract rhs
		Vector2&    operator+=(const Vector2& rhs);         // add rhs and update this object
		Vector2&    operator-=(const Vector2& rhs);         // subtract rhs and update this object
		Vector2     operator*(const float scale) const;     // scale
		Vector2     operator*(const Vector2& rhs) const;    // multiply each element
		Vector2&    operator*=(const float scale);          // scale and update this object
		Vector2&    operator*=(const Vector2& rhs);         // multiply each element and update this object
		Vector2     operator/(const float scale) const;     // inverse scale
		Vector2&    operator/=(const float scale);          // scale and update this object
		bool        operator==(const Vector2& rhs) const;   // exact compare, no epsilon
		bool        operator!=(const Vector2& rhs) const;   // exact compare, no epsilon
		bool        operator<(const Vector2& rhs) const;    // comparison for sort
		float       operator[](int index) const;            // subscript operator v[0], v[1]
		float&      operator[](int index);                  // subscript operator v[0], v[1]

		friend Vector2 operator*(const float a, const Vector2 vec);
		friend std::ostream& operator<<(std::ostream& os, const Vector2& vec);

		string ToString() const
		{
			return "(" + std::to_string(x) + " " + std::to_string(y) + ")";
		}
	};

	struct Vector2d
	{
		double x;
		double y;

		// ctors
		Vector2d() : x(0), y(0) {};
		Vector2d(double x, double y) : x(x), y(y) {};

		// utils functions
		void        set(double x, double y);
		double       length() const;                         //
		double       distance(const Vector2d& vec) const;    // distance between two vectors
		Vector2d&    normalize();                            //
		double       dot(const Vector2d& vec) const;         // dot product
		bool        equal(const Vector2d& vec, double e) const; // compare with epsilon

		// operators
		Vector2d     operator-() const;                      // unary operator (negate)
		Vector2d     operator+(const Vector2d& rhs) const;   // add rhs
		Vector2d     operator-(const Vector2d& rhs) const;   // subtract rhs
		Vector2d&    operator+=(const Vector2d& rhs);        // add rhs and update this object
		Vector2d&    operator-=(const Vector2d& rhs);        // subtract rhs and update this object
		Vector2d     operator*(const double scale) const;    // scale
		Vector2d     operator*(const Vector2d& rhs) const;   // multiply each element
		Vector2d&    operator*=(const double scale);         // scale and update this object
		Vector2d&    operator*=(const Vector2d& rhs);        // multiply each element and update this object
		Vector2d     operator/(const double scale) const;    // inverse scale
		Vector2d&    operator/=(const double scale);         // scale and update this object
		bool        operator==(const Vector2d& rhs) const;   // exact compare, no epsilon
		bool        operator!=(const Vector2d& rhs) const;   // exact compare, no epsilon
		bool        operator<(const Vector2d& rhs) const;    // comparison for sort
		double       operator[](int index) const;            // subscript operator v[0], v[1]
		double&      operator[](int index);                  // subscript operator v[0], v[1]

		friend Vector2d operator*(const double a, const Vector2d vec);
		friend std::ostream& operator<<(std::ostream& os, const Vector2d& vec);
	};

	///////////////////////////////////////////////////////////////////////////////
	// 3D vector
	///////////////////////////////////////////////////////////////////////////////
	struct Vector3
	{
		float x;
		float y;
		float z;

		// ctors
		Vector3() : x(0), y(0), z(0) {};
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {};

		// utils functions
		void        set(float x, float y, float z);
		float       length() const;                         //
		float       distance(const Vector3& vec) const;     // distance between two vectors
		float       angle(const Vector3& vec) const;        // angle between two vectors
		Vector3&    normalize();                            //
		float       dot(const Vector3& vec) const;          // dot product
		Vector3     cross(const Vector3& vec) const;        // cross product
		bool        equal(const Vector3& vec, float e) const; // compare with epsilon

		// operators
		Vector3     operator-() const;                      // unary operator (negate)
		Vector3     operator+(const Vector3& rhs) const;    // add rhs
		Vector3     operator-(const Vector3& rhs) const;    // subtract rhs
		Vector3&    operator+=(const Vector3& rhs);         // add rhs and update this object
		Vector3&    operator-=(const Vector3& rhs);         // subtract rhs and update this object
		Vector3     operator*(const float scale) const;     // scale
		Vector3     operator*(const Vector3& rhs) const;    // multiplay each element
		Vector3&    operator*=(const float scale);          // scale and update this object
		Vector3&    operator*=(const Vector3& rhs);         // product each element and update this object
		Vector3     operator/(const float scale) const;     // inverse scale
		Vector3&    operator/=(const float scale);          // scale and update this object
		bool        operator==(const Vector3& rhs) const;   // exact compare, no epsilon
		bool        operator!=(const Vector3& rhs) const;   // exact compare, no epsilon
		bool        operator<(const Vector3& rhs) const;    // comparison for sort
		float       operator[](int index) const;            // subscript operator v[0], v[1]
		float&      operator[](int index);                  // subscript operator v[0], v[1]

		friend Vector3 operator*(const float a, const Vector3 vec);
		friend std::ostream& operator<<(std::ostream& os, const Vector3& vec);
	};

	struct HashVector3
	{
		size_t operator()(const Vector3 &obj) const
		{
			return static_cast<size_t>(obj.x * 5915587277 + obj.y * 1500450271 + obj.z * 3267000013);
		}
	};


	///////////////////////////////////////////////////////////////////////////////
	// 4D vector
	///////////////////////////////////////////////////////////////////////////////
	struct Vector4
	{
		float x;
		float y;
		float z;
		float w;

		// ctors
		Vector4() : x(0), y(0), z(0), w(0) {};
		Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {};

		// utils functions
		void        set(float x, float y, float z, float w);
		float       length() const;                         //
		float       distance(const Vector4& vec) const;     // distance between two vectors
		Vector4&    normalize();                            //
		float       dot(const Vector4& vec) const;          // dot product
		bool        equal(const Vector4& vec, float e) const; // compare with epsilon

		// operators
		Vector4     operator-() const;                      // unary operator (negate)
		Vector4     operator+(const Vector4& rhs) const;    // add rhs
		Vector4     operator-(const Vector4& rhs) const;    // subtract rhs
		Vector4&    operator+=(const Vector4& rhs);         // add rhs and update this object
		Vector4&    operator-=(const Vector4& rhs);         // subtract rhs and update this object
		Vector4     operator*(const float scale) const;     // scale
		Vector4     operator*(const Vector4& rhs) const;    // multiply each element
		Vector4&    operator*=(const float scale);          // scale and update this object
		Vector4&    operator*=(const Vector4& rhs);         // multiply each element and update this object
		Vector4     operator/(const float scale) const;     // inverse scale
		Vector4&    operator/=(const float scale);          // scale and update this object
		bool        operator==(const Vector4& rhs) const;   // exact compare, no epsilon
		bool        operator!=(const Vector4& rhs) const;   // exact compare, no epsilon
		bool        operator<(const Vector4& rhs) const;    // comparison for sort
		float       operator[](int index) const;            // subscript operator v[0], v[1]
		float&      operator[](int index);                  // subscript operator v[0], v[1]

		friend Vector4 operator*(const float a, const Vector4 vec);
		friend std::ostream& operator<<(std::ostream& os, const Vector4& vec);

		string ToString() const
		{
			return to_string(x) + " " + to_string(y) + " " + to_string(z) + " " + to_string(w);
		}
	};

	struct Vector4d
	{
		double x;
		double y;
		double z;
		double w;

		// ctors
		Vector4d() : x(0), y(0), z(0), w(0) {};
		Vector4d(double x, double y, double z, double w) : x(x), y(y), z(z), w(w) {};

		// utils functions
		void        set(double x, double y, double z, double w);
		double       length() const;                         //
		double       distance(const Vector4d& vec) const;     // distance between two vectors
		Vector4d&    normalize();                            //
		double       dot(const Vector4d& vec) const;          // dot product
		bool        equal(const Vector4d& vec, float e) const; // compare with epsilon

		// operators
		Vector4d     operator-() const;                      // unary operator (negate)
		Vector4d     operator+(const Vector4d& rhs) const;    // add rhs
		Vector4d     operator-(const Vector4d& rhs) const;    // subtract rhs
		Vector4d&    operator+=(const Vector4d& rhs);         // add rhs and update this object
		Vector4d&    operator-=(const Vector4d& rhs);         // subtract rhs and update this object
		Vector4d     operator*(const double scale) const;     // scale
		Vector4d     operator*(const Vector4d& rhs) const;    // multiply each element
		Vector4d&    operator*=(const double scale);          // scale and update this object
		Vector4d&    operator*=(const Vector4d& rhs);         // multiply each element and update this object
		Vector4d     operator/(const double scale) const;     // inverse scale
		Vector4d&    operator/=(const double scale);          // scale and update this object
		bool        operator==(const Vector4d& rhs) const;   // exact compare, no epsilon
		bool        operator!=(const Vector4d& rhs) const;   // exact compare, no epsilon
		bool        operator<(const Vector4d& rhs) const;    // comparison for sort
		double       operator[](int index) const;            // subscript operator v[0], v[1]
		double&      operator[](int index);                  // subscript operator v[0], v[1]

		friend Vector4d operator*(const double a, const Vector4d vec);
		friend std::ostream& operator<<(std::ostream& os, const Vector4d& vec);

		string ToString() const
		{
			return to_string(x) + " " + to_string(y) + " " + to_string(z) + " " + to_string(w);
		}
	};


	// fast math routines from Doom3 SDK
	inline float invSqrt(float x)
	{
		float xhalf = 0.5f * x;
		int i = *(int*)&x;          // get bits for floating value
		i = 0x5f3759df - (i >> 1);    // gives initial guess
		x = *(float*)&i;            // convert bits back to float
		x = x * (1.5f - xhalf * x*x); // Newton step
		return x;
	}

	///////////////////////////////////////////////////////////////////////////////
	// inline functions for Vector2
	///////////////////////////////////////////////////////////////////////////////
	inline Vector2 Vector2::operator-() const {
		return Vector2(-x, -y);
	}

	inline Vector2 Vector2::operator+(const Vector2& rhs) const {
		return Vector2(x + rhs.x, y + rhs.y);
	}

	inline Vector2 Vector2::operator-(const Vector2& rhs) const {
		return Vector2(x - rhs.x, y - rhs.y);
	}

	inline Vector2& Vector2::operator+=(const Vector2& rhs) {
		x += rhs.x; y += rhs.y; return *this;
	}

	inline Vector2& Vector2::operator-=(const Vector2& rhs) {
		x -= rhs.x; y -= rhs.y; return *this;
	}

	inline Vector2 Vector2::operator*(const float a) const {
		return Vector2(x*a, y*a);
	}

	inline Vector2 Vector2::operator*(const Vector2& rhs) const {
		return Vector2(x*rhs.x, y*rhs.y);
	}

	inline Vector2& Vector2::operator*=(const float a) {
		x *= a; y *= a; return *this;
	}

	inline Vector2& Vector2::operator*=(const Vector2& rhs) {
		x *= rhs.x; y *= rhs.y; return *this;
	}

	inline Vector2 Vector2::operator/(const float a) const {
		return Vector2(x / a, y / a);
	}

	inline Vector2& Vector2::operator/=(const float a) {
		x /= a; y /= a; return *this;
	}

	inline bool Vector2::operator==(const Vector2& rhs) const {
		return (x == rhs.x) && (y == rhs.y);
	}

	inline bool Vector2::operator!=(const Vector2& rhs) const {
		return (x != rhs.x) || (y != rhs.y);
	}

	inline bool Vector2::operator<(const Vector2& rhs) const {
		if (x < rhs.x) return true;
		if (x > rhs.x) return false;
		if (y < rhs.y) return true;
		if (y > rhs.y) return false;
		return false;
	}

	inline float Vector2::operator[](int index) const {
		return (&x)[index];
	}

	inline float& Vector2::operator[](int index) {
		return (&x)[index];
	}

	inline void Vector2::set(float x, float y) {
		this->x = x; this->y = y;
	}

	inline float Vector2::length() const {
		return sqrtf(x*x + y * y);
	}

	inline float Vector2::distance(const Vector2& vec) const {
		return sqrtf((vec.x - x)*(vec.x - x) + (vec.y - y)*(vec.y - y));
	}

	inline Vector2& Vector2::normalize() {
		//@@const float EPSILON = 0.000001f;
		float xxyy = x * x + y * y;
		//@@if(xxyy < EPSILON)
		//@@    return *this;

		//float invLength = invSqrt(xxyy);
		float invLength = 1.0f / sqrtf(xxyy);
		x *= invLength;
		y *= invLength;
		return *this;
	}

	inline float Vector2::dot(const Vector2& rhs) const {
		return (x*rhs.x + y * rhs.y);
	}

	inline bool Vector2::equal(const Vector2& rhs, float epsilon) const {
		return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon;
	}

	inline Vector2 operator*(const float a, const Vector2 vec) {
		return Vector2(a*vec.x, a*vec.y);
	}

	inline std::ostream& operator<<(std::ostream& os, const Vector2& vec) {
		os << "(" << vec.x << ", " << vec.y << ")";
		return os;
	}
	// END OF VECTOR2 /////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// inline functions for Vector2d
	///////////////////////////////////////////////////////////////////////////////
	inline Vector2d Vector2d::operator-() const {
		return Vector2d(-x, -y);
	}

	inline Vector2d Vector2d::operator+(const Vector2d& rhs) const {
		return Vector2d(x + rhs.x, y + rhs.y);
	}

	inline Vector2d Vector2d::operator-(const Vector2d& rhs) const {
		return Vector2d(x - rhs.x, y - rhs.y);
	}

	inline Vector2d& Vector2d::operator+=(const Vector2d& rhs) {
		x += rhs.x; y += rhs.y; return *this;
	}

	inline Vector2d& Vector2d::operator-=(const Vector2d& rhs) {
		x -= rhs.x; y -= rhs.y; return *this;
	}

	inline Vector2d Vector2d::operator*(const double a) const {
		return Vector2d(x*a, y*a);
	}

	inline Vector2d Vector2d::operator*(const Vector2d& rhs) const {
		return Vector2d(x*rhs.x, y*rhs.y);
	}

	inline Vector2d& Vector2d::operator*=(const double a) {
		x *= a; y *= a; return *this;
	}

	inline Vector2d& Vector2d::operator*=(const Vector2d& rhs) {
		x *= rhs.x; y *= rhs.y; return *this;
	}

	inline Vector2d Vector2d::operator/(const double a) const {
		return Vector2d(x / a, y / a);
	}

	inline Vector2d& Vector2d::operator/=(const double a) {
		x /= a; y /= a; return *this;
	}

	inline bool Vector2d::operator==(const Vector2d& rhs) const {
		return (x == rhs.x) && (y == rhs.y);
	}

	inline bool Vector2d::operator!=(const Vector2d& rhs) const {
		return (x != rhs.x) || (y != rhs.y);
	}

	inline bool Vector2d::operator<(const Vector2d& rhs) const {
		if (x < rhs.x) return true;
		if (x > rhs.x) return false;
		if (y < rhs.y) return true;
		if (y > rhs.y) return false;
		return false;
	}

	inline double Vector2d::operator[](int index) const {
		return (&x)[index];
	}

	inline double& Vector2d::operator[](int index) {
		return (&x)[index];
	}

	inline void Vector2d::set(double x, double y) {
		this->x = x; this->y = y;
	}

	inline double Vector2d::length() const {
		return sqrt(x*x + y * y);
	}

	inline double Vector2d::distance(const Vector2d& vec) const {
		return sqrt((vec.x - x)*(vec.x - x) + (vec.y - y)*(vec.y - y));
	}

	inline Vector2d& Vector2d::normalize() {
		//@@const float EPSILON = 0.000001f;
		double xxyy = x * x + y * y;
		//@@if(xxyy < EPSILON)
		//@@    return *this;

		//float invLength = invSqrt(xxyy);
		double invLength = 1.0 / sqrt(xxyy);
		x *= invLength;
		y *= invLength;
		return *this;
	}

	inline double Vector2d::dot(const Vector2d& rhs) const {
		return (x*rhs.x + y * rhs.y);
	}

	inline bool Vector2d::equal(const Vector2d& rhs, double epsilon) const {
		return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon;
	}

	inline Vector2d operator*(const double a, const Vector2d vec) {
		return Vector2d(a*vec.x, a*vec.y);
	}

	inline std::ostream& operator<<(std::ostream& os, const Vector2d& vec) {
		os << "(" << vec.x << ", " << vec.y << ")";
		return os;
	}
	// END OF VECTOR2d /////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// inline functions for Vector3
	///////////////////////////////////////////////////////////////////////////////
	inline Vector3 Vector3::operator-() const {
		return Vector3(-x, -y, -z);
	}

	inline Vector3 Vector3::operator+(const Vector3& rhs) const {
		return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
	}

	inline Vector3 Vector3::operator-(const Vector3& rhs) const {
		return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
	}

	inline Vector3& Vector3::operator+=(const Vector3& rhs) {
		x += rhs.x; y += rhs.y; z += rhs.z; return *this;
	}

	inline Vector3& Vector3::operator-=(const Vector3& rhs) {
		x -= rhs.x; y -= rhs.y; z -= rhs.z; return *this;
	}

	inline Vector3 Vector3::operator*(const float a) const {
		return Vector3(x*a, y*a, z*a);
	}

	inline Vector3 Vector3::operator*(const Vector3& rhs) const {
		return Vector3(x*rhs.x, y*rhs.y, z*rhs.z);
	}

	inline Vector3& Vector3::operator*=(const float a) {
		x *= a; y *= a; z *= a; return *this;
	}

	inline Vector3& Vector3::operator*=(const Vector3& rhs) {
		x *= rhs.x; y *= rhs.y; z *= rhs.z; return *this;
	}

	inline Vector3 Vector3::operator/(const float a) const {
		return Vector3(x / a, y / a, z / a);
	}

	inline Vector3& Vector3::operator/=(const float a) {
		x /= a; y /= a; z /= a; return *this;
	}

	inline bool Vector3::operator==(const Vector3& rhs) const {
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
	}

	inline bool Vector3::operator!=(const Vector3& rhs) const {
		return (x != rhs.x) || (y != rhs.y) || (z != rhs.z);
	}

	inline bool Vector3::operator<(const Vector3& rhs) const {
		if (x < rhs.x) return true;
		if (x > rhs.x) return false;
		if (y < rhs.y) return true;
		if (y > rhs.y) return false;
		if (z < rhs.z) return true;
		if (z > rhs.z) return false;
		return false;
	}

	inline float Vector3::operator[](int index) const {
		return (&x)[index];
	}

	inline float& Vector3::operator[](int index) {
		return (&x)[index];
	}

	inline void Vector3::set(float x, float y, float z) {
		this->x = x; this->y = y; this->z = z;
	}

	inline float Vector3::length() const {
		return sqrtf(x*x + y * y + z * z);
	}

	inline float Vector3::distance(const Vector3& vec) const {
		return sqrtf((vec.x - x)*(vec.x - x) + (vec.y - y)*(vec.y - y) + (vec.z - z)*(vec.z - z));
	}

	inline float Vector3::angle(const Vector3& vec) const {
		// return angle between [0, 180]
		float l1 = this->length();
		float l2 = vec.length();
		float d = this->dot(vec);
		float angle = acosf(d / (l1 * l2)) / 3.141592f * 180.0f;
		return angle;
	}

	inline Vector3& Vector3::normalize() {
		//@@const float EPSILON = 0.000001f;
		float xxyyzz = x * x + y * y + z * z;
		//@@if(xxyyzz < EPSILON)
		//@@    return *this; // do nothing if it is ~zero vector

		//float invLength = invSqrt(xxyyzz);
		float invLength = 1.0f / sqrtf(xxyyzz);
		x *= invLength;
		y *= invLength;
		z *= invLength;
		return *this;
	}

	inline float Vector3::dot(const Vector3& rhs) const {
		return (x*rhs.x + y * rhs.y + z * rhs.z);
	}

	inline Vector3 Vector3::cross(const Vector3& rhs) const {
		return Vector3(y*rhs.z - z * rhs.y, z*rhs.x - x * rhs.z, x*rhs.y - y * rhs.x);
	}

	inline bool Vector3::equal(const Vector3& rhs, float epsilon) const {
		return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon && fabs(z - rhs.z) < epsilon;
	}

	inline Vector3 operator*(const float a, const Vector3 vec) {
		return Vector3(a*vec.x, a*vec.y, a*vec.z);
	}

	inline std::ostream& operator<<(std::ostream& os, const Vector3& vec) {
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ")";
		return os;
	}
	// END OF VECTOR3 /////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// inline functions for Vector4
	///////////////////////////////////////////////////////////////////////////////
	inline Vector4 Vector4::operator-() const {
		return Vector4(-x, -y, -z, -w);
	}

	inline Vector4 Vector4::operator+(const Vector4& rhs) const {
		return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}

	inline Vector4 Vector4::operator-(const Vector4& rhs) const {
		return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	inline Vector4& Vector4::operator+=(const Vector4& rhs) {
		x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this;
	}

	inline Vector4& Vector4::operator-=(const Vector4& rhs) {
		x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this;
	}

	inline Vector4 Vector4::operator*(const float a) const {
		return Vector4(x*a, y*a, z*a, w*a);
	}

	inline Vector4 Vector4::operator*(const Vector4& rhs) const {
		return Vector4(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
	}

	inline Vector4& Vector4::operator*=(const float a) {
		x *= a; y *= a; z *= a; w *= a; return *this;
	}

	inline Vector4& Vector4::operator*=(const Vector4& rhs) {
		x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this;
	}

	inline Vector4 Vector4::operator/(const float a) const {
		return Vector4(x / a, y / a, z / a, w / a);
	}

	inline Vector4& Vector4::operator/=(const float a) {
		x /= a; y /= a; z /= a; w /= a; return *this;
	}

	inline bool Vector4::operator==(const Vector4& rhs) const {
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
	}

	inline bool Vector4::operator!=(const Vector4& rhs) const {
		return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w);
	}

	inline bool Vector4::operator<(const Vector4& rhs) const {
		if (x < rhs.x) return true;
		if (x > rhs.x) return false;
		if (y < rhs.y) return true;
		if (y > rhs.y) return false;
		if (z < rhs.z) return true;
		if (z > rhs.z) return false;
		if (w < rhs.w) return true;
		if (w > rhs.w) return false;
		return false;
	}

	inline float Vector4::operator[](int index) const {
		return (&x)[index];
	}

	inline float& Vector4::operator[](int index) {
		return (&x)[index];
	}

	inline void Vector4::set(float x, float y, float z, float w) {
		this->x = x; this->y = y; this->z = z; this->w = w;
	}

	inline float Vector4::length() const {
		return sqrtf(x*x + y * y + z * z + w * w);
	}

	inline float Vector4::distance(const Vector4& vec) const {
		return sqrtf((vec.x - x)*(vec.x - x) + (vec.y - y)*(vec.y - y) + (vec.z - z)*(vec.z - z) + (vec.w - w)*(vec.w - w));
	}

	inline Vector4& Vector4::normalize() {
		//NOTE: leave w-component untouched
		//@@const float EPSILON = 0.000001f;
		float xxyyzz = x * x + y * y + z * z;
		//@@if(xxyyzz < EPSILON)
		//@@    return *this; // do nothing if it is zero vector

		//float invLength = invSqrt(xxyyzz);
		float invLength = 1.0f / sqrtf(xxyyzz);
		x *= invLength;
		y *= invLength;
		z *= invLength;
		return *this;
	}

	inline float Vector4::dot(const Vector4& rhs) const {
		return (x*rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
	}

	inline bool Vector4::equal(const Vector4& rhs, float epsilon) const {
		return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon &&
			fabs(z - rhs.z) < epsilon && fabs(w - rhs.w) < epsilon;
	}

	inline Vector4 operator*(const float a, const Vector4 vec) {
		return Vector4(a*vec.x, a*vec.y, a*vec.z, a*vec.w);
	}

	inline std::ostream& operator<<(std::ostream& os, const Vector4& vec) {
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return os;
	}
	// END OF VECTOR4 /////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////
	// inline functions for Vector4d
	///////////////////////////////////////////////////////////////////////////////
	inline Vector4d Vector4d::operator-() const {
		return Vector4d(-x, -y, -z, -w);
	}

	inline Vector4d Vector4d::operator+(const Vector4d& rhs) const {
		return Vector4d(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
	}
	inline Vector4d Vector4d::operator-(const Vector4d& rhs) const {
		return Vector4d(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
	}

	inline Vector4d& Vector4d::operator+=(const Vector4d& rhs) {
		x += rhs.x; y += rhs.y; z += rhs.z; w += rhs.w; return *this;
	}

	inline Vector4d& Vector4d::operator-=(const Vector4d& rhs) {
		x -= rhs.x; y -= rhs.y; z -= rhs.z; w -= rhs.w; return *this;
	}

	inline Vector4d Vector4d::operator*(const double a) const {
		return Vector4d(x*a, y*a, z*a, w*a);
	}

	inline Vector4d Vector4d::operator*(const Vector4d& rhs) const {
		return Vector4d(x*rhs.x, y*rhs.y, z*rhs.z, w*rhs.w);
	}

	inline Vector4d& Vector4d::operator*=(const double a) {
		x *= a; y *= a; z *= a; w *= a; return *this;
	}

	inline Vector4d& Vector4d::operator*=(const Vector4d& rhs) {
		x *= rhs.x; y *= rhs.y; z *= rhs.z; w *= rhs.w; return *this;
	}

	inline Vector4d Vector4d::operator/(const double a) const {
		return Vector4d(x / a, y / a, z / a, w / a);
	}

	inline Vector4d& Vector4d::operator/=(const double a) {
		x /= a; y /= a; z /= a; w /= a; return *this;
	}

	inline bool Vector4d::operator==(const Vector4d& rhs) const {
		return (x == rhs.x) && (y == rhs.y) && (z == rhs.z) && (w == rhs.w);
	}

	inline bool Vector4d::operator!=(const Vector4d& rhs) const {
		return (x != rhs.x) || (y != rhs.y) || (z != rhs.z) || (w != rhs.w);
	}

	inline bool Vector4d::operator<(const Vector4d& rhs) const {
		if (x < rhs.x) return true;
		if (x > rhs.x) return false;
		if (y < rhs.y) return true;
		if (y > rhs.y) return false;
		if (z < rhs.z) return true;
		if (z > rhs.z) return false;
		if (w < rhs.w) return true;
		if (w > rhs.w) return false;
		return false;
	}

	inline double Vector4d::operator[](int index) const {
		return (&x)[index];
	}

	inline double& Vector4d::operator[](int index) {
		return (&x)[index];
	}

	inline void Vector4d::set(double x, double y, double z, double w) {
		this->x = x; this->y = y; this->z = z; this->w = w;
	}

	inline double Vector4d::length() const {
		return sqrt(x*x + y * y + z * z + w * w);
	}

	inline double Vector4d::distance(const Vector4d& vec) const {
		return sqrt((vec.x - x)*(vec.x - x) + (vec.y - y)*(vec.y - y) + (vec.z - z)*(vec.z - z) + (vec.w - w)*(vec.w - w));
	}

	inline Vector4d& Vector4d::normalize() {
		//NOTE: leave w-component untouched
		//@@const float EPSILON = 0.000001f;
		double xxyyzz = x * x + y * y + z * z;
		//@@if(xxyyzz < EPSILON)
		//@@    return *this; // do nothing if it is zero vector

		//float invLength = invSqrt(xxyyzz);
		double invLength = 1.0 / sqrt(xxyyzz);
		x *= invLength;
		y *= invLength;
		z *= invLength;
		return *this;
	}

	inline double Vector4d::dot(const Vector4d& rhs) const {
		return (x*rhs.x + y * rhs.y + z * rhs.z + w * rhs.w);
	}

	inline bool Vector4d::equal(const Vector4d& rhs, float epsilon) const {
		return fabs(x - rhs.x) < epsilon && fabs(y - rhs.y) < epsilon &&
			fabs(z - rhs.z) < epsilon && fabs(w - rhs.w) < epsilon;
	}

	inline Vector4d operator*(const double a, const Vector4d vec) {
		return Vector4d(a*vec.x, a*vec.y, a*vec.z, a*vec.w);
	}

	inline std::ostream& operator<<(std::ostream& os, const Vector4d& vec) {
		os << "(" << vec.x << ", " << vec.y << ", " << vec.z << ", " << vec.w << ")";
		return os;
	}
	// END OF VECTOR4d /////////////////////////////////////////////////////////////
}