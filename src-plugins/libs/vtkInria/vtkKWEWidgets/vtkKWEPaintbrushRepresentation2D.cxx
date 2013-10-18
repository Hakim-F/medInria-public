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
#include "vtkKWEPaintbrushRepresentation2D.h"

#include "vtkKWEPaintbrushShapeBox.h"
#include "vtkKWEPaintbrushShapeEllipsoid.h"
#include "vtkKWEPaintbrushOperation.h"
#include "vtkKWEPaintbrushDrawing.h"
#include "vtkKWEPaintbrushSelectionRepresentation2D.h"
#include "vtkKWEPaintbrushBlend.h"
#include "vtkKWEPaintbrushData.h"
#include "vtkKWEPaintbrushStencilData.h"
#include "vtkKWEPaintbrushHighlightActors.h"
#include "vtkKWEPaintbrushPropertyManager.h"
#include "vtkKWEPaintbrushProperty.h"
#include "vtkKWEPaintbrushLabelData.h"
#include "vtkKWEPaintbrushUtilities.h"
#include "vtkPolyDataMapper.h"
#include "vtkProperty.h"
#include "vtkActor.h"
#include "vtkCoordinate.h"
#include "vtkRenderer.h"
#include "vtkObjectFactory.h"
#include "vtkInteractorObserver.h"
#include "vtkMath.h"
#include "vtkImageActor.h"
#include "vtkImageStencilData.h"
#include "vtkImageData.h"
#include "vtkPolyData.h"
#include "vtkCamera.h"
#include "vtkPlane.h"
#include "vtkKWEVoxelAlignedImageActorPointPlacer.h"
#include "vtkInformationExecutivePortVectorKey.h"
#include "vtkAlgorithmOutput.h"
#include "vtkExecutive.h"
<<<<<<< HEAD
#include <vtkImageActorPointPlacer.h>
#include <vtkFocalPlanePointPlacer.cxx>
=======
>>>>>>> b2dca81... Starting to work on vtkEdgewidgetss

vtkCxxRevisionMacro(vtkKWEPaintbrushRepresentation2D, "1.12");
vtkStandardNewMacro(vtkKWEPaintbrushRepresentation2D);
vtkCxxSetObjectMacro(vtkKWEPaintbrushRepresentation2D,ShapeOutlinePropertyDraw,vtkProperty);
vtkCxxSetObjectMacro(vtkKWEPaintbrushRepresentation2D,ShapeOutlinePropertyErase,vtkProperty);
vtkCxxSetObjectMacro(vtkKWEPaintbrushRepresentation2D,ShapeOutlinePropertyInteract,vtkProperty);

//----------------------------------------------------------------------
vtkKWEPaintbrushRepresentation2D::vtkKWEPaintbrushRepresentation2D()
{
  this->UseOverlay               = 1;
  this->ImageActor               = NULL;
  this->ImageData                = NULL;
  this->ShapeOutline             = vtkPolyData::New();

  // Create the mapper
  this->ShapeOutlineMapper  = vtkPolyDataMapper::New();
  vtkCoordinate *tcoord = vtkCoordinate::New();
  tcoord->SetCoordinateSystemToWorld();
  this->ShapeOutlineMapper->SetInput(this->ShapeOutline);
  tcoord->Delete();

  // Create the actor
  this->ShapeOutlineActor   = vtkActor::New();
  this->ShapeOutlineActor->SetMapper( this->ShapeOutlineMapper );

//  property->SetRepresentationToSurface();

  // Create the template outline properties in draw/erase/interact modes.
  this->ShapeOutlinePropertyDraw = vtkProperty::New();
  this->ShapeOutlinePropertyDraw->SetColor(0.7,1.0,0.0);
  this->ShapeOutlinePropertyDraw->SetOpacity(0.5);
  this->ShapeOutlinePropertyDraw->SetLineWidth( 2.0 );
  this->ShapeOutlinePropertyErase = vtkProperty::New();
  this->ShapeOutlinePropertyErase->SetColor(1.0,0.0,1.0);
  this->ShapeOutlinePropertyErase->SetOpacity(0.5);
  this->ShapeOutlinePropertyErase->SetLineWidth( 2.0 );
  this->ShapeOutlinePropertyInteract = vtkProperty::New();
  this->ShapeOutlinePropertyInteract->SetColor(1.0,1.0,0.0);
  this->ShapeOutlinePropertyInteract->SetOpacity(0.5);
  this->ShapeOutlinePropertyInteract->SetLineWidth( 2.0 );
  this->ShapeOutlineActor->SetProperty(this->ShapeOutlinePropertyDraw);

  /* TODELETE: The ellipsoid PaintbrushShape is already defined in the
   * vtkKWEPaintbrushRepresentation constructor.
  vtkKWEPaintbrushShapeEllipsoid *paintbrushShape
                          = vtkKWEPaintbrushShapeEllipsoid::New();
  this->PaintbrushOperation->SetPaintbrushShape( paintbrushShape );
  paintbrushShape->SetWidth(5.0,5.0,5.0);
  paintbrushShape->Delete();
  */

  this->PaintbrushBlend = NULL;

  this->CurrentShapePosition[0] = VTK_FLOAT_MIN;
  this->CurrentShapePosition[1] = VTK_FLOAT_MIN;
  this->CurrentShapePosition[2] = VTK_FLOAT_MIN;

  // The paintbrushes snap to the grid

  vtkKWEVoxelAlignedImageActorPointPlacer * v
    = vtkKWEVoxelAlignedImageActorPointPlacer::New();
  this->SetShapePlacer(v);
  v->Delete();

  // Used if the paintbrush goes into "select" mode.
  this->SelectionRepresentation =
    vtkKWEPaintbrushSelectionRepresentation2D::New();
  this->SelectionRepresentation->SetPaintbrushDrawing(this->PaintbrushDrawing);

  // Highlighting stuff. Selected sketches will be highlighted.
  this->HighlightActors = vtkKWEPaintbrushHighlightActors::New();
  this->HighlightActors->SetPaintbrushDrawing(this->PaintbrushDrawing);
}

//----------------------------------------------------------------------
vtkKWEPaintbrushRepresentation2D::~vtkKWEPaintbrushRepresentation2D()
{
  this->ShapeOutline->Delete();
  this->ShapeOutlineMapper->Delete();
  this->ShapeOutlineActor->Delete();
  this->ShapeOutlinePropertyDraw->Delete();
  this->ShapeOutlinePropertyErase->Delete();
  this->ShapeOutlinePropertyInteract->Delete();
  if (this->PaintbrushBlend)
    {
    this->PaintbrushBlend->Delete();
    }
  this->SetImageActor(NULL);
  this->SetImageData(NULL);
  this->SetShapePlacer(NULL);
  this->SelectionRepresentation->Delete();
  this->HighlightActors->Delete();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetImageActor( vtkImageActor * imageActor )
{
  if (this->ImageActor != imageActor)
    {
    vtkImageActor * var = this->ImageActor;
    this->ImageActor = imageActor;
    if (this->ImageActor != NULL) { this->ImageActor->Register(this); }
    if (var != NULL)
      {
      var->UnRegister(this);
      }
    this->Modified();

    if (this->ImageActor)
      {
      // Set the actor as the placer's actor, if it is an ImageActorPointPlacer.
<<<<<<< HEAD
      vtkInriaImageActorPointPlacer * imageActorPointPlacer =
        vtkInriaImageActorPointPlacer::SafeDownCast(this->ShapePlacer);
=======
      vtkImageActorPointPlacer * imageActorPointPlacer =
        vtkImageActorPointPlacer::SafeDownCast(this->ShapePlacer);
>>>>>>> b2dca81... Starting to work on vtkEdgewidgetss
      if (imageActorPointPlacer)
        {
        imageActorPointPlacer->SetImageActor(this->ImageActor);
        }
      }

    if (vtkKWEPaintbrushSelectionRepresentation2D * selectionRep =
         vtkKWEPaintbrushSelectionRepresentation2D::SafeDownCast(
                                this->SelectionRepresentation))
      {
      selectionRep->SetImageActor(imageActor);
      }
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetImageData( vtkImageData * imageData )
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

  if (this->ImageData)
    {
    this->PaintbrushDrawing->SetImageData( this->ImageData );
    this->PaintbrushDrawing->InitializeData();
    if (this->PaintbrushOperation)
      {
      this->PaintbrushOperation->SetImageData(this->ImageData);
      }
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetPaintbrushOperation(
                          vtkKWEPaintbrushOperation * filter )
{
  if (this->PaintbrushOperation != filter)
    {
    this->Superclass::SetPaintbrushOperation(filter);

    if ( this->PaintbrushOperation && this->ImageActor &&
        !this->PaintbrushOperation->GetImageData())
      {
      this->PaintbrushOperation->SetImageData(this->ImageActor->GetInput());
      }
    }
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::CreateShapeOutline( double *pos )
{
  // TODO: Create a polydata that outlines the actual template..

  if (!this->ImageActor)
    {
    vtkErrorMacro( <<
      "The image actor must be set prior to overlaying the paintbrush template");
    }

  if (this->ShapeOutline)
    {
    this->ShapeOutline->Delete();
    this->ShapeOutline = NULL;
    }

  vtkImageData *image = this->ImageActor->GetInput();
  if (!image)
    {
    vtkErrorMacro( <<
      "The image actor must be set prior to overlaying the paintbrush template");
    return;
    }

  this->ShapeOutline = vtkPolyData::New();
  this->ShapeOutlineMapper->SetInput(this->ShapeOutline);

  // Rebuild the template outline.

  double viewPlaneNormal[3];
  this->Renderer->GetActiveCamera()->GetViewPlaneNormal(viewPlaneNormal);
  vtkPlane *plane = vtkPlane::New();
  plane->SetNormal(viewPlaneNormal);
  plane->SetOrigin(pos);
  vtkSmartPointer< vtkPolyData > pd =
    this->PaintbrushOperation->GetPaintbrushShape()->GetShapePolyData(pos, plane);
  plane->Delete();

  if (pd)
    {
    this->ShapeOutline->DeepCopy(pd.GetPointer());
    }

  this->CurrentShapePosition[0] = pos[0];
  this->CurrentShapePosition[1] = pos[1];
  this->CurrentShapePosition[2] = pos[2];
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::BuildRepresentation()
{
  this->HighlightActors->SetExtent(this->ImageActor->GetDisplayExtent());
  this->HighlightActors->BuildRepresentation();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::ReleaseGraphicsResources(vtkWindow *w)
{
  this->ShapeOutlineActor->ReleaseGraphicsResources(w);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::RenderOverlay(vtkViewport *viewport)
{
  int count = 0;
  if (this->ShapeOutlineActor->GetVisibility())
    {
    count += this->ShapeOutlineActor->RenderOverlay(viewport);
    }
  if ( this->ImageActor->GetVisibility() )
    {
    count += this->ImageActor->RenderOverlay(viewport);
    }
  return count;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::RenderOpaqueGeometry(vtkViewport *viewport)
{
  int count = 0;
  this->BuildRepresentation();
  if (this->ShapeOutlineActor->GetVisibility())
    {
    count += this->ShapeOutlineActor->RenderOpaqueGeometry(viewport);
    }
  if ( this->ImageActor->GetVisibility() )
    {
    count += this->ImageActor->RenderOpaqueGeometry(viewport);
    }
  if ( this->HighlightActors->GetVisibility() )
    {
    count += this->HighlightActors->RenderOpaqueGeometry(viewport);
    }
  return count;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::ComputeInteractionState(
    int vtkNotUsed(X), int vtkNotUsed(Y), int vtkNotUsed(modified))
{
  return this->InteractionState;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::StartWidgetInteraction(double [2])
{
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::WidgetInteraction(double [2])
{
}

//#if VTKEdge_VTK_VERSION_DATE > 20070305
//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::RenderTranslucentPolygonalGeometry(vtkViewport *viewport)
{
  int count=0;
  if ( this->ImageActor->GetVisibility() )
    {
    count += this->ImageActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if (this->ShapeOutlineActor->GetVisibility())
    {
    count += this->ShapeOutlineActor->RenderTranslucentPolygonalGeometry(viewport);
    }
  if ( this->HighlightActors->GetVisibility() )
    {
    count += this->HighlightActors->RenderTranslucentPolygonalGeometry(viewport);
    }
  return count;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::HasTranslucentPolygonalGeometry()
{
  int result=0;
  if ( this->ShapeOutlineActor->GetVisibility() )
    {
    result |= this->ShapeOutlineActor->HasTranslucentPolygonalGeometry();
    }
  if ( this->ImageActor->GetVisibility() )
    {
    result |= this->ImageActor->HasTranslucentPolygonalGeometry();
    }
  if ( this->HighlightActors->GetVisibility() )
    {
    result += this->HighlightActors->HasTranslucentPolygonalGeometry();
    }
  return result;
}

//#else
//
////----------------------------------------------------------------------
//// Legacy method to support VTK source versions prior to 2007/03/05
//int vtkKWEPaintbrushRepresentation2D::RenderTranslucentGeometry(vtkViewport *viewport)
//{
//  int count=0;
//  if ( this->ImageActor->GetVisibility() )
//    {
//    count += this->ImageActor->RenderTranslucentGeometry(viewport);
//    }
//  if (this->ShapeOutlineActor->GetVisibility())
//    {
//    count += this->ShapeOutlineActor->RenderTranslucentGeometry(viewport);
//    }
//  if ( this->HighlightActors->GetVisibility() )
//    {
//    count += this->HighlightActors->RenderTranslucentGeometry(viewport);
//    }
//  return count;
//}
//#endif

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::ActivateShapeOutline( int x, int y )
{

  double displayPos[2], worldPos[3], worldOrient[9];
  displayPos[0] = x;
  displayPos[1] = y;

  // Compute world pos from the display pos.
<<<<<<< HEAD
  dynamic_cast<vtkInriaImageActorPointPlacer*>(this->ShapePlacer)->setRenderer(this->Renderer);
=======
>>>>>>> b2dca81... Starting to work on vtkEdgewidgetss
  if ( !this->ShapePlacer->ComputeWorldPosition( this->Renderer,
                                                 displayPos, worldPos,
                                                 worldOrient) )
    {
    return -1; // Do not draw the template here. The placer told us that this
            // is not a valid display location.
    }

  this->LastDisplayPosition[0] = x;  // bookkeeping
  this->LastDisplayPosition[1] = y;

  // If the paintbrush template has moved, draw it again at the new spot. The
  // tolerance checks if its moved by a sufficient enough distance to avoid
  // too many re-draws
  if ( this->InteractionState == PaintbrushResize
   ||  this->InteractionState == PaintbrushIsotropicResize
   ||  vtkMath::Distance2BetweenPoints(
         this->CurrentShapePosition, worldPos) >
          this->PaintbrushOperation->GetPaintbrushShape()->GetPositionTolerance2())
    {
    this->CreateShapeOutline(worldPos);

    vtkKWEPaintbrushVerboseMacro( "Created Shape outline at " 
        << worldPos[0] << "," << worldPos[1] << "," << worldPos[2] 
        << std::endl);
    return 1;
    }

  vtkKWEPaintbrushVerboseMacro( 
      "Shape close to previous position. Ignoring for speed.." 
      << std::endl );
  
  return 0;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::AddShapeToCurrentStroke( double p[3] )
{
  // If we are doing a slice by slice segmentation, do a clip with the 
  // current slice.
  int extent[6];
  if (this->SingleSliceThickBrush)
    {
    this->ImageActor->GetDisplayExtent(extent);
    }
  else
    {
    this->ImageData->GetExtent(extent);
    }
  this->PaintbrushDrawing->GetPaintbrushOperation()->
            GetPaintbrushShape()->SetClipExtent(extent);

  return this->Superclass::AddShapeToCurrentStroke(p);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::CreateDefaultRepresentation()
{
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::GetActors( vtkPropCollection * pc )
{
  this->ShapeOutlineActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::GetActors2D( vtkPropCollection * )
{
//  this->ShapeOutlineActor->GetActors(pc);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetStateToDraw()
{
  this->ShapeOutlineActor->SetProperty(this->ShapeOutlinePropertyDraw);
  this->SetShapeOutlineVisibility(1);
  this->Superclass::SetStateToDraw();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetStateToErase()
{
  this->ShapeOutlineActor->SetProperty(this->ShapeOutlinePropertyErase);
  this->SetShapeOutlineVisibility(1);
  this->Superclass::SetStateToErase();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetStateToInteract()
{
  this->ShapeOutlineActor->SetProperty(this->ShapeOutlinePropertyInteract);
  this->SetShapeOutlineVisibility(1);
  this->Superclass::SetStateToInteract();
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetStateToDisabled()
{
  this->SetShapeOutlineVisibility(0);
  this->Superclass::SetStateToDisabled();
}

//----------------------------------------------------------------------
// Will be called when the widget is enabled
void vtkKWEPaintbrushRepresentation2D::InstallPipeline()
{
  if (!this->ImageActor)
    {
    vtkErrorMacro( << "Cannot install pipeline before the image actor is set");
    return;
    }

  if (!this->PaintbrushBlend)
    {
    this->PaintbrushBlend = vtkKWEPaintbrushBlend::New();
    }

  // What we do here is to stick a "Blender" before the ImageActor. The blender
  // will blend the background image with the sketch to give us an overlay.
  if (!this->PipelineInstalled &&
      this->ImageActor->GetInput() != this->PaintbrushBlend->GetOutput())
    {
    // The "If" ensures that the pipeline has not already been installed.
    this->PaintbrushBlend->SetInput(this->ImageActor->GetInput());
    this->ImageActor->SetInput(this->PaintbrushBlend->GetOutput());
    }

  // Sanity check
  if (this->PaintbrushDrawing->GetRepresentation() != vtkKWEPaintbrushEnums::Binary &&
      this->PaintbrushDrawing->GetRepresentation() != vtkKWEPaintbrushEnums::Label)
    {
    vtkErrorMacro( << "This class is intended to be a representation for "
     << "overlaying vtkKWEPaintbrushStencilData or vtkKWEPaintbrushLabelData on an "
     << "image, not vtkKWEPaintbrushGrayscaleData !")
    }

  // Set the overlay paintbrush stencil as input to the blender.
  this->PaintbrushBlend->SetPaintbrushDrawing( this->PaintbrushDrawing );
  this->PaintbrushBlend->SetUseOverlay(this->UseOverlay);
  this->PipelineInstalled = 1;
}

//----------------------------------------------------------------------
// Will be called when the widget is disabled
void vtkKWEPaintbrushRepresentation2D::UnInstallPipeline()
{
  // Return if we aren't installed yet.
  if (!this->ImageActor || !this->PipelineInstalled || !this->PaintbrushBlend)
    {
    this->PipelineInstalled = 0;
    return;
    }

  this->PipelineInstalled = 0;

  // What we do here is undo what was done in InstallPipeline().

  if (this->PaintbrushBlend->GetOutput() == this->ImageActor->GetInput())
    {
    this->ImageActor->SetInput(static_cast<vtkImageData *>
              (this->PaintbrushBlend->GetInput()) );
    this->PaintbrushBlend->SetInputConnection(0,NULL);
    }
  else
    {
    // Get the consumer process object. Have the consumer's input be the
    // PaintbrushBlend's input. In otherwords, remove PaintbrushBlend
    // from the chain.
    vtkInformation *info = this->PaintbrushBlend->GetExecutive()->
                                              GetOutputInformation(0);
    vtkExecutive ** consumers = vtkExecutive::CONSUMERS()->GetExecutives(info);
    int consumerCount = vtkExecutive::CONSUMERS()->Length(info);

    for (int consumerIdx = 0; consumerIdx < consumerCount; consumerIdx++)
      {
      if (vtkAlgorithm * consumerProcessObject
            = consumers[consumerIdx]->GetAlgorithm())
        {
        // The assumption is made that PaintbrushBlend's output is always on
        // the first port of the consumer's. In general the consumer is another
        // PaintbrushBlend, or an ImageActor, the latter which is handled on
        // the other "if" block.
        consumerProcessObject->SetInputConnection( 0,
            this->PaintbrushBlend->GetInputConnection(0,0) );
        }
      }
    }

  this->PaintbrushBlend->Delete();
  this->PaintbrushBlend = NULL;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D
::SetPaintbrushDrawing(vtkKWEPaintbrushDrawing *drawing)
{
  this->Superclass::SetPaintbrushDrawing(drawing);
  this->HighlightActors->SetPaintbrushDrawing(drawing);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::ResizeShape(double d[3], int resizeType)
{
  if (this->PaintbrushOperation->GetPaintbrushShape()->Resize(d, resizeType))
    {
    this->CreateShapeOutline(this->CurrentShapePosition);
    return 1;
    }
  return 0;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::DeepCopy(vtkWidgetRepresentation *rep)
{
  if (this == rep)
    {
    return;
    }

  vtkKWEPaintbrushRepresentation2D *r
    = vtkKWEPaintbrushRepresentation2D::SafeDownCast(rep);
  if (r)
    {
    if (this->PaintbrushOperation && r->PaintbrushOperation)
      {
      this->PaintbrushOperation->DeepCopy(r->PaintbrushOperation);
      }
    }

  this->Superclass::DeepCopy(rep);
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::SetShapeOutlineVisibility(int v)
{
  this->ShapeOutlineActor->SetVisibility(v);
  this->HighlightActors->SetVisibility(v);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::GetShapeOutlineVisibility()
{
  return this->ShapeOutlineActor->GetVisibility();
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::IncreaseOpacity()
{
  if (!this->PaintbrushDrawing)
    {
    return 0;
    }

  const int nSketches = this->PaintbrushDrawing->GetNumberOfItems();
  int changed = 0;

  for (int i = 0; i < nSketches; i++)
    {
    vtkKWEPaintbrushSketch * sketch = this->PaintbrushDrawing->GetItem(i);
    vtkKWEPaintbrushProperty * property = sketch->GetPaintbrushProperty();
    double o = property->GetOpacity();
    if (o == 1.0)
      {
      continue;
      }

    property->SetOpacity(o + 0.1);
    changed = 1;
    }

  // "chnaged" is 1 if at least 1 shetch's property changed.
  return changed;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D::DecreaseOpacity()
{
  if (!this->PaintbrushDrawing)
    {
    return 0;
    }

  const int nSketches = this->PaintbrushDrawing->GetNumberOfItems();
  int changed = 0;

  for (int i = 0; i < nSketches; i++)
    {
    vtkKWEPaintbrushSketch * sketch = this->PaintbrushDrawing->GetItem(i);
    vtkKWEPaintbrushProperty * property = sketch->GetPaintbrushProperty();
    double o = property->GetOpacity();
    if (o == 0.0)
      {
      continue;
      }

    property->SetOpacity(o - 0.1);
    changed = 1;
    }

  // "chnaged" is 1 if at least 1 shetch's property changed.
  return changed;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::GetEtchExtents( int extent[6] )
{
  this->Superclass::GetEtchExtents(extent);

  // We know that the stroke can be no larger than the current shape's
  // etch on the image data.

  if (this->ImageData)
    {
    this->ImageData->GetExtent(extent);
    int extentShape[6];
    this->PaintbrushOperation->GetPaintbrushShape()->
      GetExtent(extentShape, this->CurrentShapePosition);

    double viewPlaneNormal[3];
    this->Renderer->GetActiveCamera()->GetViewPlaneNormal(viewPlaneNormal);
    for (int i = 0; i < 3; i++)
      {
      if (fabs(fabs(viewPlaneNormal[i]) - 1.0) < 1e-8)
        {
        extent[2*i]   = extentShape[2*i];
        extent[2*i+1] = extentShape[2*i+1];
        break;
        }
      }
    }

  // If we are in single slice mode, clip so that the stroke
  // has just the extents of this slice.
  int strokeExtents[6];
  if (this->SingleSliceThickBrush)
    {
    this->ImageActor->GetDisplayExtent(strokeExtents);
    }
  else
    {
    this->ImageData->GetExtent(strokeExtents);
    }
  vtkKWEPaintbrushUtilities::GetIntersectingExtents(
                      strokeExtents, extent, extent);
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D
::CopySketchToPreviousSlice( vtkKWEPaintbrushSketch *sketch,
                             bool lastStrokeOnly )
{
  // Figure out the extents of the current slice and the next slice.

  if (!this->PaintbrushDrawing)
    {
    vtkErrorMacro( << "Drawing does not exist!");
    return 0;
    }
  if (this->PaintbrushDrawing->GetRepresentation() 
      != vtkKWEPaintbrushEnums::Label)
    {
    vtkErrorMacro( << "This functionality exists only for label maps now");
    return 0;
    }
  
  // Figure out the extents of the current slice and the next slice.

  int currSliceExt[6], targetSliceExt[6], labelImageExtent[6];
  this->ImageActor->GetDisplayExtent(currSliceExt);

  for (int i = 0; i < 3; i++)
    {
    targetSliceExt[2*i] = currSliceExt[2*i];
    targetSliceExt[2*i+1] = currSliceExt[2*i+1];
    if (currSliceExt[2*i] == currSliceExt[2*i+1])
      {
      targetSliceExt[2*i] = currSliceExt[2*i] - 1;
      targetSliceExt[2*i+1] = currSliceExt[2*i+1] - 1;
      }
    }

  // Do a sanity check to make sure that the target extents are within the
  // whole extent. This may not be true if we are on the last slice.

  vtkKWEPaintbrushLabelData *lData =
    vtkKWEPaintbrushLabelData::SafeDownCast(
        this->PaintbrushDrawing->GetPaintbrushData());
  vtkImageData *labelImage = lData->GetLabelMap();
  labelImage->GetExtent(labelImageExtent);

  if (!vtkMath::ExtentIsWithinOtherExtent(targetSliceExt, labelImageExtent))
    {
    return 0;
    }

  if (lastStrokeOnly)
    {
    sketch->CopyLastStrokeFromExtentToExtent(currSliceExt, targetSliceExt);
    }
  else
    {
    sketch->CopySketchFromExtentToExtent(currSliceExt, targetSliceExt);
    }
  return 1;
}

//----------------------------------------------------------------------
int vtkKWEPaintbrushRepresentation2D
::CopySketchToNextSlice( vtkKWEPaintbrushSketch *sketch,
                         bool lastStrokeOnly )
{
  // Figure out the extents of the current slice and the next slice.

  if (!this->PaintbrushDrawing)
    {
    vtkErrorMacro( << "Drawing does not exist!");
    return 0;
    }
  if (this->PaintbrushDrawing->GetRepresentation() 
      != vtkKWEPaintbrushEnums::Label)
    {
    vtkErrorMacro( << "This functionality exists only for label maps now");
    return 0;
    }
  
  // Figure out the extents of the current slice and the next slice.

  int currSliceExt[6], targetSliceExt[6], labelImageExtent[6];
  this->ImageActor->GetDisplayExtent(currSliceExt);

  for (int i = 0; i < 3; i++)
    {
    targetSliceExt[2*i] = currSliceExt[2*i];
    targetSliceExt[2*i+1] = currSliceExt[2*i+1];
    if (currSliceExt[2*i] == currSliceExt[2*i+1])
      {
      targetSliceExt[2*i] = currSliceExt[2*i] + 1;
      targetSliceExt[2*i+1] = currSliceExt[2*i+1] + 1;
      }
    }

  // Do a sanity check to make sure that the target extents are within the
  // whole extent. This may not be true if we are on the last slice.

  vtkKWEPaintbrushLabelData *lData =
    vtkKWEPaintbrushLabelData::SafeDownCast(
        this->PaintbrushDrawing->GetPaintbrushData());
  vtkImageData *labelImage = lData->GetLabelMap();
  labelImage->GetExtent(labelImageExtent);

  if (!vtkMath::ExtentIsWithinOtherExtent(targetSliceExt, labelImageExtent))
    {
    return 0;
    }

  if (lastStrokeOnly)
    {
    sketch->CopyLastStrokeFromExtentToExtent(currSliceExt, targetSliceExt);
    }
  else
    {
    sketch->CopySketchFromExtentToExtent(currSliceExt, targetSliceExt);
    }
  return 1;
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushRepresentation2D::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os,indent);
}
