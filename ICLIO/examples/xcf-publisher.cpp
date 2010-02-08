#include <ICLIO/XCFPublisher.h>
#include <ICLIO/XCFPublisherGrabber.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/FPSEstimator.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLIO/IOFunctions.h>

#include <ICLFilter/MedianOp.h>
#include <ICLFilter/ConvolutionOp.h>

std::string uri,stream;
GUI gui("vbox");

bool first = true;
bool *ppEnabled = 0;

GenericGrabber *grabber = 0;

std::vector<string> remove_size(const vector<string> &v){
  vector<string> r;
  for(unsigned int i=0;i<v.size();++i){
    if(v[i] != "size") r.push_back(v[i]);
  }
  return r;
}  

void init_grabber(){
  grabber = new GenericGrabber(FROM_PROGARG("-input"));
  grabber->setIgnoreDesiredParams(true);
  if(pa("-size")){
    grabber->setDesiredSize(pa("-size"));
    grabber->setIgnoreDesiredParams(false);
  }
  if(pa("-depth")){
    grabber->setDesiredDepth(pa("-depth"));
    grabber->setIgnoreDesiredParams(false);
  }
  
  if(pa("-dist")){
    if(pa("-size")){
      grabber->enableDistortion(DIST_FROM_PROGARG("-dist"),pa("-size"));
    }else{
      grabber->enableDistortion(DIST_FROM_PROGARG("-dist"),grabber->grab()->getSize());
    }
  }
  if(pa("-camera-config")){
    grabber->loadProperties(pa("-camera-config"),false);
  }  
}

const ImgBase *grab_image(){
  const ImgBase *img = 0;
  //  const ImgBase *image = grabber.grab();
  if(!pa("-flip")){
    img = grabber->grab();
  }else{
    ImgBase *hack = const_cast<ImgBase*>(grabber->grab());
    std::string axis = pa("-flip");
    if(axis  ==   "x"){
      hack->mirror(axisVert);
    }else if(axis  ==  "y"){
      hack->mirror(axisHorz);
    }else if(axis == "both" || axis == "xy"){
      hack->mirror(axisBoth);
    }else{
      ERROR_LOG("nothing known about axis " <<  axis << "(allowed arguments are x,y or both)");
    }
    img = hack;
  }
  
  if(!pa("-clip")){
    return img;
    
  }else{
    if(*pa("-clip")=="interactive"){
      throw ICLException("interactive clipmode is not yet implemented ...");
    }else{
      static Rect *r = 0;
      static ImgBase *clipped = 0;
      if(!r){
        r = new Rect;
        *r = pa("-clip");
        
        ICLASSERT_THROW(r->width <= img->getWidth(),ICLException("clipping rect width is larger then image width"));
        ICLASSERT_THROW(r->height <= img->getHeight(),ICLException("clipping rect height is larger then image height"));
        ICLASSERT_THROW(r->x>= 0,ICLException("clipping x-offset < 0"));
        ICLASSERT_THROW(r->y>= 0,ICLException("clipping y-offset < 0"));
        ICLASSERT_THROW(r->right() < img->getWidth(),ICLException("clipping rect's right edge is outside the image rect"));
        ICLASSERT_THROW(r->bottom() < img->getHeight(),ICLException("clipping rect's right edge is outside the image rect"));
        clipped = imgNew(img->getDepth(),r->getSize(),img->getChannels(),img->getFormat()); 
      }
      const ImgBase *tmp = img->shallowCopy(*r);
      tmp->deepCopyROI(&clipped);
      delete tmp;
      img = clipped;
    }
  }
  return img;
}

void send_app(){
  static XCFPublisher p(stream,uri);
  ImageHandle IH;
  FPSHandle FPS;
  if(!pa("-no-gui")){
    IH = gui.getValue<ImageHandle>("image");
    FPS= gui.getValue<FPSHandle>("fps");
  }
 
  while(first || !pa("-single-shot")){
    const ImgBase *grabbedImage = grab_image();

    const ImgBase *ppImage = 0;
    if(pa("-pp") && *ppEnabled){
      static UnaryOp *pp = 0;
      if(!pp){
        static std::string pps = pa("-pp");
        if(pps == "gauss"){
          pp = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss3x3));
        }else if(pps == "gauss5") {
          pp = new ConvolutionOp(ConvolutionKernel(ConvolutionKernel::gauss5x5));
        }else if(pps == "median"){
          pp = new MedianOp(Size(3,3));
        }else if(pps == "median5"){
          pp = new MedianOp(Size(5,5));
        }else{
          ERROR_LOG("undefined preprocessing mode");
          ::exit(0);
        }
      }
      pp->setClipToROI(false);
      ppImage  = pp->apply(grabbedImage);
    }else{
      ppImage = grabbedImage;
    }
    const ImgBase *normImage = 0;
    if(pa("-normalize")){
      static ImgBase *buf = 0;
      ppImage->deepCopy(&buf);
      buf->normalizeAllChannels(Range64f(0,255));
      normImage = buf;
    }else{
      normImage = ppImage;
    }
    

    p.publish(normImage);
    if(!pa("-no-gui")){
      IH = normImage;
      IH.update();
      FPS.update();
    }
    first = false;
    
    gui_int(fpsLimit);
    static FPSLimiter limiter(15,10);
    if(limiter.getMaxFPS() != fpsLimit) limiter.setMaxFPS(fpsLimit);
    limiter.wait();
  }
}

std::string create_camcfg(const std::string&, const std::string &hint){
  return str("camcfg(")+hint+")[@maxsize=5x2]";
}


void init_gui(){
  if(pa("-pp")){
    gui << "image[@handle=image@minsize=12x8]" 
        << ( GUI("hbox[@maxsize=100x4]") 
             << create_camcfg(FROM_PROGARG("-input"))
             << ("spinner(1,100,"+*pa("-fps")+")[@out=fpsLimit@label=max fps]")
             << "fps(10)[@handle=fps]"
             << "togglebutton(off,!on)[@handle=_@out=pp-on@label=preprocessing@minsize=5x2]"
             );
    gui.show();
    ppEnabled = &gui.getValue<bool>("pp-on");
  }else{
    gui << "image[@handle=image@minsize=12x8]" 
        << ( GUI("hbox[@maxsize=100x4]") 
             << create_camcfg(FROM_PROGARG("-input"))
             << ("spinner(1,100,"+*pa("-fps")+")[@out=fpsLimit@label=max fps]")
             << "fps(10)[@handle=fps]"
             );
    gui.show();
  }
}


int main(int n, char **ppc){
  paex
  ("-input","for sender application only allowed ICL default\n"
   " input specificationn e.g. -input pwc 0 or -input file bla/*.ppm")
  ("-stream","stream name for sender and receiver application (by default: the-stream)")
  ("-uri","URI for image packages (by default the-uri)")
  ("-single-shot","no loop application")
  ("-size","output image size (sending only, default: VGA)")
  ("-depth","output image size (sending only, default: depth8u)")

  ("-fps","initial max FPS count, further adjustable in the GUI")
  ("-no-gui","dont display a GUI (sender app only)")
  ("-flip","define axis to flip (allowed sub arguments are"
   " x, y or both")
  ("-clip","define clip-rect ala ((x,y)WxH) or string interactive (which is not yet supported)")
  ("-pp","select preprocessing (one of \n"
   "\t- gauss 3x3 gaussian blur\n"
   "\t- gauss5 5x5 gaussian blur\n"
   "\t- median 3x3 median filter\n"
   "\t- median5 5x5 median filter\n")
  ("-dist","give 4 parameters for radial lens distortion.\n"
   "\tThis parameters can be obtained using ICL application\n"
   "\ticl-calib-radial-distortion")
  ("-reset","reset bus on startup")
  ("-normalize","normalize resulting image to [0,255]")
  ("-camera-config","if a valid xml-camera configuration file was given here, the grabber is set up "
   "with this parameters internally. Valid parameter files can be created with icl-camera-param-io or with "
   "the icl-camcfg tool. Please note: some grabber parameters might cause an internal grabber crash, "
   "so e.g. trigger setup parameters or the isospeed parameters must be removed from this file");
  painit(n,ppc,"-stream|-s(streamname=stream) "
         "-flip|-f(string) -uri|-u(image-URI=IMAGE) -single-shot -input|-i(device,device-params) "
         "-size|(Size) -no-gui -pp(1) -dist|-d(float,float,float,float) -reset|-r "
         "-fps(float=15.0) -clip|-c(Rect) -camera-config(filename) -depth(depth) -normalize|-n");

  if(pa("-reset")){
    GenericGrabber::resetBus();
  }
  
  uri = *pa("-u");
  stream = *pa("-s");

  init_grabber();  
  if(!pa("-no-gui")){
    return ICLApp(n,ppc,"",init_gui,send_app).exec();
  }else{
    static bool alwaysTrue = 1;
    ppEnabled = &alwaysTrue;
    init_grabber();
    send_app();
  }
}
