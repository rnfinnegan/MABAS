#!/usr/bin/env python
import SimpleITK as sitk
import sys

def RigidProp(fixedImage, movingImage, transformFile, imFlag, interp, outputName):

    rigidTransformix = sitk.TransformixImageFilter()
    rigidTransformParameterMap = sitk.ReadParameterFile(transformFile)
    if imFlag==0:
        rigidTransformParameterMap['DefaultPixelValue'] = ['0']
    rigidTransformParameterMap["FinalBSplineInterpolationOrder"]= [interp]
    rigidTransformix.SetTransformParameterMap(rigidTransformParameterMap)

    rigidTransformix.SetMovingImage(sitk.Cast(movingImage, sitk.sitkFloat32))
    rigidTransformix.SetLogToFile(False)
    rigidTransformix.Execute()

    outputImage = rigidTransformix.GetResultImage()
    outputImage.CopyInformation(fixedImage)
    if imFlag==0 and interp<=1:
        sitk.WriteImage(sitk.Cast(outputImage,sitk.sitkUInt8), outputName)
    elif imFlag==0 and interp>1:
        print("Higher order interpolation on structure file - using float32 as output.")
        outputIm = sitk.Cast(outputImage,sitk.sitkFloat32)
        outputIm = sitk.Threshold(outputIm, lower=1e-5, upper=100.0)
        sitk.WriteImage(outputIm, outputName)
    else:
        sitk.WriteImage(sitk.Cast(outputImage,movingImage.GetPixelID()), outputName)

    return 1


if __name__=="__main__":
    if len(sys.argv)!=7:
        print("Propagation of a rigid transformation using Elastix.")
        print("Arguments:")
        print("   1: Fixed image")
        print("   2: Moving image")
        print("   3: Transform file")
        print("   4: Structure (0) or Image (1)")
        print("   5: Interpolation order (1=NN, 2=Linear, 3=BSpline)")
        print("   6: Output image")
        sys.exit()
    else:
        fixed = sitk.ReadImage(sys.argv[1])
        moving = sitk.ReadImage(sys.argv[2])
        tfm = sys.argv[3]
        imFlag = int(sys.argv[4])
        interp = sys.argv[5]
        out = sys.argv[6]
        RigidProp(fixed, moving, tfm, imFlag, interp, out)
