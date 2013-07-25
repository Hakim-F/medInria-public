/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "itkFiltersErodeProcess.h"

#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medMetaDataKeys.h>

#include "itkFiltersErodeProcess_p.h"

//-------------------------------------------------------------------------------------------

itkFiltersErodeProcess::itkFiltersErodeProcess(itkFiltersErodeProcess *parent) 
    : itkFiltersProcessBase(*new itkFiltersErodeProcessPrivate(this), parent)
{
    DTK_D(itkFiltersErodeProcess);
    
    d->filter = this;
    d->output = NULL;
    d->size = 5; 
    //d->ballSize.Fill(40);
    //ball.SetRadius(ballSize);
    //ball.CreateStructuringElement();

    d->description = tr("ITK Erode filter");
}


itkFiltersErodeProcess::itkFiltersErodeProcess(const itkFiltersErodeProcess& other) 
    : itkFiltersProcessBase(*new itkFiltersErodeProcessPrivate(*other.d_func()), other)
{
}

//-------------------------------------------------------------------------------------------

itkFiltersErodeProcess::~itkFiltersErodeProcess( void )
{
}

//-------------------------------------------------------------------------------------------

bool itkFiltersErodeProcess::registered( void )
{
    return dtkAbstractProcessFactory::instance()->registerProcessType("itkErodeProcess", createitkFiltersErodeProcess);
}

//-------------------------------------------------------------------------------------------

void itkFiltersErodeProcess::setParameter(double data, int channel)
{
    if (channel != 0)
        return;
    
    DTK_D(itkFiltersErodeProcess);
    d->size = data;
    //d->ballSize.Fill(data);
    //ball.SetRadius(ballSize);
    //ball.CreateStructuringElement();

}

//-------------------------------------------------------------------------------------------

int itkFiltersErodeProcess::update ( void )
{
    DTK_D(itkFiltersErodeProcess);
    
    if ( !d->input )
        return -1;

    QString id = d->input->identifier();

    qDebug() << "itkFilters, update : " << id;

    if ( id == "itkDataImageChar3" )
    {
        d->update<char>();
    }
    else if ( id == "itkDataImageUChar3" )
    {
        d->update<unsigned char>();
    }
    else if ( id == "itkDataImageShort3" )
    {
        d->update<short>();
    }
    else if ( id == "itkDataImageUShort3" )
    {
        d->update<unsigned short>();
    }
    else if ( id == "itkDataImageInt3" )
    {
        d->update<int>();
    }
    else if ( id == "itkDataImageUInt3" )
    {
        d->update<unsigned int>();
    }
    else if ( id == "itkDataImageLong3" )
    {
        d->update<long>();
    }
    else if ( id== "itkDataImageULong3" )
    {
        d->update<unsigned long>();
    }
    else if ( id == "itkDataImageFloat3" )
    {
        d->update<float>();
    }
    else if ( id == "itkDataImageDouble3" )
    {
        d->update<double>();
    }
    else
    {
        qDebug() << "Error : pixel type not yet implemented ("
        << id
        << ")";
        return -1;
    }

    return EXIT_SUCCESS;
}


// /////////////////////////////////////////////////////////////////
// Type instanciation
// /////////////////////////////////////////////////////////////////

dtkAbstractProcess * createitkFiltersErodeProcess ( void )
{
    return new itkFiltersErodeProcess;
}
