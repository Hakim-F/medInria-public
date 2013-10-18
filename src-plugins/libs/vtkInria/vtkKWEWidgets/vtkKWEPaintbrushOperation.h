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
// .NAME vtkKWEPaintbrushOperation - Perform inplace operations on paintbrush stencils
// .SECTION Description
// This is a base class for Paintbrush operations that perform smart inplace
// operations on paintbrush stencils. This class acts as a pass through
// filter that does not modify the incoming stencils. Subclasses must 
// override the protected method \c DoOperationOnStencil to modify the
// stencil possibly based on the underlying image data.
//
// .SECTION See Also

#ifndef __vtkKWEPaintbrushOperation_h
#define __vtkKWEPaintbrushOperation_h

#include "vtkKWEWidgetsExport.h" // needed for export symbols directives
#include "vtkKWEPaintbrushEnums.h" // needed for export symbols directives
#include "vtkObject.h"

class vtkImageStencilData;
class vtkKWEPaintbrushData;
class vtkKWEPaintbrushShape;
class vtkImageData;

class VTKEdge_WIDGETS_EXPORT vtkKWEPaintbrushOperation : public vtkObject
{
public:
  
  // Description:
  // Instantiate this class.
  static vtkKWEPaintbrushOperation *New();
  
  // Description:
  // Standard methods for instances of this class.
  vtkTypeRevisionMacro(vtkKWEPaintbrushOperation, vtkObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Set the paintbrush shape. This must be set.
  virtual void SetPaintbrushShape( vtkKWEPaintbrushShape* );
  vtkGetObjectMacro( PaintbrushShape, vtkKWEPaintbrushShape); 

  // Description:
  // Set the image data on which the paintbrush is drawn. This must be set.
  virtual void SetImageData( vtkImageData * );
  vtkGetObjectMacro( ImageData, vtkImageData );

  // Description:
  // Get the Paintbrush data for the shape at point 'p' filtered through
  // this operation. This class will simply return shape->GetPaintbrushData().
  // Subclasses would generally override this, so as to filter the 
  // paintbrushData from the shape.
  // Returns 1 if something was successfully added. A return value of 0
  // would indicate that there is no need to re-render the scene
  virtual int GetPaintbrushData(vtkKWEPaintbrushData *, double p[3],
                                vtkKWEPaintbrushEnums::OperationType & op );
  
  // Description:
  // The default behaviour is "You are allowed to paint everywhere." 
  // Optionally, you may retrict this via extents. ie. the operation will
  // filter data through only if it lies within the specified extents. 
  vtkSetVector6Macro( Extent, int );  
  vtkGetVector6Macro( Extent, int );  
  
  // Description:
  // INTERNAL: Do not use.
  // Deep copy.. Synchronizes states etc.
  virtual void DeepCopy(vtkKWEPaintbrushOperation *);
  
protected:
  vtkKWEPaintbrushOperation();
  ~vtkKWEPaintbrushOperation();

  // Description:
  // Filter the incoming data (first arg) through this operation. The operation
  // is centered at the point 'p'.
  // Returns 1 if something was successfully added. A return value of 0
  // would indicate that there is no need to re-render the scene
  virtual int DoOperation( vtkKWEPaintbrushData *, double p[3],
                           vtkKWEPaintbrushEnums::OperationType & op );
  
  vtkImageData                 *ImageData;
  vtkKWEPaintbrushShape           *PaintbrushShape;
  int                           Extent[6];

private:
  vtkKWEPaintbrushOperation(const vtkKWEPaintbrushOperation&);  //Not implemented
  void         operator=(const vtkKWEPaintbrushOperation&);  //Not implemented
};

#endif
