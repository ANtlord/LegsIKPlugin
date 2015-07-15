// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AnimGraphNode_SkeletalControlBase.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "AnimNode_LegsFabrik.h"

#include "AnimGraphNode_LegsFabrik.generated.h"

/**
 * 
 */
UCLASS(MinimalAPI)
class UAnimGraphNode_LegsFabrik : public UAnimGraphNode_SkeletalControlBase
{
	GENERATED_UCLASS_BODY()
    UPROPERTY(EditAnywhere, Category = Settings)
    FAnimNode_LegsFabrik Node;

public:
    FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
    FLinearColor GetNodeTitleColor() const override;
    FString GetNodeCategory() const override;

protected:
    virtual FText GetControllerDescription() const;
};