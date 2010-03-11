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

#ifndef ICL_TYPES_H
#define ICL_TYPES_H

#include <ICLUtils/BasicTypes.h>

namespace icl {

  //forward declaration for the Image interface \ingroup TYPES
  class ImgBase;

  /// forward declaration of the Img-class \ingroup TYPES
  template<class T> class Img;

  /// typedef for 8bit integer images \ingroup TYPES
  typedef Img<icl8u> Img8u;

  /// typedef for 32bit float images \ingroup TYPES
  typedef Img<icl16s> Img16s;

  /// typedef for 8bit integer images \ingroup TYPES
  typedef Img<icl32s> Img32s;

  /// typedef for 32bit float images \ingroup TYPES
  typedef Img<icl32f> Img32f;

  /// typedef for 64bit float images \ingroup TYPES
  typedef Img<icl64f> Img64f;

  /// determines the pixel type of an image (8Bit-int or 32Bit-float) \ingroup TYPES 
  enum depth{
    depth8u  = 0, /**< 8Bit unsigned integer values range {0,1,...255} */
    depth16s = 1, /**< 16Bit signed integer values */  
    depth32s = 2, /**< 32Bit signed integer values */
    depth32f = 3, /**< 32Bit floating point values */
    depth64f = 4, /**< 64Bit floating point values */
    depthLast = depth64f
  };
  
  /// determines the color-format, that is associated with the images channels \ingroup TYPES
  enum format{
    formatGray   = 0, /**< 1-channel gray image, range of values is [0,255] as default */
    formatRGB    = 1, /**< (red,green,blue) colors pace */
    formatHLS    = 2, /**< (hue,lightness,saturation) color space (also know as HSI) */
    formatYUV    = 3, /**< (Y,u,v) color space */
    formatLAB    = 4, /**< (lightness,a*,b*) color space */
    formatChroma = 5, /**< 2 channel chromaticity color space */
    formatMatrix = 6, /**< n-channel image without a specified color space. */
    formatLast = formatMatrix
  };
  

#ifdef HAVE_IPP
  /// for scaling of Img images theses functions are provided \ingroup TYPES
  enum scalemode{
    interpolateNN=IPPI_INTER_NN,      /**< nearest neighbor interpolation */
    interpolateLIN=IPPI_INTER_LINEAR, /**< bilinear interpolation */
    interpolateRA=IPPI_INTER_SUPER    /**< region-average interpolation */
  };
#else
  /// for scaling of Img images theses functions are provided \ingroup TYPES
  enum scalemode{
    interpolateNN,  /**< nearest neighbor interpolation */
    interpolateLIN, /**< bilinear interpolation */
    interpolateRA   /**< region-average interpolation */
  };
#endif

  /// for flipping of images \ingroup TYPES
  enum axis{
#ifdef HAVE_IPP
    axisHorz=ippAxsHorizontal, /**> horizontal image axis */
    axisVert=ippAxsVertical,   /**> vertical image axis */
    axisBoth=ippAxsBoth        /**> flip both axis */
#else
    axisHorz, /**> horizontal image axis */
    axisVert, /**> vertical image axis */
    axisBoth  /**> flip both axis */
#endif
  };
}

#endif //ICL_TYPES_H
