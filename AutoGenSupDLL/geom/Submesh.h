#pragma once

#include <vector>
#include "RHVector3.h"

using namespace std;

namespace XYZSupport
{
	class SubmeshTriangle;

	class SubmeshEdge
	{
	public:
		int vertex1, vertex2;
		int color;
		SubmeshEdge(int v1, int v2, int col)
		{
			vertex1 = v1;
			vertex2 = v2;
			color = col;
		}
	};

	class Submesh
	{
	public:
		enum MeshColor {
			FrontBack = -1,
			ErrorFace = -2,
			ErrorEdge = -3,
			OutSide = -4,
			EdgeLoop = -5,
			CutEdge = -6,
			Normal = -7,
			Edge = -8,
			Back = -9,
			TransBlue = -10,
			OverhangLv1 = -11,
			OverhangLv2 = -12,
			OverhangLv3 = -13,
			// Cone Support Generator
			// Added mesh color - orange
			// Developer: RC Grey
			// 01-12-2017
			ConeSupport = -14,
			// Trunk Support Generator
			TreeSymbol = -15,
			TreeMesh = -16,
			TreeError = -17,
			TreeSlect = -18,
			TreeTest = -19
		};

		const int MESHCOLOR_FRONTBACK = -1;
		const int MESHCOLOR_ERRORFACE = -2;
		const int MESHCOLOR_ERROREDGE = -3;
		const int MESHCOLOR_OUTSIDE = -4;
		const int MESHCOLOR_EDGE_LOOP = -5;
		const int MESHCOLOR_CUT_EDGE = -6;
		const int MESHCOLOR_NORMAL = -7;
		const int MESHCOLOR_EDGE = -8;
		const int MESHCOLOR_BACK = -9;
		//--- MODEL_SLA	// milton
		const int MESHCOLOR_TRANS_BLUE = -10;
		const int MESHCOLOR_PINK = -11;
		const int MESHCOLOR_LIGHTPINK = -12;
		const int MESHCOLOR_LIGHTPINK_WHITE = -13;
		// Tree/Cone Support Generator
		// Added mesh colors
		// Developer: RC Grey
		// 01-12-2017
		const int MESHCOLOR_CONESUP = -14;
		const int MESHCOLOR_TREESYM = -15;
		const int MESHCOLOR_TREEMSH = -16;
		const int MESHCOLOR_BRNHERR = -17;
		const int MESHCOLOR_TREESEL = -18;
		const int MESHCOLOR_TREETES = -19;
		//--
		vector<Vector3> vertices;
		vector<SubmeshEdge> edges;
		vector<SubmeshTriangle> triangles;
		vector<SubmeshTriangle> trianglesError;
		bool selected = false;
		int extruder = 0;
		bool clipEnable = false;
		double clipPlaneEq[4] = { 0, 0, -1.0, 0.0 };
		float zClipHeight = 0;
		float *glVertices = nullptr;
		int glColors = 0;
		int* glEdges = nullptr;
		int* glTriangles = nullptr;
		int* glTrianglesError = nullptr;
		int* glBuffer = nullptr;
		float* glNormals = nullptr;

		//IDraw drawer;

		void Clear()
		{
			//vertices.Clear();
			//edges.Clear();
			//triangles.Clear();
			//trianglesError.Clear();
			//ClearGL();
		}

	private:
		//int ColorToRgba32(Color c)
		//{
		//	return (int)((c.A << 24) | (c.B << 16) | (c.G << 8) | c.R);
		//}

	public:
		/// <summary>
		/// Remove unneded temporary data
		/// </summary>
		void Compress()
		{
			Compress(false, 0);
		}

		void Compress(bool override_color, int color)
		{
			//glVertices = new float[3 * vertices.Count];
			//glNormals = new float[3 * vertices.Count];
			//glColors = new int[vertices.Count];
			//glEdges = new int[edges.Count * 2];
			//glTriangles = new int[triangles.Count * 3];
			//glTrianglesError = new int[trianglesError.Count * 3];
			//UpdateDrawLists();
			//UpdateColors(override_color, color);
			//vertices.Clear();
		}

		int VertexId(RHVector3 v)
		{
			int pos = (int)vertices.size();
			vertices.push_back(Vector3((float)v.x, (float)v.y, (float)v.z));
			return pos;
		}

		int VertexId(double x, double y, double z)
		{
			int pos = (int)vertices.size();
			vertices.push_back(Vector3((float)x, (float)y, (float)z));
			return pos;
		}

		void AddEdge(RHVector3 v1, RHVector3 v2, int color)
		{
			edges.push_back(SubmeshEdge(VertexId(v1), VertexId(v2), color));
		}

		void AddTriangle(RHVector3 v1, RHVector3 v2, RHVector3 v3, int color)
		{
			//if (color == MESHCOLOR_ERRORFACE)
			//	trianglesError.push_back(SubmeshTriangle(VertexId(v1), VertexId(v2), VertexId(v3), color));
			//else
			//	triangles.push_back(SubmeshTriangle(VertexId(v1), VertexId(v2), VertexId(v3), color));
		}

		private:
		void ClearGL()
		{
			//if (glBuffer != nullptr)
			//{
			//	GL.DeleteBuffers(glBuffer.Length, glBuffer);
			//	glBuffer = nullptr;
			//}
		}

		public:
		void UpdateColors(bool override_color, int color)
		{
			//foreach(SubmeshTriangle t in triangles)
			//{
			//	if (!override_color)
			//	{
			//		//--- MODEL_SLA
			//		//if (Main.main.threedview.ui.Setting.printerType_comboBox.Text == QuickTool.PropertyTranslator.GetDeviceName(PrinterType.N10))
			//		//{
			//		//    glColors[t.vertex1] = ConvertColorIndex(t.colors[0]);
			//		//    glColors[t.vertex2] = ConvertColorIndex(t.colors[1]);
			//		//    glColors[t.vertex3] = ConvertColorIndex(t.colors[2]);
			//		//}
			//		//else
			//		//---
			//			glColors[t.vertex1] = glColors[t.vertex2] = glColors[t.vertex3] = ConvertColorIndex(t.color);
			//	}
			//	else
			//		glColors[t.vertex1] = glColors[t.vertex2] = glColors[t.vertex3] = color;
			//	//glColors2[t.vertex1] = glColors2[t.vertex2] = glColors2[t.vertex3] = ConvertColorIndex((t.color == Submesh.MESHCOLOR_FRONTBACK ? Submesh.MESHCOLOR_BACK : t.color));
			//}
			//foreach(SubmeshTriangle t in trianglesError)
			//{
			//	if (!override_color)
			//		glColors[t.vertex1] = glColors[t.vertex2] = glColors[t.vertex3] = ConvertColorIndex(t.color);
			//	else
			//		glColors[t.vertex1] = glColors[t.vertex2] = glColors[t.vertex3] = color;
			//	//glColors2[t.vertex1] = glColors2[t.vertex2] = glColors2[t.vertex3] = ConvertColorIndex((t.color == Submesh.MESHCOLOR_FRONTBACK ? Submesh.MESHCOLOR_BACK : t.color));
			//}
			//foreach(SubmeshEdge e in edges)
			//{
			//	if (!override_color)
			//		glColors[e.vertex1] = glColors[e.vertex2] = ConvertColorIndex(e.color);
			//	else
			//		glColors[e.vertex1] = glColors[e.vertex2] = color;
			//}
			//if (glBuffer != nullptr)
			//{
			//	// Bind current context to Array Buffer ID
			//	GL.BindBuffer(BufferTarget.ArrayBuffer, glBuffer[2]);
			//	// Send data to buffer
			//	GL.BufferData(BufferTarget.ArrayBuffer, (IntPtr)(glColors.Length * sizeof(int)), glColors, BufferUsageHint.StaticDraw);
			//	// Validate that the buffer is the correct size
			//	int bufferSize;
			//	GL.GetBufferParameter(BufferTarget.ArrayBuffer, BufferParameterName.BufferSize, out bufferSize);
			//	if (glColors.Length * sizeof(int) != bufferSize)
			//		throw new ApplicationException("Vertex array not uploaded correctly");
			//	// Clear the buffer Binding
			//	GL.BindBuffer(BufferTarget.ArrayBuffer, 0);
			//}
		}

		public:
		void UpdateDrawLists()
		{
		/*	int idx = 0;
			foreach(SubmeshTriangle t in triangles)
			{
				int n1 = 3 * t.vertex1;
				int n2 = 3 * t.vertex2;
				int n3 = 3 * t.vertex3;
				Vector3 v1 = vertices[t.vertex1];
				Vector3 v2 = vertices[t.vertex2];
				Vector3 v3 = vertices[t.vertex3];
				t.Normal(this, out glNormals[n1], out glNormals[n1 + 1], out glNormals[n1 + 2]);
				glNormals[n2] = glNormals[n3] = glNormals[n1];
				glNormals[n2 + 1] = glNormals[n3 + 1] = glNormals[n1 + 1];
				glNormals[n2 + 2] = glNormals[n3 + 2] = glNormals[n1 + 2];
				glVertices[n1++] = v1.X;
				glVertices[n1++] = v1.Y;
				glVertices[n1] = v1.Z;
				glVertices[n2++] = v2.X;
				glVertices[n2++] = v2.Y;
				glVertices[n2] = v2.Z;
				glVertices[n3++] = v3.X;
				glVertices[n3++] = v3.Y;
				glVertices[n3] = v3.Z;
				glTriangles[idx++] = t.vertex1;
				glTriangles[idx++] = t.vertex2;
				glTriangles[idx++] = t.vertex3;
			}
			idx = 0;
			foreach(SubmeshTriangle t in trianglesError)
			{
				int n1 = 3 * t.vertex1;
				int n2 = 3 * t.vertex2;
				int n3 = 3 * t.vertex3;
				Vector3 v1 = vertices[t.vertex1];
				Vector3 v2 = vertices[t.vertex2];
				Vector3 v3 = vertices[t.vertex3];
				t.Normal(this, out glNormals[n1], out glNormals[n1 + 1], out glNormals[n1 + 2]);
				glNormals[n2] = glNormals[n3] = glNormals[n1];
				glNormals[n2 + 1] = glNormals[n3 + 1] = glNormals[n1 + 1];
				glNormals[n2 + 2] = glNormals[n3 + 2] = glNormals[n1 + 2];
				glVertices[n1++] = v1.X;
				glVertices[n1++] = v1.Y;
				glVertices[n1] = v1.Z;
				glVertices[n2++] = v2.X;
				glVertices[n2++] = v2.Y;
				glVertices[n2] = v2.Z;
				glVertices[n3++] = v3.X;
				glVertices[n3++] = v3.Y;
				glVertices[n3] = v3.Z;
				glTrianglesError[idx++] = t.vertex1;
				glTrianglesError[idx++] = t.vertex2;
				glTrianglesError[idx++] = t.vertex3;
			}
			idx = 0;
			foreach(SubmeshEdge e in edges)
			{
				int n1 = 3 * e.vertex1;
				int n2 = 3 * e.vertex2;
				Vector3 v1 = vertices[e.vertex1];
				Vector3 v2 = vertices[e.vertex2];
				glNormals[n1] = glNormals[n2] = 0;
				glNormals[n1 + 1] = glNormals[n2 + 1] = 0;
				glNormals[n1 + 2] = glNormals[n2 + 2] = 1;
				glVertices[n1++] = v1.X;
				glVertices[n1++] = v1.Y;
				glVertices[n1] = v1.Z;
				glVertices[n2++] = v2.X;
				glVertices[n2++] = v2.Y;
				glVertices[n2] = v2.Z;
				glEdges[idx++] = e.vertex1;
				glEdges[idx++] = e.vertex2;
			}*/
		}

		//GLNKG.Graphics.Color4 convertColor(Color col)
		//{
		//	return new GLNKG.Graphics.Color4(col.R, col.G, col.B, col.A);
		//}

		//void Draw(Submesh mesh, int method, Vector3 edgetrans, bool forceFaces = false)
		//{
		//	if (drawer != nullptr)
		//		this.drawer.Draw(this, method, edgetrans, forceFaces);
		//}

		//virtual void SetDrawer(IDraw newDrawer)
		//{
		//	this.drawer = newDrawer;
		//}

	};

	class SubmeshTriangle
	{
	public:
		int vertex1, vertex2, vertex3;
		int color;

	public:
		SubmeshTriangle(int v1, int v2, int v3, int col)
		{
			vertex1 = v1;
			vertex2 = v2;
			vertex3 = v3;
			color = col;
		}

		void Normal(Submesh mesh, float& nx, float& ny, float& nz)
		{
			Vector3 v0 = mesh.vertices[vertex1];
			Vector3 v1 = mesh.vertices[vertex2];
			Vector3 v2 = mesh.vertices[vertex3];
			float a1 = v1.x - v0.x;
			float a2 = v1.y - v0.y;
			float a3 = v1.z - v0.z;
			float b1 = v2.x - v1.x;
			float b2 = v2.y - v1.y;
			float b3 = v2.z - v1.z;
			nx = a2 * b3 - a3 * b2;
			ny = a3 * b1 - a1 * b3;
			nz = a1 * b2 - a2 * b1;
			float length = (float)sqrt(nx * nx + ny * ny + nz * nz);
			if (length == 0)
			{
				nx = ny = 0;
				nz = 1;
			}
			else
			{
				nx /= length;
				ny /= length;
				nz /= length;
			}
		}
	};
}
