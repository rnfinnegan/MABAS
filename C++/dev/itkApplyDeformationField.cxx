
/*******************************************************************************
Application of a dense vector field on an image
Robert Finnegan, 23 May 2018
*******************************************************************************/

#include <iostream>
#include <cstdlib>

// ITK IO includes
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// ITK filter includes
#include "itkWarpImageFilter.h"
#include "itkCastImageFilter.h"
#include <itkBSplineInterpolateImageFunction.h>
#include <itkLinearInterpolateImageFunction.h>
#include <itkNearestNeighborInterpolateImageFunction.h>

int main( int argc, char * argv [] )
{

  // Verify the number of parameters in the command line
  if( argc != 6 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " inputImage deformationField outputImage structureFlag interpolationOrder" << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Example:" << "[1] ./Structures/Case_Y_to_Case_X_WHOLEHEART_RIGID.nii.gz"  << std::endl;
    std::cerr << "        " << "[2] ./Case_Y_to_Case_X_DEMONS_FIELD.nii.gz"  << std::endl;
    std::cerr << "        " << "[3] ./Structures/Case_Y_to_Case_X_WHOLEHEART_DEMONS.nii.gz"  << std::endl;
    std::cerr << "        " << "[4] 0"  << std::endl;
    std::cerr << "        " << "[5] 2"  << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Arguments: " << argc << std::endl;

    return EXIT_FAILURE;
    }

  int structureFlag = atoi(argv[4]);
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


  // read in input image
  ReaderType::Pointer inputImageReader = ReaderType::New();
  inputImageReader->SetFileName( argv[1] );
  inputImageReader->Update();

  // cast input image to float
  InputImageCasterType::Pointer inputImageCaster = InputImageCasterType::New();
  inputImageCaster->SetInput( inputImageReader->GetOutput() );
  inputImageCaster->Update();

  InternalImageType::Pointer inputImage = InternalImageType::New();
  inputImage = inputImageCaster->GetOutput();

  // read in displacement field image
  DisplacementFieldImageReaderType::Pointer displacementFieldImageReader = DisplacementFieldImageReaderType::New();
  displacementFieldImageReader->SetFileName( argv[2] );
  displacementFieldImageReader->Update();

  DisplacementFieldType::Pointer displacementFieldImage = DisplacementFieldType::New();
  displacementFieldImage = displacementFieldImageReader->GetOutput();

  // set up the image warper
  WarperType::Pointer warper = WarperType::New();
  warper->SetInput( inputImage );
  warper->SetOutputSpacing( inputImage->GetSpacing() );
  warper->SetOutputOrigin( inputImage->GetOrigin() );
  warper->SetOutputDirection( inputImage->GetDirection() );
  if (structureFlag==1) {
    warper->SetEdgePaddingValue(0);
  }
  else if (structureFlag==0) {
    warper->SetEdgePaddingValue(-1024);
  }
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

  warper->Update();

  // re-cast to integer
  if (structureFlag==1 and interpOrder>0)
  {
    std::cout <<"Higher order interpolation on a structure image: saving values using floating point numbers." << std::endl;
    typedef itk::ImageFileWriter< InternalImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( argv[3] );
    writer->SetInput( warper->GetOutput() );
    writer->Update();
  }

  else if (structureFlag==0 or interpOrder==0)
  {
    OutputImageCasterType::Pointer outputImageCaster = OutputImageCasterType::New();
    outputImageCaster->SetInput( warper->GetOutput() );
    outputImageCaster->Update();

    typedef itk::ImageFileWriter< ImageType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( argv[3] );
    writer->SetInput( outputImageCaster->GetOutput() );
    writer->Update();
  }


  return EXIT_SUCCESS;
}
