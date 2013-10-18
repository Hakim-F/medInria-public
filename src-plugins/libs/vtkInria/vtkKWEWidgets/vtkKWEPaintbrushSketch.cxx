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
#include "vtkKWEPaintbrushSketch.h"

#include "vtkKWEPaintbrushStroke.h"
#include "vtkKWEPaintbrushOperation.h"
#include "vtkKWEPaintbrushStencilData.h"
#include "vtkKWEPaintbrushGrayscaleData.h"
#include "vtkKWEPaintbrushLabelData.h"
#include "vtkKWEPaintbrushShape.h"
#include "vtkKWEPaintbrushDrawing.h"
#include "vtkKWEPaintbrushProperty.h"
#include "vtkKWEPaintbrushUtilities.h"
#include "vtkObjectFactory.h"
#include "vtkMath.h"
#include "vtkImageData.h"
#include "vtkProperty.h"

#define max(x,y) ((x>y) ? (x) : (y))

vtkCxxRevisionMacro(vtkKWEPaintbrushSketch, "$Revision: 3597 $");
vtkStandardNewMacro(vtkKWEPaintbrushSketch);
vtkCxxSetObjectMacro(vtkKWEPaintbrushSketch,PaintbrushOperation,
                                             vtkKWEPaintbrushOperation);

//----------------------------------------------------------------------
vtkKWEPaintbrushSketch::vtkKWEPaintbrushSketch()
{
  this->PaintbrushData           = NULL;
  this->PaintbrushOperation      = NULL;
  this->ImageData                = NULL;
  this->PaintbrushProperty       = vtkKWEPaintbrushProperty::New(this);
  this->CurrentStroke            = -1;
  this->Representation           = vtkKWEPaintbrushEnums::Binary;
  this->HistoryLength            = 4;
  this->Label                    = vtkKWEPaintbrushLabelData::NoLabelValue;
  this->PaintbrushDrawing        = NULL;
}

//----------------------------------------------------------------------
vtkKWEPaintbrushSketch::~vtkKWEPaintbrushSketch()
{
  if (this->PaintbrushData)
    {
    this->PaintbrushData->Delete();
    }
  this->SetPaintbrushOperation(NULL);
  this->SetImageData(NULL);

  // Delete the internals.
  for (unsigned int i=0; i<this->Strokes.size(); i++)
    {
    this->Strokes[i]->UnRegister(this);
    }
  this->Strokes.clear();
  this->PaintbrushProperty->Delete();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::Initialize()
{
  if (!this->ImageData)
    {
    vtkErrorMacro( << "No image data");
    return;
    }

  if (this->PaintbrushData)
    {
    this->PaintbrushData->Delete();
    this->PaintbrushData = NULL;
    }

  if (this->Representation == vtkKWEPaintbrushEnums::Binary)
    {
    this->PaintbrushData = vtkKWEPaintbrushStencilData::New();
    }
  else if (this->Representation == vtkKWEPaintbrushEnums::Grayscale)
    {
    this->PaintbrushData = vtkKWEPaintbrushGrayscaleData::New();
    }

  if (this->PaintbrushData)
    {
    this->AllocatePaintbrushData();
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::Initialize( vtkKWEPaintbrushLabelData *labelMap )
{
  if (!this->PaintbrushData)
    {
    vtkErrorMacro( << "PaintbrushData is NULL. Did you add this sketch to the "
      << "drawing and Initialize the drawing prior to initializing the sketch ?");
    return;
    }

  if (this->Representation != vtkKWEPaintbrushEnums::Label)
    {
    vtkErrorMacro( << "Representation of the sketch isnt a label map!"
      << " You probably want to call SetRepresentation( "
      << "vtkKWEPaintbrushEnums::Label) on the sketch first.");
    return;
    }

  if (this->Label == vtkKWEPaintbrushLabelData::NoLabelValue)
    {
    vtkErrorMacro( << "Set the Label on this sketch first. "
                   << "Note that " << vtkKWEPaintbrushLabelData::NoLabelValue
                   << " is a reserved value.");
    return;
    }

  // Add the label data for this label as a stroke, so it can be undone or
  // redone. Note also that strokes are still managed as stencils, since
  // they are guarenteed to be binary. Stencils, inherently stored in run-
  // length encoded form still are the most efficient representations
  // for strokes.

  vtkKWEPaintbrushStencilData *strokeData = vtkKWEPaintbrushStencilData::New();

  // Extract all voxels in the label map with this->Label and store
  // it in strokeData.

  if (labelMap)
    {
    vtkKWEPaintbrushUtilities::GetStencilFromImage<
      vtkKWEPaintbrushUtilities::vtkFunctorEqualTo >(
                  labelMap->GetLabelMap(),
                  strokeData->GetImageStencilData(),
                  this->Label );

    // Add the initial segmentation as a stroke to this sketch.
    this->AddNewStroke( vtkKWEPaintbrushEnums::Draw,
                        strokeData, NULL, true );
    }
  else
    {
    // Create a dummy stroke
    vtkImageStencilData *sdata = strokeData->GetImageStencilData();
    int extent[6];
    vtkKWEPaintbrushLabelData *labelData =
      vtkKWEPaintbrushLabelData::SafeDownCast(this->PaintbrushData);
    vtkImageData *labelImage = labelData->GetLabelMap();
    labelImage->GetExtent(extent);
    extent[1] = extent[0];
    extent[3] = extent[2];
    extent[5] = extent[4];
    sdata->SetExtent(extent);
    sdata->SetSpacing(labelImage->GetSpacing());
    sdata->SetOrigin(labelImage->GetOrigin());
    sdata->AllocateExtents();

    // Add the initial segmentation as a stroke to this sketch.
    this->AddNewStroke( vtkKWEPaintbrushEnums::Draw,
                        strokeData, extent, true );
    }


  strokeData->Delete();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::SetImageData( vtkImageData * imageData )
{
  if (this->ImageData != imageData)
    {
    vtkImageData * var = this->ImageData;
    this->ImageData = imageData;
    if (this->ImageData != NULL) { this->ImageData->Register(this); }
    if (var != NULL)
      {
      var->UnRegister(this);
      }
    this->Modified();
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::AllocatePaintbrushData()
{
  if (!this->ImageData)
    {
    vtkErrorMacro( << "No image data");
    }
  if (!this->PaintbrushData)
    {
    vtkErrorMacro( << "Must call Initialize() before using this class !");
    }

  int extent[6];
  double spacing[3], origin[3];
  this->ImageData->GetWholeExtent(extent);
  this->ImageData->GetSpacing(spacing);
  this->ImageData->GetOrigin(origin);

  this->PaintbrushData->SetExtent(extent);
  this->PaintbrushData->SetSpacing(spacing);
  this->PaintbrushData->SetOrigin(origin);
  this->PaintbrushData->Allocate();
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushSketch::AddNewStroke(
    int polarity,                       // Draw or erase
    vtkKWEPaintbrushData *initialization,
    int *extents,
    bool forceMutable )
{
  this->CurrentStroke++;

  // If we've exceeded the maximum allowed queue length, keep the queue length
  // at the limit by merging the last two strokes in the queue. This will
  // ensure that the queue size is always "HistoryLength" or less.
  if (this->CurrentStroke > this->HistoryLength)
    {
    if (this->HistoryLength > 0)
      {
      vtkKWEPaintbrushStroke * stroke0 = this->Strokes[0];
      vtkKWEPaintbrushStroke * stroke1 = this->Strokes[1];
      stroke0->GetPaintbrushData()->SetLabel(this->Label);
      stroke1->GetPaintbrushData()->SetLabel(this->Label);

      // Merge, keeping the polarity in mind. Stroke 0 has to be of positive
      // polarity.
      if (stroke1->GetState() == vtkKWEPaintbrushEnums::Erase)
        {
        stroke0->GetPaintbrushData()->Subtract(stroke1->GetPaintbrushData(), forceMutable);
        }
      else
        {
        stroke0->GetPaintbrushData()->Add(stroke1->GetPaintbrushData(), forceMutable);
        }

      // Update stroke0's draw time to the most recent of stroke0 and stroke1.
      // Of course the more recent one is stroke1.
      stroke0->SetDrawTime( stroke1->GetDrawTime() );

      // Now that we've merged, remove the stroke. So if we have 5 strokes:
      //       0, 1, 2, 3, 4.
      // 1 is merged into 0. Then 1 is removed. We will end up with 4 strokes:7
      //     (0 U 1), 2, 3, 4
      this->Strokes.erase(this->Strokes.begin()+1);
      stroke1->UnRegister(this);
      }

    // We are at the end of the queue.
    this->CurrentStroke = this->HistoryLength;
    }

  // Only if we are worried about Undo/Redo
  if (this->HistoryLength)
    {
    // Erase all future sequences.. (the moment you start drawing a new
    // sequence, you forget about the last undo), exactly like MSWord
    this->EraseStrokes(
      this->CurrentStroke, static_cast<int>(this->Strokes.size()-1));
    }

  // 'stroke' will represent the stroke on which we will now etch.
  vtkSmartPointer< vtkKWEPaintbrushStroke > stroke = NULL;

  if ((this->HistoryLength == 0) && this->Strokes.size())
    {
    // If we aren't maintaining history, we will keep reusing the same stroke.
    // We will not create a new one or add it to the history queue.
    stroke = this->Strokes[this->CurrentStroke];
    }

  else
    {
    // Create a new stroke
    stroke = vtkSmartPointer< vtkKWEPaintbrushStroke >::New();
    stroke->SetPaintbrushOperation(this->PaintbrushOperation);
    stroke->SetRepresentation(this->Representation);
    stroke->SetState(polarity); // The polarity, draw / erase.
    stroke->SetLabel(this->Label);

    // Stroke extents is the intersection of the image extents, the
    // user supplied extents (if any) and the clip extents (if any)

    // Check if extents have been specified, the stroke size may be a lot
    // smaller than that of the canvas and we can save memory :)
    int strokeExtents[6];
    this->ImageData->GetExtent(strokeExtents);

    if (extents && extents[1] >= extents[0])
      {
      vtkKWEPaintbrushUtilities::GetIntersectingExtents(
                  strokeExtents, extents, strokeExtents);
      }

    stroke->SetExtent(strokeExtents);

    // allocation of the stroke happens here
    stroke->SetImageData(this->ImageData);

    // Push it onto our history queue.
    stroke->Register(this);
    this->Strokes.push_back(stroke);
    }

  if (initialization)
    {
    initialization->SetLabel( this->Label );

    // Initialize stroke with user specified data, if provided. This allows
    // user provided initializations to be undone/redone as well.
    stroke->SetPaintbrushData(initialization);
    this->PaintbrushData->Add(initialization, forceMutable);
    }

  return 1;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::Add( vtkKWEPaintbrushSketch * sketch )
{
  // Merges another sketch into this sketch.

  if (!this->PaintbrushData)
    {
    vtkErrorMacro( << "PaintbrushData is NULL. Did you add this sketch to the "
      << "drawing and Initialize the drawing prior to initializing the sketch ?");
    return;
    }

  if (this->Representation != sketch->GetRepresentation())
    {
    vtkErrorMacro( << "Both sketches should have the same representation.");
    return;
    }

  if (this->Representation == vtkKWEPaintbrushEnums::Label &&
      sketch->GetRepresentation() == vtkKWEPaintbrushEnums::Label)
    {
    if (this->Label == vtkKWEPaintbrushLabelData::NoLabelValue ||
        sketch->GetLabel() == vtkKWEPaintbrushLabelData::NoLabelValue)
      {
      vtkErrorMacro( << "Set the Label on this sketch first. "
                     << "Note that " << vtkKWEPaintbrushLabelData::NoLabelValue
                     << " is a reserved value.");
      return;
      }


    // Add the label data for this label as a stroke, so it can be undone or
    // redone. Note also that strokes are still managed as stencils, since
    // they are guarenteed to be binary. Stencils, inherently stored in run-
    // length encoded form still are the most efficient representations
    // for strokes.

    vtkKWEPaintbrushLabelData * labelMap =
      vtkKWEPaintbrushLabelData::SafeDownCast(sketch->GetPaintbrushData());
    vtkKWEPaintbrushStencilData *strokeData = vtkKWEPaintbrushStencilData::New();

    // Extract all voxels in the label map with this->Label and store
    // it in strokeData.

    vtkKWEPaintbrushUtilities::GetStencilFromImage<
        vtkKWEPaintbrushUtilities::vtkFunctorEqualTo >(
                    labelMap->GetLabelMap(),
                    strokeData->GetImageStencilData(),
                    sketch->GetLabel() );

    // Add the initial segmentation as a stroke to this sketch.
    this->AddNewStroke( vtkKWEPaintbrushEnums::Draw, strokeData, NULL, true );

    strokeData->Delete();
    }
  else
    {
    // For other representations, no need to extract the label. The generic
    // method takes care of it.
    this->AddNewStroke(
      vtkKWEPaintbrushEnums::Draw, sketch->GetPaintbrushData(), NULL, true );
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::EraseStrokes(int start, int end)
{
  if (end < start || start < 0 ||
      end >= static_cast<int>(this->Strokes.size()))
    {
    return;
    }

  for (int i=start; i <= end; i++)
    {
    this->Strokes[i]->UnRegister(this);
    }
  this->Strokes.erase(this->Strokes.begin()+start,
                                this->Strokes.begin()+end+1);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushSketch::PopStroke()
{
  if (this->Strokes.size() <= 1 || this->CurrentStroke < 1)
    {
    return 0;
    }
  this->CurrentStroke--;

  this->ComposeStrokes();
  return 1;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushSketch::PushStroke()
{
  if (!this->Strokes.size() ||
      this->CurrentStroke >= (static_cast<int>(this->Strokes.size())-1) )
    {
    return 0;
    }
  this->CurrentStroke++;

  if (this->Representation == vtkKWEPaintbrushEnums::Label)
    {
    // Need to compose all strokes for multi-label undo/redo.
    this->ComposeStrokes();
    }
  else
    {
    // Shortcut for non-label drawings. Just add the current stroke back.

    vtkKWEPaintbrushStroke *stroke = this->Strokes[this->CurrentStroke];
    stroke->GetPaintbrushData()->SetLabel(this->Label);

    // Subtract this stroke if it was an erase stroke, add it if it was a
    // draw stroke.
    if (stroke->GetState() == vtkKWEPaintbrushEnums::Erase)
      {
      this->PaintbrushData->Subtract(stroke->GetPaintbrushData());
      }
    else
      {
      this->PaintbrushData->Add(stroke->GetPaintbrushData());
      }
    }

  return 1;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushSketch::DeleteLastStroke()
{
  if (!this->Strokes.size())
    {
    return 0;
    }

  vtkKWEPaintbrushStroke *stroke = this->Strokes[this->Strokes.size()-1];
  stroke->UnRegister(this);
  this->Strokes.pop_back();

  this->ComposeStrokes();

  if (this->CurrentStroke >= 0)
    {
    this->CurrentStroke--;
    return 1;
    }

  return 0;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::ComposeStrokes()
{
  // Clear before composing
  // FIXME check if allocate erases the buffer for ImageData too
  this->PaintbrushData->Clear( this->Label );

  if (this->CurrentStroke >= static_cast<int>(this->Strokes.size()))
    {
    vtkErrorMacro(
      << "Trying to access " << this->CurrentStroke << " but number of strokes "
      << "available in sketch " << this << " is "
      << this->Strokes.size());
    return;
    }

  // Multilabel undo/redo.
  if (this->Representation == vtkKWEPaintbrushEnums::Label)
    {
    if (this->CurrentStroke >= 0 && this->PaintbrushDrawing)
      {
      this->PaintbrushDrawing->ComposeStrokes();
      }
    }
  else
    {

    if (this->CurrentStroke >= 0)
      {
      for (int i=0; i<= this->CurrentStroke; i++)
        {
        vtkKWEPaintbrushStroke *stroke = this->Strokes[i];
        stroke->GetPaintbrushData()->SetLabel(this->Label);

        // Subtract this stroke if it was an erase stroke, add it if it was a
        // draw stroke.
        if (stroke->GetState()  == vtkKWEPaintbrushEnums::Erase)
          {
          this->PaintbrushData->Subtract(stroke->GetPaintbrushData());
          }
        else
          {
          this->PaintbrushData->Add(stroke->GetPaintbrushData());
          }
        }
      }
    }
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushSketch::AddShapeToCurrentStroke(double p[3],
                                                  vtkKWEPaintbrushData *auxData)
{
  if (!this->Strokes.size() || this->CurrentStroke < 0 ||
      this->CurrentStroke >= static_cast<int>(this->Strokes.size()) ||
      !this->PaintbrushData)
    {
    return 0;
    }

  vtkKWEPaintbrushStroke *stroke = this->Strokes[this->CurrentStroke];

  // Add the shape at the current position to the stroke. We will also
  // pass in the paintbrush data from this sketch (this->PaintbrushData) and
  // the one from the drawing (auxData) and add/subtract the shape (passed
  // through the operation) to that data. This is done for efficiency, so that
  // we don't query the shape twice to compute the paintbrushData at thex
  // current location.
  //   Hey, you need some hacks for speed right :) !

  return stroke->AddShapeAtPosition(p, this->PaintbrushData, auxData);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::SetPaintbrushData(vtkKWEPaintbrushData * data )
{
  // If you set an external data as the stroke manager's data, you
  // lose all the undo-redo history..
  this->EraseStrokes(0,static_cast<int>(this->Strokes.size()-1));

  if (this->PaintbrushData != data)
    {
    vtkKWEPaintbrushData * var = this->PaintbrushData;
    this->PaintbrushData = data;
    if (this->PaintbrushData != NULL) { this->PaintbrushData->Register(this); }
    if (var != NULL)
      {
      var->UnRegister(this);
      }
    this->Modified();
    }
  this->CurrentStroke = -1;
}

//----------------------------------------------------------------------
unsigned long vtkKWEPaintbrushSketch::GetMTime()
{
  // TODO: Check this. Do we need to check the property MTime. I
  // don't want uncessary re-rendering... for instance changing the
  // highlight property when I am not using Highlight etc....

  unsigned long t = 0;
  if (this->PaintbrushData)
    {
    t = this->PaintbrushData->GetMTime();
    }
  unsigned long mtime = vtkObject::GetMTime();
  unsigned long propertyMtime = this->PaintbrushProperty->GetMTime();
  return max( max(mtime, t), propertyMtime );
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::
GetStrokes(vtkstd::vector< vtkKWEPaintbrushStroke * > &strokes)
{
  // Push all the strokes until the current stroke into strokes.
  if (this->Strokes.size())
    {
    strokes.insert( strokes.end(),
                    this->Strokes.begin(),
                    this->Strokes.begin() + this->CurrentStroke + 1 );
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::SetMutable( int m )
{
  // Sanity checks.
  if (!this->PaintbrushData)
    {
    vtkErrorMacro( << "Initialize PaintbrushSketch prior to affecting its mutability." );
    return;
    }

  vtkKWEPaintbrushLabelData *labelData
    = vtkKWEPaintbrushLabelData::SafeDownCast(this->PaintbrushData);

  if (!labelData)
    {
    vtkErrorMacro( << "Mutability makes sense only when editing label maps." );
    return;
    }

  if (this->Label == vtkKWEPaintbrushLabelData::NoLabelValue)
    {
    vtkErrorMacro( << "You cannot affect mutability before setting the label value." );
    return;
    }


  if (labelData && labelData->IsMutable(this->Label) != m)
    {
    // Let's collapse the history. Consider a scenario where we make a
    // sketch immutable, then draw, (it doesn't actually change the sketch).
    // then undo, then make the sketch mutable, then redo. This time the
    // stroke will change the sketch, because the stroke has no idea that
    // the sketch it was affecting when created was immutable at that time.
    //    So to be on the same side, we will clear this history every time we
    // change the mutability flag.
    for (int i = 0; i < this->PaintbrushDrawing->GetNumberOfItems(); i++)
      {
      //this->PaintbrushDrawing->GetItem(i)->CollapseHistory();
      }

    labelData->SetMutable( m, this->Label );
    }
}

// ----------------------------------------------------------------------------
void vtkKWEPaintbrushSketch::CollapseHistory()
{
  if (this->HistoryLength)
    {
    // Remove all future strokes.
    this->EraseStrokes(this->CurrentStroke+1,
                       static_cast<int>(this->Strokes.size()-1));

    // Recursively collapse the bottom two strokes, until we have just
    // one stroke left.
    while (this->Strokes.size() > 1)
      {
      vtkKWEPaintbrushStroke * stroke0 = this->Strokes[0];
      vtkKWEPaintbrushStroke * stroke1 = this->Strokes[1];
      stroke0->GetPaintbrushData()->SetLabel(this->Label);
      stroke1->GetPaintbrushData()->SetLabel(this->Label);

      // Merge, keeping the polarity in mind. Stroke 0 has to be of positive
      // polarity.
      if (stroke1->GetState() == vtkKWEPaintbrushEnums::Erase)
        {
        stroke0->GetPaintbrushData()->Subtract(stroke1->GetPaintbrushData());
        }
      else
        {
        stroke0->GetPaintbrushData()->Add(stroke1->GetPaintbrushData());
        }

      // Update stroke0's draw time to the most recent of stroke0 and stroke1.
      // Of course the more recent one is stroke1.
      stroke0->SetDrawTime( stroke1->GetDrawTime() );

      // Now that we've merged, remove the stroke. So if we have 5 strokes:
      //       0, 1, 2, 3, 4.
      // 1 is merged into 0. Then 1 is removed. We will end up with 4 strokes:7
      //     (0 U 1), 2, 3, 4
      this->Strokes.erase(this->Strokes.begin()+1);
      stroke1->UnRegister(this);
      }

    this->CurrentStroke = 0;


    // Re-create the initial stroke to match what's there.
    if (this->Strokes.size())
      {
      if (vtkKWEPaintbrushLabelData *labelMap = 
          vtkKWEPaintbrushLabelData::SafeDownCast(this->PaintbrushData))
        {
        if (vtkKWEPaintbrushStencilData *stencilData =
            vtkKWEPaintbrushStencilData::SafeDownCast(
                this->Strokes[0]->GetPaintbrushData()))
          {
          vtkKWEPaintbrushUtilities::GetStencilFromImage<
            vtkKWEPaintbrushUtilities::vtkFunctorEqualTo >(
                  labelMap->GetLabelMap(),
                  stencilData->GetImageStencilData(),
                  this->Label );
          }
        }
      }      
    }
}

// ----------------------------------------------------------------------------
void vtkKWEPaintbrushSketch::CopySketchFromExtentToExtent(
    int sourceExtent[6], int targetExtent[6] )
{
  // This method is useful for copying a sketch from one slice to another.
  // The source and target extents should be of the same dimension.
  // Its currently implemented only if the sketch is of a label map type.
  if (this->Representation != vtkKWEPaintbrushEnums::Label)
    {
    vtkErrorMacro( <<
      "Currently the copy functionality is implemented only for label maps.");
    return;
    }

  vtkKWEPaintbrushLabelData *labelData =
    vtkKWEPaintbrushLabelData::SafeDownCast(this->PaintbrushData);
  vtkImageData *labelImage = labelData->GetLabelMap();
  
  typedef vtkKWEPaintbrushEnums::LabelType LabelType;
  typedef vtkImageIterator< LabelType > IteratorType;

  // Copy over.. We won't do any extent checking..
  int labelImageExtent[6];
  labelImage->GetExtent(labelImageExtent);
  
  if (!vtkMath::ExtentIsWithinOtherExtent(sourceExtent, labelImageExtent) ||
      !vtkMath::ExtentIsWithinOtherExtent(targetExtent, labelImageExtent))
    {
    vtkErrorMacro( << "Copy extents are not within the labelmap's extent");
    return;
    }

  IteratorType it1(labelImage, sourceExtent);
  IteratorType it2(labelImage, targetExtent);
  while( !it1.IsAtEnd() )
    {
    LabelType *inSI1    = it1.BeginSpan();
    LabelType *inSIEnd1 = it1.EndSpan();
    LabelType *inSI2    = it2.BeginSpan();
    while (inSI1 != inSIEnd1)
      {
      LabelType l = *inSI1;
      if (l == this->Label)
        {
        *inSI2 = l;
        }
      ++inSI1;
      ++inSI2;
      }
    it1.NextSpan();    
    it2.NextSpan();    
    }

  this->Modified();
}

// ----------------------------------------------------------------------------
void vtkKWEPaintbrushSketch::CopyLastStrokeFromExtentToExtent(
    int sourceExtent[6], int targetExtent[6] )
{
  // This method is useful for copying a sketch from one slice to another.
  // The source and target extents should be of the same dimension.
  // Its currently implemented only if the sketch is of a label map type.

  if (this->Representation != vtkKWEPaintbrushEnums::Label)
    {
    vtkErrorMacro( <<
      "Currently the copy functionality is implemented only for label maps.");
    return;
    }

  // The last stroke is the only stroke.. copy the sketch itself then..
  if (this->Strokes.size() <= 1 || this->CurrentStroke < 1)
    {
    this->CopySketchFromExtentToExtent(sourceExtent, targetExtent);
    return;
    }

  // Get the last stroke
  vtkKWEPaintbrushStroke *stroke = this->Strokes[this->Strokes.size() - 1];

  vtkKWEPaintbrushStencilData *mask =
    vtkKWEPaintbrushStencilData::SafeDownCast(
        vtkKWEPaintbrushStroke::GetStoreNonIncrementalData() ? 
          stroke->GetEtchData() : stroke->GetPaintbrushData());
  
  // Now copy from the source to the target extent whatever is within this
  // mask.

  vtkKWEPaintbrushLabelData *labelData =
    vtkKWEPaintbrushLabelData::SafeDownCast(this->PaintbrushData);
  vtkImageData *labelImage = labelData->GetLabelMap();
  
  typedef vtkKWEPaintbrushEnums::LabelType LabelType;
  typedef vtkImageIterator< LabelType > IteratorType;

  int labelImageExtent[6];
  labelImage->GetExtent(labelImageExtent);
  
  if (!vtkMath::ExtentIsWithinOtherExtent(sourceExtent, labelImageExtent) ||
      !vtkMath::ExtentIsWithinOtherExtent(targetExtent, labelImageExtent))
    {
    vtkErrorMacro( << "Copy extents are not within the labelmap's extent");
    return;
    }

  // Create an auxiliary image data that represents what we've added, so
  // as to update the stroke as well.
  vtkSmartPointer< vtkImageData > increment =
        vtkSmartPointer< vtkImageData >::New();
  increment->SetExtent(targetExtent);
  increment->SetSpacing(this->ImageData->GetSpacing());
  increment->SetOrigin(this->ImageData->GetOrigin());
  increment->SetScalarTypeToUnsignedChar();
  increment->AllocateScalars();

  // Update the drawing label map directly.

  int ks, kt, js, jt, is, it;
  bool modified = false;
  for (ks = sourceExtent[4], kt = targetExtent[4]; ks <= sourceExtent[5]; ++ks, ++kt )
    {
    for (js = sourceExtent[2], jt = targetExtent[2]; js <= sourceExtent[3]; ++js, ++jt )
      {
      for (is = sourceExtent[0], it = targetExtent[0]; is <= sourceExtent[1]; ++is, ++it )
        {
        LabelType label = *(
            static_cast< LabelType *>(labelImage->GetScalarPointer(is, js, ks)));
        unsigned char *iPtr = static_cast< unsigned char * >(
                      increment->GetScalarPointer(it, jt, kt));
        if (label == this->Label && mask->IsInside(is, js, ks))
          {
          LabelType *targetLabelPtr = 
            static_cast< LabelType *>(labelImage->GetScalarPointer(it, jt, kt));
          *targetLabelPtr = this->Label;
          *iPtr = 255;
          modified = true;
          }
        else
          {
          *iPtr = 0;
          }
        }
      }
    }

  // Now update the stroke as well. We will convert this incremental image that
  // represents what we've copied over to the next slice as a stencil and then
  // add that to the stencil image.

  if (modified)
    {
    mask->Add(increment, 255);
    this->Modified();
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch
::SetLabel( vtkKWEPaintbrushEnums::LabelType label )
{
  if (this->Label == label)
    {
    return; // nothing to do
    }

  if (label == vtkKWEPaintbrushLabelData::NoLabelValue)
    {
    return; // this is not a valid label.
    }

  if (this->Label == vtkKWEPaintbrushLabelData::NoLabelValue)
    {
    // We are setting the label for the first time. Set it to what
    // the user requested and return.
    this->Label = label;
    this->Modified();
    return;
    }

  if (this->Label != vtkKWEPaintbrushLabelData::NoLabelValue)
    {
    // We are changing the label of this sketch from one label to another
    // This amounts to a relabel operation. We need to change the labels
    // in the underlying data as well from the old to the new label value.
    // Its the application's responsiblity to ensure that there is no
    // collision with another label in the dataset.

    // This function makes sense only for label maps
    if (this->Representation != vtkKWEPaintbrushEnums::Label)
      {
      vtkErrorMacro( << 
        "Relabel makes sense only if the represenstation is a label representation!" );
      return;
      }

    if (!this->PaintbrushDrawing)
      {
      // Drawing must be set, so we can change the label.
      vtkErrorMacro( << "Need to set the drawing first" );
      return;
      }
    
    // Make sure that there is no conflict, ie the label we are changing to
    // doesn't already exist in the drawing.
    if (this->PaintbrushDrawing->GetItemWithLabel(label))
      {
      vtkErrorMacro( << "Another sketch with this label already exists! " );
      return;
      }

    if (vtkKWEPaintbrushLabelData *labelData =
        vtkKWEPaintbrushLabelData::SafeDownCast(this->PaintbrushData))
      {
      labelData->ReplaceLabel( this->Label, label );
      }
    this->Label = label;
    this->Modified();
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushSketch::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  os << indent << "PaintbrushOperation: " << this->PaintbrushOperation << endl;
  if (this->PaintbrushOperation)
    {
    this->PaintbrushOperation->PrintSelf( os, indent.GetNextIndent() );
    }

  this->Superclass::PrintSelf(os,indent);
}

