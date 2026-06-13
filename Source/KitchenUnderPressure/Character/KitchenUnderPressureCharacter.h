// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Templates/SubclassOf.h"
#include "AbilitySystemInterface.h"
#include "Spells/SpellTypes.h"
#include "KitchenUnderPressureCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class USceneComponent;
class UInputAction;
class UPickupComponent;
class USpellCasterComponent;
class UStaticMeshComponent;
class UMaterialInstanceDynamic;
class UAbilitySystemComponent;
class UGameplayEffect;
struct FInputActionValue;

/**
 *  A basic first person character. Its ability system lives on the AAlchemistPlayerState (so it
 *  survives respawns/travel); this pawn is the GAS avatar and binds the ASC to itself on possession.
 */
UCLASS(abstract)
class AKitchenUnderPressureCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** Picks up, carries and throws physics objects (replicated; server-authoritative) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UPickupComponent* PickupComponent;

	/** Attach point in front of the body where carried objects sit (drag it in the Blueprint to tune the hold) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USceneComponent* HoldPoint;

	/** Owns the two-hand spell loadout and casts through the ability system (replicated) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USpellCasterComponent* SpellCaster;

	/** Rarity aura for the left hand's spell (element-coloured; hidden for common spells) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* LeftHandAura;

	/** Rarity aura for the right hand's spell */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RightHandAura;

public:
	AKitchenUnderPressureCharacter();

	//~ Begin IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//~ End IAbilitySystemInterface

protected:

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Called from the pause Input Action; forwards the toggle to the player controller */
	void PauseInput();

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

	/** Pick up the aimed object, or drop the held one (forwarded to the pickup component) */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoInteract();

	/** Start charging a throw (throw button pressed) */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoThrowStart();

	/** Release the throw with the charged force (throw button released) */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoThrowRelease();

	/** Cast the left-hand spell (bound to the left mouse button). */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoCastLeft();

	/** Cast the right-hand spell (bound to the right mouse button). */
	UFUNCTION(BlueprintCallable, Category="Input")
	void DoCastRight();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void BeginPlay() override;

	//~ GAS init: bind the PlayerState's ability system to this pawn on both server and client.
	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_PlayerState() override;

	/** Applied once on the server to set starting Health/MaxHealth/MoveSpeed (assign GE_PlayerInit). */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> DefaultAttributesEffect;

public:
	/** DEBUG: deal damage to self on the server. Console: "KUP_DamageSelf 25". Temporary test aid. */
	UFUNCTION(Exec)
	void KUP_DamageSelf(float Amount = 25.f);

private:
	/** Point the PlayerState's ASC at this pawn; the server also applies init attributes, binds the
	 *  death signal and grants the starting spells. Safe to call from PossessedBy (server) and
	 *  OnRep_PlayerState (client) in any order. */
	void InitAbilitySystem();

	/** Server: react to this pawn's Health reaching zero (idempotent). */
	void HandleDeath(AActor* DeadAvatar);

	UFUNCTION(Server, Reliable)
	void ServerDebugDamageSelf(float Amount);

	/** DEBUG: damage effect used by KUP_DamageSelf (assign GE_Damage). */
	UPROPERTY(EditDefaultsOnly, Category = "Abilities")
	TSubclassOf<UGameplayEffect> DebugDamageEffect;

	/** Refresh a hand's rarity aura when its spell changes (bound to the caster; runs on all machines). */
	UFUNCTION()
	void OnHandSpellChanged(EHand Hand);

	void UpdateHandAura(EHand Hand);

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> LeftAuraMID;

	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> RightAuraMID;

	bool bAttributesInitialised = false;
	bool bDeathBound = false;

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Returns the pickup/carry/throw component **/
	UPickupComponent* GetPickupComponent() const { return PickupComponent; }

	/** Returns the attach point for carried objects **/
	USceneComponent* GetHoldPoint() const { return HoldPoint; }

	/** Returns the two-hand spell caster component **/
	USpellCasterComponent* GetSpellCaster() const { return SpellCaster; }

};
