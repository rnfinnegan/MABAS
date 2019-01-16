#!/usr/bin/env python
import SimpleITK as sitk
import sys

def RigidReg(fixedImage, movingImage, outputName, parName, sgFlag):

	rigidElastix = sitk.ElastixImageFilter()
	print fixedImage.GetSize()
	print movingImage.GetSize()

	if sgFlag==1:
		print("Normalising images")
		fMax = sitk.GetArrayFromImage(fixedImage).max()
		mMax = sitk.GetArrayFromImage(movingImage).max()
		fixedImage = sitk.Cast(fixedImage, sitk.sitkFloat32)/fMax
		movingImage = sitk.Cast(movingImage, sitk.sitkFloat32)/mMax

	rigidElastix.SetFixedImage(fixedImage)
	rigidElastix.SetMovingImage(movingImage)
	rigidElastix.LogToConsoleOn()

	MABASDir = '/home/robbie/Documents/University/PhD/Research/Software/MABAS/ParameterFiles/'

	try:
		rigidParameterMap = sitk.ReadParameterFile(parName)
	except:
		print('No transform parameter found in current directory, searching library...')
		try:
			parName = MABASDir+parName
			rigidParameterMap = sitk.ReadParameterFile(parName)
		except:
			print('No transform parameter in library, using default.')
			parName = MABASDir+'RigidTransformParameters.txt'
			rigidParameterMap = sitk.ReadParameterFile(parName)

	rigidElastix.SetParameterMap(rigidParameterMap)

	if sgFlag==1:
		rigidElastix.SetParameter(0, 'FinalBSplineInterpolationOrder','1')
		rigidElastix.SetParameter(0, 'DefaultPixelValue','0')

	rigidElastix.SetLogToFile(False)

	rigidElastix.Execute()
	registeredImage = rigidElastix.GetResultImage()
	tfm = rigidElastix.GetTransformParameterMap()

	outputImage = '{0}.nii.gz'.format(outputName)
	outputTfm = '{0}.txt'.format(outputName)

	if sgFlag==1:
		registeredImage = sitk.Cast(registeredImage, sitk.sitkFloat32)
		registeredImage = sitk.Threshold(registeredImage, lower=1e-5, upper=100)

	registeredImage.CopyInformation(fixedImage)

	print 'Saving image to disk.'
	sitk.WriteImage(sitk.Cast(registeredImage, movingImage.GetPixelID()), outputImage)

	print 'Saving transform file to disk.'
	sitk.WriteParameterFile(tfm[0], outputTfm)

	return 1


if __name__=="__main__":
	if len(sys.argv)<5 or len(sys.argv)>6:
		print("Rigid registration using Elastix.")
		print("Arguments:")
		print("   1: Fixed image")
		print("   2: Moving image")
		print("   3: Output name")
		print("   4: Parameter file name")
		print("   5: (Optional) structure guided flag {0,1}")
		sys.exit()
	else:
		fixed = sitk.ReadImage(sys.argv[1])
		moving = sitk.ReadImage(sys.argv[2])
		out = sys.argv[3]
		par = sys.argv[4]
		try:
			sgFlag = int(sys.argv[5])
		except:
			sgFlag = 0
		RigidReg(fixed, moving, out, par, sgFlag)
