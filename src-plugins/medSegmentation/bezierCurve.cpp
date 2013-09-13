/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "bezierCurve.h"

#include <dtkCore/dtkAbstractProcessFactory.h>

// /////////////////////////////////////////////////////////////////
// bezierCurvePrivate
// /////////////////////////////////////////////////////////////////

class bezierCurvePrivate
{
public:
};

// /////////////////////////////////////////////////////////////////
// bezierCurve
// /////////////////////////////////////////////////////////////////

bezierCurve::bezierCurve() : dtkAbstractProcess(), d(new bezierCurvePrivate)
{
    
}

bezierCurve::~bezierCurve()
{
    
}

bool bezierCurve::registered()
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("bezierCurve", createBezierCurve);
}

QString bezierCurve::description() const
{
    return "bezierCurve";
}

// /////////////////////////////////////////////////////////////////
// Type instantiation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess *createBezierCurve()
{
    return new bezierCurve;
}
