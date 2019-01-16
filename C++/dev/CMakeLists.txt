PROJECT(DemonsNRR)

cmake_minimum_required(VERSION 2.8)
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_executable(itkLocalWeightedVote itkLocalWeightedVote.cxx)
TARGET_LINK_LIBRARIES(itkLocalWeightedVote ${ITK_LIBRARIES} )

add_executable(itkThresholdProbabilityImage itkThresholdProbabilityImage.cxx)
TARGET_LINK_LIBRARIES(itkThresholdProbabilityImage ${ITK_LIBRARIES} )

add_executable(itkApplyDeformationField itkApplyDeformationField.cxx)
TARGET_LINK_LIBRARIES(itkApplyDeformationField ${ITK_LIBRARIES} )

add_executable(itkApplyDeformationFieldMultiple itkApplyDeformationFieldMultiple.cxx)
TARGET_LINK_LIBRARIES(itkApplyDeformationFieldMultiple ${ITK_LIBRARIES} )

add_executable(itkMultiResFSDRegistration itkMultiResFSDRegistration.cxx)
TARGET_LINK_LIBRARIES(itkMultiResFSDRegistration ${ITK_LIBRARIES} )
