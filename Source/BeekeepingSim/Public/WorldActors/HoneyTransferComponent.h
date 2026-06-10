#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoneyTransferComponent.generated.h"

class AHoneyContainerActor;
class AHoneyContainerSlotActor;
class UNiagaraComponent;

UENUM(BlueprintType)
enum class EHoneyTransferState : uint8
{
	Idle,
	GrowingDrop,
	Transferring
};

UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UHoneyTransferComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHoneyTransferComponent();

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Honey Transfer")
	void ConfigureSlots(AHoneyContainerSlotActor* SourceSlot, AHoneyContainerSlotActor* TargetSlot);

	UFUNCTION(BlueprintCallable, Category = "Honey Transfer")
	void SetHoneyStreamNiagara(UNiagaraComponent* NiagaraComponent);

	UFUNCTION(BlueprintPure, Category = "Honey Transfer")
	bool CanStartTransfer() const;

	UFUNCTION(BlueprintCallable, Category = "Honey Transfer")
	bool StartTransfer();

	UFUNCTION(BlueprintCallable, Category = "Honey Transfer")
	void StopTransfer(bool bImmediateVfx);

	UFUNCTION(BlueprintCallable, Category = "Honey Transfer")
	bool ToggleTransferFromNozzle(AHoneyContainerActor* SourceContainer);

	UFUNCTION(BlueprintPure, Category = "Honey Transfer")
	bool IsTransferActive() const { return TransferState != EHoneyTransferState::Idle; }

	UFUNCTION(BlueprintPure, Category = "Honey Transfer")
	EHoneyTransferState GetTransferState() const { return TransferState; }

	UFUNCTION(BlueprintPure, Category = "Honey Transfer")
	AHoneyContainerActor* GetActiveSourceContainer() const { return ActiveSourceContainer; }

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Transfer", meta = (ClampMin = "0.0"))
	float TransferRateMlPerSecond = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Transfer", meta = (ClampMin = "0.0"))
	float DropLengthGrowSpeedCmPerSecond = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Transfer", meta = (ClampMin = "0.0"))
	float DefaultDropLengthCm = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Transfer|Niagara")
	FName HoneyDensityNiagaraParameterName = TEXT("User.HoneyDensity");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Transfer|Niagara")
	FName HoneyRipenessNiagaraParameterName = TEXT("User.HoneyRipeness");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Honey Transfer|Niagara")
	FName DropLengthNiagaraParameterName = TEXT("User.DropLength");

private:
	bool ResolveConfiguredContainers(AHoneyContainerActor*& OutSourceContainer, AHoneyContainerActor*& OutTargetContainer) const;
	bool ValidateTransferConfiguration(AHoneyContainerActor*& OutSourceContainer, AHoneyContainerActor*& OutTargetContainer) const;
	bool ValidateActiveTransfer() const;
	float ResolveTargetDropLengthCm() const;
	UNiagaraComponent* ResolveActiveHoneyStreamNiagara() const;
	void ApplyNiagaraTransferParameters(float DropLengthCm) const;
	void ApplyDropLengthParameter(float DropLengthCm) const;
	void ActivateHoneyStream() const;
	void DeactivateHoneyStream(bool bImmediateVfx) const;
	void TickGrowingDrop(float DeltaTime);
	void TickTransfer(float DeltaTime);

	UPROPERTY(Transient)
	TObjectPtr<AHoneyContainerSlotActor> ConfiguredSourceSlot;

	UPROPERTY(Transient)
	TObjectPtr<AHoneyContainerSlotActor> ConfiguredTargetSlot;

	UPROPERTY(Transient)
	TObjectPtr<UNiagaraComponent> LegacyHoneyStreamNiagara;

	UPROPERTY(Transient)
	TObjectPtr<AHoneyContainerActor> ActiveSourceContainer;

	UPROPERTY(Transient)
	TObjectPtr<AHoneyContainerActor> ActiveTargetContainer;

	EHoneyTransferState TransferState = EHoneyTransferState::Idle;
	float CurrentDropLengthCm = 0.0f;
	float TargetDropLengthCm = 0.0f;
};
