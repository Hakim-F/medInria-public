/*=========================================================================

 medInria

 Copyright (c) INRIA 2013 - 2014. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include <medToolBox.h>
#include <medGuiExport.h>
#include <medAbstractWorkspace.h>

class medAbstractData;
class medSegmentationSelectorToolBox;
class medSegmentationAbstractToolBoxPrivate;

//! Base class for custom segmentation algoithms
class MEDGUI_EXPORT medSegmentationAbstractToolBox : public medToolBox
{
    Q_OBJECT

public:
    //! Parent should be a medSegmentationSelectorToolBox
             medSegmentationAbstractToolBox(QWidget *parent = 0);
    virtual ~medSegmentationAbstractToolBox();

    virtual dtkPlugin* plugin() = 0;

    virtual medAbstractData *processOutput() = 0;

    virtual void setWorkspace(medAbstractWorkspace*);
    medAbstractWorkspace * getWorkspace();

signals:
    void success();

protected:
    //! Get the segmentationToolbox (usually one instance)
    medSegmentationSelectorToolBox *segmentationToolBox();

private:
    medSegmentationAbstractToolBoxPrivate *d;
};


