/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "medToolBox.h"

#include "v3dViewPluginExport.h"

class dtkAbstractView;
class dtkAbstractData;
class medMeshViewToolBoxPrivate;
class medDataIndex;

/**
  * @class medMeshViewToolBox
  * @brief 
  */
class V3DVIEWPLUGIN_EXPORT medMeshViewToolBox : public medToolBox
{
    Q_OBJECT
public:
     medMeshViewToolBox(QWidget *parent);
    ~medMeshViewToolBox();

    static bool registered();

    /**
      * 
      * 
      */
    virtual void setData(dtkAbstractData *data);

signals:
    /**
      * This signal is emitted when the user has picked a point.
      */
    void pointPicked();

public slots:

    void setInput(dtkAbstractData * data);

protected slots:

    /**
      * 
      */
    virtual void update (dtkAbstractView *view);

private:
    medMeshViewToolBoxPrivate *d;

};



