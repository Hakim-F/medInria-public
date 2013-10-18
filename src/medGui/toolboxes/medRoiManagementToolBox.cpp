/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medRoiManagementToolBox.h"

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewInteractor.h>
#include <dtkLog/dtkLog.h>

#include <medAbstractView.h>
#include <medMessageController.h>
#include <medMetaDataKeys.h>
#include <medToolBoxFactory.h>
#include <medToolBoxTab.h>
#include <medViewManager.h>
#include <medWorkspace.h>
#include <medToolBoxTab.h>
#include <medAbstractRoi.h>
#include <QtGui>

class medRoiManagementToolBoxPrivate
{
public:
    medRoiManagementToolBoxPrivate() : workspace(NULL) { }

    typedef QList<medAbstractRoi*> * ListRois;

    medWorkspace * workspace;
    dtkSmartPointer<medAbstractView> currentView;
    medToolBoxTab * layoutToolBoxTab;
    
    medToolBox * toolBoxTab;
    QListWidget * ListAllRois;
    QListWidget * ListPolygonRois;
    QListWidget * ListBrushRois;
    QListWidget * ListCurrentSliceRois;

    QHash<medAbstractView*,ListRois> * viewsRoisMap;
};

medRoiManagementToolBox::medRoiManagementToolBox( medWorkspace * workspace, QWidget *parent) : medToolBox(parent), d(new medRoiManagementToolBoxPrivate)
{
    this->setTitle("Roi Management");
    d->workspace = workspace;
    d->currentView = NULL;
    
    d->layoutToolBoxTab = new medToolBoxTab(this);
    d->toolBoxTab = new medToolBox(this);
    d->toolBoxTab->setTitle("Regions of interest");
    d->toolBoxTab->setTabWidget(d->layoutToolBoxTab);
    
    QWidget * allRoisTab = new QWidget(this);
    QVBoxLayout *allLayout = new QVBoxLayout(allRoisTab);
    d->ListAllRois = new QListWidget(this);
    d->ListAllRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    allLayout->addWidget(d->ListAllRois);

    QWidget * currentSliceTab = new QWidget(this);
    QVBoxLayout *currentSliceLayout = new QVBoxLayout(currentSliceTab);
    d->ListCurrentSliceRois = new QListWidget(this);
    d->ListCurrentSliceRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    currentSliceLayout->addWidget(d->ListCurrentSliceRois);
    
    QWidget * PolygonRoisTab = new QWidget(this);
    QVBoxLayout *polygonLayout = new QVBoxLayout(PolygonRoisTab);
    d->ListPolygonRois = new QListWidget(this);
    d->ListPolygonRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    polygonLayout->addWidget(d->ListPolygonRois);

    QWidget * brushRoisTab = new QWidget(this);
    QVBoxLayout *brushLayout = new QVBoxLayout(brushRoisTab);
    d->ListBrushRois = new QListWidget(this);
    d->ListBrushRois->setSelectionMode(QAbstractItemView::ExtendedSelection);
    brushLayout->addWidget(d->ListBrushRois);

    d->layoutToolBoxTab->addTab(allRoisTab, tr("All"));
    d->layoutToolBoxTab->addTab(currentSliceTab, tr("Current Slice"));
    d->layoutToolBoxTab->addTab(PolygonRoisTab, tr("Bezier"));
    d->layoutToolBoxTab->addTab(brushRoisTab, tr("Brush"));

    this->addWidget(d->toolBoxTab);
}

medRoiManagementToolBox::~medRoiManagementToolBox(void)
{
    delete d;

    d = NULL;
}

void medRoiManagementToolBox::update( dtkAbstractView *view )
{
    medToolBox::update(view);
    d->currentView = dynamic_cast<medAbstractView*>(view);
    // TODO : update all the tabs for this current view
}

void medRoiManagementToolBox::addRoi(medAbstractView * view, medAbstractRoi * roi)
{
    if (!d->viewsRoisMap->contains(view))
    {
        QList<medAbstractRoi*> * listRois = new QList<medAbstractRoi*>();
        d->viewsRoisMap->insert(view,listRois);
    }
    d->viewsRoisMap->value(view)->append(roi);
}