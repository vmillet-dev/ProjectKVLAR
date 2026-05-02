// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "CityBuilderCameraPawn.generated.h"

class UFloatingPawnMovement;
class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;


UCLASS()
class CITYBUILDER_API ACityBuilderCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ACityBuilderCameraPawn();

protected:
	virtual void BeginPlay() override;
 
public:
	virtual void Tick(float DeltaTime) override;
 
	// ─── Composants ───────────────────────────────────────────────────────────
 
	/** Racine visible (sphère invisible, posée au sol — c'est le pivot d'orbite) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USceneComponent> PivotComponent;
 
	/** Spring arm partant du pivot vers le haut/arrière */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<USpringArmComponent> SpringArmComponent;
 
	/** Caméra au bout du spring arm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraComponent> CameraComponent;
 
	// ─── Paramètres de déplacement ────────────────────────────────────────────
 
	/** Vitesse de déplacement au sol (unités/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float MoveSpeed = 1200.f;
 
	/** Multiplicateur de vitesse selon l'altitude de la caméra */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	bool bScaleMoveSpeedWithZoom = true;
 
	// ─── Paramètres d'orbite ──────────────────────────────────────────────────
 
	/** Sensibilité de la rotation horizontale (Yaw) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitYawSensitivity = 0.99f;
 
	/** Sensibilité de la rotation verticale (Pitch) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float OrbitPitchSensitivity = 0.99f;
 
	/** Pitch minimum (en degrés, valeur négative = vue de dessus) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float MinPitch = -80.f;
 
	/** Pitch maximum */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Orbit")
	float MaxPitch = -5.f;
 
	// ─── Paramètres de zoom ───────────────────────────────────────────────────
 
	/** Distance minimale du spring arm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MinArmLength = 40.f;
 
	/** Distance maximale du spring arm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float MaxArmLength = 6000.f;
 
	/** Vitesse d'interpolation du zoom */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float ZoomInterpSpeed = 8.f;
 
	/** Pas de zoom par cran de molette */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Zoom")
	float ZoomStep = 400.f;
 
	// ─── Fonctions appelées par le PlayerController ───────────────────────────
 
	/** Déplacement dans le plan XY, relatif à l'orientation de la caméra */
	void Move(const FInputActionValue& Value);
 
	/** Activation / désactivation du mode orbite (appui clic maintenu) */
	void StartOrbit();
	void StopOrbit();
 
	/** Rotation orbite : reçoit le delta souris quand l'orbite est active */
	void Orbit(const FInputActionValue& Value);
 
	/** Zoom molette */
	void Zoom(const FInputActionValue& Value);
 
private:
	/** Vecteur de déplacement accumulé depuis le handler Move(), consommé dans Tick() */
	FVector2D PendingMove = FVector2D::ZeroVector;
 
	/** Yaw courant du pivot (rotation autour de Z) */
	float CurrentYaw = 0.f;
 
	/** Pitch courant du spring arm */
	float CurrentPitch = -45.f;
 
	/** Distance cible du spring arm (interpolée) */
	float TargetArmLength = 2000.f;
 
	/** Vrai quand le bouton orbite est maintenu */
	bool bOrbitActive = false;
};
