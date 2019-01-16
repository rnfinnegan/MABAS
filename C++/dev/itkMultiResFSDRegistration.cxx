
/*******************************************************************************
Based on:

  Abstract:  -  Multiresolution  symmetric forces demons registration - 4 multiresolution levels
  Created: July 8 2008
  Last Revision 7/9/2008
  by Vidya Rajagopalan  on 7/9/2008

  Copyright (c) 2008, Bioimaging Systems Lab, Virginia Tech
  All rights reserved.

*******************************************************************************/

#include <iostream>
#include <cstdlib>

// ITK IO includes
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// ITK Registration includes
#include "itkMultiResolutionPDEDeformableRegistration.h"
#include "itkMultiResolutionImageRegistrationMethod.h"
#include "itkFastSymmetricForcesDemonsRegistrationFilter.h"
#include "itkHistogramMatchingImageFilter.h"
#include "itkCastImageFilter.h"
#include "itkWarpImageFilter.h"


unsigned int RmsCounter = 0;
unsigned int counter = 0;
float metricValue;
double MaxRmsE[4] = {0,0,0,0};
unsigned int nIterations[4] = { 100, 80, 65, 50};

class CommandIterationUpdate : public itk::Command
{
public:
  typedef  CommandIterationUpdate   Self;
  typedef  itk::Command             Superclass;
  typedef  itk::SmartPointer<Self>  Pointer;
  itkNewMacro( Self );

protected:
  CommandIterationUpdate() {};

  // define ITK short-hand types
  typedef short                                  PixelType;
  typedef float                                  InternalPixelType;
  typedef itk::Image< PixelType, 3 >             ImageType;
  typedef itk::Image< InternalPixelType, 3 >     InternalImageType;
    typedef itk::Vector< float, 3 >              VectorPixelType;
  typedef itk::Image< VectorPixelType, 3 >       DisplacementFieldType;
  typedef itk::FastSymmetricForcesDemonsRegistrationFilter< InternalImageType,
    InternalImageType, DisplacementFieldType>    RegistrationFilterType;

  public:

    void Execute(const itk::Object *, const itk::EventObject & ) ITK_OVERRIDE
      {
      std::cout << "Warning: The const Execute method shouldn't be called" << std::endl;
      }

    void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE
      {
      RegistrationFilterType * filter = static_cast<  RegistrationFilterType * >( caller );
      filter->SetMaximumRMSError(MaxRmsE[RmsCounter]);
      counter++;
      metricValue = filter->GetMetric();
      std::cout << "Iteration: " << counter << "  " << metricValue <<  "  RMS Change: " << filter->GetRMSChange() << std::endl;

  }
};


//
// The following command observer reports the progress of the registration
// inside a given resolution level.
//
class CommandResolutionLevelUpdate : public itk::Command
{
public:
  typedef  CommandResolutionLevelUpdate   Self;
  typedef  itk::Command                   Superclass;
  typedef  itk::SmartPointer<Self>        Pointer;
  itkNewMacro( Self );

protected:
  CommandResolutionLevelUpdate() {};
  typedef float                                  InternalPixelType;
  typedef itk::Image< InternalPixelType, 3 >     InternalImageType;

public:
  void Execute(itk::Object *caller, const itk::EventObject & event) ITK_OVERRIDE
    {
    Execute( (const itk::Object *)caller, event);
    }
  void Execute(const itk::Object *, const itk::EventObject & ) ITK_OVERRIDE
    {
    std::cout << "----------------------------------" << std::endl;
    std::cout << "Level Tolerance=  "<<MaxRmsE[RmsCounter]<<std::endl;
    RmsCounter = RmsCounter + 1;

    // Reset iteration counter
    counter = 0;

    // Reset metric measurement
    std::cout << "----------------------------------" << std::endl;

    }
};


int main( int argc, char * argv [] )
{

  // Verify the number of parameters in the command line
  if( argc != 7 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " fixedImage movingImage registeredImage deformationField updateSigma structureFlag" << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Example:" << "[1] ./Case_X.nii.gz"  << std::endl;
    std::cerr << "        " << "[2] ./Case_Y_to_Case_X_RIGID.nii.gz"  << std::endl;
    std::cerr << "        " << "[3] ./Case_Y_to_Case_X_DEMONS.nii.gz"  << std::endl;
    std::cerr << "        " << "[4] ./Case_Y_to_Case_X_DEMONS_field.nii.gz"  << std::endl;
    std::cerr << "        " << "[5] 1.5"  << std::endl;
    std::cerr << "        " << "[6] 0"  << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Arguments: " << argc << std::endl;

    return EXIT_FAILURE;
    }

  // define variables used
  float updateSigma = atof(argv[5]);
  std::cout << "Using smoothing on update field with sigma: " << updateSigma << std::endl;

  int structureFlag = atoi(argv[6]);

  // define ITK short-hand types
  const unsigned int Dimension = 3;
  typedef int                                                  PixelType;
  typedef float                                                InternalPixelType;
  typedef itk::Image< PixelType, Dimension >                   ImageType;
  typedef itk::Image< InternalPixelType, Dimension >           InternalImageType;
  typedef itk::CastImageFilter< ImageType, InternalImageType > ImageCasterType;

  // setup input file readers
  typedef itk::ImageFileReader< ImageType >  ReaderType;
  ReaderType::Pointer targetReader = ReaderType::New();
  targetReader->SetFileName( argv[1] );
  targetReader->Update();

  ReaderType::Pointer sourceReader = ReaderType::New();
  sourceReader->SetFileName( argv[2] );
  sourceReader->Update();

  // cast target and source to float
  ImageCasterType::Pointer targetImageCaster = ImageCasterType::New();
  ImageCasterType::Pointer sourceImageCaster = ImageCasterType::New();
  targetImageCaster->SetInput( targetReader->GetOutput() );
  sourceImageCaster->SetInput( sourceReader->GetOutput() );

  // setup the deformation field and filter
  typedef itk::Vector< float, Dimension > VectorPixelType;

  typedef itk::Image< VectorPixelType, Dimension > DisplacementFieldType;

  typedef itk::FastSymmetricForcesDemonsRegistrationFilter<
    InternalImageType,
    InternalImageType,
    DisplacementFieldType>         RegistrationFilterType;

  RegistrationFilterType::Pointer filter = RegistrationFilterType::New();

  // if (structureFlag==1) {
  //    double MaxRmsE[4] = {0.0, 0.0, 0.0, 0.0};
  // }
  //
  // if (structureFlag==0) {
  //     double MaxRmsE[4] =  {1.0, 0.75, 0.5, 0.25};
  // }


  if (updateSigma >0) {
    filter->SetSmoothUpdateField(true);
    filter->SetSmoothDisplacementField(true);
    filter->SetStandardDeviations( updateSigma );
  }
  else {
      filter->SetSmoothUpdateField(false);
  }


  //filter->SetIntensityDifferenceThreshold(20.0);

  // Create the Command observer and register it with the registration filter.
  CommandIterationUpdate::Pointer observer = CommandIterationUpdate::New();
  filter->AddObserver( itk::IterationEvent(), observer );

  // use multiresolution scheme
  typedef itk::MultiResolutionPDEDeformableRegistration<
    InternalImageType,
    InternalImageType,
    DisplacementFieldType >       MultiResRegistrationFilterType;

  MultiResRegistrationFilterType::Pointer multires =
    MultiResRegistrationFilterType::New();

  multires->SetRegistrationFilter( filter );
  multires->SetNumberOfLevels( 4 );
  multires->SetFixedImage( targetImageCaster->GetOutput() );
  multires->SetMovingImage( sourceImageCaster->GetOutput() );
  multires->SetNumberOfIterations( nIterations );

  // Create the Command observer and register it with the registration filter.
  CommandResolutionLevelUpdate::Pointer levelobserver = CommandResolutionLevelUpdate::New();
  multires->AddObserver( itk::IterationEvent(), levelobserver );

  // apply the registration filter
  multires->Update();

  // compute the output (warped) image
  typedef itk::WarpImageFilter< ImageType, ImageType, DisplacementFieldType > WarperType;
  typedef itk::LinearInterpolateImageFunction< ImageType, double > InterpolatorType;

  WarperType::Pointer warper = WarperType::New();

  InterpolatorType::Pointer interpolator = InterpolatorType::New();

  ImageType::Pointer targetImage = targetReader->GetOutput();
  warper->SetInput( sourceReader->GetOutput() );
  warper->SetInterpolator( interpolator );
  warper->SetOutputSpacing( targetImage->GetSpacing() );
  warper->SetOutputOrigin( targetImage->GetOrigin() );
  warper->SetOutputDirection( targetImage->GetDirection() );
  if (structureFlag==1) {
    warper->SetEdgePaddingValue(0);
  }
  if (structureFlag==0) {
    warper->SetEdgePaddingValue(-1024);
  }
  warper->SetDisplacementField( multires->GetOutput() );

  typedef itk::ImageFileWriter< ImageType >  WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( argv[3] );
  writer->SetInput( warper->GetOutput() );

  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }


  // write the deformation field
  typedef itk::ImageFileWriter< DisplacementFieldType >  DeformationWriterType;
  DeformationWriterType::Pointer defwriter = DeformationWriterType::New();
  defwriter->SetFileName( argv[4] );
  defwriter->SetInput( multires->GetOutput() );

  try
    {
    defwriter->Update();
    }
  catch( itk::ExceptionObject & excp )
    {
    std::cerr << excp << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
