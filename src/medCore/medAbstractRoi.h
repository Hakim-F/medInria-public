/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <dtkCore\dtkAbstractObject.h>

#include "medCoreExport.h"

class medAbstractRoiPrivate;

/**
 * 
 */
class MEDCORE_EXPORT medAbstractRoi : public dtkAbstractObject
{
    Q_OBJECT

public:
    medAbstractRoi( dtkAbstractObject * parent = NULL );
    virtual ~medAbstractRoi();

    virtual void Off()=0;
    virtual void On()=0;

public slots:
    
signals:
    
private:
    medAbstractRoiPrivate * d;
};


