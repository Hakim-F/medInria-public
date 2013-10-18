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
// .NAME vtkKWEITKConnectedThresholdPaintbrushOperation -
// .SECTION Description
// This is an abstract base class.
// .SECTION See Also

#ifndef __vtkKWEITKConnectedThresholdPaintbrushOperation_h
#define __vtkKWEITKConnectedThresholdPaintbrushOperation_h

#include "vtkKWEWidgetsExport.h" // needed for export symbols directives
#include "vtkKWEITKPaintbrushOperation.h"

class vtkImageStencilData;

class VTKEdge_WIDGETS_EXPORT vtkKWEITKConnectedThresholdPaintbrushOperation 
                                : public vtkKWEITKPaintbrushOperation
{
public:
  
  // Description:
  // Instantiate this class.
  static vtkKWEITKConnectedThresholdPaintbrushOperation *New();
  
  // Description:
  // Standard methods for instances of this class.
  vtkTypeRevisionMacro(vtkKWEITKConnectedThresholdPaintbrushOperation, 
                       vtkKWEITKPaintbrushOperation);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkKWEITKConnectedThresholdPaintbrushOperation();
  ~vtkKWEITKConnectedThresholdPaintbrushOperation();

  // Description:
  // See superclass Doc
  virtual void DoOperationOnStencil(vtkImageStencilData *, double p[3]);
  virtual int DoOperation( vtkKWEPaintbrushData *, double p[3],
                           vtkKWEPaintbrushEnums::OperationType & op );
  
private:
  vtkKWEITKConnectedThresholdPaintbrushOperation(
    const vtkKWEITKConnectedThresholdPaintbrushOperation&);  //Not implemented
  void operator=(const 
      vtkKWEITKConnectedThresholdPaintbrushOperation&);  //Not implemented
};

#endif
