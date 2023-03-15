#pragma once
#include "pugixml.hpp"
#include "PointsSplitter.h"

using namespace pugi;
using namespace std;

namespace XYZSupport
{
	class GetdatafromSVG
	{
	public:
		bool LoadSVGFile(/*out*/ LayerModel &Printmodel, wstring path)
		{		
			xml_document xmldoc;
			if (!xmldoc.load_file(path.c_str())) return false;
			xml_node xmlsvg = xmldoc.child("svg");
			return SVGNewParse(xmlsvg, Printmodel);
		}
		
		bool SVGNewParse(const xml_node node, /*out*/ LayerModel &Printmodel)
		{
			PointsSplitter pointstring;

			if (strcmp(node.name(), "svg")!=0)
				return false;

			for(pugi::xml_node g = node.first_child(); g; g = g.next_sibling())
			{
				if (strcmp(g.name(), "g") == 0)
				{
					Printmodel.AddLayer();

					//if (layernode.ChildNodes.Count == 0)
					//    return false;

					for (xml_node polygon = g.first_child(); polygon; polygon = polygon.next_sibling())
					{
						if (strcmp(polygon.name(), "polygon") == 0)
						{
							if (0 == strcmp(polygon.attribute("style").value(), "fill: white"))
							{
								Printmodel.Layers[Printmodel.Layers.size() - 1].AddPolygon();
								Printmodel.Layers[Printmodel.Layers.size() - 1].AddPolygonColor("W");//contour,white

								pointstring.Pointsclear();
								pointstring.Setstring(polygon.attribute("points").value());
								pointstring.Split();

								Printmodel.Layers[Printmodel.Layers.size() - 1].Polygons[Printmodel.Layers[Printmodel.Layers.size() - 1].Polygons.size() - 1].Points = pointstring.points;
							}
							else if (0 == strcmp(polygon.attribute("style").value(), "fill: black"))
							{
								//PrintModel.Layers[PrintModel.Layers.Count - 1].AddHole();
								Printmodel.Layers[Printmodel.Layers.size() - 1].AddPolygon();
								Printmodel.Layers[Printmodel.Layers.size() - 1].AddPolygonColor("B");//hole,black

								pointstring.Pointsclear();
								pointstring.Setstring(polygon.attribute("points").value());
								pointstring.Split();

								Printmodel.Layers[Printmodel.Layers.size() - 1].Polygons[Printmodel.Layers[Printmodel.Layers.size() - 1].Polygons.size() - 1].Points = pointstring.points;
							}
						}
					}
				}
			}
			return true;
		}
	};
}
