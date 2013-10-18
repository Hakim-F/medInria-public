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
#ifndef __vtkKWEPaintbrushData_h
#define __vtkKWEPaintbrushData_h

#include "vtkKWEWidgetsExport.h" // needed for export symbols directives
#include "vtkKWEPaintbrushEnums.h"
#include "vtkDataObject.h"

class vtkImageData;

class VTKEdge_WIDGETS_EXPORT vtkKWEPaintbrushData : public vtkDataObject
{
public:
  vtkTypeRevisionMacro(vtkKWEPaintbrushData, vtkDataObject);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Minkowski operators. Return 1 if something changed.
  virtual int Add(      vtkKWEPaintbrushData *, bool forceMutable=false ) = 0;
  virtual int Subtract( vtkKWEPaintbrushData *, bool forceMutable=false ) = 0;
  virtual int Replace(  vtkKWEPaintbrushData *, bool forceMutable=false ) = 0;

  // Description:
  // Check if two paintbrush data's are identical.
  virtual int IsEqual( vtkKWEPaintbrushData * ) { return 0; }
  
  // Description:
  // Clip self with supplied extents. Return 1 if something changed
  virtual int Clip( int extent[6] ) = 0;

  // Description:
  // Set/Get extents.
  virtual void SetExtent( int extent[6] ) = 0;
  virtual void GetExtent( int extent[6] ) = 0;
  
  // Description:
  // Allocate the bulk data (once you've set the extents).
  virtual void Allocate( double fillValue = 0.0 ) = 0;

  // Description:
  // Metadata.
  virtual void SetSpacing(double spacing[3]) = 0;
  virtual void SetOrigin( double origin[3] ) = 0;
  virtual void GetSpacing( double spacing[3] ) = 0;
  virtual void GetOrigin( double origin[3] ) = 0;

  // Description:
  // Is the point "p" inside ?
  virtual int IsInside( double p[3] ) = 0;

  // Description:
  // Get the brush data as an image data. Note that for grayscale brushes,
  // this is just a shallow copy of the data. For binary brushes, this 
  // converts from the internal representation to an image data.
  virtual void GetPaintbrushDataAsImageData( vtkImageData * ) = 0;
  
  // Description:
  // Get the data type as an integer (this will return VTK_DATA_OBJECT
  // for now, maybe a proper type constant will be reserved later).
  int GetDataObjectType() { return VTK_DATA_OBJECT; }
  
  // Description:
  // The extent type is 3D, just like vtkImageData.
  int GetExtentType() { return VTK_3D_EXTENT; };
  
  // Description:
  // INTERNAL - Do not use.
  void SetLabel(vtkKWEPaintbrushEnums::LabelType l) { this->Label = l; }
  vtkGetMacro( Label, vtkKWEPaintbrushEnums::LabelType );

  // Description:
  // Clear data corresponding to a label.
  // - For vtkKWEPaintbrushStencilData, labels make no sense, since the data is
  //   binary. So   Clear(.) <==> Allocate()
  // - For vtkKWEPaintbrushGrayscaleData, labels make no sense. So 
  //   Clear(.) <==> Allocate(0.0)
  // - For vtkKWEPaintbrushLabelData, this sets all the voxels with the supplied
  //   label value to 0.
  virtual void Clear( vtkKWEPaintbrushEnums::LabelType label );
    
  // Description:
  // INTERAL - Do not use.
  // This is internally called to compute deltas for the Undo-Redo guys
  void SetComputeDelta( int i ) { this->ComputeDelta = i; }
  int GetComputeDelta() { return this->ComputeDelta; }

  // Description:
  // Copy the metadata from that paintbrush data to ourselves. This does not copy the
  // bulk data. It merely copies the metadata. So the spacing, origin, extent and
  // for Label/binary data's the label value is copied over. The paintbrush data passed
  // in must be the same type as ourselves.
  virtual void CopyMetaData( vtkKWEPaintbrushData *that );

protected:
  vtkKWEPaintbrushData();
  ~vtkKWEPaintbrushData();

  // Relevant only for vtkKWEPaintbrushStencilData when editing into label maps.
  vtkKWEPaintbrushEnums::LabelType Label;

  int ComputeDelta;

private:
  vtkKWEPaintbrushData(const vtkKWEPaintbrushData&);  // Not implemented.
  void operator=(const vtkKWEPaintbrushData&);  // Not implemented.
};

#endif



