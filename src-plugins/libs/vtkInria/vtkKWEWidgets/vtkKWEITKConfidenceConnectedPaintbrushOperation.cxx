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
#include "vtkKWEITKConfidenceConnectedPaintbrushOperation.h"
#include "vtkKWEITKConfidenceConnectedImageFilter.h"
#include "vtkKWEPaintbrushStroke.h"
#include "vtkKWEPaintbrushShape.h"
#include "vtkImageStencilData.h"
#include "vtkKWEPaintbrushStencilData.h"
#include "vtkKWEPaintbrushGrayscaleData.h"
#include "vtkObjectFactory.h"
#include "vtkImageExtractComponents.h"
#include "vtkExtractVOI.h"
#include "vtkKWEPaintbrushUtilities.h"

vtkCxxRevisionMacro(vtkKWEITKConfidenceConnectedPaintbrushOperation, "$Revision: 3591 $");
vtkStandardNewMacro(vtkKWEITKConfidenceConnectedPaintbrushOperation);

//----------------------------------------------------------------------
vtkKWEITKConfidenceConnectedPaintbrushOperation::vtkKWEITKConfidenceConnectedPaintbrushOperation()
{
}

//----------------------------------------------------------------------
vtkKWEITKConfidenceConnectedPaintbrushOperation::~vtkKWEITKConfidenceConnectedPaintbrushOperation()
{
}

template< class T > int 
PaintbrushRunner( vtkKWEITKConfidenceConnectedPaintbrushOperation * self, 
                  double center[3], 
                  vtkImageStencilData *stencil,
                  T )
{
  self->InternalFilter = NULL;

  typedef vtkitk::vtkKWEITKConfidenceConnectedImageFilter< T > PaintbrushFilterType;
  typename PaintbrushFilterType::Pointer filter = PaintbrushFilterType::New();
  self->InternalFilter = filter;

  // Set the update region to a certain region on either side of the center
  int extent[6], imageExtent[6];
  double spacing[3], origin[3];
  self->GetImageData()->GetSpacing(spacing);
  self->GetImageData()->GetExtent(imageExtent);

  // Set the center and radius to mask out a spherical stencil, instead of 
  // one with rectangular jagged edges.
  self->GetImageData()->GetOrigin(origin);
  double radiusFactor = 10.0 * spacing[0];  

  int xyz[3] = { (int)(center[0]/spacing[0] + 0.5),
                 (int)(center[1]/spacing[1] + 0.5),
                 (int)(center[2]/spacing[2] + 0.5) };
  
  double radius[3] = { radiusFactor/spacing[0],
                       radiusFactor/spacing[1],
                       radiusFactor/spacing[2] };
  
  extent[0] = (int)((center[0] - radiusFactor)/spacing[0] + 0.5);
  extent[1] = (int)((center[0] + radiusFactor)/spacing[0] + 0.5);
  extent[2] = (int)((center[1] - radiusFactor)/spacing[1] + 0.5);
  extent[3] = (int)((center[1] + radiusFactor)/spacing[1] + 0.5);
  extent[4] = (int)((center[2] - radiusFactor)/spacing[2] + 0.5);
  extent[5] = (int)((center[2] + radiusFactor)/spacing[2] + 0.5);
  vtkKWEPaintbrushUtilities::GetIntersectingExtents(extent, imageExtent, extent);
  
  // Despite the fact that the FilterModule framework supports updates on 
  // requested extents, a lot of filters in ITK (such as the 
  // ConfidenceConnectedImageFilter don't really support updating a subextent.
  // So in most cases, you will have to extract a VOI).
  vtkExtractVOI *extractVOI = vtkExtractVOI::New();
  extractVOI->SetInput( self->GetImageData());
  extractVOI->SetVOI(extent);
  
  // Extract the first component
  vtkImageExtractComponents * extractComponent = vtkImageExtractComponents::New();
  extractComponent->SetInput( extractVOI->GetOutput() );
  extractComponent->GetOutput()->SetUpdateExtent( extent );
  extractComponent->SetComponents(0);
  extractComponent->Update();
  filter->SetRequestedExtent(extent);
  
  // This is the filter that does most of the work. This is where most of the
  // time for this operation is spent
  filter->SetInput( extractComponent->GetOutput() );
  filter->AddSeed( xyz );
  filter->SetRequestedExtent(extent);
  filter->Update();
  filter->BoundWithRadiusOn();
  filter->SetCenter( xyz );
  filter->SetRadius( radius );
  filter->GetOutputAsStencil(stencil);

  extractComponent->Delete();
  extractVOI->Delete();
  return 1;
}

//----------------------------------------------------------------------
void vtkKWEITKConfidenceConnectedPaintbrushOperation::
DoOperationOnStencil(vtkImageStencilData *stencilData, double p[3])
{
  this->PaintbrushShape->GetStencil( stencilData, p);

  vtkImageStencilData * stencil = vtkImageStencilData::New();
  switch (this->ImageData->GetScalarType())
    {
    vtkitkTemplateMacro( PaintbrushRunner( this, p, stencil, 
          static_cast< VTK_TT >(0)));
    
    default:
      {
      vtkErrorMacro(<< 
          "vtkKWEITKConfidenceConnectedPaintbrushOperation: Unknown ScalarType");
      break;
      }
    }
  
  stencilData->Add(stencil);
  stencil->Delete();
}

//----------------------------------------------------------------------
int vtkKWEITKConfidenceConnectedPaintbrushOperation::
DoOperation( vtkKWEPaintbrushData *data, double p[3],
             vtkKWEPaintbrushEnums::OperationType & op )
{
  op = vtkKWEPaintbrushEnums::Add;

  // The paintbrush data can be binary or grayscale. Invoke the appropriate
  // filtering operation in each case.

  if (vtkKWEPaintbrushStencilData *sdata = 
    vtkKWEPaintbrushStencilData::SafeDownCast(data))
    {
    this->DoOperationOnStencil( sdata->GetImageStencilData(), p ); 
    }

  return 1;
}

//----------------------------------------------------------------------
void vtkKWEITKConfidenceConnectedPaintbrushOperation::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

