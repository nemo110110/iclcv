/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLOpenCV module of ICL                       **
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

#ifndef ICL_OPEN_CV_H
#define ICL_OPEN_CV_H

#include <cxcore.h>


/** CvMat type
    
struct CvMat{
    int type;
    int step;
    int* refcount;

    uchar* ptr;

    int rows;
    int cols;
};
*/

namespace icl{
  
  // TODO: everything !

  // p = 7
  // size is 1
  template<class T>
  T* icl_allign(T *p, int n=4){
    return reinterpret_cast<T*>( ((size_t)p+n-1) & -n);
  }
  
  template<class T>
  Img<T> *deep_copy_to_img_base_t(const void *data, int w, int h){
    Img<T> * image = new Img<T>(Size(w,h),1);
    // rows a 4-byte alligned 
    const T* src = reinterpret_cast<const T*>(data);
    T* dst = image->begin(0);
    for(int y=0;y<h;++y){
      std::copy(src,src+w,dst+w*y);
      src = icl_allign(src);
    }
  }
  
  ImgBase *deepCopyToImgBase(const CvMat *m){
    ICLASSERT_RETURN_VAL(m,0);
    switch(CV_MAT_DEPTH(m->type)){
      case CV_8S:
        WARNING_LOG("using icl8u to handle CV_8S data type!");
      case CV_8U:
        return deep_copy_to_img_base_t<icl8u>(m.ptr,m.cols,m.rows);
      case CV_16U:
        WARNING_LOG("using icl16s to handle CV_16U data type!");
      case CV_16S:
        return deep_copy_to_img_base_t<icl16s>(m.ptr,m.cols,m.rows);
      case CV_32S:
        return deep_copy_to_img_base_t<icl32s>(m.ptr,m.cols,m.rows);
      case CV_32F:
        return deep_copy_to_img_base_t<icl32f>(m.ptr,m.cols,m.rows);
      case CV_64F:
        return deep_copy_to_img_base_t<icl64f>(m.ptr,m.cols,m.rows);
    }
    return 0;
  }
  
}

#endif
