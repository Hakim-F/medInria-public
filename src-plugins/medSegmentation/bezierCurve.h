/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCore/dtkAbstractProcess.h>

#include "msegPluginExport.h"

class bezierCurvePrivate;
    
class MEDVIEWSEGMENTATIONPLUGIN_EXPORT bezierCurve : public dtkAbstractProcess
{
    Q_OBJECT
    
public:
    bezierCurve();
    virtual ~bezierCurve();
    
    virtual QString description() const;
    
    static bool registered();
    
private:
    bezierCurvePrivate *d;
};

dtkAbstractProcess *createBezierCurve();


