/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medToolBox.h>
#include <medAbstractView.h>
#include <medGuiExport.h>
#include <medAbstractRoi.h>

class dtkAbstractView;
class medRoiManagementToolBoxPrivate;
class medWorkspace;

//! Roi Management toolbox
class MEDGUI_EXPORT medRoiManagementToolBox : public medToolBox
{
    Q_OBJECT;

public:
    medRoiManagementToolBox(medWorkspace * workspace, QWidget *parent = 0);
    ~medRoiManagementToolBox();

signals:
    
public slots:
    // Override base class
    void update (dtkAbstractView *view);
    void addRoi(medAbstractView * view, medAbstractRoi * roi);

private:
    medRoiManagementToolBoxPrivate *d;
};


