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
    , EffectorTransformSpace(BCS_BoneSpace)
    , TipTranslationSpace(BCS_WorldSpace)
    , EffectorRotationSource(BRS_KeepLocalSpaceRotation)
    , LeftEffectorVector(FVector::ZeroVector)
    , RightEffectorVector(FVector::ZeroVector)
    , HipTargetVector(FVector::ZeroVector)
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
            FVector EndVector(SocketLocation.X, SocketLocation.Y, ActorLocation.Z - CapsuleOffset - 100);
            
            //DrawDebugLine(Actor->GetWorld(), StartVector, EndVector, FColor(1,0,0));
            //UE_LOG(LogTemp, Warning, TEXT("%s Start: %s, end: %s"), *(SocketName.ToString()), *(StartVector.ToString()), *(EndVector.ToString()));

            FCollisionQueryParams RV_TraceParams = FCollisionQueryParams(FName(TEXT("RV_Trace")), true, Actor);
            RV_TraceParams.bTraceComplex = false;
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

void FAnimNode_LegsFabrik::FootTrace(const FName &SocketName,
    float &OutFootOffset, float &OutHipOffset) const
{
    if (Actor && Component)
    {
        FVector ActorLocation = Actor->GetActorLocation();
        float CapsuleOffset = Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()/2;
        //DrawDebugSphere(Actor->GetWorld(), RV_Hit.Location, 7, 12, FColor(0,0,0));

        float DownOffsetThreshold = ActorLocation.Z - 2.f*CapsuleOffset;
        //UE_LOG(LogTemp, Warning, TEXT("ActorLocation.Z: %f, CapsuleOffset: %f"), ActorLocation.Z, 2.f*CapsuleOffset);
        //UE_LOG(LogTemp, Warning, TEXT("DownOffsetThreshold: %f"), DownOffsetThreshold);
        OutHipOffset = 0.f;
        FHitResult RV_Hit(ForceInit);
        bool res = GetSocketProection(SocketName, RV_Hit);
        if (res)
        {
            OutHipOffset = (DownOffsetThreshold - RV_Hit.ImpactPoint.Z) * (-1);
            //UE_LOG(LogTemp, Warning, TEXT("RV_Hit.ImpactPoint.Z: %f"), RV_Hit.ImpactPoint.Z);
        }
        //UE_LOG(LogTemp, Error, TEXT("%s. Res: %i, %f"), *(SocketName.ToString()), res, OutHipOffset);
        //UE_LOG(LogTemp, Warning, TEXT("RV_Hit.ImpactPoint.Z: %f. DownOffsetThreshold: %f, OutHipOffset: %f"),
        //    RV_Hit.ImpactPoint.Z, DownOffsetThreshold, OutHipOffset);
        OutFootOffset = RV_Hit.ImpactPoint.Z - (DownOffsetThreshold - (-HipTargetVector.Z));
    }
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

        // Set effectors.
        FootTrace(LeftSocketName, LeftFootOffset, LeftHipOffset);
        FootTrace(RightSocketName, RightFootOffset, RightHipOffset);

        float HipOffset = FMath::Min(LeftHipOffset, RightHipOffset);

        FRotator LeftRot;
        FRotator RightRot;
        GetSocketRotator(LeftBallSocketName, LeftRot);
        GetSocketRotator(RightBallSocketName, RightRot);
        // This so strange, but it works.
        RightFootOffset *= -1;

        float DeltaTime = Actor->GetWorld()->GetDeltaSeconds();

        // Set target.
        float OldHipOffset = HipTargetVector.Z;
        HipTargetVector.Z = HipOffset;
        HipTargetVector.Z = FMath::FInterpTo(OldHipOffset, HipOffset,
            DeltaTime, 10);

        float OldLeftOffset = LeftEffectorVector.X;
        LeftEffectorVector.X = FMath::FInterpTo(OldLeftOffset, LeftFootOffset, DeltaTime, 10);

        OldLeftOffset = RightEffectorVector.X;
        RightEffectorVector.X = FMath::FInterpTo(OldLeftOffset, RightFootOffset, DeltaTime, 10);

        // Set rotators.
        UE_LOG(LogTemp, Warning, TEXT("!%s"), *(LeftRot.ToString()))
        LeftTarsusRot = FMath::RInterpTo(LeftTarsusRot, LeftRot, DeltaTime, 10);
        RightTarsusRot = FMath::RInterpTo(RightTarsusRot, RightRot, DeltaTime, 10);


        //if (LeftTipBone.IsValid(RequiredBones) && LeftRootBone.IsValid(RequiredBones))
        //{
            //FootFabrik(FTransform(LeftEffectorVector), LeftTipBone, LeftRootBone, RequiredBones, OutBoneTransforms);
        //}
        //else
        //{
        //    UE_LOG(LogTemp, Warning, TEXT("LeftTipBone is valid: %i, LeftRootBone is valid: %i"),
        //        LeftTipBone.IsValid(RequiredBones), LeftRootBone.IsValid(RequiredBones));
        //}
        //if (RightTipBone.IsValid(RequiredBones) && RightRootBone.IsValid(RequiredBones))
        //{
        //    FootFabrik(FTransform(LeftEffectorVector), RightTipBone, RightRootBone, RequiredBones, OutBoneTransforms);
        //}
        //else
        //{
        //    UE_LOG(LogTemp, Warning, TEXT("RightTipBone is valid: %i, RightRootBone is valid: %i"),
        //        RightTipBone.IsValid(RequiredBones), RightRootBone.IsValid(RequiredBones));
        //}
        // Transformations.
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

        
        //else
        //{
        //    UE_LOG(LogTemp, Error, TEXT("!!!HipBone doesn't exist, please point it in Node parameters."));
        //}

        //UE_LOG(LogTemp, Warning, TEXT("%i, %i, %i"), LeftTipBone.BoneIndex, LeftRootBone.BoneIndex, HipBone.BoneIndex);

        //int32 LastIndex = INDEX_NONE;

        //UE_LOG(LogTemp, Warning, TEXT("OutBoneTransforms.Num() - %i"), OutBoneTransforms.Num());
        //for (const auto &item: OutBoneTransforms)
        //{
        //    UE_LOG(LogTemp, Warning, TEXT("%i"), item.BoneIndex);
        //    if (!(item.BoneIndex >= LastIndex))
        //    {
        //        OutBoneTransforms.RemoveAt(0, OutBoneTransforms.Num());
        //        break;
        //    }
        //}

    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("some shit happens"));
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
            BoneTransforms.RemoveAt(0, BoneTransforms.Num());
            //EvaluateLeftFabrik(Component, Output.AnimInstance->RequiredBones, Output.Pose, BoneTransforms);
            UpdateFabrikNode(FTransform(LeftEffectorVector), LeftTipBone, LeftRootBone, LeftFootFabrik);
            LeftFootFabrik.EvaluateBoneTransforms(Component, Output.AnimInstance->RequiredBones, Output.Pose, BoneTransforms);
            checkSlow(!ContainsNaN(BoneTransforms));

            if (BoneTransforms.Num() > 0)
            {
                const float BlendWeight = FMath::Clamp<float>(ActualAlpha, 0.f, 1.f);
                Output.Pose.LocalBlendCSBoneTransforms(BoneTransforms, BlendWeight);
            }

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

void FAnimNode_LegsFabrik::EvaluateLeftFabrik(
    USkeletalMeshComponent* SkelComp,
    const FBoneContainer& RequiredBones,
    FA2CSPose& MeshBases,
    TArray<FBoneTransform>& OutBoneTransforms
    )
{
    UpdateFabrikNode(FTransform(LeftEffectorVector), LeftTipBone, LeftRootBone, LeftFootFabrik);
    LeftFootFabrik.EvaluateBoneTransforms(Component, RequiredBones, MeshBases, OutBoneTransforms);
}
