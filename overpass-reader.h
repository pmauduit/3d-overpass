#ifndef __OVERPASS_READER
#define __OVERPASS_READER

#include <string>
#include <iostream>
#include <vector>

#include <curl/curl.h>

#include <osg/Geometry>

namespace otd {

  class OverPassReader {

   private:
    std::string query;
    std::vector<osg::Geometry *> geoms;

   public:
    void doCall();
    void read(const char * filePath);
    OverPassReader(const std::string & query);
    std::vector<osg::Geometry *> getGeometries() { return geoms; };
    ~OverPassReader();

  };

}

#endif // __OVERPASS_READER

