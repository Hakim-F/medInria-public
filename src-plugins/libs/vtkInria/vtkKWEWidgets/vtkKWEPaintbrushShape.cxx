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
#include "vtkKWEPaintbrushShape.h"
#include "vtkKWEPaintbrushData.h"
#include "vtkKWEPaintbrushGrayscaleData.h"
#include "vtkKWEPaintbrushStencilData.h"
#include "vtkKWEPaintbrushUtilities.h"
#include "vtkImageData.h"
#include "vtkObjectFactory.h"
#include <algorithm>

vtkCxxRevisionMacro(vtkKWEPaintbrushShape, "$Revision: 3591 $");

//----------------------------------------------------------------------
vtkKWEPaintbrushShape::vtkKWEPaintbrushShape()
{
  this->Spacing[0]       = this->Spacing[1] = this->Spacing[2] = 1.0;
  this->Origin[0]        = this->Origin[1]  = this->Origin[2]  = 0.0;
  this->ScalarType       = VTK_UNSIGNED_CHAR;
  this->Polarity         = vtkKWEPaintbrushEnums::Draw;
  this->ResizeConstraint = vtkKWEPaintbrushShape::PaintbrushResizeUnConstrained;
  this->Representation   = vtkKWEPaintbrushEnums::Binary;
  this->MaxWidth[0]      = this->MaxWidth[1] = this->MaxWidth[2] = -1.0;
  this->ClipExtent[0]    = VTK_INT_MIN;
  this->ClipExtent[2]    = VTK_INT_MIN;
  this->ClipExtent[4]    = VTK_INT_MIN;
  this->ClipExtent[1]    = VTK_INT_MAX;
  this->ClipExtent[3]    = VTK_INT_MAX;
  this->ClipExtent[5]    = VTK_INT_MAX;

  this->CachedShapeData  = NULL;
}

//----------------------------------------------------------------------
vtkKWEPaintbrushShape::~vtkKWEPaintbrushShape()
{
  if (this->CachedShapeData)
    {
    this->CachedShapeData->Delete();
    this->CachedShapeData = NULL;
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushShape::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushShape::GetPaintbrushData(vtkKWEPaintbrushData *d,
                                           int x,
                                           int y,
                                           int z)
{
  double worldPos[3];
  worldPos[0] = x * this->Spacing[0] + this->Origin[0];
  worldPos[1] = y * this->Spacing[1] + this->Origin[1];
  worldPos[2] = z * this->Spacing[2] + this->Origin[2];
  return this->GetPaintbrushData(d, worldPos);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushShape::UpdateCache(vtkKWEPaintbrushData *data)
{
  if (!this->CachedShapeData)
    {
    this->CachedShapeData = data->NewInstance();
    }

  // If different from what in cache, update our cache.
  int equal = this->CachedShapeData->IsEqual(data);
  if (!equal)
    {
    this->CachedShapeData->DeepCopy(data);    
    }

  vtkKWEPaintbrushVerboseMacro( "Does current shape result in same stencil as previous ?" << equal << std::endl);
  
  return equal ? 0 : 1;
}

//---------------------------------------------------------------------
int vtkKWEPaintbrushShape::GetPaintbrushData(vtkKWEPaintbrushData *d,
                                           double p[3])
{
  if (vtkKWEPaintbrushStencilData *s = vtkKWEPaintbrushStencilData::SafeDownCast(d))
    {
    this->GetStencil( s->GetImageStencilData(), p);
    }
  else if (vtkKWEPaintbrushGrayscaleData *t =
           vtkKWEPaintbrushGrayscaleData::SafeDownCast(d))
    {
    this->GetGrayscaleData( t->GetImageData(), p);
    }

  return this->UpdateCache(d);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushShape::DeepCopy(vtkKWEPaintbrushShape *s)
{
  if (s == this || !s)
    {
    return;
    }

  for (unsigned int i=0; i<3; i++)
    {
    this->Spacing[i] = s->Spacing[i];
    this->Origin[i]  = s->Origin[i];
    this->ScalarType = s->ScalarType;
    this->Polarity   = s->Polarity;
    }
  this->Modified();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushShape::SetClipExtent(int e[6])
{
  for (int i = 0; i < 6; i++)
    {
    this->ClipExtent[i] = e[i];
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushShape::GetClipExtent(int e[6])
{
  for (int i = 0; i < 6; i++)
    {
    e[i] = this->ClipExtent[i];
    }
}

//----------------------------------------------------------------------
int *vtkKWEPaintbrushShape::GetClipExtent()
{
  return this->ClipExtent;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushShape::SetPolarityToDraw()
{
  this->SetPolarity(vtkKWEPaintbrushEnums::Draw);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushShape::SetPolarityToErase()
{
  this->SetPolarity(vtkKWEPaintbrushEnums::Erase);
}

//----------------------------------------------------------------------
double vtkKWEPaintbrushShape::GetPositionTolerance2()
{
  const double halfMinSpacing = 
    std::min(std::min(this->Spacing[0], this->Spacing[1]), 
             this->Spacing[2]) / 2.0;
  return halfMinSpacing * halfMinSpacing;
}

