// Fill out your copyright notice in the Description page of Project Settings.


#include "cPlayerABP.h"
UcPlayerABP::UcPlayerABP(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	//set any default values for your variables here
	m_fGripVal = 0.0f;
}
void UcPlayerABP::SetGripValue(float a_fGrip)
{
	m_fGripVal = a_fGrip;
}

