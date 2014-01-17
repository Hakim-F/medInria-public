/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "medMeshViewToolBox.h"

#include <dtkCore/dtkAbstractData.h>
#include <dtkCore/dtkAbstractDataFactory.h>
#include <dtkCore/dtkAbstractView.h>
#include <dtkCore/dtkAbstractViewInteractor.h>
#include <dtkCore/dtkSmartPointer.h>

#include <medDataManager.h>
#include <medAbstractDbController.h>
#include <medDbControllerFactory.h>
#include <v3dViewMeshInteractor.h>
#include <medMessageController.h>
#include <medDropSite.h>
#include <medMetaDataKeys.h>
#include <medImageFileLoader.h>

#include <medToolBoxFactory.h>
#include <v3dView.h>

#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera2.h>
#include <vtkRendererCollection.h>
#include <vtkObjectFactory.h>
#include <vtkImageView2D.h>
#include <vtkImageView3D.h>

// Define interaction style
class MouseInteractorStylePP : public vtkInteractorStyleTrackballCamera2
{
  public:
    static MouseInteractorStylePP* New();
    vtkTypeMacro(MouseInteractorStylePP, vtkInteractorStyleTrackballCamera2);
 
    virtual void OnLeftButtonDown() 
    { 
        if (this->Interactor->GetShiftKey() )
        {

            QString text1("Picking pixel: " + QString::number(this->Interactor->GetEventPosition()[0]) + " " + QString::number(this->Interactor->GetEventPosition()[1]));
            this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
                this->Interactor->GetEventPosition()[1], 
                0,  // always zero.
                this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
            double picked[3];
            this->Interactor->GetPicker()->GetPickPosition(picked);
            QString text2("Picked value: " + QString::number(picked[0]) + " " + QString::number(picked[1]) + " " + QString::number(picked[2]));
            tb->setPointPicked(picked);
            /*tb->setCoordsOfVertice(text2);*/
            // Forward events
        }
      vtkInteractorStyleTrackballCamera2::OnLeftButtonDown();
    };

    void setToolBox(medMeshViewToolBox * tb){this->tb = tb;};

private: 
    medMeshViewToolBox * tb;

};

vtkStandardNewMacro(MouseInteractorStylePP);

class medMeshViewToolBoxPrivate
{
public:
    v3dView * view;
    v3dViewMeshInteractor * interactor;
    dtkSmartPointer<dtkAbstractData> data;
    vtkSmartPointer<vtkPointPicker> pointPicker;

    double pointPicked[3];
    QDoubleSpinBox * sizeOfPoints;
    QCheckBox * followPickedPoints;
    QLabel * coordsOfVertice;
};

medMeshViewToolBox::medMeshViewToolBox(QWidget *parent) : medToolBox(parent), d(new medMeshViewToolBoxPrivate)
{
    QWidget *toolBoxBody = new QWidget(this);

    this->setTitle("Mesh View ToolBox");
    this->addWidget(toolBoxBody);
    QVBoxLayout * layoutBody = new QVBoxLayout(toolBoxBody);
    d->coordsOfVertice = new QLabel("",toolBoxBody);
    d->followPickedPoints = new QCheckBox(toolBoxBody);
    d->sizeOfPoints = new QDoubleSpinBox(toolBoxBody);
    layoutBody->addWidget(d->sizeOfPoints);
    layoutBody->addWidget(d->followPickedPoints);
    layoutBody->addWidget(d->coordsOfVertice);
    this->hide();

    d->pointPicker = vtkSmartPointer<vtkPointPicker>::New();
}

medMeshViewToolBox::~medMeshViewToolBox()
{
    delete d;
    d = NULL;
}

bool medMeshViewToolBox::registered()
{
    return medToolBoxFactory::instance()->registerToolBox<medMeshViewToolBox>("medMeshViewToolBox",
                                                                                   "medMeshViewToolBox",
                                                                                   "Mesh View toolbox",
                                                                                   QStringList()<<"mesh" << "view");
}

void medMeshViewToolBox::setData(dtkAbstractData *data)
{
    if ((!data) || (d->data==data) || (data->identifier() != "vtkDataMesh4D" && data->identifier() != "vtkDataMesh"))
        return;

    d->data = data;
}

void medMeshViewToolBox::update(dtkAbstractView *view)
{
    medToolBox::update(view);

    if (!view) {
        d->view = 0;
        d->data = 0;
        return;
    }

    d->interactor = qobject_cast<v3dViewMeshInteractor*>(view->dtkAbstractView::interactor("v3dViewMeshInteractor"));
    if (d->view == view) {
        if (d->interactor)
            this->setData (d->interactor->data()); // data may have changed
        return;
    }

    if (d->interactor) {

    /*    disconnect (this, SIGNAL(fiberSelectionValidated(const QString&, const QColor&)),
                    d->interactor, SLOT(validateSelection(QString,QColor)));
        disconnect (this, SIGNAL(fiberSelectionTagged()),
                    d->interactor, SLOT(tagSelection()));
        disconnect (this, SIGNAL(fiberSelectionReset()),
                    d->interactor, SLOT(resetSelection()));*/
    }

    d->view = qobject_cast<v3dView*>(view);
    connect(d->view, SIGNAL(propertySet(QString,QString)), this, SLOT(reSetInteractorStyle(QString,QString)),Qt::UniqueConnection);

    d->view->interactor()->SetPicker(d->pointPicker);
    if (d->view->property("Orientation")=="3D")
    {   
        vtkSmartPointer<MouseInteractorStylePP> style = vtkSmartPointer<MouseInteractorStylePP>::New();
        d->view->view3d()->GetInteractor()->SetInteractorStyle( style );
        style->setToolBox(this);
    }

    if (d->interactor) {
        connect(d->sizeOfPoints,SIGNAL(valueChanged(double)),d->interactor,SLOT(changeSizePoints(double)));

        if (d->interactor->enabled())
            this->show();

    }

    
}

void medMeshViewToolBox::setInput(dtkAbstractData *data)
{
    this->setData(data);
}

void medMeshViewToolBox::reSetInteractorStyle(QString key, QString value)
{
    if (key != "Orientation")
        return;
    
    if (value=="3D")
    {
        vtkSmartPointer<MouseInteractorStylePP> style = vtkSmartPointer<MouseInteractorStylePP>::New();
        d->view->view3d()->GetInteractor()->SetInteractorStyle( style );
        style->setToolBox(this);
    }
}

void medMeshViewToolBox::setPointPicked(double * point)
{
    d->pointPicked[0] = point[0];
    d->pointPicked[1] = point[1];
    d->pointPicked[2] = point[2];
    int indices[3];
    d->view->view2d()->GetImageCoordinatesFromWorldCoordinates(d->pointPicked,indices);
    QString text("Picked point: " + QString::number(indices[0]+1) + " " + QString::number(indices[1]+1) + " " + QString::number(indices[2]+1));
    d->coordsOfVertice->setText(text);
    d->view->view3d()->SetCurrentPoint(d->pointPicked);
}

