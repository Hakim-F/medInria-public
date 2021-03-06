/*============================================================================

The Hardware Shading (HWShading) module is protected by the
following copyright:
Copyright (c) 2007 Biomedical Image Analysis (BMIA) - Department of
Biomedical Engineering - Eindhoven University of Technology.
All rights reserved. See Copyright.txt for details.
The HWShading implementation was originally written by Tim Peeters (BMIA - TUe)
and published at the "11th International Fall Workshop on Vision, Modeling,
and Visualization 2006" (VMV'06):
"Visualization of the Fibrous Structure of the Heart", by T. Peeters et al.
See http://timpeeters.com/research/vmv2006 for more information.
This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.
============================================================================*/
/**
 * vtkLookupTableCollection.h
 *
 * 2006-04-06	Tim Peeters
 * - First version
 */

#include "vtkLookupTableCollection.h"
#include <vtkLookupTable.h>
#include <vtkObjectFactory.h>

vtkStandardNewMacro(vtkLookupTableCollection);

vtkLookupTableCollection::vtkLookupTableCollection()
{
  // nothing to do
}

vtkLookupTableCollection::~vtkLookupTableCollection()
{
  // emtpy
}

void vtkLookupTableCollection::AddItem(vtkLookupTable* lut)
{
  this->vtkCollection::AddItem((vtkObject*)lut);
}

vtkLookupTable* vtkLookupTableCollection::GetNextItem()
{
  return static_cast<vtkLookupTable *>(this->GetNextItemAsObject());
}

vtkLookupTable* vtkLookupTableCollection::GetItem(int i)
{
  return static_cast<vtkLookupTable *>(this->GetItemAsObject(i));;
}
