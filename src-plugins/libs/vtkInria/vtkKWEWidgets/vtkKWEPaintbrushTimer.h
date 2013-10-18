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

// .NAME vtkKWEPaintbrushTimer - A class useful to profile the paintbrush performance
// .SECTION Description
// The class is active only when the VTK_PAINTBRUSH_VERBOSE_MODE is ON. See
// vtkKWEPaintbrushUtilities.
//
// .SECTION See Also

#ifndef __vtkKWEPaintbrushTimer_h
#define __vtkKWEPaintbrushTimer_h

#include "vtkKWEWidgetsExport.h" // needed for export symbols directives
#include <string>
#include <iostream>

class vtkTimerLog;

class VTKEdge_WIDGETS_EXPORT vtkKWEPaintbrushTimer
{
public:
  vtkKWEPaintbrushTimer( const char *logString="timer" );
  ~vtkKWEPaintbrushTimer();
  void Log();
  void Log(std::ostream &);

protected:

  vtkTimerLog *TimerLog;
  std::string LogString;
  
private:
  vtkKWEPaintbrushTimer(const vtkKWEPaintbrushTimer&); //Not implemented
  void operator=(const vtkKWEPaintbrushTimer&); //Not implemented
};

#endif

