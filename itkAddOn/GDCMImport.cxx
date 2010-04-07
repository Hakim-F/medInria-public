/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: ImageRegistrationFactory.cxx 1 2008-01-22 19:01:33Z ntoussaint $
Language:  C++
Author:    $Author: ntoussaint $
Date:      $Date: 2008-01-22 20:01:33 +0100 (Tue, 22 Jan 2008) $
Version:   $Revision: 1 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

/**
 * This is a small tool that shows how to use the Registration Factory
 * Mostly inspired by Tom Vercauteren DemonsRegistration test code.
 *
 * The user can choose between different registration methods.
 * The user can also choose input transform file or input displacement field;
 *
 *
 * \author Nicolas Toussaint, INRIA
 */

#include <itkGDCMImporter3.h>

#ifndef WIN32
#include <getopt.h>
#else
#include "getoptcompat/getopt.h"
#endif //WIN32
#include <iostream>
#include <itksys/SystemTools.hxx>

struct arguments
{
  std::string  InputDirectory;  /* -i option */
  std::string  OutputDirectory; /* -o option */
  std::string  OutputFilename;  /* -f option */
  unsigned int AutoMode;        /* -a option */
  unsigned int Recursivity;     /* -r option */
  unsigned int UseDicomDir;     /* -d option */

  arguments () :
    InputDirectory(""),
    OutputDirectory(""),
    OutputFilename(""),
    AutoMode(0u),
    Recursivity(1u),
    UseDicomDir(0u)
  {
  }

  friend std::ostream& operator<< (std::ostream& o, const arguments& args)
  {
    std::ostringstream osstr;
    std::string iterstr = "[ " + osstr.str() + "]";

    return o
      <<"Arguments structure:"<<std::endl
      <<"  InputDirectory: "<<args.InputDirectory<<std::endl
      <<"  OutputDirectory: "<<args.OutputDirectory<<std::endl;
    
  }
};


static const char *optString = "i:o:f:a:r:d:h?";

static const struct option longOpts[] = {
  { "input-directory", required_argument, NULL, 'i' },
  { "output-directory", optional_argument, NULL, 'o' },
  { "help", no_argument, NULL, 'h' },
  { NULL, no_argument, NULL, 0 }
};

/*
  Display program usage, and exit.
*/
void display_usage( const std::string progname )
{
  struct arguments defargs = arguments();

  std::ostringstream osstr;
  std::string iterstr = "[ " + osstr.str() + "]";

  std::cout<<std::endl;
  std::cout<<progname<<" - Reconstruct volume images from DICOM exam"<<std::endl;
  std::cout<<"Usage: "<<progname<<" [OPTION...]"<<std::endl;

  std::cout<<"  -i/--input-directory(=STRING)  Input directory, root path to the DICOM exam - mandatory"<<std::endl;
  std::cout<<"  -o/--output-directory(=STRING) Output directory, where volume images shall be written (default: current directory)"<<std::endl;
  std::cout<<"  -h/--help                      Display this message and exit"<<std::endl;

  std::cout<<std::endl;
  std::cout<<"Copyright (c) 2010 INRIA."<<std::endl;
  std::cout<<"Code: Nicolas Toussaint."<<std::endl;
  std::cout<<"Report bugs to <nicolas.toussaint@sophia.inria.com>."<<std::endl;

  exit( EXIT_FAILURE );
}


void parseOpts (int argc, char **argv, struct arguments & args)
{
  const std::string progname( "GDCMImport" );

  // Default values.
  args = arguments();

  if (argc == 1)
  {
    display_usage(progname);
  }

  int opt = 0; /* it's actually going to hold a char */
  int longIndex = 0;

  while ( (opt = getopt_long(argc, argv, optString, longOpts, &longIndex)) != -1 )
  {
    switch( opt ) {
	case 'i':
	  if (! optarg) display_usage(progname);
	  args.InputDirectory = optarg;
	  break;
	case 'o':
	  if (! optarg) display_usage(progname);
	  args.OutputDirectory = optarg;
	  break;
	  args.OutputFilename = optarg;
	  break;

	case 'h':	/* fall-through is intentional */
	case '?':   /* fall-through is intentional */
	default:
	  display_usage(progname);
	  break;
    }
  }
}






int main( int argc, char *argv[] )
{
  struct arguments args;
  parseOpts (argc, argv, args);

  itk::MultiThreader::SetGlobalDefaultNumberOfThreads(2);

  std::cout<<"Starting GDCM DICOM import with the following arguments:"<<std::endl;
  std::cout<<args<<std::endl<<std::endl;

  typedef itk::GDCMImporter3<short> ImporterType;
  ImporterType::Pointer importer = ImporterType::New();

  try
  {
    importer->SetInputDirectory (args.InputDirectory);
    importer->Scan();
    
    importer->Update();

    std::string directory = itksys::SystemTools::GetCurrentWorkingDirectory();
    if (args.OutputDirectory.size())
      directory = args.OutputDirectory;    
    
    importer->SaveOutputsInDirectory (directory.c_str());
  }
  catch(itk::ExceptionObject &e)
  {
    std::cerr << e;
    exit( EXIT_FAILURE );
  }


  return EXIT_SUCCESS;
}

