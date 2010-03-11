/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCore module of ICL                         **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLCore/Line.h>
#include <math.h>
#include <algorithm>

using namespace std;

namespace icl{
  namespace{
    inline void swap_int(int &a, int &b){
      int tmp = a;
      a = b;
      b = tmp;
    }

    void bresenham(int x0, int x1, int y0, int y1, vector<int> &xs, vector<int> &ys,int minX, int maxX, int minY, int maxY){
      // {{{ open
      
      int steep = std::abs(y1 - y0) > std::abs(x1 - x0);
      if(steep){
        swap(x0, y0);
        swap(x1, y1);
      }
      int steep2 = x0 > x1;
      if(steep2){
        swap(x0, x1);
        swap(y0, y1);
      }
      
      int deltax = x1 - x0;
      int deltay = std::abs(y1 - y0);
      int error = 0;
      int ystep = y0 < y1 ? 1 : -1;

      if(minX == maxX || minY == maxY){
        for(int x=x0,y=y0;x<=x1;x++){
          if(steep){
            xs.push_back(y); 
            ys.push_back(x);
          }else{ // limits x <--> y ??
            xs.push_back(x); 
            ys.push_back(y);
          }
          
          error += deltay;
          if (2*error >= deltax){
            y += ystep;
            error -=deltax;
          }
        }
        
      }else{
        
        for(int x=x0,y=y0;x<=x1;x++){
          if(steep){
            if(x>=minY && x<maxY && y>=minX && y<maxX){
              xs.push_back(y); 
              ys.push_back(x);
            }
          }else{
            if(x>=minX && x<maxX && y>=minY && y<maxY){ // limits x <--> y ??
              xs.push_back(x); 
              ys.push_back(y);
            }
          }
          
          error += deltay;
          if (2*error >= deltax){
            y += ystep;
            error -=deltax;
          }
        }
      }
      if(steep2){
        reverse(xs.begin(),xs.end());
        reverse(ys.begin(),ys.end());
      }
    }
    // }}}
    
    void bresenham(int x0, int x1, int y0, int y1, vector<Point> &xys,int minX, int maxX, int minY, int maxY){
      // {{{ open

      int steep = std::abs(y1 - y0) > std::abs(x1 - x0);
      if(steep){
        swap(x0, y0);
        swap(x1, y1);
      }
      int steep2 = x0 > x1;
      if(steep2){
        swap(x0, x1);
        swap(y0, y1);
      }
      
      int deltax = x1 - x0;
      int deltay = std::abs(y1 - y0);
      int error = 0;
      int ystep = y0 < y1 ? 1 : -1;

      if(minX == maxX || minY == maxY){
         
        for(int x=x0,y=y0;x<=x1;x++){
          if(steep){
            xys.push_back(Point(y,x));
          }else{
            xys.push_back(Point(x,y));
          }
          
          error += deltay;
          if (2*error >= deltax){
            y += ystep;
            error -=deltax;
          }
        }
        
      }else{
        for(int x=x0,y=y0;x<=x1;x++){
          if(steep){
            if( x>=minY && x<maxY && y>=minX && y<=maxX){
              xys.push_back(Point(y,x));
            }
          }else{
            if(x>=minX && x<maxX && y>=minY && y<maxY){ // limits x <--> y ??
              xys.push_back(Point(x,y));
            }
          }
          
          error += deltay;
          if (2*error >= deltax){
            y += ystep;
            error -=deltax;
          }
        }

      }
      if(steep2){
        reverse(xys.begin(),xys.end());
      }
    }
  // }}}
    
  }

  Line::Line(Point start, float arc, float length):
    start(start){
    end.x = start.x + (int)(cos(arc)*length);
    end.y = start.y + (int)(sin(arc)*length);
  }
  
  float Line::length() const{
    return ::sqrt (pow((float) (start.x-end.x),2 ) +  pow((float) (start.y -end.y) ,2) );
  }
  
  std::vector<Point> Line::sample( const Rect &limits) const{
    std::vector<Point> l;
    bresenham(start.x,end.x,start.y,end.y,l,limits.x, limits.right(), limits.y, limits.bottom());
    return l;
  }
  void Line::sample(vector<int> &xs, vector<int> &ys, const Rect &limits ) const{
    bresenham(start.x,end.x,start.y,end.y,xs,ys,limits.x, limits.right(), limits.y, limits.bottom());
  }


  /// ostream operator (start-x,start-y)(end-x,end-y)
  std::ostream &operator<<(std::ostream &s, const Line &l){
    return s << l.start << l.end;
  }
  
  /// istream operator
  std::istream &operator>>(std::istream &s, Line &l){
    return s >> l.start >> l.end;
  }

}
