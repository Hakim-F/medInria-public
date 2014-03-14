 /*=========================================================================

 medInria

 Copyright (c) INRIA 2013. All rights reserved.
 See LICENSE.txt for details.
 
  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.

=========================================================================*/

#pragma once

#ifdef WIN32
    #ifdef polygonRoiPlugin_EXPORTS
        #define POLYGONROIPLUGIN_EXPORT __declspec(dllexport) 
    #else
        #define POLYGONROIPLUGIN_EXPORT __declspec(dllimport) 
    #endif
#else
    #define POLYGONROIPLUGIN_EXPORT
#endif

// override is a C++0x feature implemented in MSVC from VS 2005 onwards.
#if defined(_MSC_VER) && (_MSC_VER >= 1400 ) 
#define MED_OVERRIDE override
#else 
// Not in gcc (yet?)
#define MED_OVERRIDE
#endif


