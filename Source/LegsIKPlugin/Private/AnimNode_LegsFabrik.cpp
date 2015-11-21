#include "StandAlonePrivatePCH.h"
#include "Runtime/Engine/Classes/Animation/BoneControllers/AnimNode_Fabrik.h"
#include "AnimNode_LegsFabrik.h"

const float MAX_RENDER_SPEED = 100;
FAnimNode_LegsFabrik::FAnimNode_LegsFabrik()
    : Component(nullptr)
    , Actor(nullptr)
    , Character(nullptr)
    , MeshBases(nullptr)
    , HipTranslationSpace(BCS_ComponentSpace)
    , Precision(1.f)
    , MaxIterations(10)
    , bEnableDebugDraw(false)
    , bDoInverseRightFootOffset(false)
    , EffectorTransformSpace(BCS_BoneSpace)
    , TipTranslationSpace(BCS_WorldSpace)
    , EffectorRotationSource(BRS_KeepLocalSpaceRotation)
    , LeftEffectorVector(FVector::ZeroVector)
    , RightEffectorVector(FVector::ZeroVector)
    , HipTargetVector(FVector::ZeroVector)
    , TraceOffset(100.f)
    , FootAxis(1)
{
    UE_LOG(LogTemp, Warning, TEXT("Fabrik node has been cunstructed"));

    // TODO: inherit from FAnimNode_Fabrik for defining default values.
    LeftFootFabrik.EffectorTransformSpace = BCS_BoneSpace;
    RightFootFabrik.EffectorTransformSpace = BCS_BoneSpace;
}

bool FAnimNode_LegsFabrik::GetSocketProection(const FName &SocketName, FHitResult &OutRV_Hit) const
{
    if (Actor && Component && Character)
    {
        FVector ActorLocation = Actor->GetActorLocation();
        const USkeletalMeshSocket * Socket = Component->GetSocketByName(SocketName);
        if (Socket)
        {
            FVector SocketLocation = Component->GetSocketLocation(SocketName);
            
            float CapsuleOffset = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()/2;
            FVector StartVector(SocketLocation.X, SocketLocation.Y, ActorLocation.Z - CapsuleOffset);
            FVector EndVector(SocketLocation.X, SocketLocation.Y, ActorLocation.Z - CapsuleOffset - TraceOffset);
            
            if (bEnableDebugDraw)
            {
                DrawDebugSphere(Actor->GetWorld(), SocketLocation, 5, 12, FColor(255,0,0));
                DrawDebugLine(Actor->GetWorld(), StartVector, EndVector, FColor(255,255,0));
            }

            FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, Actor);
            RV_TraceParams.bTraceComplex = true;
            RV_TraceParams.bTraceAsyncScene = true;
            RV_TraceParams.bReturnPhysicalMaterial = false;

            //Re-initialize hit info
            bool res = Actor->GetWorld()->LineTraceSingleByChannel(
                OutRV_Hit,        //result
                StartVector,    //start
                EndVector, //end
                ECC_Pawn, //collision channel
                RV_TraceParams
            );
            return res;
        }

        UE_LOG(LogTemp, Error, TEXT("Socket %s does not exist"), *(SocketName.ToString()));
    }

    return false;
}

bool FAnimNode_LegsFabrik::FootTrace(const FName &SocketName, float DownOffsetThreshold,
    float &OutHipOffset, FHitResult &OutRV_Hit) const
{
    if (Actor && Component)
    {
        const float ScaleZ = Character->GetMesh()->GetTransformMatrix().GetScaleVector().Z;
        
        OutHipOffset = 0.f;
        bool res = GetSocketProection(SocketName, OutRV_Hit);
        if (res)
        {
            if (ScaleZ > 0)
            {
                OutHipOffset = ((DownOffsetThreshold - OutRV_Hit.ImpactPoint.Z) * (-1)) / ScaleZ;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Your scaling by Z of mesh less than 0. Bad thing."))
            }
        }

        if (bEnableDebugDraw)
        {
            DrawDebugSphere(Actor->GetWorld(), OutRV_Hit.ImpactPoint, 7, 12, FColor(255,255,255));
        }
        return res;
    }

    return false;
}

void FAnimNode_LegsFabrik::EvaluateBoneTransforms(
    USkeletalMeshComponent* SkelComp, const FBoneContainer& RequiredBones,
    FA2CSPose& MeshBases, TArray<FBoneTransform>& OutBoneTransforms)
{
    Component = SkelComp;
    if (Component && Actor && Character && !Character->GetCharacterMovement()->IsFalling()
        && Actor->GetVelocity().Size() < MAX_RENDER_SPEED)
    {
        // Get hip offset and offset of foots.
        float LeftHipOffset = 0;
        float LeftFootOffset = 0;
        float RightHipOffset = 0;
        float RightFootOffset = 0;
        FVector ActorLocation = Actor->GetActorLocation();
        float CapsuleOffset = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
        float DownOffsetThreshold = ActorLocation.Z - CapsuleOffset;
        FHitResult LeftRV_Hit(ForceInit);
        FHitResult RightRV_Hit(ForceInit);
        const float ScaleZ = Character->GetMesh()->GetTransformMatrix().GetScaleVector().Z;

        // Set effectors.
        bool LeftRes = FootTrace(LeftSocketName, DownOffsetThreshold, LeftHipOffset, LeftRV_Hit);
        bool RightRes = FootTrace(RightSocketName, DownOffsetThreshold, RightHipOffset, RightRV_Hit);

        float HipOffset = 0;
        if (LeftRes && LeftHipOffset < RightHipOffset)
        {
            HipOffset = LeftHipOffset;
            if (RightRes && ScaleZ > 0)
            {
                RightFootOffset = (RightRV_Hit.ImpactPoint.Z - LeftRV_Hit.ImpactPoint.Z) / ScaleZ;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Your scaling by Z of mesh less than 0. Bad thing."))
            }
        }
        else if (RightRes)
        {
            HipOffset = RightHipOffset;
            if (LeftRes && ScaleZ > 0)
            {
                LeftFootOffset = (LeftRV_Hit.ImpactPoint.Z - RightRV_Hit.ImpactPoint.Z) / ScaleZ;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Your scaling by Z of mesh less than 0. Bad thing."))
            }
        }

        FRotator LeftRot;
        FRotator RightRot;
        GetSocketRotator(LeftBallSocketName, LeftRot);
        GetSocketRotator(RightBallSocketName, RightRot);
        if (bDoInverseRightFootOffset) {
            RightFootOffset *= -1;
        }

        float DeltaTime = Actor->GetWorld()->GetDeltaSeconds();

        // Set target.
        float OldHipOffset = 0.f;
        float OldOffset = 0.f;

        if (FootAxis.GetValue() > 0)    // First constant of enum is NONE.
        {
            OldHipOffset = HipTargetVector.Z;
            HipTargetVector.Z = FMath::FInterpTo(OldHipOffset, HipOffset,
                DeltaTime, 10);

            OldOffset = LeftEffectorVector[FootAxis.GetValue()-1];
            LeftEffectorVector[FootAxis.GetValue()-1] = FMath::FInterpTo(OldOffset, LeftFootOffset, DeltaTime, 10);

            OldOffset = RightEffectorVector[FootAxis.GetValue()-1];
            RightEffectorVector[FootAxis.GetValue()-1] = FMath::FInterpTo(OldOffset, RightFootOffset, DeltaTime, 10);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Incorrect value of Foot Axis."));
        }

        // Set rotators.
        LeftTarsusRot = FMath::RInterpTo(LeftTarsusRot, LeftRot, DeltaTime, 10);
        RightTarsusRot = FMath::RInterpTo(RightTarsusRot, RightRot, DeltaTime, 10);

        if (HipBone.IsValid(RequiredBones))
        {
            FTransform NewBoneTM = MeshBases.GetComponentSpaceTransform(HipBone.BoneIndex);
            FAnimationRuntime::ConvertCSTransformToBoneSpace(SkelComp, MeshBases,
                NewBoneTM, HipBone.BoneIndex, HipTranslationSpace);
            NewBoneTM.AddToTranslation(HipTargetVector);
            FAnimationRuntime::ConvertBoneSpaceTransformToCS(Component, MeshBases,
                NewBoneTM, HipBone.BoneIndex, HipTranslationSpace);
            OutBoneTransforms.Add(FBoneTransform(HipBone.BoneIndex, NewBoneTM));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Legs IK is not working"));
    }
}

void FAnimNode_LegsFabrik::UpdateFabrikNode(const FTransform &Transform,
    const FBoneReference &TipBone, const FBoneReference &RootBone,
    FAnimNode_Fabrik &OutNode)
{
    OutNode.EffectorTransform = Transform;
    OutNode.EffectorTransformBone = TipBone;
    OutNode.TipBone = TipBone;
    OutNode.RootBone = RootBone;
}

void FAnimNode_LegsFabrik::EvaluateComponentSpace(FComponentSpacePoseContext& Output)
{
    Actor = Output.AnimInstance->GetOwningActor();
    Character = Cast<ACharacter>(Actor);
    MeshBases = &Output.Pose;

    ComponentPose.EvaluateComponentSpace(Output);

    // Apply the skeletal control if it's valid
    const float ActualAlpha = AlphaScaleBias.ApplyTo(Alpha);
    if ((ActualAlpha >= ZERO_ANIMWEIGHT_THRESH) && IsValidToEvaluate(Output.AnimInstance->CurrentSkeleton, Output.AnimInstance->RequiredBones))
    {
        USkeletalMeshComponent* Component = Output.AnimInstance->GetSkelMeshComponent();

#if WITH_EDITORONLY_DATA
        // save current pose before applying skeletal control to compute the exact gizmo location in AnimGraphNode
        ForwardedPose = Output.Pose;
#endif // #if WITH_EDITORONLY_DATA

        TArray<FBoneTransform> BoneTransforms;
        // Animates pelvis transformation.
        EvaluateBoneTransforms(Component, Output.AnimInstance->RequiredBones, Output.Pose, BoneTransforms);

        checkSlow(!ContainsNaN(BoneTransforms));

        if (BoneTransforms.Num() > 0)
        {
            const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
            Output.Pose.LocalBlendCSBoneTransforms(BoneTransforms, BlendWeight);
        }

        if (Component && Actor && Character && !Character->GetCharacterMovement()->IsFalling()
            && Actor->GetVelocity().Size() < MAX_RENDER_SPEED)
        {
            // Animates left foot IK.
            BoneTransforms.RemoveAt(0, BoneTransforms.Num());
            UpdateFabrikNode(FTransform(LeftEffectorVector), LeftTipBone, LeftRootBone, LeftFootFabrik);
            LeftFootFabrik.EvaluateBoneTransforms(Component, Output.AnimInstance->RequiredBones, Output.Pose, BoneTransforms);
            checkSlow(!ContainsNaN(BoneTransforms));

            if (BoneTransforms.Num() > 0)
            {
                const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
                Output.Pose.LocalBlendCSBoneTransforms(BoneTransforms, BlendWeight);
            }

            // Animates right foot IK.
            BoneTransforms.RemoveAt(0, BoneTransforms.Num());
            UpdateFabrikNode(FTransform(RightEffectorVector), RightTipBone, RightRootBone, RightFootFabrik);
            RightFootFabrik.EvaluateBoneTransforms(Component, Output.AnimInstance->RequiredBones, Output.Pose, BoneTransforms);
            checkSlow(!ContainsNaN(BoneTransforms));

            if (BoneTransforms.Num() > 0)
            {
                const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
                Output.Pose.LocalBlendCSBoneTransforms(BoneTransforms, BlendWeight);
            }

            //-----------------------
            // Animates left tarsus rotation.
            BoneTransforms.RemoveAt(0, BoneTransforms.Num());

            FTransform NewBoneTM = Output.Pose.GetComponentSpaceTransform(LeftTipBone.BoneIndex);
            FAnimationRuntime::ConvertCSTransformToBoneSpace(Component, Output.Pose,
                NewBoneTM, LeftTipBone.BoneIndex, TipTranslationSpace);
            const FQuat BoneQuat(LeftTarsusRot);
            NewBoneTM.SetRotation(BoneQuat * NewBoneTM.GetRotation());
            FAnimationRuntime::ConvertBoneSpaceTransformToCS(Component, Output.Pose,
                NewBoneTM, LeftTipBone.BoneIndex, TipTranslationSpace);

            BoneTransforms.Add(FBoneTransform(LeftTipBone.BoneIndex, NewBoneTM));
            if (BoneTransforms.Num() > 0)
            {
                const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
                Output.Pose.LocalBlendCSBoneTransforms(BoneTransforms, BlendWeight);
            }

            // Animates right tarsus rotation.
            BoneTransforms.RemoveAt(0, BoneTransforms.Num());
            NewBoneTM = Output.Pose.GetComponentSpaceTransform(RightTipBone.BoneIndex);
            FAnimationRuntime::ConvertCSTransformToBoneSpace(Component, Output.Pose,
                NewBoneTM, RightTipBone.BoneIndex, TipTranslationSpace);
            const FQuat BoneQuat2(RightTarsusRot);
            NewBoneTM.SetRotation(BoneQuat2 * NewBoneTM.GetRotation());
            FAnimationRuntime::ConvertBoneSpaceTransformToCS(Component, Output.Pose,
                NewBoneTM, RightTipBone.BoneIndex, TipTranslationSpace);

            BoneTransforms.Add(FBoneTransform(RightTipBone.BoneIndex, NewBoneTM));
            if (BoneTransforms.Num() > 0)
            {
                const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
                Output.Pose.LocalBlendCSBoneTransforms(BoneTransforms, BlendWeight);
            }
        }
    }
}

bool FAnimNode_LegsFabrik::GetSocketRotator(const FName &BallSocketName, FRotator &OutRot) const
{
    if (Actor && Component)
    {
        FHitResult RV_Hit(ForceInit);
        if (GetSocketProection(BallSocketName, RV_Hit))
        {
            // Conversion of normal to rotator.
            FVector Normal = RV_Hit.Normal;
            OutRot.Pitch = (180.f)/PI * FMath::Atan2(Normal.X, Normal.Z)*(-1);
            OutRot.Yaw = 0;
            OutRot.Roll = (180.f)/PI * FMath::Atan2(Normal.Y, Normal.Z);

            return true;
        }
    }

    return false;
}

void FAnimNode_LegsFabrik::InitializeBoneReferences(const FBoneContainer& RequiredBones)
{
    const uint8 N = 5;
    FBoneReference * BonesForInit[N] = {
        &HipBone, &LeftTipBone, &LeftRootBone, &RightTipBone, &RightRootBone
    };
    for (size_t i = 0; i < N; ++i) BonesForInit[i]->Initialize(RequiredBones);
}

bool FAnimNode_LegsFabrik::IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones)
{
    bool Flag = true;
    const uint8 N = 5;
    FBoneReference * BonesForInit[N] = {
        &HipBone, &LeftTipBone, &LeftRootBone, &RightTipBone, &RightRootBone
    };
    for (size_t i = 0; i < N; ++i)
    {
        bool IsValid = BonesForInit[i]->IsValid(RequiredBones);
        Flag = Flag && IsValid;
        if (!IsValid)
        {
            UE_LOG(LogTemp, Error, TEXT("Bone %s is not valid"), *(BonesForInit[i]->BoneName.ToString()));
        }
    }
    return Flag;
}