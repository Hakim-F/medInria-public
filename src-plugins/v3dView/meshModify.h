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
#include <v3dView.h>
#include <v3dViewPluginExport.h>
#include <v3dViewMeshInteractor.h>
#include <vtkSmartPointer.h>
#include <vtkPointPicker.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera2.h>
#include <vtkRendererCollection.h>

class vtkBoxWidget;
class vtkMyCallback;
class vtkMetaDataSet;
class MouseInteractorStylePP;

class V3DVIEWPLUGIN_EXPORT meshModifyToolBox : public medToolBox
{
    Q_OBJECT
    
public:
    meshModifyToolBox(QWidget * parent = 0);
    virtual ~meshModifyToolBox();
    
    virtual QString description() const;
    void update(dtkAbstractView * view);
    
    static bool registered();

    void setPointPicked(double * point);

public slots:

    void toggleWidget();
    void cancel();
    void dataAdded(dtkAbstractData* data, int index);

    void exportTransform();
    void importTransform();

    void reSetInteractorStyle(QString key, QString value);
    void moveToMPRmode(bool val);

private:
    v3dView * _view;
    QPushButton * _modifyButton;
    QPushButton * _cancelButton;
    QPushButton * _exportButton;
    QPushButton * _importButton;
    QSpinBox * _spinBox;
    vtkSmartPointer<vtkBoxWidget> _boxWidget;
    vtkSmartPointer<vtkMyCallback> _callback;
    vtkMetaDataSet * _dataset;
    bool _modifying;

    v3dViewMeshInteractor * _interactor;
    vtkSmartPointer<vtkPointPicker> _pointPicker;
    QDoubleSpinBox * _sizeOfPoints;
    QLabel * _coordsOfPoint;
    double _pointPicked[3];
};

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

    void setToolBox(meshModifyToolBox * tb){this->tb = tb;};

private: 
    meshModifyToolBox * tb;

};





