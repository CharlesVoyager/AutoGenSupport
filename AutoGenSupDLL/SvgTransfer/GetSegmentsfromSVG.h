#pragma once
#include "pugixml.hpp"
#include "PointsSplitter.h"

using namespace pugi;
using namespace std;

namespace XYZSupport
{
	class GetSegmentsfromSVG
	{
	public:
		bool LoadSVGFile(/*out*/ SegmentModel &segmentModel, wstring path)
		{
			xml_document xmldoc;
			if (!xmldoc.load_file(path.c_str())) return false;
			xml_node xmlsvg = xmldoc.child("svg");
			return SVGNewParse(xmlsvg, segmentModel);
		}
		bool SVGNewParse(xml_node node, /*out*/ SegmentModel &segmentModel)
		{
			PointsSplitter pointstring;

			if (strcmp(node.name(), "svg") != 0)
			{
				return false;
			}

			for (pugi::xml_node g = node.first_child(); g; g = g.next_sibling())
			{
				if (strcmp(g.name(), "g") == 0)
				{
					segmentModel.AddLayerSwgment();

					for (xml_node polygon = g.first_child(); polygon; polygon = polygon.next_sibling())
					{
						if (strcmp(polygon.name(), "segment") == 0)
						{
							pointstring.Pointsclear();
							pointstring.Setstring(polygon.attribute("points").value());
							pointstring.Segmentsplit();
							segmentModel.AllSegments[segmentModel.AllSegments.size() - 1].Segments.push_back(pointstring.segments);
						}
					}
				}
			}
			return true;
		}
	};
}
