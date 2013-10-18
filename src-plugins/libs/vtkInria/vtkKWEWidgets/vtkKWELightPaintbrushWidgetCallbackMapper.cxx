//=========================================================================
// 
//   Copyright (c) Kitware, Inc.
//   All rights reserved.
//   See Copyright.txt or http://www.kitware.com/VolViewCopyright.htm for details.
// 
//      This software is distributed WITHOUT ANY WARRANTY; without even
//      the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//      PURPOSE.  See the above copyright notice for more information.
// 
//=========================================================================
#include "vtkKWELightPaintbrushWidgetCallbackMapper.h"
#include "vtkWidgetEventTranslator.h"
#include "vtkAbstractWidget.h"
#include "vtkCommand.h"
#include "vtkObjectFactory.h"
#include "vtkKWEPaintbrushWidget.h"
#include "vtkEvent.h"
#include "vtkWidgetEvent.h"

vtkStandardNewMacro(vtkKWELightPaintbrushWidgetCallbackMapper);

//----------------------------------------------------------------------------
void vtkKWELightPaintbrushWidgetCallbackMapper::Bindings()
{
  // Remove any events from the existing callback mapper.

  this->EventTranslator->ClearEvents();

  // These are the event callbacks supported by this widget

  this->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
           vtkEvent::NoModifier, 0, 0, NULL,
           vtkKWEPaintbrushWidget::BeginDrawStrokeForThisSketchEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::BeginDrawThisSketchCallback);  
  this->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
           vtkEvent::ControlModifier, 0, 0, NULL,
           vtkKWEPaintbrushWidget::BeginEraseStrokeForThisSketchEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::BeginEraseThisSketchCallback);
  this->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
           vtkEvent::ShiftModifier, 0, 0, NULL,
           vtkKWEPaintbrushWidget::BeginDrawStrokeEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::BeginDrawCallback);
  this->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
           vtkEvent::ControlModifier | vtkEvent::ShiftModifier, 0, 0, NULL,
           vtkKWEPaintbrushWidget::BeginEraseStrokeEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::BeginEraseCallback);
  this->SetCallbackMethod(vtkCommand::MouseMoveEvent,
           vtkWidgetEvent::Move,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::MoveCallback);
  this->SetCallbackMethod(vtkCommand::LeftButtonReleaseEvent,
           vtkKWEPaintbrushWidget::EndStrokeEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::EndStrokeCallback);
  this->SetCallbackMethod(vtkCommand::LeftButtonPressEvent,
           vtkEvent::ShiftModifier, 0, 0, NULL,
           vtkKWEPaintbrushWidget::BeginIsotropicResizeEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::BeginIsotropicResizeShapeCallback);
  this->SetCallbackMethod(vtkCommand::EnterEvent,
           vtkKWEPaintbrushWidget::EnterEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::EnterWidgetCallback);
  this->SetCallbackMethod(vtkCommand::LeaveEvent,
           vtkKWEPaintbrushWidget::LeaveEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::LeaveWidgetCallback);
  this->SetCallbackMethod(vtkCommand::KeyPressEvent, // Ctrl+z
           vtkEvent::ControlModifier, 26, 0,"z",
           vtkKWEPaintbrushWidget::UndoStrokeEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::UndoCallback);
  this->SetCallbackMethod(vtkCommand::KeyPressEvent, // Ctrl+y
           vtkEvent::ControlModifier, 25, 0,"y",
           vtkKWEPaintbrushWidget::RedoStrokeEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::RedoCallback);
  this->SetCallbackMethod(vtkCommand::KeyPressEvent,
           vtkEvent::NoModifier, 27, 1,"Escape",
           vtkKWEPaintbrushWidget::ToggleSelectStateEvent,
           this->PaintbrushWidget, vtkKWEPaintbrushWidget::ToggleSelectStateCallback);
}

//----------------------------------------------------------------------------
void vtkKWELightPaintbrushWidgetCallbackMapper::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);
}


