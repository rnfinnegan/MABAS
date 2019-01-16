
/*******************************************************************************
Local weighted voting for label combination
Robert Finnegan, 22 May 2018
*******************************************************************************/

#include <iostream>
#include <cstdlib>
#include <stdexcept>
#include <string>
#include <sstream>
#include <fstream>

// ITK IO includes
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

// ITK Operations includes
#include "itkBinaryThresholdImageFilter.h"
#include "itkBinaryFillholeImageFilter.h"
#include "itkConnectedComponentImageFilter.h"
#include "itkLabelShapeKeepNObjectsImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"

#include "itkMinimumMaximumImageCalculator.h"

// define ITK short-hand types
const unsigned int Dimension = 3;
typedef int                                                  PixelType;
typedef float                                                InternalPixelType;
typedef itk::Image< PixelType, Dimension >                   ImageType;
typedef itk::Image< InternalPixelType, Dimension >           InternalImageType;


int main( int argc, char * argv [] )
{

  // Verify the number of parameters in the command line
  if( argc != 4 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0]    << " inputImage outputImage threshold`" << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Example:" << "[1] ./Case_X_WHOLEHEART_PROBABILITY.nii.gz"  << std::endl;
    std::cerr << "        " << "[2] ./Case_X_WHOLEHEART_BINARY.nii.gz"  << std::endl;
    std::cerr << "        " << "[3] 0.5"  << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Arguments: " << argc << std::endl;

    return EXIT_FAILURE;
    }

  // define variables used
  float threshold = atof(argv[3]);

  typedef itk::ImageFileReader< InternalImageType >  ReaderType;
  typedef itk::ImageFileWriter< InternalImageType >  WriterType;
  typedef itk::ImageFileWriter< ImageType >  FinalWriterType;

  typedef itk::BinaryThresholdImageFilter<InternalImageType, ImageType> BinaryThresholdImageFilterType;
  typedef itk::BinaryFillholeImageFilter<ImageType> BinaryFillholeImageFilterType;
  typedef itk::ConnectedComponentImageFilter<ImageType, ImageType> ConnectedComponentImageFilterType;
  typedef itk::LabelShapeKeepNObjectsImageFilter<ImageType> LabelShapeKeepNObjectsImageFilterType;
  typedef itk::RescaleIntensityImageFilter<ImageType> RescaleIntensityImageFilterType;


  // setup input file readers
  ReaderType::Pointer targetReader = ReaderType::New();
  targetReader->SetFileName( argv[1] );
  targetReader->Update();
  InternalImageType::Pointer targetImage = InternalImageType::New();
  targetImage = targetReader->GetOutput();


  BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
  thresholdFilter->SetInput(targetImage);
  thresholdFilter->SetLowerThreshold(threshold);
  thresholdFilter->SetUpperThreshold(1.0);
  thresholdFilter->SetInsideValue(1);
  thresholdFilter->SetOutsideValue(0);
  thresholdFilter->Update();

  BinaryFillholeImageFilterType::Pointer holefillingFilter = BinaryFillholeImageFilterType::New();
  holefillingFilter->SetInput(thresholdFilter->GetOutput());
  holefillingFilter->SetForegroundValue(1);
  holefillingFilter->Update();

  ConnectedComponentImageFilterType::Pointer connectedComponentFilter = ConnectedComponentImageFilterType::New();
  connectedComponentFilter->SetInput(holefillingFilter->GetOutput());
  connectedComponentFilter->Update();

  LabelShapeKeepNObjectsImageFilterType::Pointer labelShapeKeepNObjectFilter = LabelShapeKeepNObjectsImageFilterType::New();
  labelShapeKeepNObjectFilter->SetInput(connectedComponentFilter->GetOutput());
  labelShapeKeepNObjectFilter->SetBackgroundValue(0);
  labelShapeKeepNObjectFilter->SetNumberOfObjects(1);
  labelShapeKeepNObjectFilter->SetAttribute(LabelShapeKeepNObjectsImageFilterType::LabelObjectType::NUMBER_OF_PIXELS);


  RescaleIntensityImageFilterType::Pointer rescaleIntensityFilter = RescaleIntensityImageFilterType::New();
  rescaleIntensityFilter->SetOutputMinimum(0);
  rescaleIntensityFilter->SetOutputMaximum(1);
  rescaleIntensityFilter->SetInput(labelShapeKeepNObjectFilter->GetOutput());
  rescaleIntensityFilter->Update();

  FinalWriterType::Pointer finalWriter = FinalWriterType::New();
  finalWriter->SetFileName( argv[2] );
  finalWriter->SetInput( rescaleIntensityFilter->GetOutput() );
  finalWriter->Update();

  return EXIT_SUCCESS;
}
