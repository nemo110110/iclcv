/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/Grabber.cpp                                  **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/Grabber.h>
#include <ICLFilter/WarpOp.h>
#include <ICLUtils/StringUtils.h>
#include <ICLUtils/ConfigFile.h>
#include <ICLCC/Converter.h>

using namespace std;

namespace icl{


  namespace {
    inline string toStr(double val){ return str(val); }
    inline string toStrComma(double val){ std::ostringstream str; str << val << ','; return str.str(); }
    inline bool inList(const string &s, const std::vector<string> &vec){
      return find(vec.begin(),vec.end(),s) != vec.end();
    }
  }
  
  struct Grabber::Data{
    Size desiredSize;
    format desiredFormat;
    depth desiredDepth;
    Converter converter;
    ImgBase  *image;
    WarpOp *warp;
  };

  Grabber::Grabber():
    data(new Data){
    data->desiredSize = Size::null;
    data->desiredFormat = (format)-1;
    data->desiredDepth = (depth)-1;
    data->image = 0;
    data->warp = 0;
  }

  Grabber::~Grabber() {
    ICL_DELETE( data->image );
    ICL_DELETE( data->warp );
    ICL_DELETE( data );
  }

  void Grabber::useDesired(depth d, const Size &size, format fmt){
    useDesired(d); useDesired(size);useDesired(fmt);
  }
  void Grabber::ignoreDesired(){
    ignoreDesired<depth>(); 
    ignoreDesired<Size>(); 
    ignoreDesired<format>();
  }

  void Grabber::setDesiredFormatInternal(format fmt){
    data->desiredFormat = fmt;
  }
  void Grabber::setDesiredSizeInternal(const Size &size){
    data->desiredSize = size;
  }
  void Grabber::setDesiredDepthInternal(depth d){
    data->desiredDepth = d;
  }
  format Grabber::getDesiredFormatInternal() const{
    return data->desiredFormat;
  }
  depth Grabber::getDesiredDepthInternal() const{
    return data->desiredDepth;
  }
  Size Grabber::getDesiredSizeInternal() const{
    return data->desiredSize;
  }

  
  bool Grabber::supportsProperty(const std::string &property){
    return inList(property,getPropertyList());
  }

  string Grabber::translateSteppingRange(const SteppingRange<double>& range){
    return str(range);
  }

  SteppingRange<double> Grabber::translateSteppingRange(const string &rangeStr){
    return parse<SteppingRange<double> >(rangeStr);
  }
  
  string Grabber::translateDoubleVec(const vector<double> &v){
    return cat(v,",");
  }
  vector<double> Grabber::translateDoubleVec(const string &s){
    return parseVecStr<double>(s,",");
  }
  string Grabber::translateStringVec(const vector<string> &v){
    return cat(v,",");
  }
  vector<string> Grabber::translateStringVec(const string &v){
    return tok(v,",");
  }

  const ImgBase *Grabber::grab(ImgBase **ppoDst){
    const ImgBase *acquired = acquireImage();
    
    // todo, on which image is the warping applied ?
    // on the aqcuired image or on the adapte image?
    // for now, we use the adapted which seem to make
    // much more sence
    if(data->warp){
      return data->warp->apply(adaptGrabResult(acquired,ppoDst));
    }else{
      return adaptGrabResult(acquired,ppoDst);
    }
  }

  static inline void undistort_point(const double params[4], int xi, int yi,float &xd, float &yd){
    const double &x0 = params[0];
    const double &y0 = params[1];
    const double &f = params[2]/100000000.0;
    const double &s = params[3];
    
    float x = s*(xi-x0);
    float y = s*(yi-y0);
    float p = 1 - f * (x*x + y*y);
    xd = (p*x + x0);
    yd = (p*y + y0);
  }
  
 void Grabber::enableUndistortion(const ImageUndistortion &udist, const Size &size, scalemode m){
    Img32f warpMap(size,2);
    Channel32f cs[2];
    warpMap.extractChannels(cs);
 
    for(float xi=0;xi<size.width;++xi){
      for(float yi=0;yi<size.height; ++yi){
        Point32f point(xi,yi);
        Point32f p = udist.undistort(point);
        cs[0](xi,yi) = p.x;
        cs[1](xi,yi) = p.y; 
      }
    }
    enableUndistortion(warpMap, m);
  }
  
  void Grabber::enableUndistortion(const ProgArg &pa, scalemode m){
    std::string fn =  icl::pa(pa.getID(),0);
    ImageUndistortion udist(fn);
    std::string sz = icl::pa(pa.getID(),1);
    Size size(sz);
    enableUndistortion(udist,size,m);
  }

  bool Grabber::isUndistortionEnabled() const{
    return data->warp;
  }
  
  void Grabber::enableUndistortion(const Img32f &warpMap, scalemode m){
    if(!data->warp){
      data->warp = new WarpOp;
    }
    data->warp->setWarpMap(warpMap);
    data->warp->setScaleMode(m);
  }
  
  void Grabber::disableUndistortion(){
    ICL_DELETE(data->warp);
  }
  
  
  const ImgBase *Grabber::adaptGrabResult(const ImgBase *src, ImgBase **dst){
    bool adaptDepth = desiredUsed<depth>() && (getDesired<depth>() != src->getDepth());
    bool adaptSize = desiredUsed<Size>() && (getDesired<Size>() != src->getSize());
    bool adaptFormat = desiredUsed<format>() && (getDesired<format>() != src->getFormat());
    
    if(adaptDepth || adaptSize || adaptFormat){
      if(!dst){
        dst = &data->image;
      }
      ensureCompatible(dst,
                       adaptDepth ? getDesired<depth>() : src->getDepth(),
                       adaptSize ? getDesired<Size>() : src->getSize(),
                       adaptFormat ? getDesired<format>() : src->getFormat());
      data->converter.apply(src,*dst);
      return *dst;
    }else{
      if(dst){
        src->deepCopy(dst);
        return *dst;
      }else{
        return src;
      }
    }
  }
  
  static std::vector<std::string> filter_unstable_params(const std::vector<std::string> ps){
    std::vector<std::string> fs; fs.reserve(ps.size());

    static std::string unstable[6]={
      "trigger-from-software",
      "trigger-mode",
      "trigger-polarity",
      "trigger-power",
      "trigger-source",
      "iso-speed"
    };
    for(unsigned int i=0;i<ps.size();++i){
      if(std::find(unstable,unstable+6,ps[i]) == unstable+6){
        fs.push_back(ps[i]);
      }
    }
    return fs;
  }

  void Grabber::saveProperties(const std::string &filename, bool writeDesiredParams, bool skipUnstable){
    ConfigFile f;
    f["config.title"] = std::string("Camera Configuration File");
    std::vector<std::string> ps = get_io_property_list();
    
    if(skipUnstable){
      ps = filter_unstable_params(ps);
    }

    // f["config.property-list"] = cat(ps,","); this is no longer needed!
    
    if(writeDesiredParams){
      f.setPrefix("config.desired-params.");
      f["size"] = desiredUsed<Size>() ? str(getDesired<Size>()) : str("any");
      f["format"] = desiredUsed<format>() ? str(getDesired<format>()) : str("any");
      f["depth"] = desiredUsed<depth>() ? str(getDesired<depth>()) : str("any");
    }
    
    f.setPrefix("config.properties.");
    
    for(unsigned int i=0;i<ps.size();++i){
      string &prop = ps[i];
      string type = getType(prop); 

      if(type == "range" || type == "value-list"){
        f[prop] = to32f(getValue(prop));
      }else if(type == "menu"){
        f[prop] = getValue(prop);
      }// type command is skipped!
    }
    f.save(filename);
  }
  
  void Grabber::loadProperties(const std::string &filename, bool loadDesiredParams, bool skipUnstable){
    ConfigFile f(filename);
    std::vector<std::string> psSupported = get_io_property_list();
    if(skipUnstable){
      psSupported = filter_unstable_params(psSupported);
    }
    f.setPrefix("config.properties.");
    for(unsigned int i=0;i<psSupported.size();++i){
      std::string &prop = psSupported[i];
      std::string type = getType(prop);
      
      if(type == "info") continue;
      if(type == "command") continue;

      if(type == "range" || type == "value-list"){
        try{
          setProperty(prop,str((icl32f)f[prop]));
        }catch(...){
          std::cout << "Grabber::loadProperties: property '"  << prop << "' was not set" << std::endl;
          std::cout << "(it was either not not found in the given property file or it or it's value is not"
                    << " supported by the current grabber type)" << std::endl;
        }
      }else if(type == "menu"){
        try{
          std::string val = f[prop];
          setProperty(prop,f[prop]); 
        }catch(...){
          std::cout << "Grabber::loadProperties: property '"  << prop << "' was not set" << std::endl;
          std::cout << "(it was either not not found in the given property file or it or it's value is not"
                    << " supported by the current grabber type)" << std::endl;
        }
      }
    }

    if(loadDesiredParams){
      f.setPrefix("config.desired-params.");
      try{
        if(f["size"].as<std::string>() == "any"){
          useDesired<Size>(f["size"]);
        }else{
          ignoreDesired<Size>();
        }
        if(f["format"].as<std::string>() == "any"){
          useDesired<format>(f["format"]);
        }else{
          ignoreDesired<format>();
        }
        if(f["depth"].as<std::string>() == "any"){
          useDesired<depth>(f["depth"]);
        }else{
          ignoreDesired<depth>();
        }
      }catch(...){
        std::cerr << "Warning: no desired params were found in given property file" << std::endl;
      }
    }
  }

}
