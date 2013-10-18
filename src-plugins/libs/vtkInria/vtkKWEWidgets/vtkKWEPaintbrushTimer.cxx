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
#include "vtkKWEPaintbrushTimer.h"
#include "vtkTimerLog.h"
#include "vtkKWEPaintbrushUtilities.h"

//----------------------------------------------------------------------
vtkKWEPaintbrushTimer::vtkKWEPaintbrushTimer(const char *log)
{
#ifdef VTK_PAINTBRUSH_VERBOSE_MODE
  this->LogString = log;
  this->TimerLog = vtkTimerLog::New();
  this->TimerLog->StartTimer();
#endif
}

//----------------------------------------------------------------------
vtkKWEPaintbrushTimer::~vtkKWEPaintbrushTimer()
{
#ifdef VTK_PAINTBRUSH_VERBOSE_MODE
  this->TimerLog->StopTimer();
  this->Log(std::cout);
  this->TimerLog->Delete();
#endif
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushTimer::Log()
{
#ifdef VTK_PAINTBRUSH_VERBOSE_MODE
  this->Log(std::cout);
#endif
}

//----------------------------------------------------------------------
void vtkKWEPaintbrushTimer::Log(std::ostream& os)
{
#ifdef VTK_PAINTBRUSH_VERBOSE_MODE
  this->TimerLog->StopTimer();
  os << this->LogString << " took " 
     << this->TimerLog->GetElapsedTime() << std::endl;
#endif
}

