#ifndef __OGR_PARSER
#define __OGR_PARSER

#include <gdal/ogr_geometry.h>
#include <gdal/ogr_feature.h>

#include <osg/Geometry>
#include <osg/TriangleFunctor>
#include <osgUtil/Tessellator>
#include <osgUtil/SmoothingVisitor>

namespace otd {
  class OgrParser {
    private:
     osg::Geometry * pointToDrawable(const OGRPoint * points) const;
     osg::Geometry * linearRingToDrawable(OGRLinearRing * ring) const;
     osg::Geometry * lineStringToDrawable(OGRLineString* lineString) const;
     osg::Geometry * multiPolygonToDrawable(OGRMultiPolygon* mpolygon) const;
     osg::Geometry * polygonToDrawable(OGRPolygon * polygon) const;
     osg::Geometry * multiLineStringToDrawable(OGRMultiLineString* mlineString) const;
     osg::Geometry * extrudeSurface(osg::Vec3Array* vertices, const osg::Vec3& direction, float length ) const;

    public:
     osg::Geometry * readFeature(OGRFeature * ogrFeature) const;
  };

}

#endif // __OGR_PARSER
