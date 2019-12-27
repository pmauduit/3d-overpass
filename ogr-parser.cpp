#include "ogr-parser.h"
// Code mainly stolen from http://forum.openscenegraph.org/viewtopic.php?t=1578

namespace otd {

  struct TriangulizeFunctor {
	osg::Vec3Array* _vertexes;

	  // do nothing
	  void operator ()(const osg::Vec3& v1, const osg::Vec3& v2, const osg::Vec3& v3, bool treatVertexDataAsTemporary = false) {
	  	_vertexes->push_back(v1);
	  	_vertexes->push_back(v2);
	  	_vertexes->push_back(v3);
	  }
	};

  static osg::Vec3Array* triangulizeGeometry(osg::Geometry* src) {
  	if (src->getNumPrimitiveSets() == 1 &&
  			src->getPrimitiveSet(0)->getType() == osg::PrimitiveSet::DrawArraysPrimitiveType &&
  			src->getVertexArray() &&
  			src->getVertexArray()->getType() == osg::Array::Vec3ArrayType)
  		return static_cast<osg::Vec3Array*>(src->getVertexArray());

  	osg::TriangleFunctor<otd::TriangulizeFunctor> functor;
  	osg::Vec3Array* array = new osg::Vec3Array;
  	functor._vertexes = array;
  	src->accept(functor);
  	return array;
  }

  osg::Geometry * OgrParser::pointToDrawable(const OGRPoint* point) const {
  	osg::Geometry* pointGeom = new osg::Geometry();
  	osg::Vec3Array* vertices = new osg::Vec3Array();
  	vertices->push_back(osg::Vec3(point->getX(), point->getY(), point->getZ()));
  	pointGeom->setVertexArray(vertices);
  	pointGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
  	return pointGeom;
  }

  osg::Geometry * OgrParser::linearRingToDrawable(OGRLinearRing* ring) const {
  	osg::Geometry* contourGeom = new osg::Geometry();
  	osg::Vec3Array* vertices = new osg::Vec3Array();
  	OGRPoint point;
  	for(int j = 0; j < ring->getNumPoints(); j++)
  	{
  		ring->getPoint(j, &point);
  		vertices->push_back(osg::Vec3(point.getX(), point.getY(),point.getZ()));
  	}
  	contourGeom->setVertexArray(vertices);
  	contourGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, vertices->size()));
  	return contourGeom;
  }

  osg::Geometry * OgrParser::lineStringToDrawable(OGRLineString* lineString) const {
  	osg::Geometry* contourGeom = new osg::Geometry();
  	osg::Vec3Array* vertices = new osg::Vec3Array();
  	OGRPoint point;
  	for(int j = 0; j < lineString->getNumPoints(); j++) {
  		lineString->getPoint(j, &point);
  		vertices->push_back(osg::Vec3(point.getX(), point.getY(), point.getZ()));
  	}
  	contourGeom->setVertexArray(vertices);
  	contourGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, 0, vertices->size()));
  	return contourGeom;
  }

  osg::Geometry* OgrParser::multiPolygonToDrawable(OGRMultiPolygon* mpolygon) const {
  	osg::Geometry* geom = new osg::Geometry;

  	for (int i = 0; i < mpolygon->getNumGeometries(); i++ ) {
  		OGRGeometry* ogrGeom = mpolygon->getGeometryRef(i);
  		OGRwkbGeometryType ogrGeomType = ogrGeom->getGeometryType();

  		if (wkbPolygon != ogrGeomType && wkbPolygon25D != ogrGeomType)
  			continue; // skip

  		OGRPolygon* polygon = static_cast<OGRPolygon*>(ogrGeom);
  		osg::ref_ptr<osg::Drawable> drw = polygonToDrawable(polygon);
  		osg::ref_ptr<osg::Geometry> geometry = drw->asGeometry();
  		if (geometry.valid() && geometry->getVertexArray() &&
  				geometry->getVertexArray()->getNumElements() &&
  				geometry->getNumPrimitiveSets() &&
  				geometry->getVertexArray()->getType() == osg::Array::Vec3ArrayType) {
  			if (!geom->getVertexArray()) {
  				// no yet data we put the first in
  				geom->setVertexArray(geometry->getVertexArray());
  				geom->setPrimitiveSetList(geometry->getPrimitiveSetList());
  			}
  			else {
  				// already a polygon then append
  				int size = geom->getVertexArray()->getNumElements();
  				osg::Vec3Array* arrayDst = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  				osg::ref_ptr<osg::Vec3Array> triangulized = triangulizeGeometry(geometry.get());
  				if (triangulized.valid()) {
  					arrayDst->insert(arrayDst->end(), triangulized->begin(), triangulized->end());
  					// shift index
  					geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, size, triangulized->size()));
  				}
  			}
  		}
  		else
  			osg::notify(osg::WARN) << "Warning something wrong with a polygon in a multi polygon" << std::endl;
  	}

  	if (geom->getVertexArray()) {
  		osg::notify() << "osgOgrFeature::multiPolygonToGeode " << geom->getVertexArray()->getNumElements() << " vertexes"<< std::endl;
  	}
    // return extrudeSurface(static_cast<osg::Vec3Array *>(geom->getVertexArray()),
	  //			osg::Vec3(0.0f, 0.0f, 1.0f), 0.0005);
		return geom;
  }



  osg::Geometry * OgrParser::multiLineStringToDrawable(OGRMultiLineString* mlineString) const {
  	osg::Geometry* geom = new osg::Geometry;

  	for (int i = 0; i < mlineString->getNumGeometries(); i++) {
  		OGRGeometry* ogrGeom = mlineString->getGeometryRef(i);
  		OGRwkbGeometryType ogrGeomType = ogrGeom->getGeometryType();

  		if (wkbLineString != ogrGeomType && wkbLineString25D != ogrGeomType)
  			continue; // skip

  		OGRLineString * lineString = static_cast<OGRLineString*>(ogrGeom);
  		osg::ref_ptr<osg::Geometry> geometry = lineStringToDrawable(lineString);
  		if (geometry.valid() &&
  				geometry->getVertexArray() &&
  				geometry->getNumPrimitiveSets() &&
  				geometry->getVertexArray()->getType() == osg::Array::Vec3ArrayType) {
  			if (!geom->getVertexArray()) {
  				geom->setVertexArray(geometry->getVertexArray());
  				geom->setPrimitiveSetList(geometry->getPrimitiveSetList());
  			}
  			else {
  				int size = geom->getVertexArray()->getNumElements();

  				osg::Vec3Array* arraySrc = static_cast<osg::Vec3Array*>(geometry->getVertexArray());
  				osg::Vec3Array* arrayDst = static_cast<osg::Vec3Array*>(geom->getVertexArray());
  				arrayDst->insert(arrayDst->end(), arraySrc->begin(), arraySrc->end());
  				// shift index
  				geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, size, arraySrc->size()));
  			}
  		}
  	}
  	return geom;
  }

  /**
   * stolen from https://github.com/xarray/osgRecipes/blob/master/cookbook/chapter3/ch03_02/extrusion.cpp
   */
  osg::Geometry* OgrParser::extrudeSurface(osg::Vec3Array* vertices, const osg::Vec3& direction, float length ) const {
    osg::ref_ptr<osg::Vec3Array> newVertices = new osg::Vec3Array;
    newVertices->insert( newVertices->begin(), vertices->begin(), vertices->end() );

    unsigned int numVertices = vertices->size();
    osg::Vec3 offset = direction * length;
    for (osg::Vec3Array::iterator itr = vertices->begin(); itr != vertices->end(); ++itr) {
        newVertices->push_back((* itr) + offset);
    }

		// newVertices->size() is now numVertices * 2

    for (int i = 0 ; i < numVertices; ++i) {
				newVertices->push_back(newVertices->at(i));
				newVertices->push_back(newVertices->at(i+1));
				newVertices->push_back(newVertices->at(numVertices + i + 1));
				newVertices->push_back(newVertices->at(numVertices + i));
    }
		// Last face
		newVertices->push_back(newVertices->at(numVertices - 1));
		newVertices->push_back(newVertices->at(0));
		newVertices->push_back(newVertices->at(numVertices));
		newVertices->push_back(newVertices->at(numVertices * 2));


		// draws each faces
		osg::ref_ptr<osg::Geometry> extrusion = new osg::Geometry;
    extrusion->setVertexArray(newVertices.get());
    extrusion->addPrimitiveSet( new osg::DrawArrays(GL_POLYGON, 0, numVertices) );
    extrusion->addPrimitiveSet( new osg::DrawArrays(GL_POLYGON, numVertices, numVertices) );
		for (int i = numVertices * 2; i < newVertices->size() ; i+= 4) {
			extrusion->addPrimitiveSet( new osg::DrawArrays(GL_POLYGON, i, 4));
    }

		return extrusion.release();
    //osgUtil::Tessellator tessellator;
    //tessellator.setTessellationType( osgUtil::Tessellator::TESS_TYPE_POLYGONS );
    //tessellator.setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD );
    //tessellator.retessellatePolygons( *extrusion );

    //osg::ref_ptr<osg::DrawElementsUInt> sideIndices = new osg::DrawElementsUInt( GL_QUAD_STRIP );
    //for (unsigned int i=0; i < numVertices; ++i) {
    //    sideIndices->push_back( i );
    //    sideIndices->push_back( (numVertices-1-i) + numVertices );
    //}
    //sideIndices->push_back( 0 );
    //sideIndices->push_back( numVertices*2 - 1 );
    //extrusion->addPrimitiveSet( sideIndices.get() );

    //osgUtil::SmoothingVisitor::smooth( *extrusion );
    //return extrusion.release();
	}

  osg::Geometry * OgrParser::polygonToDrawable(OGRPolygon * polygon) const {
  	osg::Geometry * geom = new osg::Geometry();
  	osg::Vec3Array * vertices = new osg::Vec3Array();
  	geom->setVertexArray(vertices);
  	{
  		OGRLinearRing *ring = polygon->getExteriorRing();
  		OGRPoint point;
  		for(int i = 0; i < ring->getNumPoints(); i++) {
  			ring->getPoint(i, &point);
  			vertices->push_back(osg::Vec3(point.getX(), point.getY(), point.getZ()));
  		}
  		geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, 0, vertices->size()));
  	}

  	if (polygon->getNumInteriorRings()) {
  		for (int i = 0; i < polygon->getNumInteriorRings(); i++) {
  			OGRLinearRing *ring = polygon->getInteriorRing(i);
  			OGRPoint point;
  			for (int j = 0; j < ring->getNumPoints(); j++) {
  				ring->getPoint(j, &point);
  				vertices->push_back(osg::Vec3(point.getX(), point.getY(), point.getZ()));
  			}
  			geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP, vertices->size()-ring->getNumPoints() , ring->getNumPoints()));
  		}
  	}
  	osgUtil::Tessellator tsl;
  	tsl.setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
  	tsl.setBoundaryOnly(false);
  	tsl.retessellatePolygons(*geom);

  	osg::Vec3Array* array = triangulizeGeometry(geom);
  	geom->setVertexArray(array);
  	geom->removePrimitiveSet(0,geom->getNumPrimitiveSets());
  	geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, array->size()));
    return extrudeSurface(static_cast<osg::Vec3Array *>(geom->getVertexArray()),
				osg::Vec3(0.0f, 0.0f, 1.0f), 0.0005);
		return geom;
  }


  osg::Geometry * OgrParser::readFeature(OGRFeature* ogrFeature) const {
  	if (!ogrFeature || !ogrFeature->GetGeometryRef())
  		return 0;
  	osg::Geometry* drawable = 0;
  	// Read the geometry
  	switch(ogrFeature->GetGeometryRef()->getGeometryType()) {
  		case wkbPoint:
  		case wkbPoint25D:
  			// point to drawable
  			drawable = pointToDrawable(static_cast<OGRPoint *>(ogrFeature->GetGeometryRef()));
  			break;
  		case wkbLinearRing:
  			drawable = linearRingToDrawable(static_cast<OGRLinearRing *>(ogrFeature->GetGeometryRef()));
  			break;
  		case wkbLineString:
  		case wkbLineString25D:
  			drawable = lineStringToDrawable(static_cast<OGRLineString*>(ogrFeature->GetGeometryRef()));
  			break;
  		case wkbPolygon:
  		case wkbPolygon25D:
  			drawable = polygonToDrawable(static_cast<OGRPolygon*>(ogrFeature->GetGeometryRef()));
  			break;
  		case wkbMultiPoint:
  		case wkbMultiPoint25D:
  			break;
  		case wkbMultiLineString:
  		case wkbMultiLineString25D:
  			drawable = multiLineStringToDrawable(static_cast<OGRMultiLineString*>(ogrFeature->GetGeometryRef()));
  			break;
  		case wkbMultiPolygon:
  		case wkbMultiPolygon25D:
  			drawable = multiPolygonToDrawable(static_cast<OGRMultiPolygon*>(ogrFeature->GetGeometryRef()));
  			break;
  		case wkbGeometryCollection:
  		case wkbGeometryCollection25D:
  			osg::notify(osg::WARN) << "This geometry is not yet implemented " << OGRGeometryTypeToName(ogrFeature->GetGeometryRef()->getGeometryType()) << std::endl;
  			break;
  		case wkbNone:
  			osg::notify(osg::WARN) << "No WKB Geometry " << OGRGeometryTypeToName(ogrFeature->GetGeometryRef()->getGeometryType()) << std::endl;
  			break;
  		case wkbUnknown:
  		default:
  			osg::notify(osg::WARN) << "Unknown WKB Geometry " << OGRGeometryTypeToName(ogrFeature->GetGeometryRef()->getGeometryType()) << std::endl;
  			break;
  	}
  	return drawable;
  }
}
