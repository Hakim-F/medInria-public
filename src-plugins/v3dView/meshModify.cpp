/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/


#include <QDebug>

#include <dtkCore/dtkAbstractData>
#include <dtkCore/dtkAbstractDataFactory>

#include <medMetaDataKeys.h>
#include <meshModify.h>
#include <medToolBoxFactory.h>
#include <medDataManager.h>

#include <vtkActor.h>
#include <vtkBoxWidget.h>
#include <vtkCamera.h>
#include <vtkCommand.h>
#include <vtkTransform.h>
#include <vtkPolyData.h>

#include <vtkImageView3D.h>
#include <vtkMetaDataSet.h>
#include <vtkTransformFilter.h>
#include <vtkMetaSurfaceMesh.h>

class vtkMyCallback : public vtkCommand
{
public:
    static vtkMyCallback *New()
    { return new vtkMyCallback; }
    virtual void Execute(vtkObject *caller, unsigned long, void*)
    {
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        vtkBoxWidget * widget = reinterpret_cast<vtkBoxWidget*>(caller);
        widget->GetTransform(t);
        for(unsigned int i = 0; i < _dataset->GetNumberOfActors(); i++) {
            _dataset->GetActor(i)->SetUserTransform(t);
        }

    }
    void setDataSet(vtkMetaDataSet * dataset) {_dataset = dataset;}

private:
    vtkMetaDataSet * _dataset;
};


meshModifyToolBox::meshModifyToolBox(QWidget * parent)
    : medToolBox(parent)
    , _boxWidget(0)
    , _dataset(0)
    , _modifying(true)
{
    this->setTitle(tr("Mesh manipulation"));

    QWidget * w = new QWidget(this);
    this->addWidget(w);
    w->setLayout(new QVBoxLayout);
    _modifyButton = new QPushButton("Modify");
    _modifyButton->setEnabled(false);
    w->layout()->addWidget(_modifyButton);

    _cancelButton = new QPushButton("Cancel");
    _cancelButton->setEnabled(false);
    w->layout()->addWidget(_cancelButton);

    _spinBox = new QSpinBox();
    _spinBox->setRange(0,10);
    w->layout()->addWidget(_spinBox);

    connect(_modifyButton, SIGNAL(clicked()), this, SLOT(toggleWidget()));
    connect(_cancelButton, SIGNAL(clicked()), this, SLOT(cancel()));
}


meshModifyToolBox::~meshModifyToolBox()
{
    
}


bool meshModifyToolBox::registered()
{
    return medToolBoxFactory::instance()->registerToolBox<meshModifyToolBox>("meshModifyToolbox",
                                                                             "meshModifyToolbox",
                                                                             "Toolbox to translate/rotate a mesh",
                                                                             QStringList() << "mesh" << "view");
}


QString meshModifyToolBox::description() const
{
    return "meshModify";
}


void meshModifyToolBox::update(dtkAbstractView * view)
{
    if (_view != view)
        cancel();

    v3dView * view3d = qobject_cast<v3dView*>(view);
    if (! view3d) {
        _modifyButton->setEnabled(false);
        return;
    }

    _view = view3d;
    _modifyButton->setEnabled(true);
}


void meshModifyToolBox::toggleWidget()
{
    dtkAbstractData * data = _view->dataInList(_spinBox->value());

    if ( ! _dataset && ! data->identifier().contains("vtkDataMesh"))
        return;

    if ( ! _dataset) {
        _dataset = reinterpret_cast<vtkMetaDataSet*>(data->data());
        if ( ! _dataset ) return;
    }

    vtkPointSet * pointset = dynamic_cast<vtkPointSet*>(_dataset->GetDataSet());
    if ( ! pointset ) {_dataset = 0;return;}

    if (_modifying) {
        _boxWidget = vtkSmartPointer<vtkBoxWidget>::New();
        _boxWidget->SetInteractor(_view->view3d()->GetInteractor());
        _boxWidget->SetPlaceFactor(1.25);

        double bounds[6] = {}; // init to zero
        pointset->GetBounds(bounds);
        _boxWidget->PlaceWidget(bounds);
        _callback = vtkSmartPointer<vtkMyCallback>::New();
        _callback->setDataSet(_dataset);
        _boxWidget->AddObserver(vtkCommand::InteractionEvent, _callback);

        _boxWidget->On();
        _spinBox->setEnabled(false);
        _cancelButton->setEnabled(true);

    } else {
        vtkSmartPointer<vtkTransform> t = vtkSmartPointer<vtkTransform>::New();
        _boxWidget->GetTransform(t);

        vtkSmartPointer<vtkTransformFilter> transformFilter =
          vtkSmartPointer<vtkTransformFilter>::New();
        transformFilter->SetInput(pointset);
        transformFilter->SetTransform(t);
        transformFilter->Update();

        vtkPointSet * newPointset = pointset->NewInstance();
        newPointset->DeepCopy(transformFilter->GetOutput());

        dtkSmartPointer<dtkAbstractData> newData = dtkAbstractDataFactory::instance()->createSmartPointer("vtkDataMesh");

        newData->setMetaData(medMetaDataKeys::PatientName.key(), "John Doe");
        newData->setMetaData(medMetaDataKeys::StudyDescription.key(), "generated");
        newData->setMetaData(medMetaDataKeys::SeriesDescription.key(), "generated mesh");

        vtkMetaSurfaceMesh * smesh = vtkMetaSurfaceMesh::New();
        smesh->SetDataSet(newPointset);
        newData->setData(smesh);
        medDataManager::instance()->importNonPersistent( newData.data() );

        cancel();
    }

    _modifying = ! _modifying;
    _modifyButton->setText(_modifying ? "Modify" : "Save");
}


void meshModifyToolBox::cancel()
{
    if (_dataset) {
        // reset transforms on the original
        vtkSmartPointer<vtkTransform> t_id = vtkSmartPointer<vtkTransform>::New();
        for(unsigned int i = 0; i < _dataset->GetNumberOfActors(); i++) {
            _dataset->GetActor(i)->SetUserTransform(t_id);
        }
    }

    if (_boxWidget)
        _boxWidget->Off();

    _modifying = true;
    _modifyButton->setText("Modify");
    _spinBox->setEnabled(true);
    _cancelButton->setEnabled(false);
    _dataset = 0;
}


void meshModifyToolBox::dataAdded(dtkAbstractData * data, int index)
{
    if (data->identifier().contains("vtkDataMesh"))
    _modifyButton->setEnabled(true);
}
