/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medAbstractRoi.h"

#include <dtkCore/dtkSmartPointer.h>

class medAbstractRoiPrivate
{
public:
    
};

medAbstractRoi::medAbstractRoi( dtkAbstractObject *parent )
    : dtkAbstractObject(parent)
    , d(new medAbstractRoiPrivate)
{
}


medAbstractRoi::~medAbstractRoi( void )
{
    delete d;
    d = NULL;
}

