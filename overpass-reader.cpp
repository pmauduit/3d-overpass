#include "overpass-reader.h"
#include "ogr-parser.h"

#include <gdal/gdal.h>
#include <gdal/cpl_string.h>
#include <gdal/gdal_priv.h>
#include <gdal/ogr_core.h>
#include <gdal/ogr_api.h>
#include <gdal/ogrsf_frmts.h>

#include <osg/Geode>
#include <osgViewer/Viewer>
#include <osgGA/TerrainManipulator>

namespace otd {

  void OverPassReader::read(const char * path) {
    std::cout << "reading " << path << std::endl;
    char **osmOptions = NULL;
    osmOptions = CSLSetNameValue(osmOptions, "USE_CUSTOM_INDEXING", "NO" );
    GDALDataset * dataset = (GDALDataset *) GDALOpenEx(path, GDAL_OF_VECTOR | GDAL_OF_READONLY,
                                                       NULL, osmOptions, NULL);

    if(dataset == NULL)
    {
      fprintf(stderr, "Error opening file %s\n", path);
      return;
    }
    OgrParser p = OgrParser();
    unsigned int numfeat = 0;

    for (int i = 0 ; i < dataset->GetLayerCount() ; ++i) {
      OGRLayer * layer = dataset->GetLayer(i);
      OGRFeature * feat =  layer->GetNextFeature();
      while (feat != NULL) {
        geoms.push_back(p.readFeature(feat));

        // Next feature
        numfeat++;
        OGRFeature::DestroyFeature(feat);
        feat = layer->GetNextFeature();
      }
    }

    std::cout << numfeat << " features parsed in " << dataset->GetLayerCount() << " layers" << std::endl;
    CSLDestroy(osmOptions);
    delete dataset;
  }

  void OverPassReader::doCall() {
    // libCurl to the rescue !
    // see code commented out below
  }

  OverPassReader::OverPassReader(const std::string & query = ""): query(query) {}

  OverPassReader::~OverPassReader() {
    geoms.clear();
  }
}


int main (int argc, char **argv) {

  if (argc != 2) {
    fprintf(stdout, "Usage: %s <filename>\n", argv[0]);
    return 0;
  }

  GDALAllRegister();
  otd::OverPassReader reader = otd::OverPassReader();
  reader.read(argv[1]);
  std::vector<osg::Geometry *> geoms = reader.getGeometries();
  std::cout << geoms.size() << " elements to be drawn" << std::endl;

  osg::ref_ptr<osg::Geode> geode = new osg::Geode;
  for (unsigned int i = 0 ; i < geoms.size(); ++i)
    geode->addDrawable(geoms.at(i));

  osg::ref_ptr<osg::Group> root = new osg::Group;
  root->addChild(geode.get());



  osgViewer::Viewer viewer;
  osg::ref_ptr< osg::Light > light = new osg::Light;
  light->setAmbient(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
  viewer.setLight(light);
  osg::Camera * camera = viewer.getCamera();
  camera->setClearColor(osg::Vec4(0.094f, 0.247f, 0.539f, 0.1f));



  viewer.setSceneData(root.get());
  viewer.setCameraManipulator(new osgGA::TerrainManipulator());
  return viewer.run();
}

/*static const std::string defaultOverPassQuery = "                                                     \
data=[out:xml][timeout:25];                                                                           \
(                                                                                                     \
  way[\"building\"](45.5623512944901,5.916695594787597,45.56921648523651,5.926598310470581);          \
    relation[\"building\"]( 45.5623512944901,5.916695594787597,45.56921648523651,5.926598310470581);  \
      way[\"highway\"](45.5623512944901,5.916695594787597,45.56921648523651,5.926598310470581);       \
      );                                                                                              \
      out body;                                                                                       \
      >;                                                                                              \
      out skel qt;                                                                                    \
";*/
//
//static size_t responseCb(void *buffer, size_t size, size_t nmemb, void *stream)
//{
//  return fwrite(buffer, size, nmemb, stdout);
//}
//
//int main(int argc, char **argv) {
//  CURL *conn = NULL;
//
//  std::string buffer;
//  std::string url = "http://overpass-api.de/api/interpreter";
//
//  curl_global_init(CURL_GLOBAL_DEFAULT);
//  conn = curl_easy_init();
//  curl_easy_setopt(conn, CURLOPT_URL, url.c_str());
//  curl_easy_setopt(conn, CURLOPT_POSTFIELDS,
//                   defaultOverPassQuery.c_str());
//  curl_easy_setopt(conn, CURLOPT_WRITEFUNCTION, responseCb);
//  curl_easy_setopt(conn, CURLOPT_VERBOSE, 1L);
//  //otd::OverPassReader reader = otd::OverPassReader(defaultOverPassQuery);
//
//  CURLcode res = curl_easy_perform(conn);
//  if(res != CURLE_OK) {
//    fprintf(stderr, "curl_easy_perform() failed: %s\n",
//            curl_easy_strerror(res));
//  }
//  curl_easy_cleanup(conn);
//
//  curl_global_cleanup();
//  return 0;
//}
