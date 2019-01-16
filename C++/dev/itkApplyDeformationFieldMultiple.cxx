
/*******************************************************************************
Application of a dense vector field on an image
Robert Finnegan, 23 May 2018
*******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <glob.h>
#include <string.h>
#include <vector>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>
#include <boost/format.hpp>

// ITK IO includes
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// ITK filter includes
#include "itkWarpImageFilter.h"
#include "itkCastImageFilter.h"
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>


// function to search directory for registered images
std::vector<std::string> glob(const std::string& pattern) {
    using namespace std;

    // glob struct resides on the stack
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_TILDE, NULL, &glob_result);
    if(return_value != 0) {
        globfree(&glob_result);
        stringstream ss;
        ss << "glob() failed with return_value " << return_value << endl;
        throw std::runtime_error(ss.str());
    }

    // collect all the filenames into a std::list<std::string>
    vector<string> filenames;
    for(size_t i = 0; i < glob_result.gl_pathc; ++i) {
        filenames.push_back(string(glob_result.gl_pathv[i]));
    }

    // cleanup
    globfree(&glob_result);

    // done
    return filenames;
}


int main( int argc, char * argv [] )
{

  // Verify the number of parameters in the command line
  if( argc != 6 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImageGlob deformationField outputBaseName structureTextFile interpolationOrder" << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Example:" << "[1] ./Structures/Case_Y_to_Case_X_%1%_RIGID.nii.gz"  << std::endl;
    std::cerr << "        " << "     Note: must contain '%1%' where structure name is expected'"  << std::endl;
    std::cerr << "        " << "[2] ./Case_Y_to_Case_X_DEMONS_FIELD.nii.gz"  << std::endl;
    std::cerr << "        " << "[3] ./Structures/Case_Y_to_Case_X_%1%_DEMONS.nii.gz"  << std::endl;
    std::cerr << "        " << "     Note: must contain '%1%' where structure name is expected'"  << std::endl;
    std::cerr << "        " << "[4] structureFile.txt"  << std::endl;
    std::cerr << "        " << "[5] 1"  << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Arguments: " << argc << std::endl;

    return EXIT_FAILURE;
    }

  int interpOrder   = atoi(argv[5]);

  // define ITK short-hand types
  const unsigned int Dimension = 3;
  typedef int                                                  PixelType;
  typedef float                                                InternalPixelType;
  typedef itk::Image< PixelType, Dimension >                   ImageType;
  typedef itk::Image< InternalPixelType, Dimension >           InternalImageType;
  typedef itk::Vector< float, 3 >                              VectorPixelType;
  typedef itk::Image< VectorPixelType, 3 >                     DisplacementFieldType;

  typedef itk::CastImageFilter< ImageType, InternalImageType > InputImageCasterType;
  typedef itk::CastImageFilter< InternalImageType, ImageType > OutputImageCasterType;

  typedef itk::ImageFileReader<DisplacementFieldType>          DisplacementFieldImageReaderType;
  typedef itk::ImageFileReader< ImageType >                    ReaderType;

  typedef itk::BSplineInterpolateImageFunction<InternalImageType>                 InterpolatorTypeBSP;
  typedef itk::LinearInterpolateImageFunction<InternalImageType, double>          InterpolatorTypeTRI;
  typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double> InterpolatorTypeNN;

  typedef itk::WarpImageFilter< InternalImageType, InternalImageType, DisplacementFieldType > WarperType;

  // read in displacement field image
  DisplacementFieldImageReaderType::Pointer displacementFieldImageReader = DisplacementFieldImageReaderType::New();
  displacementFieldImageReader->SetFileName( argv[2] );
  displacementFieldImageReader->Update();

  DisplacementFieldType::Pointer displacementFieldImage = DisplacementFieldType::New();
  displacementFieldImage = displacementFieldImageReader->GetOutput();

  // set up the image warper
  WarperType::Pointer warper = WarperType::New();
  warper->SetEdgePaddingValue(0);
  warper->SetDisplacementField( displacementFieldImage );

  // set up the interpolator of choice
  if(interpOrder == 2)
  {
    InterpolatorTypeBSP::Pointer interpolator = InterpolatorTypeBSP::New();
    warper->SetInterpolator( interpolator );
    std::cout <<" BSpline Interpolation" << std::endl;
  }
  else if (interpOrder == 1)
  {
    InterpolatorTypeTRI::Pointer interpolator = InterpolatorTypeTRI::New();
    warper->SetInterpolator( interpolator );
    std::cout <<" Linear Interpolation" << std::endl;
  }
  else if (interpOrder == 0)
  {
    InterpolatorTypeNN::Pointer interpolator = InterpolatorTypeNN::New();
    warper->SetInterpolator( interpolator );
    std::cout <<" Nearest Neighbour Interpolation" << std::endl;
  }

  // process structure-by-structure
  std::ifstream infile(argv[4]);
  std::string line;
  std::vector<std::string> structureList;
  while (std::getline(infile, line)) {
    structureList.push_back(line);
  }

  std::string inputName;
  std::string outputName;
  std::string organName;

  for (int i=0; i<structureList.size(); i++)
  {
    organName = structureList[i];
    std::cout << "Processing structures: " << organName << std::endl;
    inputName = (boost::format(argv[1]) % organName).str();

    // read in input image
    ReaderType::Pointer inputImageReader = ReaderType::New();
    inputImageReader->SetFileName( inputName );
    inputImageReader->Update();

    // cast input image to float
    InputImageCasterType::Pointer inputImageCaster = InputImageCasterType::New();
    inputImageCaster->SetInput( inputImageReader->GetOutput() );
    inputImageCaster->Update();

    InternalImageType::Pointer inputImage = InternalImageType::New();
    inputImage = inputImageCaster->GetOutput();

    warper->SetInput( inputImage );
    warper->SetOutputSpacing( inputImage->GetSpacing() );
    warper->SetOutputOrigin( inputImage->GetOrigin() );
    warper->SetOutputDirection( inputImage->GetDirection() );

    warper->Update();

    outputName = (boost::format(argv[3]) % organName).str();

    // re-cast to integer
    if (interpOrder>0)
    {
      std::cout <<"Higher order interpolation on a structure image: saving values using floating point numbers." << std::endl;
      typedef itk::ImageFileWriter< InternalImageType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( outputName );
      writer->SetInput( warper->GetOutput() );
      writer->Update();
    }

    else if (interpOrder==0)
    {
      OutputImageCasterType::Pointer outputImageCaster = OutputImageCasterType::New();
      outputImageCaster->SetInput( warper->GetOutput() );
      outputImageCaster->Update();

      typedef itk::ImageFileWriter< ImageType > WriterType;
      WriterType::Pointer writer = WriterType::New();
      writer->SetFileName( outputName );
      writer->SetInput( outputImageCaster->GetOutput() );
      writer->Update();
    }
  }


  return EXIT_SUCCESS;
}
