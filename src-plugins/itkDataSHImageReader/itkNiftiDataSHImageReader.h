/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <itkDataSHImageReaderBase.h>
#include <itkDataSHImageReaderPluginExport.h>

class ITKDATASHIMAGEREADERPLUGIN_EXPORT itkNiftiDataSHImageReader : public itkDataSHImageReaderBase
{
    Q_OBJECT
	
public:
    itkNiftiDataSHImageReader();
    virtual ~itkNiftiDataSHImageReader();
	
    virtual QString description() const;
    virtual QString identifier() const;
	
    static bool registered();	
};

dtkAbstractDataReader *createItkNiftiDataSHImageReader();
