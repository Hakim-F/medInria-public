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
// .NAME vtkKWEITKConfidenceConnectedPaintbrushOperation -
// .SECTION Description
// This is an abstract base class.
// .SECTION See Also

#ifndef __vtkKWEITKConfidenceConnectedPaintbrushOperation_h
#define __vtkKWEITKConfidenceConnectedPaintbrushOperation_h

#include "vtkKWEWidgetsExport.h" // needed for export symbols directives
#include "vtkKWEITKPaintbrushOperation.h"

class vtkImageStencilData;

class VTKEdge_WIDGETS_EXPORT vtkKWEITKConfidenceConnectedPaintbrushOperation 
                                : public vtkKWEITKPaintbrushOperation
{
public:
  
  // Description:
  // Instantiate this class.
  static vtkKWEITKConfidenceConnectedPaintbrushOperation *New();
  
  // Description:
  // Standard methods for instances of this class.
  vtkTypeRevisionMacro(vtkKWEITKConfidenceConnectedPaintbrushOperation, 
                       vtkKWEITKPaintbrushOperation);
  void PrintSelf(ostream& os, vtkIndent indent);

protected:
  vtkKWEITKConfidenceConnectedPaintbrushOperation();
  ~vtkKWEITKConfidenceConnectedPaintbrushOperation();

  // Description:
  // See superclass Doc
  virtual void DoOperationOnStencil(vtkImageStencilData *, double p[3]);
  virtual int DoOperation( vtkKWEPaintbrushData *, double p[3],
                           vtkKWEPaintbrushEnums::OperationType & op );

  
private:
  vtkKWEITKConfidenceConnectedPaintbrushOperation(
    const vtkKWEITKConfidenceConnectedPaintbrushOperation&);  //Not implemented
  void operator=(const 
      vtkKWEITKConfidenceConnectedPaintbrushOperation&);  //Not implemented
};

#endif
