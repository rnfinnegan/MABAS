
/*******************************************************************************
Local weighted voting for label combination
Robert Finnegan, 22 May 2018
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

// ITK Operations includes
#include "itkAddImageFilter.h"
#include "itkDivideImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkPowImageFilter.h"
#include "itkSquaredDifferenceImageFilter.h"
#include "itkDiscreteGaussianImageFilter.h"
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


// Compute the weight images from a target and moving image
InternalImageType::Pointer computeWeightImage(InternalImageType::Pointer targetImage,
                                              InternalImageType::Pointer movingImage,
                                              float epsilon,
                                              float neighbourhoodVariance)
  {
  // compute squared difference (automatically casts inputs to double)
  typedef itk::SquaredDifferenceImageFilter <InternalImageType, InternalImageType, InternalImageType> SquaredDifferenceImageFilterType;
  SquaredDifferenceImageFilterType::Pointer squaredDifferenceFilter = SquaredDifferenceImageFilterType::New ();
  squaredDifferenceFilter->SetInput1(targetImage);
  squaredDifferenceFilter->SetInput2(movingImage);
  squaredDifferenceFilter->Update();

  // smooth the difference image to compute the weighting
  typedef itk::DiscreteGaussianImageFilter<InternalImageType, InternalImageType > DiscreteGaussianImageFilterType;
  DiscreteGaussianImageFilterType::Pointer gaussianFilter = DiscreteGaussianImageFilterType::New();
  gaussianFilter->SetInput( squaredDifferenceFilter->GetOutput() );
  gaussianFilter->SetVariance(neighbourhoodVariance);
  gaussianFilter->Update();

  // add constant and invert values
  typedef itk::AddImageFilter<InternalImageType,InternalImageType,InternalImageType> AddImageFilterType;
  AddImageFilterType::Pointer addFilter = AddImageFilterType::New();
  addFilter->SetInput1(gaussianFilter->GetOutput());
  addFilter->SetConstant2(epsilon);
  addFilter->Update();

  typedef itk::PowImageFilter<InternalImageType,InternalImageType,InternalImageType> PowImageFilterType;
  PowImageFilterType::Pointer powFilter = PowImageFilterType::New();
  powFilter->SetInput1(addFilter->GetOutput());
  powFilter->SetConstant2(-1.0);
  powFilter->Update();

  return powFilter->GetOutput();
}

int main( int argc, char * argv [] )
{

  // Verify the number of parameters in the command line
  if( argc != 6 )
    {
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0]    << " fixedImage movingImageGlob movingLabelGlob structureTextFile outputBaseName" << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Example:" << "[1] ./Case_X.nii.gz"  << std::endl;
    std::cerr << "        " << "[2] ./Case_*_to_Case_X_DEMONS.nii.gz"  << std::endl;
    std::cerr << "        " << "[3] ./Structures/Case_*_to_Case_X_%1%_DEMONS.nii.gz"  << std::endl;
    std::cerr << "        " << "    Note: must contain '%1%' where structure name is expected'"  << std::endl;
    std::cerr << "        " << "[4] ./structureList.txt"  << std::endl;
    std::cerr << "        " << "[5] ./Case_X_%1%.nii.gz"  << std::endl;
    std::cerr << "        " << " ----------------------------------------------------------------------------"  << std::endl;
    std::cerr << "Arguments: " << argc << std::endl;

    return EXIT_FAILURE;
    }

  // define variables used
  float epsilon = 0.00001;
  float neighbourhoodVariance = 4.0;
  float smoothVariance = 0.25;

  typedef itk::ImageFileReader< InternalImageType >  ReaderType;
  typedef itk::ImageFileWriter< InternalImageType >  WriterType;
  typedef itk::ImageFileWriter< ImageType >  FinalWriterType;

  typedef itk::AddImageFilter<InternalImageType,InternalImageType,InternalImageType> AddImageFilterType;
  typedef itk::DivideImageFilter<InternalImageType,InternalImageType,InternalImageType> DivideImageFilterType;
  typedef itk::MultiplyImageFilter<InternalImageType,InternalImageType,InternalImageType> MultiplyImageFilterType;
  typedef itk::DiscreteGaussianImageFilter<InternalImageType,InternalImageType> DiscreteGaussianImageFilterType;
  typedef itk::BinaryThresholdImageFilter<InternalImageType, ImageType> BinaryThresholdImageFilterType;
  typedef itk::BinaryFillholeImageFilter<ImageType> BinaryFillholeImageFilterType;
  typedef itk::ConnectedComponentImageFilter<ImageType, ImageType> ConnectedComponentImageFilterType;
  typedef itk::LabelShapeKeepNObjectsImageFilter<ImageType> LabelShapeKeepNObjectsImageFilterType;
  typedef itk::RescaleIntensityImageFilter<ImageType> RescaleIntensityImageFilterType;

  typedef itk::MinimumMaximumImageCalculator<InternalImageType> MinimumMaximumImageCalculatorType;


  // setup input file readers
  ReaderType::Pointer targetReader = ReaderType::New();
  targetReader->SetFileName( argv[1] );
  targetReader->Update();
  InternalImageType::Pointer targetImage = InternalImageType::New();
  targetImage = targetReader->GetOutput();

  // read in moving files and compute corresponding weight image
  std::vector<std::string> movingImageFilenames = glob( argv[2] );
  InternalImageType::Pointer weightImageArray[movingImageFilenames.size()];

  InternalImageType::Pointer weightSumImage = InternalImageType::New();

  std::cout << "Moving image files: " << movingImageFilenames.size() << ". Reading and generating weight maps now." << std::endl;
  for (int i=0; i<movingImageFilenames.size(); i++)
  {
    std::cout << movingImageFilenames[i] << std::endl;
    ReaderType::Pointer movingReader = ReaderType::New();
    movingReader->SetFileName( movingImageFilenames[i] );
    movingReader->Update();

    // calculate weight images
    InternalImageType::Pointer weightImage = InternalImageType::New();
    weightImage = computeWeightImage(targetImage,
                                     movingReader->GetOutput(),
                                     epsilon,
                                     neighbourhoodVariance);
    weightImageArray[i] = weightImage;

    // compute the sum of weights on a per-voxel basis
    if (i==0)
    {
      weightSumImage = weightImage;
    }
    else
    {
      AddImageFilterType::Pointer addFilter = AddImageFilterType::New();
      addFilter->SetInput1(weightSumImage);
      addFilter->SetInput2(weightImage);
      addFilter->Update();
      weightSumImage = addFilter->GetOutput();
    }
  }

  // process structure-by-structure
  std::ifstream infile(argv[4]);
  std::string line;
  std::vector<std::string> structureList;
  while (std::getline(infile, line)) {
    structureList.push_back(line);

  }

  for (int i=0; i<structureList.size(); i++)
  {
    std::string organName = structureList[i];
    std::cout << "Processing structures: " << organName << std::endl;
    // read in the label image files, compute the weighted label
    InternalImageType::Pointer weightedLabelImageArray[movingImageFilenames.size()];
    std::string pattern = (boost::format(argv[3]) % organName).str();
    std::cout << "Searching for label files matching: " << pattern << std::endl;
    std::vector<std::string> movingLabelImageFilenames = glob( pattern );
    std::cout << "Moving label image files found: " << movingLabelImageFilenames.size() << ". Reading and processing now." << std::endl;
    // put in a check here for number of files being equal and also check the first ~10 characters to make sure they are the same?
    for (int i=0; i<movingLabelImageFilenames.size(); i++)
    {
      std::cout << movingLabelImageFilenames[i] << std::endl;
      ReaderType::Pointer movingLabelReader = ReaderType::New();
      movingLabelReader->SetFileName( movingLabelImageFilenames[i] );
      movingLabelReader->Update();

      // create the weighted labels
      InternalImageType::Pointer weightedLabel = InternalImageType::New();
      MultiplyImageFilterType::Pointer multiplyFilter = MultiplyImageFilterType::New();
      multiplyFilter->SetInput1(weightImageArray[i]);
      multiplyFilter->SetInput2(movingLabelReader->GetOutput());
      multiplyFilter->Update();
      weightedLabelImageArray[i] = multiplyFilter->GetOutput();
    }

    std::cout << "Fusing weighted labels." << std::endl;
    // fuse weighted labels and perform voxel-wise normalisation
    InternalImageType::Pointer weightedLabelImageSum = InternalImageType::New();
    weightedLabelImageSum = weightedLabelImageArray[0];
    for (int i=1; i<movingLabelImageFilenames.size(); i++)
    {
      AddImageFilterType::Pointer addFilter = AddImageFilterType::New();
      addFilter->SetInput1(weightedLabelImageSum);
      addFilter->SetInput2(weightedLabelImageArray[i]);
      addFilter->Update();
      weightedLabelImageSum = addFilter->GetOutput();
    }

    std::cout << "Normalising (voxel-wise) fused label." << std::endl;
    DivideImageFilterType::Pointer divideFilter = DivideImageFilterType::New();
    divideFilter->SetInput1(weightedLabelImageSum);
    divideFilter->SetInput2(weightSumImage);
    divideFilter->Update();

    std::cout << "Smoothing normalised fused label." << std::endl;
    DiscreteGaussianImageFilterType::Pointer gaussianFilter = DiscreteGaussianImageFilterType::New();
    gaussianFilter->SetInput( divideFilter->GetOutput() );
    gaussianFilter->SetVariance(smoothVariance);
    gaussianFilter->Update();

    std::cout << "Re-Normalising to unit scale." << std::endl;
    MinimumMaximumImageCalculatorType::Pointer minMaxCalculator = MinimumMaximumImageCalculatorType::New();
    minMaxCalculator->SetImage(gaussianFilter->GetOutput());
    minMaxCalculator->Compute();

    DivideImageFilterType::Pointer divideNormFilter = DivideImageFilterType::New();
    divideNormFilter->SetInput1(gaussianFilter->GetOutput());
    divideNormFilter->SetConstant2(minMaxCalculator->GetMaximum());
    divideNormFilter->Update();


    std::cout << "Saving probabilistic image." << std::endl;
    InternalImageType::Pointer probabilisticLabel = InternalImageType::New();
    probabilisticLabel = divideNormFilter->GetOutput();


    std::string outputName = (boost::format(argv[5]) % (organName+"_probability")).str();
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( outputName );
    writer->SetInput( probabilisticLabel );
    writer->Update();

    BinaryThresholdImageFilterType::Pointer thresholdFilter = BinaryThresholdImageFilterType::New();
    thresholdFilter->SetInput(probabilisticLabel);
    thresholdFilter->SetLowerThreshold(0.5);
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


    outputName = (boost::format(argv[5]) % (organName+"_processed")).str();
    FinalWriterType::Pointer finalWriter = FinalWriterType::New();
    finalWriter->SetFileName( outputName );
    finalWriter->SetInput( rescaleIntensityFilter->GetOutput() );
    finalWriter->Update();

  }


  return EXIT_SUCCESS;
}
