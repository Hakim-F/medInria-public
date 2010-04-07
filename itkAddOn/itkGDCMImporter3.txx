/*=========================================================================

Program:   vtkINRIA3D
Module:    $Id: itkGDCMImporter3.cxx 522 2007-12-05 15:06:51Z ntoussaint $
Language:  C++
Author:    $Author: ntoussaint $
Date:      $Date: 2007-12-05 16:06:51 +0100 (Wed, 05 Dec 2007) $
Version:   $Revision: 522 $

Copyright (c) 2007 INRIA - Asclepios Project. All rights reserved.
See Copyright.txt for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "itkGDCMImporter3.h"

#include "gdcmReader.h"
#include "gdcmDirectionCosines.h"
#include "gdcmStringFilter.h"


namespace itk
{


  
//----------------------------------------------------------------------------
  template <class TPixelType>
  void GDCMVolume<TPixelType>::Write (std::string filename)
  {
    typename WriterType::Pointer writer = WriterType::New();
    writer->SetFileName (filename);
    writer->SetInput (this);
    
    try
    {
      std::cout << "writing: " << filename << std::endl;
      writer->Write();
    }
    catch (itk::ExceptionObject & e)
    {
      std::cerr<<e<<std::endl;
    }
  }
  

//----------------------------------------------------------------------------
  template <class TPixelType>
  void GDCMVolume<TPixelType>::Build(void)
  {

    FileListMapType map = this->GetFileListMap();

    typedef typename ImageType::RegionType RegionType;
    typedef typename ImageType::SpacingType SpacingType;
    typedef typename ImageType::PointType PointType;
    typedef typename ImageType::DirectionType DirectionType;
    typedef typename itk::ImageRegionIterator<ImageType> IteratorType;
    typedef typename IteratorType::IndexType IndexType;
    
    typename ImageType::Pointer image = ImageType::New();
    
    
    typename FileListMapType::iterator it;
    typename itk::GDCMImageIO::Pointer io = itk::GDCMImageIO::New();
    bool metadatacopied = 0;
    IteratorType itOut;

    std::cout<<"building volume "<<this->GetName()<<" containing "<<map.size()<<" subvolumes."<<std::endl;
    
    for (it = map.begin(); it != map.end(); ++it)
    {
      typename SeriesReaderType::Pointer seriesreader = SeriesReaderType::New();
      seriesreader->UseStreamingOn();
      
      seriesreader->SetFileNames( (*it).second );
      seriesreader->SetImageIO( io );
      try
      {
	seriesreader->Update();
      }
      catch (itk::ExceptionObject & e)
      {
	std::cerr << e;
	throw itk::ExceptionObject (__FILE__,__LINE__,"Error in GDCMVolume::Build()");
      }
      
      typename SubImageType::Pointer t_image = seriesreader->GetOutput();

      if (!metadatacopied)
      {
	RegionType region;
	region.SetSize (0, t_image->GetLargestPossibleRegion().GetSize()[0]);
	region.SetSize (1, t_image->GetLargestPossibleRegion().GetSize()[1]);
	region.SetSize (2, t_image->GetLargestPossibleRegion().GetSize()[2]);
	region.SetSize (3, map.size());
	image->SetRegions (region);
	image->Allocate();
	SpacingType spacing;
	spacing[0] = t_image->GetSpacing()[0];
	spacing[1] = t_image->GetSpacing()[1];
	spacing[2] = t_image->GetSpacing()[2];
	spacing[3] = 1;
	image->SetSpacing (spacing);
	PointType origin;
	origin[0] = t_image->GetOrigin()[0];
	origin[1] = t_image->GetOrigin()[1];
	origin[2] = t_image->GetOrigin()[2];
	origin[3] = 0;
	image->SetOrigin (origin);
	DirectionType direction;
	for (unsigned int i=0; i<4; i++)
	  for (unsigned int j=0; j<4; j++)
	  {
	    if ((i < 3) && (j < 3))
	      direction[i][j] = t_image->GetDirection()[i][j];
	    else
	      direction[i][j] = (i == j) ? 1 : 0;
	  }
	image->SetDirection(direction);
	itOut = IteratorType (image, region);

	this->SetMetaDataDictionary (io->GetMetaDataDictionary());
	
	metadatacopied = 1;
      }
      
      typename itk::ImageRegionIterator<SubImageType> itIn(t_image, t_image->GetLargestPossibleRegion());
      while (!itIn.IsAtEnd())
      {
	itOut.Set(itIn.Get());
	++itIn;
	++itOut;
      }
    }
    std::cout<<"done"<<std::endl;

    this->Graft (image);
    this->SetMetaDataDictionary (io->GetMetaDataDictionary());
  }

//----------------------------------------------------------------------------
  template <class TPixelType>
  void GDCMVolume<TPixelType>::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf( os, indent );
    unsigned long NumberOfFiles = 0;
    NumberOfFiles = this->GetFileListMap().size();
    os << indent << "Number of Files: " << NumberOfFiles << std::endl;
    os << indent << "Name: " << this->GetName() << std::endl;
    os << indent << "Is image built: " << m_IsBuilt << std::endl;
    os << indent << "Dictionary : " << std::endl;

    std::string value;
    itk::MetaDataDictionary dict = this->GetMetaDataDictionary();
    //Smarter approach using real iterators
    itk::MetaDataDictionary::ConstIterator itr = dict.Begin();
    itk::MetaDataDictionary::ConstIterator end = dict.End();

    while(itr != end)
    {
      const std::string &key = itr->first;
      ExposeMetaData<std::string>(dict, key, value);
      os << indent<< indent << key.c_str() << " : " << value.c_str()<< std::endl;
      ++itr;
    }
  }
  


//----------------------------------------------------------------------------
  template <class TPixelType>
  GDCMImporter3<TPixelType>::GDCMImporter3()
  {
    itk::GDCMImageIOFactory::RegisterOneFactory();

    m_InputDirectory  = "";
    m_IsScanned = 0;
    
    this->SetNumberOfRequiredInputs (0);
    this->SetNumberOfRequiredOutputs (0);
    this->SetNumberOfOutputs (0);

        // 0010 0010 Patient name
    // we want to reconstruct an image from a single
    // patient
    this->m_FirstScanner.AddTag( gdcm::Tag(0x10,0x10) ); // patient's name
    // 0020 000d Study uid
    // single study
    this->m_FirstScanner.AddTag( gdcm::Tag(0x20,0xd) );  // study uid
    // 0020 000e Series uid
    // single serie
    this->m_FirstScanner.AddTag( gdcm::Tag(0x20,0xe) );  // series uid
    // 0020 0037 Orientation
    // single orientation matrix
    this->m_FirstScanner.AddTag( gdcm::Tag(0x20,0x37) ); // orientation
    // 0020 0011 Series Number
    // A scout scan prior to a CT volume scan can share the same
    //   SeriesUID, but they will sometimes have a different Series Number
    this->m_FirstScanner.AddTag( gdcm::Tag(0x0020, 0x0011) );
    // 0018 0024 Sequence Name
    // For T1-map and phase-contrast MRA, the different flip angles and
    //   directions are only distinguished by the Sequence Name
    this->m_FirstScanner.AddTag( gdcm::Tag(0x0018, 0x0024) );
    // 0018 0050 Slice Thickness
    // On some CT systems, scout scans and subsequence volume scans will
    //   have the same SeriesUID and Series Number - YET the slice 
    //   thickness will differ from the scout slice and the volume slices.
    this->m_FirstScanner.AddTag( gdcm::Tag(0x0018, 0x0050) );
    // 0028 0010 Rows
    // If the 2D images in a sequence don't have the same number of rows,
    // then it is difficult to reconstruct them into a 3D volume.
    this->m_FirstScanner.AddTag( gdcm::Tag(0x0028, 0x0010));
    // 0028 0011 Columns
    // If the 2D images in a sequence don't have the same number of columns,
    // then it is difficult to reconstruct them into a 3D volume.
    this->m_FirstScanner.AddTag( gdcm::Tag(0x0028, 0x0011));


    // 0018 9087 Diffusion B-Factor
    // If the 2D images in a sequence don't have the b-value,
    // then we separate the DWIs.
    this->m_SecondScanner.AddTag( gdcm::Tag(0x18,0x9087));
    // 0018 9089 Gradient Orientation
    // If the 2D images in a sequence don't have the same gradient orientation,
    // then we separate the DWIs.
    this->m_SecondScanner.AddTag( gdcm::Tag(0x18,0x9089));
    // 0018 1060 Trigger Time
    this->m_SecondScanner.AddTag( gdcm::Tag(0x18,0x1060));

    
    // 0020 0032 Position Patient
    this->m_ThirdScanner.AddTag( gdcm::Tag(0x20,0x32) );  
    // 0020 0037 Orientation Patient
    this->m_ThirdScanner.AddTag( gdcm::Tag(0x20,0x37) );  
        
    
  }


//----------------------------------------------------------------------------
  template <class TPixelType>
  void GDCMImporter3<TPixelType>::Scan (void)
  {

    if (m_IsScanned)
      return;
    
    if (!itksys::SystemTools::FileExists (this->m_InputDirectory.c_str()))
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error in GDCMImporter3::Load(): directory not found");

    this->UpdateProgress(0.00);

    this->m_FileListMapofMap.clear();
    
    gdcm::Directory directory;
    std::cout<<"loading"<<std::endl;    
    unsigned long nfiles = directory.Load( this->m_InputDirectory, true);
    std::cout<<"done : "<<nfiles<< " files included."<<std::endl;

    if (!nfiles)
    {
      itkExceptionMacro (<<"The GDCM loader did not succeed loading directory "<<this->m_InputDirectory<<" (or there is no file in directory)"<<std::endl);
      return;
    }    
    
    // First sort of the filenames.
    // it will distinguish patient name, series and instance uids, sequence name
    // as well as the image orientation and the nb of columns and rows 
    FileListMapType map = this->PrimarySort (directory.GetFilenames());

    std::cout<<"primary sort gives "<<map.size()<<" different volumes (outputs)"<<std::endl;
    
    typename FileListMapType::const_iterator it;
    for (it = map.begin(); it != map.end(); ++it)
    {
      
      // Second sort of the filenames.
      // it will distinguish the gradient orientation, the b-value, the trigger time
      // and the T2/pr* density.
      FileListMapType mapi = this->SecondarySort ((*it).second);

      std::cout<<"  secondary division gives "<<mapi.size()<<" subvolumes"<<std::endl;    
      
      typename FileListMapType::iterator it2;
      for (it2 = mapi.begin(); it2 != mapi.end(); ++it2)
      {
	
	// Third sort of the filenames.
	// it will detect identical image positions
	// and separate distinct volumes.
	// If all image positions are identical, no sort is done.
	FileListMapType mapij = this->TertiarySort ((*it2).second);
      
	if (mapij.size() >= 1)
	{
	  typename FileListMapType::iterator it3;
	  typename FileListMapType::iterator end = mapij.end();
	  std::string rootid = (*it2).first;
	  mapi.erase (it2);
	  it2--;
	  for(it3 = mapij.begin(); it3 != mapij.end(); it3++)
	  {
	    std::ostringstream os;
	    os << rootid << (*it3).first;
	    typename FileListMapType::value_type newpair(os.str(),(*it3).second );
	    it2 = mapi.insert (it2, newpair);
	  }
	  
	}
      }
      // We add the map of volumes in the global "database".
      // If more than 1 volume is in the map, it will be saved as a 4D image.
      this->m_FileListMapofMap[(*it).first] = mapi;
    }

    std::cout<<"sorting finished "<<std::endl;
    m_IsScanned = 1;
    
    this->Reset();
  }

  //----------------------------------------------------------------------------
  template <class TPixelType>
  typename GDCMImporter3<TPixelType>::FileListMapType
  GDCMImporter3<TPixelType>::PrimarySort (FileList list)
  {

    FileListMapType ret;
    
    std::cout<<"first scan"<<std::endl;    
    if (!this->m_FirstScanner.Scan (list))
    {
      itkExceptionMacro (<<"The GDCM scanner did not succeed scanning directory "<<this->m_InputDirectory<<std::endl);
      return ret;
    }
    std::cout<<"done"<<std::endl;
    
    gdcm::Directory::FilenamesType::const_iterator file;
    gdcm::Scanner::TagToValue::const_iterator it;
    
    
    for (file = list.begin(); file != list.end(); ++file)
    {
      if( this->m_FirstScanner.IsKey((*file).c_str()) )
      {
	const gdcm::Scanner::TagToValue& mapping = this->m_FirstScanner.GetMapping((*file).c_str());
	
	if ( !this->m_FirstScanner.GetValue  ((*file).c_str(), gdcm::Tag(0x0028, 0x0010)) )
	  continue;
	
	std::ostringstream os;
	for(it = mapping.begin(); it != mapping.end(); ++it)
	  os <<(*it).second<<".";
	
	if (ret.find(os.str()) == ret.end())
	{
	  FileList newlist;
	  newlist.push_back ((*file).c_str());
	  ret[os.str()] = newlist;
	}
	else
	  ret[os.str()].push_back ((*file).c_str());
      }
      else
	std::cout<<"The file "<<(*file).c_str()<<" does not appear in the scanner mappings, skipping. "<<std::endl;
    }    

    return ret;
    
  }

  //----------------------------------------------------------------------------
  template <class TPixelType>
  typename GDCMImporter3<TPixelType>::FileListMapType
  GDCMImporter3<TPixelType>::SecondarySort (FileList list)
  {

    FileListMapType ret;
    
    if (!this->m_SecondScanner.Scan (list))
    {
      std::cerr<<"The GDCM scanner did not succeed scanning the list, skipping"<<std::endl;
      return ret;
    }
      
    gdcm::Directory::FilenamesType::const_iterator file;
    gdcm::Scanner::TagToValue::const_iterator it;
    
    for (file = list.begin(); file != list.end(); ++file)
    {
      if( this->m_FirstScanner.IsKey((*file).c_str()) )
      {
	const gdcm::Scanner::TagToValue& mapping = this->m_SecondScanner.GetMapping((*file).c_str());
	
	std::ostringstream os;
	for(it = mapping.begin(); it != mapping.end(); ++it)
	  os <<(*it).second<<".";

	if (ret.find(os.str()) == ret.end())
	{
	  FileList newlist;
	  newlist.push_back ((*file).c_str());
	  ret[os.str()] = newlist;
	}
	else
	  ret[os.str()].push_back ((*file).c_str());
      }
      else
	std::cout<<"The file "<<(*file).c_str()<<" does not appear in the scanner mappings, skipping. "<<std::endl;
    }    

    return ret;
    
  }
  

//----------------------------------------------------------------------------
  template <class TPixelType>
  typename GDCMImporter3<TPixelType>::FileListMapType
  GDCMImporter3<TPixelType>::TertiarySort (FileList list)
  {

    FileListMapType ret;
    
    if (!this->m_ThirdScanner.Scan (list))
    {
      std::cerr<<"The GDCM scanner did not succeed scanning the list, skipping"<<std::endl;
      return ret;
    }

    const char *reference = list[0].c_str();
    gdcm::Scanner::TagToValue const &t2v = this->m_ThirdScanner.GetMapping(reference);
    gdcm::Scanner::TagToValue::const_iterator firstit = t2v.find( gdcm::Tag(0x20,0x37) );
    if( (*firstit).first != gdcm::Tag(0x20,0x37) )
    {
    // first file does not contains Image Orientation (Patient), let's give up
      std::cerr<<"No iop in first file "<<std::endl;
      return ret;
    }

    const char *dircos = (*firstit).second;
    std::stringstream ss;
    ss.str( dircos );
    gdcm::Element<gdcm::VR::DS,gdcm::VM::VM6> cosines;
    cosines.Read( ss );
    
    // http://www.itk.org/pipermail/insight-users/2003-September/004762.html
    // Compute normal:
    // The steps I take when reconstructing a volume are these: First,
    // calculate the slice normal from IOP:
    double normal[3];
    normal[0] = cosines[1]*cosines[5] - cosines[2]*cosines[4];
    normal[1] = cosines[2]*cosines[3] - cosines[0]*cosines[5];
    normal[2] = cosines[0]*cosines[4] - cosines[1]*cosines[3];
    
    gdcm::DirectionCosines dc;
    dc.SetFromString( dircos );
    if( !dc.IsValid() )
    {
      std::cerr<<"cosines not valid "<<std::endl;
      return ret;
    }

    
    double normal2[3];
    dc.Cross( normal2 );
    // You only have to do this once for all slices in the volume. Next, for
    // each slice, calculate the distance along the slice normal using the IPP
    // tag ("dist" is initialized to zero before reading the first slice) :
    typedef std::map<double, FileList> SortedFilenames;
    SortedFilenames sorted;

    std::vector<std::string>::const_iterator it;

    for(it = list.begin(); it != list.end(); ++it)
    {
      const char *filename = (*it).c_str();
      bool iskey = this->m_ThirdScanner.IsKey(filename);
      if( iskey )
      {
	const char *value =  this->m_ThirdScanner.GetValue(filename, gdcm::Tag(0x20,0x32));
	if( value )
        {
	  gdcm::Element<gdcm::VR::DS,gdcm::VM::VM3> ipp;
	  std::stringstream ss;
	  ss.str( value );
	  ipp.Read( ss );
	  double dist = 0;
	  for (int i = 0; i < 3; ++i)
	    dist += normal[i]*ipp[i];
	  
	  if( sorted.find(dist) == sorted.end() )
	  {
	    FileList newlist;
	    newlist.push_back (filename);
	    typename SortedFilenames::value_type newpair(dist,newlist);
	    sorted.insert( newpair );
	  }
	  else
	    sorted[dist].push_back (filename);
	}
      }
      else
	std::cout<<"The file "<<filename<<" does not appear in the scanner mappings, skipping. "<<std::endl;
    }

    if ((list.size() % sorted.size()) != 0)
    {
      std::cerr<<"inconsistent sizes : "<<list.size()<<" in "<<sorted.size()<<", , no sorting"<<std::endl;
      return ret;
    }

    if (sorted.size() == 1)
    {
      // single position, no sorting;
      return ret;
    }

    unsigned int nb_of_volumes = list.size() / sorted.size();
    
    for (unsigned int i=0; i<nb_of_volumes; i++)
    {
      std::ostringstream os;
      os<<"."<<i;
      FileList newfilelist;

      typename SortedFilenames::const_iterator newit;
      for (newit = sorted.begin(); newit != sorted.end(); ++newit)
	newfilelist.push_back ((*newit).second[i]);

      typename FileListMapType::value_type newpair(os.str(),newfilelist);
      ret.insert (newpair);
    }

    return ret;
  }

//----------------------------------------------------------------------------
  template <class TPixelType>
  void GDCMImporter3<TPixelType>::Reset (void)
  {

    std::cout<<"reseting..."<<std::endl;
    
    this->SetNumberOfOutputs (0);
    this->PrepareOutputs();

    try
    {

      this->UpdateProgress(0.0);

      typename FileListMapofMapType::iterator it;

      for (it = this->m_FileListMapofMap.begin(); it != this->m_FileListMapofMap.end(); it++)
      {
	typename FileListMapType::iterator it2;
	
	FileListMapType filelistmap = (*it).second;
	if ( !filelistmap.size() )
	  continue;
	
	std::string name = this->GenerateUniqueName(filelistmap);
	typename ImageType::Pointer image = ImageType::New();
	
	image->SetName (name.c_str());
	image->SetFileListMap (filelistmap);
	this->AddOutput (image);
      }
    }
    
    catch (itk::ExceptionObject & e)
    {
      this->UpdateProgress(1.0);
      this->SetProgress(0.0);
      std::cerr << e << std::endl;
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error in GDCMImporter3::InitializeOutputs(), during tree structuring");
    }
    
    this->UpdateProgress(1.0);
    this->SetProgress(0.0);
  }

//----------------------------------------------------------------------------
  template <class TPixelType>
  void
  GDCMImporter3<TPixelType>::SaveOutputInFile (unsigned int N, const char* filename)
  {
    
    try
    {
      this->GetOutput (N)->Build();
      this->GetOutput (N)->Write (filename);      
    }
    catch (itk::ExceptionObject & e)
    {
      std::cerr << e << std::endl;
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error in GDCMImporter3::SaveOutputInFile()");
    }

  }



//----------------------------------------------------------------------------
  template <class TPixelType>
  void
  GDCMImporter3<TPixelType>::SaveOutputsInDirectory (const char* directory)
  {
    for( unsigned int i=0; i<this->GetNumberOfOutputs(); i++)
    {
      std::ostringstream os;
      os << this->GetOutput(i)->GetName() <<".mha";
      this->GetOutput(i)->Write (os.str());
    }
    
  }

//----------------------------------------------------------------------------
  template <class TPixelType>
  void
  GDCMImporter3<TPixelType>::GenerateData (void)
  {
    if (!m_IsScanned)
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error in GDCMImporter3::GenerateData(): MUST call Scan() before Update()");

    this->UpdateProgress(0.0);
    
    try
    {
      for (unsigned int i=0; i<this->GetNumberOfOutputs(); i++)
      {
	this->GetOutput (i)->Build();
	this->UpdateProgress((double)i/(double)this->GetNumberOfOutputs());
      }
    }
    catch (itk::ExceptionObject & e)
    {
      this->UpdateProgress(1.0);
      this->SetProgress(0.0);
      std::cerr << e << std::endl;
      throw itk::ExceptionObject (__FILE__,__LINE__,"Error in GDCMImporter3::GenerateData()");
    }
  }
  
//----------------------------------------------------------------------------
  template <class TPixelType>
  std::string
  GDCMImporter3<TPixelType>::GenerateUniqueName (FileListMapType map)
  {
    std::string ret;
    std::string description = "output";
    if (map.size())
    {
      if ((*map.begin()).second.size())
      {
	std::string file = (*map.begin()).second[0];
	gdcm::Reader reader;
	reader.SetFileName (file.c_str());
	std::set<gdcm::Tag> set;
	set.insert (gdcm::Tag(0x8, 0x103e));
	
	if (reader.ReadSelectedTags(set))
	{
	  gdcm::File gdcmfile = reader.GetFile();
	  gdcm::StringFilter filter;
	  filter.SetFile (reader.GetFile());
	  description = filter.ToString (gdcm::Tag(0x8, 0x103e));
	}
      }
    }
    
    ret = description;

    itksys::SystemTools::ReplaceString (ret, " ", "-");
    itksys::SystemTools::ReplaceString (ret, "_", "-");
    bool finished = 0;
    while(!finished)
    {
      std::string::size_type dot_pos = ret.rfind("-");
      if(dot_pos == (ret.size()-1))
	ret = ret.substr(0, dot_pos);
      else
	finished = 1;
    }
    
    bool taken = 0;
    unsigned int toadd = 0;
    for (unsigned int i=0; i<this->GetNumberOfOutputs(); i++)
      if (! strcmp ( this->GetOutput (i)->GetName(), ret.c_str() ) )
      {
	taken = 1;
	toadd++;
	break;
      }
    while(taken)
    {
      std::ostringstream replacement;      
      replacement << description << "-";
      if (toadd < 10) replacement << "0";
      replacement << toadd;
      ret = replacement.str();
      
      taken = 0;
      for (unsigned int i=0; i<this->GetNumberOfOutputs(); i++)
	if (! strcmp ( this->GetOutput (i)->GetName(), ret.c_str() ) )
	{
	  taken = 1;
	  toadd++;
	  break;
	}
    }
    return ret;
  }
  
//----------------------------------------------------------------------------
  template <class TPixelType>
  void
  GDCMImporter3<TPixelType>::PrintSelf(std::ostream& os, Indent indent) const
  {
    Superclass::PrintSelf( os, indent );
    os << indent << "Input Directory: " << m_InputDirectory.c_str() << std::endl;
    os << indent << "Number of Volumes: " << this->GetNumberOfOutputs() << std::endl;
  }


} // end of namespace ITK

