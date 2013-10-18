/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#include "bezierPolygonRoi.h"
#include <vtkContourWidget.h>
#include <vtkContourOverlayRepresentation.h>
#include <vtkSmartPointer.h>
#include <vtkProperty2D.h>
#include <vtkImageView2D.h>
#include <vtkWidgetEventTranslator.h>

class BezierRoiObserver : public vtkCommand
{
public:
    static BezierRoiObserver* New()
    {
        return new BezierRoiObserver;
    }

    void Execute ( vtkObject *caller, unsigned long event, void *callData );

    void setRoi ( bezierPolygonRoi *roi )
    {
        this->roi = roi;
    }
    
    inline void lock()
    {
        this->m_lock = 1;
    }
    inline void unlock()
    {
        this->m_lock = 0;
    }

protected:
    BezierRoiObserver();
    ~BezierRoiObserver();

private:
    int m_lock;
    bezierPolygonRoi * roi;
};

BezierRoiObserver::BezierRoiObserver()
{
    this->m_lock = 0;
}

BezierRoiObserver::~BezierRoiObserver(){}

void BezierRoiObserver::Execute ( vtkObject *caller, unsigned long event, void *callData )
{
    if (this->m_lock )
        return;

    if (!this->roi->getView())
        return;

    switch ( event )
    {
        case vtkImageView2D::SliceChangedEvent:
        default:
        {
            roi->showOrHide(roi->getView()->GetViewOrientation(),roi->getView()->GetSlice());
            break;
        }
    }
}

class bezierPolygonRoiPrivate
{
public:
    vtkContourWidget * contour;
    unsigned int idSlice;
    unsigned char orientation; // 0 => Sagittal ... 1 => Coronal ... 2 => Axial
    vtkImageView2D *view;
    BezierRoiObserver *observer;
};

bezierPolygonRoi::bezierPolygonRoi(vtkImageView2D * view, medAbstractRoi *parent )
    : medAbstractRoi(parent)
    , d(new bezierPolygonRoiPrivate)
{
    vtkSmartPointer<vtkContourOverlayRepresentation> contourRep = vtkSmartPointer<vtkContourOverlayRepresentation>::New();
    contourRep->GetLinesProperty()->SetColor(0, 0, 1); 
    contourRep->GetLinesProperty()->SetLineWidth(1);
    contourRep->GetProperty()->SetPointSize(4);

    d->contour = vtkContourWidget::New();
    d->contour->SetRepresentation(contourRep);
    contourRep->SetRenderer(view->GetRenderer());
    d->contour->SetInteractor(view->GetInteractor());

    d->contour->GetEventTranslator()->SetTranslation(vtkCommand::RightButtonPressEvent,NULL);
    
    d->view = view;
    d->orientation = view->GetViewOrientation();
    d->idSlice = view->GetSlice();
    d->observer = BezierRoiObserver::New();
    d->observer->setRoi(this);
    d->view->AddObserver(vtkImageView2D::SliceChangedEvent,d->observer,0);
}


bezierPolygonRoi::~bezierPolygonRoi( void )
{
    delete d;
    d = NULL;
}

vtkContourWidget * bezierPolygonRoi::getContour()
{
    return d->contour;
}

void bezierPolygonRoi::Off()
{
    d->contour->Off();
}
void bezierPolygonRoi::On()
{
    d->contour->On();
}

void bezierPolygonRoi::showOrHide(int orientation,int idSlice)
{
    if (!d->view->GetRenderWindow())
        return;

    if (d->idSlice==idSlice && d->orientation==orientation)
        On();
    else
        Off();
}

unsigned int bezierPolygonRoi::getIdSlice()
{
    return d->idSlice;
}

void bezierPolygonRoi::setIdSlice(unsigned int idSlice)
{
    d->idSlice=idSlice;
}

unsigned char bezierPolygonRoi::getOrientation()
{
    return d->orientation;
}

void bezierPolygonRoi::setOrientation(unsigned char orientation)
{
    d->orientation=orientation;
}

vtkImageView2D * bezierPolygonRoi::getView()
{
    return d->view;
}

void bezierPolygonRoi::lockObserver(bool val)
{
    if (val)
        d->observer->lock();
    else
        d->observer->unlock();
}