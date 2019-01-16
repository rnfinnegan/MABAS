#!/usr/bin/env python
import SimpleITK as sitk
import sys


def ApplyField(inputImageList, inputField, order, imType, outputNameList):
	#Output displacement vector field
	outputTransform = sitk.DisplacementFieldTransform( inputField )
	deformationField = outputTransform.GetDisplacementField()

	resampler = sitk.ResampleImageFilter()
	resampler.SetReferenceImage(inputImageList[0])
	resampler.SetInterpolator(order)
	if imType==0:
		resampler.SetDefaultPixelValue(0)
	elif imType==1:
		resampler.SetDefaultPixelValue(-1024)
	resampler.SetTransform(outputTransform)

	for (inputImage, outputName) in zip(inputImageList, outputNameList):
		registeredImage = resampler.Execute(sitk.Cast(inputImage, sitk.sitkFloat32))
		if imType==0 and order>1:
			print("Higher order interpolation on structure image - saving image as 32-bit float.")
			sitk.WriteImage(registeredImage, outputName)
		else:
			sitk.WriteImage(sitk.Cast(registeredImage, inputImage.GetPixelIDValue()), outputName)

	return 1


if __name__=="__main__":
	if len(sys.argv)<=6:
		print("Apply a deformation field to multiple input files,")
		print("using a field from Demons registration using Elastix.")
		print("Arguments:")
		print("   1: Input field")
		print("   2: Output base name")
		print("      e.g. Case_01_to_Case_02_{0}_DEMONS.nii.gz")
		print("   3: Interpolation order")
		print("   4: Structure (0) or image (1)")
		print("      Sets the background value")
		print("   5: Input base name")
		print("      e.g. Case_01_to_Case_02_{0}_RIGID.nii.gz")
		print("   6: List on modifications to input base name")
		print("      e.g. WHOLEHEART RIGHTATRIUM")
		sys.exit()
	else:
		inputField = sitk.ReadImage(sys.argv[1], sitk.sitkVectorFloat64)
		outputName = sys.argv[2]
		order = int(sys.argv[3])
		imType = int(sys.argv[4])
		baseName = sys.argv[5]
		inputImageList = [sitk.ReadImage(baseName.format(i)) for i in sys.argv[6:]]
		outputNameList = [outputName.format(i) for i in sys.argv[6:]]

		ApplyField(inputImageList, inputField, order, imType, outputNameList)
