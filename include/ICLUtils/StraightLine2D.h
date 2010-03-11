/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
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

/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
** University of Bielefeld                                                **
** Contact: nivision@techfak.uni-bielefeld.de                             **
**                                                                        **
** This file is part of the ICLUtils module of ICL                        **
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

#ifndef ICL_STRAIGHT_LINE_2D_H
#define ICL_STRAIGHT_LINE_2D_H

#include <ICLUtils/FixedVector.h>
#include <ICLUtils/Point32f.h>
#include <ICLUtils/Exception.h>

namespace icl{
  
  /// A straight line is parameterized in offset/direction form
  /** This formular is used: 
      \f[ L(x) = \vec{o} + x\vec{v} \f]
      
      The template is instantiated for template parameter Pos type
      Point32f and FixedColVector<float,2>
  */
  struct StraightLine2D{
    /// internal typedef 
    typedef FixedColVector<float,2> PointPolar;

    /// internal typedef for 2D points
    typedef FixedColVector<float,2> Pos;
    
    /// creates a straight line from given angle and distance to origin
    StraightLine2D(float angle, float distance);
    
    /// creates a straight line from given 2 points
    StraightLine2D(const Pos &o=Pos(0,0), const Pos &v=Pos(0,0));

    /// 2D offset vector
    Pos o;
    
    /// 2D direction vector
    Pos v;
    
    /// computes closest distance to given 2D point
    float distance(const Pos &p) const;
    
    /// computes closest distance to given 2D point
    /* result is positive if p is left of this->v
        and negative otherwise */
    float signedDistance(const Pos &p) const;
    
    /// computes intersection with given other straight line
    /** if lines are parallel, an ICLException is thrown */
    Pos intersect(const StraightLine2D &o) const throw(ICLException);
    
    /// returns current angle and distance
    PointPolar getAngleAndDistance() const;
    
    /// retunrs the closest point on the straight line to a given other point
    Pos getClosestPoint(const Pos &p) const;
  };  
}

#endif
