#include "StandAlonePrivatePCH.h"
// Fill out your copyright notice in the Description page of Project Settings.

#include "AnimGraphNode_LegsFabrik.h"

UAnimGraphNode_LegsFabrik::UAnimGraphNode_LegsFabrik(const FObjectInitializer& PCIP) :
    Super(PCIP)
{

}

FText UAnimGraphNode_LegsFabrik::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
    return FText::FromString(FString("Legs FABRIK"));
}

FLinearColor UAnimGraphNode_LegsFabrik::GetNodeTitleColor() const
{
    return FLinearColor(0, 12, 12, 1);
}

FString UAnimGraphNode_LegsFabrik::GetNodeCategory() const
{
    return FString("Victory Anim Nodes");
}

FText UAnimGraphNode_LegsFabrik::GetControllerDescription() const
{
    return FText::FromString(FString("qwe"));
}
