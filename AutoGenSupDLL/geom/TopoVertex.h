#pragma once
#include "RHVector3.h"
#include "Matrix4.h"

namespace XYZSupport
{
	class TopoVertex
	{
	public:
		RHVector3 pos;
		int id = 0;			// serial number: 0, 1, 2, 3,...

		TopoVertex() {}
		TopoVertex(int _id, const RHVector3 &_pos)
		{
			id = _id;
			pos = _pos;
		}
		TopoVertex(int _id, const RHVector3 &_pos, const Matrix4 &trans)
		{
			id = _id;
			pos = RHVector3(
				_pos.x*trans.Column0().x + _pos.y*trans.Column0().y + _pos.z*trans.Column0().z + trans.Column0().w,
				_pos.x*trans.Column1().x + _pos.y*trans.Column1().y + _pos.z*trans.Column1().z + trans.Column1().w,
				_pos.x*trans.Column2().x + _pos.y*trans.Column2().y + _pos.z*trans.Column2().z + trans.Column2().w
			);
		}
		bool operator== (const TopoVertex &comp) const
		{
			if (pos == comp.pos && id == comp.id)
				return true;
			else
				return false;
		}
		bool operator!= (const TopoVertex &comp) const
		{
			return !(*this == comp);
		}
		double distance(const TopoVertex &vertex)
		{
			return pos.Distance(vertex.pos);
		}
		double distance(const RHVector3 &vertex)
		{
			return pos.Distance(vertex);
		}
		string ToString() const
		{
			return  pos.ToString() + " (" + to_string(id) + ")";
		}
	};
}