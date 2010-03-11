/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
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

#ifndef ICL_GRADIENT_IMAGE_H
#define ICL_GRADIENT_IMAGE_H

#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>

namespace icl{
  
  /// Class for calculation of gradient image information
  /** The GradientImage class provides functionalities for
      the calculation of the following gradient image information:
      - X-Gradient (using 3x3 Sobel-X Convolution Kenrnel)
      - Y-Gradient (using 3x3 Sobel-Y Convolution Kenrnel)
      - Gradient Intensity (internally accelerated using a LUT)
        \f$ I=\sqrt(gx^2+gy^2) \f$ 
      - Gradient Angle (also accelerated using a LUT)
        \f$ I=atan2(gy,gx) \f$
      
      For best performance, the input image is restricted to be
      of type Img8u. The update function will firstly calculate
      X- and Y-Gradient information using a ConvolutionOp object.
      X- and Y-Gradient image have range [0,255] now (result of 
      Sobel-Filter is internally scaled to the range of the
      used datatype icl8u. Further calculation of gradient intensity
      and gradient angle depends on the given calculation mode
      flag.      
  */
  class GradientImage{
    public:
    
    /// flags to indicate which gradient information should be calculated
    enum calculationMode{
      calculateXYOnly=0,    ///< only calculate X- and Y-Gradient 
      calculateIntensity=1, ///< X/Y-Gradient and intensity map
      calculateAngle=2,     ///< X/Y-Gradient and angle map
      calculateAll=3        ///< X/Y-Gradient, intensity- and angle map
    };
    
    /// Empty constructor
    GradientImage(){}
    
    /// Main function calculates new gradient image information
    void update(const ImgBase *src, calculationMode mode=calculateAll); 
    
    /// return current X-Gradient image
    const Img16s &getGradXImage() const{ return m_oX; }

    /// return current Y-Gradient image
    const Img16s &getGradYImage() const{ return m_oY; }

    /// return current Gradient-Intensity image
    const Img32f &getGradIntensityImage() const { return m_oI; }

    /// return current Gradient-Angle image
    const Img32f &getGradAngleImage() const { return m_oA; }

    /// internally normalizes all images to range [0,255]
    void normalize();
    private:
    Img16s m_oBuf;
    Img16s m_oX,m_oY;
    Img32f m_oI,m_oA;
  };
}

#endif
