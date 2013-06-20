// /////////////////////////////////////////////////////////////////
// Generated by medPluginGenerator
// /////////////////////////////////////////////////////////////////


#include "displacementFieldVisualization.h"
#include "displacementFieldVisualizationToolBox.h"

#include <QtGui>

#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractProcessFactory.h>
#include <dtkCore/dtkAbstractProcess.h>
#include <dtkCore/dtkAbstractViewFactory.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medAbstractView.h>

#include <medAbstractDataImage.h>

#include <medToolBoxFactory.h>
#include <medRegistrationSelectorToolBox.h>
#include <registrationFactory/registrationFactory.h>


class displacementFieldVisualizationToolBoxPrivate
{
public:
    QPushButton * saveButton;
    QPushButton * loadButton;
    QPushButton * warpButton;
    dtkSmartPointer<displacementFieldVisualization> m_VisuProcess; 
};

displacementFieldVisualizationToolBox::displacementFieldVisualizationToolBox(QWidget *parent) : medRegistrationAbstractToolBox(parent), d(new displacementFieldVisualizationToolBoxPrivate)
{

    d->saveButton = new QPushButton(QPixmap("E:/Medinria-OldCode/Modules/ImageFusion/pixmaps/fieldsave.xpm"),tr("Save"),this);
    d->loadButton = new QPushButton(QPixmap("E:/Medinria-OldCode/Modules/ImageFusion/pixmaps/fieldopen.xpm"),tr("Load"),this);
    d->warpButton = new QPushButton(QPixmap("E:/Medinria-OldCode/Modules/ImageFusion/pixmaps/fieldupdate.xpm"),tr("Warp"),this);
    d->warpButton->setToolTip("Display a grid warped according to the current transformation");
    d->saveButton->setToolTip("Save the displacement (vector) field to a file");
    d->loadButton->setToolTip("Load a displacement (vector) field");
    d->saveButton->setMinimumHeight(45);
    d->saveButton->setIconSize(QSize(40,40));
    d->loadButton->setMinimumHeight(45);
    d->loadButton->setIconSize(QSize(40,40));
    d->warpButton->setMinimumHeight(45);
    d->warpButton->setIconSize(QSize(40,40));
    d->warpButton->setEnabled(false);
    d->saveButton->setEnabled(false);
    
    connect(d->saveButton,SIGNAL(clicked()),this,SLOT(save()));
    connect(d->loadButton,SIGNAL(clicked()),this,SLOT(load()));
    connect(d->warpButton,SIGNAL(clicked()),SLOT(warpGrid()));
    
    d->m_VisuProcess = new displacementFieldVisualization();
    
    QHBoxLayout *layoutButton = new QHBoxLayout;
    layoutButton->addWidget(d->loadButton);
    layoutButton->addWidget(d->saveButton);
    layoutButton->addWidget(d->warpButton);
    
    QWidget * layoutSection = new QWidget(this);
    layoutSection->setLayout(layoutButton);

    addWidget(layoutSection);
    
    this->setTitle(tr("Displacement Field"));
}

displacementFieldVisualizationToolBox::~displacementFieldVisualizationToolBox()
{
    delete d;
    
    d = NULL;
}

bool displacementFieldVisualizationToolBox::registered()
{
    return medToolBoxFactory::instance()->
    registerToolBox<displacementFieldVisualizationToolBox>("displacementFieldVisualizationToolBox",
                               tr("Friendly name"),
                               tr("short tooltip description"),
                               QStringList() << "registration");
}

void displacementFieldVisualizationToolBox::load()
{

}

void displacementFieldVisualizationToolBox::save()
{

}

void displacementFieldVisualizationToolBox::warpGrid()
{

}