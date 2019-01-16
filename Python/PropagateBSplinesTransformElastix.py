#!/usr/bin/env python
import SimpleITK as sitk
import sys

def PropagateRegistrationToLabelUsingFile(fixedImage, movingImage, transformFilename, binaryFlag, interp, saveDefField, outputName):

    stx = sitk.TransformixImageFilter()
    parMap=sitk.ReadParameterFile(transformFilename)
    if binaryFlag==1:
        parMap['DefaultPixelValue'] = ['0']

    parMap['ResampleInterpolator']=['FinalBSplineInterpolator']
    parMap['FinalBSplineInterpolationOrder']=[interp]

    stx.AddTransformParameterMap(parMap)


    if saveDefField:
        stx.ComputeDeformationFieldOn()

    stx.SetMovingImage(movingImage)
    stx.Execute()

    outputImage = stx.GetResultImage()
    outputImage.CopyInformation(fixedImage)

    if binaryFlag:
        sitk.WriteImage(sitk.Cast(outputImage,sitk.sitkUInt8), outputName)
    else:
        sitk.WriteImage(outputImage, outputName)

    return True

if __name__=="__main__":
    if len(sys.argv)!=8:
        print("B-Splines registration propagation using Elastix.")
        print("Arguments:")
        print("   1: Fixed image")
        print("   2: Moving image")
        print("   3: Transform file name")
        print("   4: Binary flag (1 or 0)")
        print("   5: Interpolation order (0=NN, 1=Linear, etc.)")
        print("   6: Save deformation field (1 or 0)")
        print("   7: Output name")
        sys.exit()
    else:
        fixed = sitk.ReadImage(sys.argv[1])
        moving = sitk.ReadImage(sys.argv[2], sitk.sitkFloat32)
        tfm = sys.argv[3]
        binFlag = int(sys.argv[4])
        interp = sys.argv[5]
        defFlag = int(sys.argv[6])
        out = sys.argv[7]+'.nii.gz'
        PropagateRegistrationToLabelUsingFile(fixed, moving, tfm, binFlag, interp, defFlag, out)
