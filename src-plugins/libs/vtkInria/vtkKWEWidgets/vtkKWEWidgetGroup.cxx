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
#include "vtkKWEWidgetGroup.h"

#include "vtkObjectFactory.h"
#include "vtkGarbageCollector.h"
#include <algorithm>
#include "vtkKWEAbstractPaintbrushWidget.h" // REMOVE

vtkCxxRevisionMacro(vtkKWEWidgetGroup, "$Revision: 3601 $");
vtkStandardNewMacro(vtkKWEWidgetGroup);

//----------------------------------------------------------------------
vtkKWEWidgetGroup::vtkKWEWidgetGroup()
{
  this->InteractiveRenderAllWidgets = 1;
  this->Description = NULL;
}

//----------------------------------------------------------------------
vtkKWEWidgetGroup::~vtkKWEWidgetGroup()
{
  for (WidgetIteratorType it  = this->Widget.begin();
                          it != this->Widget.end()  ; ++it)
    {
    if (*it)
      {
      this->RemoveWidget( *it );
      (*it)->Delete();
      }
    }

  this->SetDescription(NULL);
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::Register( vtkObjectBase *o )
{
  // Enable garbage collection due to ref counting cycles with vtkAbstractWidget
  this->RegisterInternal(o, 1);
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::UnRegister( vtkObjectBase *o )
{
  // Enable garbage collection due to ref counting cycles with vtkAbstractWidget
  this->UnRegisterInternal(o, 1);
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::SetEnabled(int enabling)
{
  for (WidgetIteratorType it  = this->Widget.begin();
                          it != this->Widget.end()  ; ++it)
    {
    (*it)->SetEnabled(enabling);
    }
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::SetProcessEvents(int p)
{
  for (WidgetIteratorType it  = this->Widget.begin();
                          it != this->Widget.end()  ; ++it)
    {
    (*it)->SetProcessEvents(p);
    }
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::AddWidget( vtkAbstractWidget *w )
{
  for ( unsigned int i = 0; i < this->Widget.size(); i++)
    {
    if (this->Widget[i] == w)
      {
      return;
      }
    }

  // TODO : Won't be necessary if we move this to the AbstractWidget.. superclass
  vtkKWEAbstractPaintbrushWidget *obj=
    static_cast<vtkKWEAbstractPaintbrushWidget *>(w);

  // If the widget was attached to someother widget set, remove it from that.
  if (vtkKWEWidgetGroup * otherWidgetGroup = obj->WidgetGroup)
    {
    otherWidgetGroup->RemoveWidget(w);
    }

  // Here is where we introduce the ref-counting cycle.
  w->Register(this);
  this->Register(w);

  this->Widget.push_back(w);
  obj->WidgetGroup = this;
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::RemoveWidget( vtkAbstractWidget * w)
{
  for (WidgetIteratorType it  = this->Widget.begin();
                          it != this->Widget.end()  ; ++it)
    {
    if (*it == w)
      {
      this->Widget.erase(it);
      static_cast<vtkKWEAbstractPaintbrushWidget *>(w)->WidgetGroup = NULL;
      w->UnRegister(this);
      this->UnRegister(w);
      break;
      }
    }
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::Render()
{
  for (WidgetIteratorType it  = this->Widget.begin();
                          it != this->Widget.end()  ; ++it)
    {
    (*it)->Render();
    }
}

//----------------------------------------------------------------------
int vtkKWEWidgetGroup::HasWidget( vtkAbstractWidget * w )
{
  return (std::find(this->Widget.begin(), this->Widget.end(), w) 
                                    == this->Widget.end() ? 0 : 1);
}

//----------------------------------------------------------------------
vtkAbstractWidget *
vtkKWEWidgetGroup::GetNthWidget( unsigned int i )
{
  return this->Widget[i];
}

//----------------------------------------------------------------------
unsigned int vtkKWEWidgetGroup::GetNumberOfWidgets()
{
  return static_cast< unsigned int >(this->Widget.size());
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::ReportReferences(vtkGarbageCollector* collector)
{
  const unsigned int n = this->GetNumberOfWidgets();
  if (n)
    {
    this->Superclass::ReportReferences(collector);
    for (unsigned int i = 0; i < n; i++)
      {
      vtkGarbageCollectorReport(collector, this->Widget[i],
          "A widget from the WidgetGroup");
      }
    }
}

//----------------------------------------------------------------------
void vtkKWEWidgetGroup::PrintSelf(ostream& os, vtkIndent indent)
{
  //Superclass typedef defined in vtkTypeMacro() found in vtkSetGet.h
  this->Superclass::PrintSelf(os,indent);

  if (this->Description)
  {
    os << indent << "Description: " << this->Description << std::endl;
  }
}

