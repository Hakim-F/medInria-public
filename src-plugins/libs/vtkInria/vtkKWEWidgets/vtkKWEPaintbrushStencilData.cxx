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
#include "vtkKWEPaintbrushStencilData.h"

#include "vtkInformation.h"
#include "vtkInformationVector.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkImageIterator.h"
#include "vtkImageStencilData.h"
#include "vtkKWEPaintbrushUtilities.h"
#include "vtkImageData.h"
#include <math.h>

vtkCxxRevisionMacro(vtkKWEPaintbrushStencilData, "$Revision: 3597 $");
vtkStandardNewMacro(vtkKWEPaintbrushStencilData);

//----------------------------------------------------------------------------
vtkKWEPaintbrushStencilData::vtkKWEPaintbrushStencilData()
{
  this->ImageStencilData = vtkImageStencilData::New();
  this->Information->Set(vtkDataObject::DATA_EXTENT_TYPE(), VTK_3D_EXTENT);
  this->Information->Set(vtkDataObject::DATA_EXTENT(), 
                         this->ImageStencilData->GetExtent(), 6);
}

//----------------------------------------------------------------------------
vtkKWEPaintbrushStencilData::~vtkKWEPaintbrushStencilData()
{
  this->SetImageStencilData(NULL);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::SetImageStencilData(vtkImageStencilData *s)
{
  vtkSetObjectBodyMacro( ImageStencilData, vtkImageStencilData, s );
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::ShallowCopy(vtkDataObject *o)
{
  vtkKWEPaintbrushStencilData *s=vtkKWEPaintbrushStencilData::SafeDownCast(o);

  if (s)
    {
    this->ImageStencilData->ShallowCopy(s->GetImageStencilData());
    }

  vtkDataObject::ShallowCopy(o);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::DeepCopy(vtkDataObject *o)
{
  vtkKWEPaintbrushStencilData *s=vtkKWEPaintbrushStencilData::SafeDownCast(o);

  if(s)
    {
    this->ImageStencilData->DeepCopy(s->GetImageStencilData());
    }

  vtkDataObject::DeepCopy(o);
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Add(vtkImageStencilData *s,
                                     bool vtkNotUsed(forceMutable))
{
  // FIXME Must actually be vtkImageStencilData::InternalAdd here
  // Make the protected method public later and give it a sensible name
  this->ImageStencilData->Add(s);
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Subtract(vtkImageStencilData *s,
                                          bool vtkNotUsed(forceMutable))
{
  this->ImageStencilData->Subtract(s);
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Replace(vtkImageStencilData *s,
                                          bool vtkNotUsed(forceMutable))
{
  this->ImageStencilData->Replace(s);
  return 1;
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Add(vtkKWEPaintbrushData *d,
                                     bool forceMutable)
{
  vtkKWEPaintbrushStencilData *s=
    vtkKWEPaintbrushStencilData::SafeDownCast(d);
  if(s)
    {
    return this->Add(s->GetImageStencilData(), forceMutable);
    }
  return 0;
}

//----------------------------------------------------------------------------
// Add all pixels matching the 'label' in the image to ourself
int vtkKWEPaintbrushStencilData::Add(vtkImageData *i, int label,
                                     bool forceMutable)
{
  // Create a stencil from the image
  vtkSmartPointer< vtkImageStencilData > stencilData =
          vtkSmartPointer< vtkImageStencilData >::New();
  stencilData->SetExtent(i->GetExtent());
  stencilData->SetSpacing(i->GetSpacing());
  stencilData->SetOrigin(i->GetOrigin());
  stencilData->AllocateExtents();

  // Extract everything with a scalar value of 'label' from the image onto
  // a stencil
  vtkKWEPaintbrushUtilities::GetStencilFromImage<
      vtkKWEPaintbrushUtilities::vtkFunctorEqualTo >(
                  i, stencilData, label );

  // now add the stencil.
  return this->Add(stencilData, forceMutable);
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Subtract(vtkKWEPaintbrushData *d,
                                          bool forceMutable)
{
  vtkKWEPaintbrushStencilData *s=vtkKWEPaintbrushStencilData::SafeDownCast(d);
  if(s)
    {
    return this->Subtract(s->GetImageStencilData(), forceMutable);
    }
  return 0;
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Replace(vtkKWEPaintbrushData *d,
                                          bool forceMutable)
{
  vtkKWEPaintbrushStencilData *s=vtkKWEPaintbrushStencilData::SafeDownCast(d);
  if(s)
    {
    return this->Replace(s->GetImageStencilData(), forceMutable);
    }
  return 0;
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::Allocate(double value)
{
  int extent[6];
  this->GetExtent(extent);

  vtkKWEPaintbrushVerboseMacro(
    "Allocating stencil(" << this << ") with extent: ["
      << extent[0] << "," << extent[1] << "," 
      << extent[2] << "," << extent[3] << "," 
      << extent[4] << "," << extent[5] << "]" << std::endl);

  this->ImageStencilData->AllocateExtents();
  if( value != 0.0 )
    {
    this->ImageStencilData->Fill();
    }
  this->ImageStencilData->Modified();
  this->Modified();
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::Resize(int extent[6], double f)
{
  int oldExtent[6];
  this->ImageStencilData->GetExtent(oldExtent);
  if (oldExtent[1] < oldExtent[0])
    {
    // We haven't been allocated yet. Just allocate and return.
    this->SetExtent(extent);
    this->Allocate(f);
    return;
    }

  if (oldExtent[0] != extent[0] ||
      oldExtent[1] != extent[1] ||
      oldExtent[2] != extent[2] ||
      oldExtent[3] != extent[3] ||
      oldExtent[4] != extent[4] ||
      oldExtent[5] != extent[5])
    {
    vtkImageStencilData * oldData = vtkImageStencilData::New();
    oldData->DeepCopy(this->ImageStencilData);

    this->SetExtent(extent);
    this->Allocate(f);

    int r1, r2, moreSubExtents, iter;
    for (int z=extent[4]; z <= extent[5]; z++)
      {
      for (int y=extent[2]; y <= extent[3]; y++)
        {
        iter = 0;
        moreSubExtents = 1;
        while( moreSubExtents )
          {
          moreSubExtents = oldData->GetNextExtent(
            r1, r2, extent[0], extent[1], y, z, iter);

          if (r1 <= r2)
            {
            this->ImageStencilData->InsertNextExtent(r1, r2, y, z);
            }
          } // end for each extent tuple
        } // end for each scan line
      } // end of each slice       

    oldData->Delete();
    }
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::SetSpacing( double s[3] )
{
  this->ImageStencilData->SetSpacing(s);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::GetSpacing( double s[3] )
{
  this->ImageStencilData->GetSpacing(s);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::SetOrigin( double s[3] )
{
  this->ImageStencilData->SetOrigin(s);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::GetOrigin( double s[3] )
{
  this->ImageStencilData->GetOrigin(s);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::SetExtent( int extent[6] )
{
  this->ImageStencilData->SetExtent(extent);
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::GetExtent( int extent[6] )
{
  this->ImageStencilData->GetExtent(extent);
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::Clip( int extent[6] )
{
  int currentExtent[6], idy, idz, newExtent[6], r1, r2, iter;
  this->Update();
  this->GetExtent( currentExtent );

  if (vtkMath::ExtentIsWithinOtherExtent( currentExtent, extent ))
    {
    // Nothing to do, we are already within the clipping extents.
    return 0;
    }

  if (vtkKWEPaintbrushUtilities::GetIntersectingExtents( 
        currentExtent, extent, newExtent ))
    {
    // Copy into a temporary
    vtkImageStencilData *tmp = vtkImageStencilData::New();
    tmp->DeepCopy(this->ImageStencilData);

    // Clear and re-allocate ourself.
    this->SetExtent(newExtent);
    this->ImageStencilData->AllocateExtents(); // Reallocate extents.

    for (idz=newExtent[4]; idz<=newExtent[5]; idz++)
      {
      for (idy = newExtent[2]; idy <= newExtent[3]; idy++)
        {

        iter = 0;
        int moreSubExtents = 1;
        while( moreSubExtents )
          {
          moreSubExtents = tmp->GetNextExtent( 
            r1, r2, newExtent[0], newExtent[1], idy, idz, iter);
          
          if (r1 <= r2 ) // sanity check 
            {
            this->ImageStencilData->InsertNextExtent( r1, r2, idy, idz );
            }
          }
        }
      }
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------------
void vtkKWEPaintbrushStencilData::GetPaintbrushDataAsImageData(
                                        vtkImageData *image)
{
  vtkKWEPaintbrushUtilities::GetImageFromStencil(
        image, this->ImageStencilData, 255, 0);
  image->SetSpacing(this->ImageStencilData->GetSpacing());
  image->SetOrigin(this->ImageStencilData->GetOrigin());
}

//----------------------------------------------------------------------------
unsigned long vtkKWEPaintbrushStencilData::GetMTime()
{
  unsigned long t = this->ImageStencilData->GetMTime();
  unsigned long mtime = vtkObject::GetMTime();
  return (mtime > t ? mtime : t);
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::IsInside( double p[3] )
{
  int pixelPos[3], extent[6];
  double origin[3], spacing[3];
  this->ImageStencilData->GetSpacing(spacing);
  this->ImageStencilData->GetOrigin(origin);
  this->GetExtent(extent);

  for (int i = 0; i < 3; i++)
    {
    pixelPos[i] = static_cast<int>(((p[i] - origin[i])/spacing[i])+0.5);
    if (pixelPos[i] < extent[2*i] || pixelPos[i] > extent[2*i+1])
      {
      return 0;
      }
    }

  int moreSubExtents = 1, iter = 0, r1, r2;
  while( moreSubExtents )
    {
    moreSubExtents = this->ImageStencilData->GetNextExtent( 
      r1, r2, extent[0], extent[1], pixelPos[1], pixelPos[2], iter);

    if (r1 <= pixelPos[0] && r2 >= pixelPos[0] )
      {
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::IsInside( int i, int j, int k )
{
  int extent[6];
  this->GetExtent(extent);
  if (i < extent[0] || i > extent[1] ||
      j < extent[2] || j > extent[3] ||
      k < extent[4] || k > extent[5])
    {
    return 0;
    }

  int moreSubExtents = 1, iter = 0, r1, r2;
  while( moreSubExtents )
    {
    moreSubExtents = this->ImageStencilData->GetNextExtent( 
      r1, r2, extent[0], extent[1], j, k, iter);

    if (r1 <= i && r2 >= i )
      {
      return 1;
      }
    }

  return 0;
}

//----------------------------------------------------------------------------
int vtkKWEPaintbrushStencilData::IsEqual(vtkKWEPaintbrushData *data)
{
  if (vtkKWEPaintbrushStencilData *sdata =
      vtkKWEPaintbrushStencilData::SafeDownCast(data))
    {
    return this->ImageStencilData==sdata->GetImageStencilData();
    }
  return 0;
}

