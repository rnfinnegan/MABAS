#/usr/bin/bash

# STEP 1
../Python/RigidRegistrationElastix.py 	./Inputs/Case_01.nii.gz \
										./Atlases/Images/Case_10.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_10_to_Case_01_RIGID \
										../ParameterFiles/RigidTransformParameters.txt \
										0

../Python/RigidRegistrationElastix.py 	./Inputs/Case_01.nii.gz \
										./Atlases/Images/Case_20.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_20_to_Case_01_RIGID \
										../ParameterFiles/RigidTransformParameters.txt \
										0

../Python/RigidRegistrationElastix.py 	./Inputs/Case_01.nii.gz \
										./Atlases/Images/Case_30.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_30_to_Case_01_RIGID \
										../ParameterFiles/RigidTransformParameters.txt \
										0


# STEP 2
../Python/PropagateRigidTransform.py 	./Inputs/Case_01.nii.gz \
										./Atlases/Structures/Case_10_HEART.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_10_to_Case_01_RIGID.txt \
										0 \
										1 \
										./Registration/Rigid/LeaveOut01/Structures/Case_10_to_Case_01_HEART_RIGID.nii.gz

../Python/PropagateRigidTransform.py 	./Inputs/Case_01.nii.gz \
										./Atlases/Structures/Case_20_HEART.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_20_to_Case_01_RIGID.txt \
										0 \
										1 \
										./Registration/Rigid/LeaveOut01/Structures/Case_20_to_Case_01_HEART_RIGID.nii.gz

../Python/PropagateRigidTransform.py 	./Inputs/Case_01.nii.gz \
										./Atlases/Structures/Case_30_HEART.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_30_to_Case_01_RIGID.txt \
										0 \
										1 \
										./Registration/Rigid/LeaveOut01/Structures/Case_30_to_Case_01_HEART_RIGID.nii.gz

# STEP 3
../C++/bin/itkMultiResFSDRegistration	./Inputs/Case_01.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_10_to_Case_01_RIGID.nii.gz \
										./Registration/Demons/LeaveOut01/Case_10_to_Case_01_DEMONS.nii.gz \
										./Registration/Demons/LeaveOut01/Case_10_to_Case_01_DEMONS_FIELD.nii.gz \
										1.5 \
										0

../C++/bin/itkMultiResFSDRegistration	./Inputs/Case_01.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_20_to_Case_01_RIGID.nii.gz \
										./Registration/Demons/LeaveOut01/Case_20_to_Case_01_DEMONS.nii.gz \
										./Registration/Demons/LeaveOut01/Case_20_to_Case_01_DEMONS_FIELD.nii.gz \
										1.5 \
										0

../C++/bin/itkMultiResFSDRegistration	./Inputs/Case_01.nii.gz \
										./Registration/Rigid/LeaveOut01/Case_30_to_Case_01_RIGID.nii.gz \
										./Registration/Demons/LeaveOut01/Case_30_to_Case_01_DEMONS.nii.gz \
										./Registration/Demons/LeaveOut01/Case_30_to_Case_01_DEMONS_FIELD.nii.gz \
										1.5 \
										0

# STEP 4
../Python/ApplyDemonsDeformationField.py	./Registration/Demons/LeaveOut01/Case_10_to_Case_01_DEMONS_FIELD.nii.gz \
											./Registration/Demons/LeaveOut01/Structures/Case_10_to_Case_01_HEART_DEMONS.nii.gz \
											1 \
											0 \
											./Registration/Rigid/LeaveOut01/Structures/Case_10_to_Case_01_HEART_RIGID.nii.gz


../Python/ApplyDemonsDeformationField.py	./Registration/Demons/LeaveOut01/Case_20_to_Case_01_DEMONS_FIELD.nii.gz \
											./Registration/Demons/LeaveOut01/Structures/Case_20_to_Case_01_HEART_DEMONS.nii.gz \
											1 \
											0 \
											./Registration/Rigid/LeaveOut01/Structures/Case_20_to_Case_01_HEART_RIGID.nii.gz

../Python/ApplyDemonsDeformationField.py	./Registration/Demons/LeaveOut01/Case_30_to_Case_01_DEMONS_FIELD.nii.gz \
											./Registration/Demons/LeaveOut01/Structures/Case_30_to_Case_01_HEART_DEMONS.nii.gz \
											1 \
											0 \
											./Registration/Rigid/LeaveOut01/Structures/Case_30_to_Case_01_HEART_RIGID.nii.gz

# STEP 5
../Python/LabelFusion.py	./Inputs/Case_01.nii.gz \
							./Registration/Demons/LeaveOut01 \
							./Registration/Demons/LeaveOut01/Structures/Case_{0}_to_Case_{1}_{2}_DEMONS.nii.gz \
							local \
							structures.txt \
							Output/Case_01_{0}_LOCALVOTE_{1}.nii.gz
