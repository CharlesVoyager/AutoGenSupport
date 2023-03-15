#pragma once
#include "pugixml.hpp"
#include "PointsSplitter.h"
#include "MeshModel.h"

using namespace pugi;
using namespace std;

namespace XYZSupport
{
	class GetMeshfromSVG
	{
	public:
		bool LoadSVGFile(/*out*/ MeshModel &meshModel, wstring path)
		{
			xml_document xmldoc;
			if (!xmldoc.load_file(path.c_str())) return false;
			xml_node xmlsvg = xmldoc.child("svg");
			return SVGNewParse(xmlsvg, meshModel);
		}

		bool SVGNewParse(const xml_node node, /*out*/ MeshModel &meshModel)
		{
			PointsSplitter pointstring;

			if (strcmp(node.name(), "svg") != 0)
				return false;

			for (pugi::xml_node g = node.first_child(); g; g = g.next_sibling())
			{
				if (strcmp(g.name(), "g") == 0)
				{
					meshModel.AddPolygon();

					for (xml_node polygon = g.first_child(); polygon; polygon = polygon.next_sibling())
					{
						if (strcmp(polygon.name(), "polygon") == 0)
						{
							pointstring.Pointsclear();
							pointstring.Setstring(polygon.attribute("points").value());
							pointstring.Meshsplit();
							meshModel.Meshs[meshModel.Meshs.size() - 1].Vector3ds = pointstring.vector3s;
						}
					}
				}
			}
			return true;
		}
	};
}