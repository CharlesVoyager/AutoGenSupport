#pragma once
#include "RHVector3.h"
#include "SupportSymbol.h"
#include "TopoModel.h"
#include "Matrix4.h"
#include "Helper.h"

namespace XYZSupport
{
	class SupportData
	{
	private:
		Coord3D position;		// shift position		
		Coord3D scale;			// scaler magnitude		// EX: 1 1 1
		Coord3D rotation;		// rotate vector		// EX: 0 0 0

	public:
		int supType = 0;		// HEAD, BODY, ...
		RHVector3 orientation;	//
		TopoModel originalModel;// Charles: vertices data and triangles data for support
		Matrix4 curPos;			// x, y, z, w
		Matrix4 curPos2;
		Matrix4 trans;			// Charles: This will be transform matrix after UpdatedMatrix() called.

		//Coord3D OriginalPosition{ get; set; }		// C# AP: Not used.
		RHBoundingBox BoundingBox;					// ===> SupportMeshsMark: TreeSymbol, 
		//PointEditModeCode type;					// C# AP: Not used. 

		SupportSymbol *psymbol;						// 如果這個SupportData是Cone, 這個是儲存這個生成的cone對應的點. 如果這個SupportData不是cone, 那這個通常是null.
		SupportSymbol *pbranchpoint;				// ===> SupportMeshsMark

		string name = "Unknown";

		// properties used for .3ws file
	public:
		double depthAdjust = 0.5;
		double length = 0;
		double radius1 = 0;
		double radius2 = 0;
		Matrix4 transMatrix;						// Charles: This will be the same as "trans" with calling UpdateMatrix().								
		// end of .3ws file properties.

	public:
		SupportData()
		{
			position.x = NAN;
			position.y = NAN;
			position.z = NAN;

			scale.x = 1; scale.y = 1; scale.z = 1;
			orientation.x = 0; orientation.y = 0; orientation.z = -1;
			trans.set(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);	//Synced with C# XYZware.
		}

	public:
		Coord3D GetPosition() const { return position; }
		void SetPosition(Coord3D value)	{ position = value; }

		Coord3D GetScale() const { return scale; }
		void SetScale(Coord3D value) { scale = value; }

		Coord3D GetRotation() const { return rotation; }
		void SetRotation(Coord3D value ) { rotation = value; }

		double RotateModel()
		{
			// calculate the cross product of (0,0,-1) and supTouchNor
			RHVector3 vertical(0, 0, -1);
			orientation.NormalizeSafe();
			if (vertical.Equals(orientation)) return 0;
			RHVector3 cross = vertical.CrossProduct(orientation);
			// calculate the angle between (0,0,-1) and supTouchNor
			double innerProduct = vertical.ScalarProduct(orientation);
			double cosine = innerProduct / (vertical.Length() * orientation.Length());
			double angleBetween = acos(cosine); // unit: radian
			if (angleBetween != 0) // OpenGL library bug: when angle is zero, CreateFromAxisAngle will be NaN (Not a Number)
			{
				Matrix4 m4;
				curPos2 = curPos = m4.rotate_unit_radian((float)angleBetween, Vector3((float)cross.x, (float)cross.y, (float)cross.z));// the same as GL.Rotate!
			}
			curPos.transpose();
			curPos2.transpose();
			return angleBetween;
		}

		//void RotateZ2(double zRot)
		//{
		//	Matrix4 rotMatrix = Matrix4.CreateRotationZ(((float)-zRot) * (float)M_PI / 180.0f);
		//	Vector4 relativePos = Vector4(position.x - OriginalPosition.x, position.y - OriginalPosition.y, position.z - OriginalPosition.z, 0);
		//	Vector4 rotRelativePos = Vector4.Transform(relativePos, rotMatrix);
		//	position.x = rotRelativePos.X + OriginalPosition.x;
		//	position.y = rotRelativePos.Y + OriginalPosition.y;
		//	position.z += (float)zRot;
		//}

		void Translate(double moveX, double moveY, double moveZ)
		{
			position.x += (float)moveX;
			position.y += (float)moveY;
			position.z += (float)moveZ;
		}
		
		//bool IsIdentical(SupportData item)
		//{
		//	if (GetPosition() == item.GetPosition() &&
		//		orientation == item.orientation &&
		//		trans == item.trans)
		//		return true;

		//	return false;
		//}
		
		void Clear()
		{
			originalModel.Clear();
		}
		// Convert SupportData's position/scale/rotation/curPos2 to translation matrix for saving SupportData to a STL file.
		// Update SupportData's trans and curPos2!

		// Note 1. position.z: UI's Z mean bottom of the model, while the position argument's Z means center of the model.
		//                 EX: Model ===> Cube (10mm x 10mm x 10mm)
		//                     UI position: 64, 64, 20 ===> for ToTransMatrix's position argumenet should be 64, 64, 25.
		//      2. This function is equivalent to SupportData.UpdateMatrix().

		void UpdateMatrix()
		{
			if (isnan(position.x))	// It means that the client application calls AddSupportDataWOMesh().
			{
				trans = transMatrix;
				return;
			}
			float x = rotation.x;
			float y = rotation.y;
			float z = rotation.z;

			Matrix4 transl = Matrix4::CreateTranslation(position.x, position.y, position.z);

			Matrix4 identity;
			Matrix4 scaleM4 = identity.scale(scale.x != 0 ? scale.x : 1, scale.y != 0 ? scale.y : 1, scale.z != 0 ? scale.z : 1);
			identity.identity();

			Matrix4 rotx = identity.rotateX(x);	//unit: degree
			rotx.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationX().
			identity.identity();
			trans = rotx;

			Matrix4 roty = identity.rotateY(y);
			roty.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationY().
			identity.identity();
			trans *= roty;

			Matrix4 rotz = identity.rotateZ(z);
			rotz.invert();						//NOTE: Cpp Matrix4 rotation has inverse result comparing with C# Matrix4.CreateRotationZ().
			identity.identity();
			trans *= rotz;

			curPos2 = trans * curPos2;
			Matrix4 cT = curPos2;
			cT.transpose();
			trans = scaleM4 * cT;
			trans = trans * transl;
		
			transMatrix = trans;
		}

		//void RotateZ(RHVector3 point, double zRot)
		//{
		//	Matrix4 rotMatrix = Matrix4.CreateRotationZ(((float)-zRot) * (float)Math.PI / 180.0f);
		//	Vector4 relativePos = new Vector4((float)(this.Position.x - point.x), (float)(this.Position.y - point.y), (float)(this.Position.z - point.z), 0);
		//	Vector4 rotRelativePos = Vector4.Transform(relativePos, rotMatrix);
		//	position.x = (float)(rotRelativePos.X + point.x);
		//	position.y = (float)(rotRelativePos.Y + point.y);
		//	rotation.z += (float)zRot;
		//}

		void UpdateBoundingBox()
		{
			for (const TopoVertex &v : originalModel.vertices._v)
			{
				float x, y, z;
				Vector4 v4((float)v.pos.x, (float)v.pos.y, (float)v.pos.z, 1);
				x = trans.Column0().dot(v4);		
				y = trans.Column1().dot(v4);	
				z = trans.Column2().dot(v4);		
				BoundingBox.Add(RHVector3(x, y, z));
			}
		}

		void ToFile(const string filename)	// for debug purpose
		{
			TextFile textFile(filename);
			if (textFile.IsSuccess() == false)
				return;

			textFile.Write("position: %s\n", position.ToString().c_str());
			textFile.Write("scale: %s\n", scale.ToString().c_str());
			textFile.Write("rotation: %s\n", rotation.ToString().c_str());

			textFile.Write("supType: %d\n", supType);
			textFile.Write("orientation: %s\n", orientation.ToString().c_str());
			textFile.Write("curPos:\n");
			textFile.Write("%s\n", curPos.Row0().ToString().c_str());
			textFile.Write("%s\n", curPos.Row1().ToString().c_str());
			textFile.Write("%s\n", curPos.Row2().ToString().c_str());
			textFile.Write("%s\n", curPos.Row3().ToString().c_str());
			textFile.Write("curPos2:\n");
			textFile.Write("%s\n", curPos2.Row0().ToString().c_str());
			textFile.Write("%s\n", curPos2.Row1().ToString().c_str());
			textFile.Write("%s\n", curPos2.Row2().ToString().c_str());
			textFile.Write("%s\n", curPos2.Row3().ToString().c_str());
			textFile.Write("trans:\n");
			textFile.Write("%s\n", trans.Row0().ToString().c_str());
			textFile.Write("%s\n", trans.Row1().ToString().c_str());
			textFile.Write("%s\n", trans.Row2().ToString().c_str());
			textFile.Write("%s\n", trans.Row3().ToString().c_str());
			textFile.Write("BoundingBox: %s\n", BoundingBox.ToString().c_str());

			textFile.Write("Triangles Count: %d\n", originalModel.triangles.Count());
			textFile.Write("Mesh Size: %d\n", originalModel.Size());
		}
#if 0	// C# debug version.
		public void ToFile(string filename)
		{
			using (StreamWriter sw = new System.IO.StreamWriter(filename))
			{
				sw.WriteLine("position: " + position.x.ToString() + " " + position.y.ToString() + " " + position.z.ToString());
				sw.WriteLine("scale: " + scale.x.ToString() + " " + scale.y.ToString() + " " + scale.z.ToString());
				sw.WriteLine("rotation: " + rotation.x.ToString() + " " + rotation.y.ToString() + " " + rotation.z.ToString());

				sw.WriteLine("supType: " + supType.ToString());
				sw.WriteLine("orientation: " + orientation.ToString());
				sw.WriteLine("curPos: ");
				sw.WriteLine(curPos.Row0.ToString());
				sw.WriteLine(curPos.Row1.ToString());
				sw.WriteLine(curPos.Row2.ToString());
				sw.WriteLine(curPos.Row3.ToString());
				sw.WriteLine("curPos2:");
				sw.WriteLine(curPos2.Row0.ToString());
				sw.WriteLine(curPos2.Row1.ToString());
				sw.WriteLine(curPos2.Row2.ToString());
				sw.WriteLine(curPos2.Row3.ToString());

				sw.WriteLine("trans:");
				sw.WriteLine(trans.Row0.ToString());
				sw.WriteLine(trans.Row1.ToString());
				sw.WriteLine(trans.Row2.ToString());
				sw.WriteLine(trans.Row3.ToString());
				if (BoundingBox.maxPoint != null && BoundingBox.minPoint != null)
					sw.WriteLine("BoundingBox: min " + BoundingBox.minPoint.ToString() + " max " + BoundingBox.maxPoint.ToString());

				// TopoModel property
				sw.WriteLine("---------------------------------");
				sw.WriteLine("originalModel.selectedTrunkSup: " + originalModel.selectedTrunkSup.ToString());
				sw.WriteLine("originalModel.branchSupError: " + originalModel.branchSupError.ToString());
				sw.WriteLine("originalModel.selectedDepthAdjust: " + originalModel.selectedDepthAdjust.ToString());
				sw.WriteLine("originalModel.selectedCell: " + originalModel.selectedCell.ToString());
			}
		}
#endif
	};
}