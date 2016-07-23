#pragma once

#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_SkeletalControlBase.h"
#include "Runtime/AnimGraphRuntime/Public/BoneControllers/AnimNode_Fabrik.h"
#include "Runtime/Engine/Classes/GameFramework/Character.h"
#include "AnimNode_LegsFabrik.generated.h"

struct LegsOffsets;

/**
 * Example UStruct declared in a plugin module
 */
USTRUCT()
struct FAnimNode_LegsFabrik : public FAnimNode_SkeletalControlBase
{
    GENERATED_USTRUCT_BODY();
 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
    FName LeftSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
    FName RightSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
    FName LeftBallSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
    FName RightBallSocketName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Links, meta = (PinShownByDefault))
    float TraceOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category=SkeletalControl) 
    FBoneReference HipBone;

    // FABRKIK fields.
    /** Reference frame of Effector Transform. */
    TEnumAsByte<enum EBoneControlSpace> EffectorTransformSpace;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EndEffector)
    TEnumAsByte<enum EAxis> FootAxis;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = EndEffector)
    TEnumAsByte<enum EBoneRotationSource> EffectorRotationSource;

    /** Tolerance for final tip location delta from EffectorLocation*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
    float Precision;

    /** Maximum number of iterations allowed, to control performance. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
    int32 MaxIterations;

    /** Toggle drawing of axes to debug joint rotation*/
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Solver)
    bool bEnableDebugDraw;

    /** Inverse offset of right foot. Some models need it. */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FabrikBones)
    bool bDoInverseRightFootOffset;

    // Our modifications
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FabrikBones)
    FBoneReference LeftTipBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FabrikBones)
    FBoneReference LeftRootBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FabrikBones)
    FBoneReference RightTipBone;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = FabrikBones)
    FBoneReference RightRootBone;

public:
//    virtual void EvaluateComponentSpace(FComponentSpacePoseContext& Output) override;
    virtual void InitializeBoneReferences(const FBoneContainer& RequiredBones) override;

    virtual void EvaluateBoneTransforms(USkeletalMeshComponent* SkelComp,
        FCSPose<FCompactPose>& MeshBases,
        TArray<FBoneTransform>& OutBoneTransforms
    ) override;

    void EvaluateLeftFabrik(
        USkeletalMeshComponent* SkelComp,
        const FBoneContainer& RequiredBones,
        FA2CSPose& MeshBases,
        TArray<FBoneTransform>& OutBoneTransforms
    );

    void Evaluate(FPoseContext &Output) override;

    virtual bool IsValidToEvaluate(const USkeleton* Skeleton, const FBoneContainer& RequiredBones) override;
    FAnimNode_LegsFabrik();


private:
    void SetLegsOffset(LegsOffsets &legsOffsets, float &outHipOffset, float DownOffsetThreshold);
    bool FootTrace(const FName &SocketName, float DownOffsetThreshold,
        float &OutHipOffset, FHitResult &OutRV_Hit) const;
    void UpdateFabrikNode(const FTransform &Transform, const FBoneReference &TipBone,
        const FBoneReference &RootBone, FAnimNode_Fabrik &OutNode);

    bool GetSocketRotator(const FName &BallSocketName, FRotator &OutRot) const;
    bool GetSocketProection(const FName &SocketName, FHitResult &OutRV_Hit) const;
    bool IsValidHipBone;

    USkeletalMeshComponent* Component;
    AActor * Actor;
    ACharacter * Character;
    FCompactPose * MeshBases;

    FVector HipTargetVector;

    FVector LeftEffectorVector;
    FVector RightEffectorVector;

    FRotator LeftTarsusRot;
    FRotator RightTarsusRot;

    TEnumAsByte<enum EBoneControlSpace> HipTranslationSpace;
    TEnumAsByte<enum EBoneControlSpace> TipTranslationSpace;

    FAnimNode_Fabrik LeftFootFabrik;
    FAnimNode_Fabrik RightFootFabrik;

//    void UpdateInternal(const FAnimationUpdateContext& Output) override;
};
