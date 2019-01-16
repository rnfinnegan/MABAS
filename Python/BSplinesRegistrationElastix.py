#!/usr/bin/env python
import SimpleITK as sitk
import sys


def BSplinesNonRigidReg(fixedImage, movingImage, outputName, parFile):

	fixedImage = sitk.Cast(fixedImage, sitk.sitkFloat32)
	movingImage = sitk.Cast(movingImage, sitk.sitkFloat32)

	SimpleElastix = sitk.ElastixImageFilter()
	SimpleElastix.LogToConsoleOn()
	SimpleElastix.SetFixedImage(fixedImage)
	SimpleElastix.SetMovingImage(movingImage)
	parMap = sitk.ReadParameterFile(parFile)
	SimpleElastix.SetParameterMap(parMap)
	registeredImage = SimpleElastix.Execute()

	output = "{0}.nii.gz".format(outputName)
	outputTfm = "{0}.txt".format(outputName)

	sitk.WriteImage(sitk.Cast(registeredImage, sitk.sitkInt32), output)
	sitk.WriteParameterFile(SimpleElastix.GetTransformParameterMap()[0],outputTfm)

	return 1


if __name__=="__main__":
    if len(sys.argv)!=5:
        print("B-Splines registration using Elastix.")
        print("Arguments:")
        print("   1: Fixed image")
        print("   2: Moving image")
        print("   3: Parameter file name")
        print("   4: Output name")
        sys.exit()
    else:
		fixed = sitk.ReadImage(sys.argv[1])
		moving = sitk.ReadImage(sys.argv[2])
		parfile = sys.argv[3]
		out = sys.argv[4]
		BSplinesNonRigidReg(fixed, moving, out, parfile)
