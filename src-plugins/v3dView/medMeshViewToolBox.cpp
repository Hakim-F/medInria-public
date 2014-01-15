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
      std::cout << "Picking pixel: " << this->Interactor->GetEventPosition()[0] << " " << this->Interactor->GetEventPosition()[1] << std::endl;
      this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0], 
                         this->Interactor->GetEventPosition()[1], 
                         0,  // always zero.
                         this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());
      double picked[3];
      this->Interactor->GetPicker()->GetPickPosition(picked);
      std::cout << "Picked value: " << picked[0] << " " << picked[1] << " " << picked[2] << std::endl;
      // Forward events
      vtkInteractorStyleTrackballCamera2::OnLeftButtonDown();
    }
 
};

vtkStandardNewMacro(MouseInteractorStylePP);

class medMeshViewToolBoxPrivate
{
public:
    v3dView * view;
    v3dViewMeshInteractor * interactor;
    dtkSmartPointer<dtkAbstractData> data;
    vtkSmartPointer<vtkPointPicker> pointPicker;

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

    d->view->interactor()->SetPicker(d->pointPicker);
    vtkSmartPointer<MouseInteractorStylePP> style = vtkSmartPointer<MouseInteractorStylePP>::New();
    d->view->view2d()->GetInteractor()->SetInteractorStyle( style );
    //d->view->view3d()->GetInteractor()->SetInteractorStyle( style );

    if (d->interactor) {
        connect(d->sizeOfPoints,SIGNAL(valueChanged(double)),d->interactor,SLOT(changeSizePoints(double)));
        /*connect (this, SIGNAL(fiberSelectionValidated(const QString&, const QColor&)),
                 d->interactor, SLOT(validateSelection(const QString&, const QColor&)));
        connect (this, SIGNAL(fiberSelectionTagged()),
                 d->interactor, SLOT(tagSelection()));
        connect (this, SIGNAL(fiberSelectionReset()),
                 d->interactor, SLOT(resetSelection()));

        d->bundleBoxCheckBox->blockSignals (true);
        d->bundleBoxCheckBox->setChecked(d->interactor->property("BoxVisibility")=="true" );
        d->bundleBoxCheckBox->blockSignals (false);

        this->setData (d->interactor->data());*/
        if (d->interactor->enabled())
            this->show();

    }

    
}

//TODO : watch changemenet dorientation ou passage en 3d pour reactiver le bon style. pas la peine de le mettre en 2d on voit pas les pts.

void medMeshViewToolBox::setInput(dtkAbstractData *data)
{
    this->setData(data);
}

