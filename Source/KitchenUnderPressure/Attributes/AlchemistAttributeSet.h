// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AlchemistAttributeSet.generated.h"

// Generates the standard getter/setter/initter helpers for a gameplay attribute.
#define ATTRIBUTE_ACCESSORS(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_PROPERTY_GETTER(ClassName, PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_GETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_SETTER(PropertyName) \
	GAMEPLAYATTRIBUTE_VALUE_INITTER(PropertyName)

// Server-side signal raised when an owner's Health reaches zero (passes the dying avatar actor).
DECLARE_MULTICAST_DELEGATE_OneParam(FOnOutOfHealth, AActor* /*DeadAvatar*/);

/**
 * Shared attribute set for players and enemies. Health/MaxHealth/MoveSpeed are replicated; Damage is
 * a transient meta-attribute (never replicated) that incoming damage GameplayEffects write to and
 * PostGameplayEffectExecute consumes. All health maths and the death signal run server-side here.
 */
UCLASS()
class KITCHENUNDERPRESSURE_API UAlchemistAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:
	UAlchemistAttributeSet();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	/** Bound by the owning actor (player state / enemy) to run its death logic. Fires on the server. */
	FOnOutOfHealth OnOutOfHealth;

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Health, Category = "Attributes")
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS(UAlchemistAttributeSet, Health);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MaxHealth, Category = "Attributes")
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS(UAlchemistAttributeSet, MaxHealth);

	UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_MoveSpeed, Category = "Attributes")
	FGameplayAttributeData MoveSpeed;
	ATTRIBUTE_ACCESSORS(UAlchemistAttributeSet, MoveSpeed);

	/** Meta-attribute: inbound damage accumulator. Not replicated; consumed in PostGameplayEffectExecute. */
	UPROPERTY(BlueprintReadOnly, Category = "Attributes")
	FGameplayAttributeData Damage;
	ATTRIBUTE_ACCESSORS(UAlchemistAttributeSet, Damage);

protected:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue);

	UFUNCTION()
	void OnRep_MoveSpeed(const FGameplayAttributeData& OldValue);
};
