#ifdef HAVE_QT
#include <QtGui/QFontMetrics>
#include <QtCore/QRectF>
#include <QtGui/QImage>
#include <QtGui/QPainter>
#include <QtGui/QFont>
#include <ICLCC/CCFunctions.h>
#endif

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <ICLGeom/Primitive.h>
#include <ICLGeom/SceneObject.h>


namespace icl{

  Img8u TextPrimitive::create_texture(const std::string &text,const GeomColor &color, int textSize){
#ifdef HAVE_QT
    int r = color[0];
    int g = color[1];
    int b = color[2];
    int a = color[3];
    QFont font("Arial",textSize);
    QFontMetrics m(font);
    QRectF br = m.boundingRect(text.c_str());
    //    QImage img(br.width()+2,br.height()+2,QImage::Format_ARGB32);
    // sometimes some of the right-most letters where cropped, so 
    // we add 10-extra pixels to the right hand side of the texture image
    QImage img(br.width()+12,br.height()+2,QImage::Format_ARGB32); 
    img.fill(0);
    QPainter painter(&img);
    painter.setFont(font);
    painter.setPen(QColor( b,g,r,iclMin(254,a)));
    painter.drawText(QPointF(1,img.height()-m.descent()-1),text.c_str());
    painter.end();

    Img8u buf(Size(img.width(),img.height()),4);   
    interleavedToPlanar(img.bits(),&buf);

    return buf;
#else
    // think of an ugly fallback implementation (maybe with this funky label image function from the IO package)
    return Img8u();
#endif
  }


  static void gl_color(const Primitive::RenderContext &ctx, int vertexIndex, bool condition=true){
    if(condition) glColor4fv((ctx.vertexColors[vertexIndex]/255).data());
  }
  static void gl_vertex(const Primitive::RenderContext &ctx, int vertexIndex){
    glVertex3fv(ctx.vertices[vertexIndex].data());
  }
  static void gl_normal(const Primitive::RenderContext &ctx, int normalIndex){
    if(normalIndex >= 0) glNormal3fv(ctx.normals[normalIndex].data());
  }
  static void gl_auto_normal(const Primitive::RenderContext &ctx, int a, int b, int c, bool condition=true){
    if(!condition) return;
    const Vec &va = ctx.vertices[a];
    const Vec &vb = ctx.vertices[b];
    const Vec &vc = ctx.vertices[c];
    
    glNormal3fv(normalize(cross(va-vc,vb-vc)).data());
  }

  void LinePrimitive::render(const Primitive::RenderContext &ctx){
    GLboolean lightWasOn = true;
    glGetBooleanv(GL_LIGHTING,&lightWasOn);
    glDisable(GL_LIGHTING);
    glBegin(GL_LINES);
    //    glNormal3f(0,0,0);

    for(int j=0;j<2;++j){
      gl_color(ctx,i(j),ctx.lineColorsFromVertices);
      gl_vertex(ctx,i(j));
    }

    glEnd();
    
    if(lightWasOn){
      glEnable(GL_LIGHTING);
    }           
  }

  void TrianglePrimitive::render(const Primitive::RenderContext &ctx){
    glBegin(GL_TRIANGLES);

    gl_auto_normal(ctx, i(0), i(1), i(2), i(3)==-1);
    
    glColor4f(color[0]/255, color[1]/255, color[2]/255, color[3]/255);

    for(int j=0;j<3;++j){
      gl_normal(ctx,i(j+3));
      gl_color(ctx,i(j),ctx.triangleColorsFromVertices);
      gl_vertex(ctx,i(j));
    }
    glEnd();
  }

  void QuadPrimitive::render(const Primitive::RenderContext &ctx){
    glBegin(GL_QUADS);
    
    gl_auto_normal(ctx, i(3), i(1), i(2), i(4)==-1);
    
    glColor4f(color[0]/255, color[1]/255, color[2]/255, color[3]/255);

    for(int j=0;j<4;++j){
      gl_normal(ctx,i(j+4));
      gl_color(ctx,i(j),ctx.quadColorsFromVertices);
      gl_vertex(ctx,i(j));
    }
    glEnd();
  }

  void PolygonPrimitive::render(const Primitive::RenderContext &ctx){
    glBegin(GL_POLYGON);
    
    // no autonormals supported!
    bool haveNormals = (idx.getHeight() == 2);
    glColor4f(color[0]/255, color[1]/255, color[2]/255, color[3]/255);
    
    for(int j=0;j<idx.getWidth();++j){
      if(haveNormals) gl_normal(ctx,idx(j,1));
      gl_color(ctx,idx(j,0),ctx.polygonColorsFromVertices);
      gl_vertex(ctx,idx(j,0));
    }
    glEnd();
  }
  
  void TexturePrimitive::render(const Primitive::RenderContext &ctx){
    if(image){
      texture.update(image);
    }

    const Vec &a = ctx.vertices[i(0)];
    const Vec &b = ctx.vertices[i(1)];
    const Vec &c = ctx.vertices[i(2)];
    const Vec &d = ctx.vertices[i(3)];
    
    if(i(4) != -1 && i(5) != -1 && i(6) != -1 && i(7) != -1){
      const Vec &na = ctx.normals[i(4)];
      const Vec &nb = ctx.normals[i(5)];
      const Vec &nc = ctx.normals[i(6)];
      const Vec &nd = ctx.normals[i(7)];
      texture.draw3D(a.data(),b.data(),c.data(),d.data(),
                     na.data(), nb.data(), nc.data(), nd.data());
    }else{
      gl_auto_normal(ctx, i(3), i(1), i(2));
      texture.draw3D(a.data(),b.data(),c.data(),d.data());
    }
  }

  void SharedTexturePrimitive::render(const Primitive::RenderContext &ctx){
    GLImg &gli = const_cast<GLImg&>(*ctx.sharedTextures[sharedTextureIndex]);
    const Vec &a = ctx.vertices[i(0)];
    const Vec &b = ctx.vertices[i(1)];
    const Vec &c = ctx.vertices[i(2)];
    const Vec &d = ctx.vertices[i(3)];
    if(i(4) != -1 && i(5) != -1 && i(6) != -1 && i(7) != -1){
      const Vec &na = ctx.normals[i(4)];
      const Vec &nb = ctx.normals[i(5)];
      const Vec &nc = ctx.normals[i(6)];
      const Vec &nd = ctx.normals[i(7)];
      gli.draw3D(a.data(),b.data(),c.data(),d.data(),
                 na.data(), nb.data(), nc.data(), nd.data());
    }else{
      gl_auto_normal(ctx, i(3), i(1), i(2));
      gli.draw3D(a.data(),b.data(),c.data(),d.data());
    }
  }
  
  void TextPrimitive::render(const Primitive::RenderContext &ctx){
    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_GREATER,0.3); 
    
    if(billboardHeight > 0){
      const Vec &a = ctx.vertices[i(0)];

      glMatrixMode(GL_MODELVIEW);
      float m[16];
      glGetFloatv(GL_MODELVIEW_MATRIX, m);
             
      /// inverted rotation matrix
      Mat R(m[0],m[1],m[2],0,
            m[4],m[5],m[6],0,
            m[8],m[9],m[10],0,
            0,0,0,1);
              
      float ry = billboardHeight/2;
      float rx = ry * (float(texture.getWidth())/float(texture.getHeight()));
              
      Vec p1 = a + R*Vec(-rx,-ry,0,1);
      Vec p2 = a + R*Vec(rx,-ry,0,1);
      Vec p3 = a + R*Vec(rx,ry,0,1);
      Vec p4 = a + R*Vec(-rx,ry,0,1);
              
      /// -normal as we draw the backface
      glNormal3fv(icl::normalize(-(cross(p2-p3,p4-p3))).data());
              
      /// draw the backface to flip x direction
      texture.draw3D(p2.begin(),p1.begin(),p4.begin(),p3.begin());
    }else{
      TexturePrimitive::render(ctx);
    }

    glAlphaFunc(GL_GREATER,0.05); 
  }


}
#if 0
  /*
      Primitive &Primitive::operator=(const Primitive &other){
      vertexIndices = other.vertexIndices;
      normalIndices = other.normalIndices;
      color = other.color;
      tex = other.tex.detached();
      type = other.type;
      hasNormals = other.hasNormals;
      return *this;
      }
  */
  void Primitive::detachTextureIfDeepCopied(){
    if(type == texture && texDeepCopied){
      tex.detach();
    }
  }


  Primitive::Primitive():
    type(nothing),billboardHeight(0){
  }
    
  /// Line-Constructor
  Primitive::Primitive(int a, int b, const GeomColor &color):
    vertexIndices(2),color(color),type(line),hasNormals(false),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
  }
  /// Line-Constructor
  Primitive::Primitive(int a, int b, const GeomColor &color, int na, int nb):
    vertexIndices(2),normalIndices(2),color(color),type(line),hasNormals(true),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    normalIndices[0] = na;
    normalIndices[1] = nb;
  }

  /// Triangle constructor
  Primitive::Primitive(int a, int b, int c, const GeomColor &color):
    vertexIndices(3),color(color),type(triangle),hasNormals(false),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
  }
    
  /// Triangle constructor
  Primitive::Primitive(int a, int b, int c, const GeomColor &color,int na, int nb, int nc):
    vertexIndices(3),normalIndices(3),color(color),type(triangle),hasNormals(true),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
  }
    
  /// Quad constructor
  Primitive::Primitive(int a, int b, int c, int d,const GeomColor &color):
    vertexIndices(4),color(color),type(quad),hasNormals(false),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
  }

  /// Quad constructor
  Primitive::Primitive(int a, int b, int c, int d,const GeomColor &color, int na, int nb, int nc, int nd):
    vertexIndices(4),normalIndices(4),color(color),type(quad),hasNormals(true),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
    normalIndices[3] = nd;
  }
    
  /// texture constructor
  Primitive::Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy, scalemode mode):
    vertexIndices(4),tex(tex),texDeepCopied(deepCopy),type(texture),mode(mode),hasNormals(false),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    if(deepCopy) this->tex.detach();
  }

  /// texture constructor
  Primitive::Primitive(int a, int b, int c, int d,const Img8u &tex, bool deepCopy, scalemode mode, 
                       int na, int nb, int nc, int nd):
    vertexIndices(4),normalIndices(4),tex(tex),texDeepCopied(deepCopy),type(texture),mode(mode),hasNormals(true),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
    normalIndices[3] = nd;
    if(deepCopy) this->tex.detach();
  }
    
  /// Special constructor to create a texture primitive that contains 3D text
  /** There is not special TEXT-type: type remains 'texture' */
  Primitive::Primitive(int a, int b, int c, int d, const std::string &text, const GeomColor &color, 
                       int textSize, scalemode mode):
    vertexIndices(4),tex(create_text_texture(text,color,textSize)),texDeepCopied(true),type(Primitive::text),
    mode(mode),hasNormals(false),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
  }

  /// Special constructor to create a texture primitive that contains 3D text
  /** There is not special TEXT-type: type remains 'texture' */
  Primitive::Primitive(int a, int b, int c, int d, const std::string &text, const GeomColor &color, 
                       int textSize, scalemode mode, int na, int nb, int nc, int nd):
    vertexIndices(4),normalIndices(4),tex(create_text_texture(text,color,textSize)),
    texDeepCopied(true),type(Primitive::text),
    mode(mode),hasNormals(true),billboardHeight(0){
    vertexIndices[0] = a;
    vertexIndices[1] = b;
    vertexIndices[2] = c;
    vertexIndices[3] = d;
    normalIndices[0] = na;
    normalIndices[1] = nb;
    normalIndices[2] = nc;
    normalIndices[3] = nd;
  }


  /// Creates a polygon primitive
  Primitive::Primitive(const std::vector<int> &polyData, const GeomColor &color):
    vertexIndices(polyData),color(color),type(polygon),hasNormals(false),billboardHeight(0){
  }

  /// Creates a polygon primitive
  Primitive::Primitive(const std::vector<int> &polyData, const GeomColor &color, const std::vector<int> &normalIndices):
    vertexIndices(polyData),normalIndices(normalIndices),color(color),type(polygon),hasNormals(true),billboardHeight(0){
  }

  Primitive::Primitive(int a, const std::string &text, const GeomColor &color, 
                      int textSize, float billboardHeight, scalemode mode):
    vertexIndices(1),tex(create_text_texture(text,color,textSize)),texDeepCopied(true),type(Primitive::text),
    mode(mode), hasNormals(false), billboardHeight(billboardHeight){
    vertexIndices[0] = a;
    
    //SHOW(color);
    //tex.mirror(axisVert);
    //tex.print();
    
  }
  /// Creates a deep copy (in particular deep copy of the texture image)
  //Primitive::Primitive(const Primitive &other){
  //  *this = other;
  //}


}
#endif
