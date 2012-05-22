/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/examples/rotate-image-3D.cpp                   **
** Module : ICLGeom                                                **
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

#include <ICLQuick/Common.h>
#include <ICLGeom/Geom.h>
GUI gui("hbox");
Scene scene;
GenericGrabber grabber;
Img8u image;

struct Grid : public SceneObject{
  Mutex mutex;
  int w,h;
  int idx(int x, int y) const { return x + w* y; }
  Grid(const ImgBase *image):w(40),h(40){
    
    m_vertices.resize(w*h,Vec(0,0,0,1));
    m_vertexColors.resize(w*h,GeomColor(1,0,0,1));
    

    addTextureGrid(40,40,image,
                   &m_vertices[0][0], 
                   &m_vertices[0][1], 
                   &m_vertices[0][2], 
                   0,0,0,4,false);
    
    setVisible(Primitive::texture,true);
    setVisible(Primitive::vertex,false);
  }
  
  void prepareForRendering(){
    const float freq = 10*(gui["freq"].as<float>()+0.05);
    const float ar = float(image.getWidth())/image.getHeight();
    static Time ref = Time::now();
    float t = ref.age().toSecondsDouble() * freq;
    for(int x=0;x<w;++x){
      for(int y=0;y<h;++y){
        Vec &v = m_vertices[idx(x,y)];
        v[0] = (x-w/2)*ar;
        v[1] = y-h/2;
        v[2] = 5 * sin(x/10. +t) + 3 * cos(y/5.+t*2);
        v[3] = 1;
      }
    }
  }
} *obj = 0;


void run(){
  grabber.grab()->convert(&image);  
  gui["draw"].render();
  
  gui["offscreen"] = scene.render(0);
}

void init(){
  Scene::enableSharedOffscreenRendering();
  
  grabber.init(pa("-i"));

  gui << "draw3D()[@handle=draw@minsize=20x15@label=interaction area]" 
      << "image[@handle=offscreen@label=offscreen rendered]"
      << "fslider(0,1,0.2,vertical)[@out=freq@label=frequence]"
      << "!show";

  grabber.grab()->convert(&image);  
  obj = new Grid(&image);

  scene.addObject(obj);
  scene.addCamera(Camera(Vec(0,0,60,1),
                         Vec(0,0,-1,1),
                         Vec(0,1,0,1)));

  gui["draw"].link(scene.getGLCallback(0));
  gui["draw"].install(scene.getMouseHandler(0));
}



int main(int n, char **ppc){
  return ICLApp(n,ppc,"-input|-i(2)" ,init,run).exec();
}
