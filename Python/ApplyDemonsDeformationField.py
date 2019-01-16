#!/usr/bin/env python
import SimpleITK as sitk
import sys


def ApplyField(inputImage, inputField, order, imType, outputName):
	#Output displacement vector field
	outputTransform = sitk.DisplacementFieldTransform( inputField )
	deformationField = outputTransform.GetDisplacementField()

	resampler = sitk.ResampleImageFilter()
	resampler.SetReferenceImage(inputImage)
	resampler.SetInterpolator(order)
	if imType==0:
		resampler.SetDefaultPixelValue(0)
	elif imType==1:
		resampler.SetDefaultPixelValue(-1024)
	resampler.SetTransform(outputTransform)

	registeredImage = resampler.Execute(sitk.Cast(inputImage, sitk.sitkFloat32))
	sitk.WriteImage(sitk.Cast(registeredImage, inputImage.GetPixelIDValue()), outputName)


	return 1


if __name__=="__main__":
	if len(sys.argv)!=6:
		print("Apply a deformation field from Demons registration using Elastix.")
		print("Arguments:")
		print("   1: Input field")
		print("   2: Output name")
		print("   3: Interpolation order")
		print("   4: Structure (0) or image (1)")
		print("      Sets the background value")
		print("   5: Input file")
		sys.exit()
	else:
		inputField = sitk.ReadImage(sys.argv[1], sitk.sitkVectorFloat64)
		outputName = sys.argv[2]
		order = int(sys.argv[3])
		imType = int(sys.argv[4])
		inputImage = sitk.ReadImage(sys.argv[5])

		ApplyField(inputImage, inputField, order, imType, outputName)
