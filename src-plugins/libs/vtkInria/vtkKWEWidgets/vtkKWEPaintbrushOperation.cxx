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
#include "vtkKWEPaintbrushOperation.h"
#include "vtkKWEPaintbrushShapeEllipsoid.h"
#include "vtkKWEPaintbrushData.h"
#include "vtkImageData.h"
#include "vtkMath.h"
#include "vtkObjectFactory.h"

vtkCxxRevisionMacro(vtkKWEPaintbrushOperation, "$Revision: 3591 $");
vtkStandardNewMacro(vtkKWEPaintbrushOperation);
vtkCxxSetObjectMacro(vtkKWEPaintbrushOperation,PaintbrushShape,vtkKWEPaintbrushShape);
vtkCxxSetObjectMacro(vtkKWEPaintbrushOperation,ImageData,vtkImageData);

//----------------------------------------------------------------------
vtkKWEPaintbrushOperation::vtkKWEPaintbrushOperation()
{
  this->ImageData          = NULL;  
  this->PaintbrushShape    = NULL;
  this->Extent[0] = this->Extent[2] = this->Extent[4] = 0;
  this->Extent[1] = this->Extent[3] = this->Extent[5] = -1;

  // Set a default shape
  vtkKWEPaintbrushShapeEllipsoid * paintbrushShape = vtkKWEPaintbrushShapeEllipsoid::New(); 
  this->SetPaintbrushShape( paintbrushShape );
  paintbrushShape->SetWidth(5.0,5.0,5.0);
  paintbrushShape->Delete();  
}

//----------------------------------------------------------------------
vtkKWEPaintbrushOperation::~vtkKWEPaintbrushOperation()
{
  this->SetPaintbrushShape(NULL);
  this->SetImageData(NULL);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushOperation::
GetPaintbrushData(vtkKWEPaintbrushData *paintbrushData, double p[3],
                  vtkKWEPaintbrushEnums::OperationType & op )
{
  if (!this->PaintbrushShape)
    {
    vtkErrorMacro( << "Set the shape prior to use" );
    return 0;
    }
  
  // Do the operation only if the paintbrush shape (whose center is denoted
  // by "p" is within the bounding extents) or of no bounding extents have 
  // been specified.
 
  bool isWithinBounds = false;
  bool uninitializedExtent = (this->Extent[0] > this->Extent[1]);
  if (!uninitializedExtent)
    {
    double spacing[3], origin[3];
    this->PaintbrushShape->GetSpacing(spacing);
    this->PaintbrushShape->GetOrigin(origin);
    
    double bounds[6] = 
      { this->Extent[0] * spacing[0] + origin[0],
        this->Extent[1] * spacing[0] + origin[0],
        this->Extent[2] * spacing[1] + origin[1],
        this->Extent[3] * spacing[1] + origin[1],
        this->Extent[4] * spacing[2] + origin[2],
        this->Extent[5] * spacing[2] + origin[2]  };
    
    double tolerance[3] = {1e-10, 1e-10, 1e-10};
    if (vtkMath::PointIsWithinBounds( p, bounds, tolerance ))
      {
      isWithinBounds = true;
      }
    }
  
  int retval = 0;
  if (isWithinBounds || uninitializedExtent)
    {
    // Perform the operation.
    retval = this->DoOperation(paintbrushData, p, op);
    
    // Clip the resulting paintbrushdata, if bounding extents have been 
    // specified.
    
    if (!uninitializedExtent )
      {
      paintbrushData->Clip( this->Extent );
      }
    }

  return retval;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushOperation::
DoOperation( vtkKWEPaintbrushData *data, double p[3],
             vtkKWEPaintbrushEnums::OperationType & op )
{
  op = vtkKWEPaintbrushEnums::Add;

  // The default, pass through operation, simply pulls data from the shape.
  return this->PaintbrushShape->GetPaintbrushData( data, p );
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushOperation::DeepCopy(vtkKWEPaintbrushOperation *op)
{
  if (op == this || !op)
    {
    return;
    }

  if (this->ImageData != op->ImageData) // operations on different image data
                                        // may be different.
    {
    return;
    }

  if (this->PaintbrushShape && op->PaintbrushShape)
    {
    this->PaintbrushShape->DeepCopy(op->PaintbrushShape);
    }
  this->Modified();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

