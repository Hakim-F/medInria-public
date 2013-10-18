/*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#include "vtkBalloonWidget.h"
#include <vtkContourRepresentation.h>
#include "vtkWidgetsAddOnExport.h"

class vtkBalloonRepresentation;
class vtkProp;
class vtkAbstractPropPicker;
class vtkStdString;
class vtkPropMap;
class vtkImageData;


class VTK_WIDGETSADDON_EXPORT vtkRoiBalloonWidget : public vtkBalloonWidget
{
public:
  // Description:
  // Instantiate this class.
  static vtkRoiBalloonWidget *New();

  // Description:
  // Standard methods for a VTK class.
  vtkTypeMacro(vtkRoiBalloonWidget,vtkBalloonWidget);
   void AttachToRightNode(vtkContourRepresentation * contourRep);
 protected:
  vtkRoiBalloonWidget();
  ~vtkRoiBalloonWidget();

  // This class implements the method called from its superclass.
  virtual int SubclassEndHoverAction();
  virtual int SubclassHoverAction();
  //void AttachToRightNode(vtkContourRepresentation * contourRep);
 
private:
  vtkRoiBalloonWidget(const vtkRoiBalloonWidget&);  //Not implemented
  void operator=(const vtkRoiBalloonWidget&);  //Not implemented
};

