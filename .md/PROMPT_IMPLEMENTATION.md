# 훈연기와 벌통 공격성 구현 프롬프트

## 목표

기존 소독약(`UDisinfectantUseAction`)과 동일한 FocusEngaged item-use-area hold-use 구조로 **훈연기 사용 시 해당 벌통의 공격성 수치가 내려가는 기능**을 구현한다.

이번 작업 범위는 **공격성 수치와 훈연기 동작만**이다.

구현하지 않는 것:

- 벌 공격/피해/공격력 시스템
- `EffectiveAttackPower`, `BaseAttackPower`, 공격력 multiplier API
- 공격성 자동 회복
- 시간 bucket 기반 aggression recovery
- Tick 기반 aggression recovery
- 훈연기 stack/durability/fuel 소모
- Content asset 직접 수정/저장

## 확정 QnA

반드시 `.md/QNA_ARCHITECTURE.md`의 `[훈연기와 벌통 공격성]` 답변을 기준으로 한다.

- 공격성 초기값: `AggressionValue = MaxAggressionValue`, 기본 100/100
- 공격성 자동 회복: 없음
- 공격력 계산/구현: 이번 범위에서 제외
- 훈연기 use-area: 기존 벌통 item-use-area에 `Item.UseArea.Beehive.Smoker` 태그를 추가하는 방식
- 훈연기 자원 소모: 1차 구현에서는 없음

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/Inventory/SmokerUseAction.h` 신규
- `Source/BeekeepingSim/Private/Inventory/SmokerUseAction.cpp` 신규

Config:

- `Config/DefaultGameplayTags.ini`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/USER_UNREAL.md`

수정 금지:

- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect가 필요한 변경
- 공격력/피해 시스템 추가
- Content `.uasset` 직접 수정

## 벌통 공격성 상태

`ABeehive`가 공격성 상태의 owner다. 위생성(`SanitationValue`)처럼 벌통 actor 내부 상태로 둔다.

권장 public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Aggression")
void DecreaseAggression(float Delta);

UFUNCTION(BlueprintCallable, Category = "Beehive|Aggression")
void SetAggressionValue(float NewValue);

UFUNCTION(BlueprintPure, Category = "Beehive|Aggression")
float GetAggressionValue() const { return AggressionValue; }

UFUNCTION(BlueprintPure, Category = "Beehive|Aggression")
float GetAggressionRatio() const;
```

권장 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Aggression", meta = (ClampMin = "0.0"))
float MaxAggressionValue = 100.0f;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Aggression", meta = (ClampMin = "0.0"))
float AggressionValue = 100.0f;
```

동작:

- 기본값은 `MaxAggressionValue=100`, `AggressionValue=100`.
- `DecreaseAggression(Delta)`는 `Delta <= 0`이면 no-op, 아니면 `SetAggressionValue(AggressionValue - Delta)`.
- `SetAggressionValue(NewValue)`는 `0..MaxAggressionValue`로 clamp한다.
- `GetAggressionRatio()`는 `MaxAggressionValue <= KINDA_SMALL_NUMBER`이면 0, 아니면 `AggressionValue / MaxAggressionValue`를 `0..1`로 clamp한다.
- `OnConstruction`/`BeginPlay` 등 기존 초기화 흐름에서 필요하면 현재 값을 clamp한다. 단, BeginPlay에서 항상 max로 reset해서 placed instance authoring 값을 덮어쓰면 안 된다.

주의:

- 공격력 계산용 API/필드는 추가하지 않는다.
- 공격성 회복 subscription을 `UGameTimeBucketSubsystem`에 추가하지 않는다.
- 상태 변경 delegate는 1차 구현 범위에 넣지 않는다. Blueprint 표시가 필요해지면 별도 설계로 확장한다.

## 훈연기 Use Action

`USmokerUseAction`을 `UHoldItemUseAction` subclass로 추가한다. 구조는 `UDisinfectantUseAction`을 기준으로 한다.

권장 header:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Inventory/HoldItemUseAction.h"
#include "SmokerUseAction.generated.h"

class ABeehive;

UCLASS(Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API USmokerUseAction : public UHoldItemUseAction
{
	GENERATED_BODY()

public:
	USmokerUseAction();

	virtual bool BeginUse(const FItemActionContext& Context) override;
	virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled) override;
	virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime) override;

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Smoker")
	void ReceiveSmokerUseStarted(const FItemActionContext& Context);

	UFUNCTION(BlueprintImplementableEvent, Category = "Smoker")
	void ReceiveSmokerUseEnded(const FItemActionContext& Context, bool bWasCanceled);

private:
	ABeehive* ResolveTargetBeehive(const FItemActionContext& Context) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Smoker", meta = (ClampMin = "0.0"))
	float AggressionDecreasePerSecond = 10.0f;
};
```

동작:

- 생성자에서 `Item.UseArea.Beehive.Smoker` tag query를 구성한다.
- `BeginUse`:
  - `Super::BeginUse(Context)` 실패 시 false
  - 소독약과 동일하게 `Context.Character->GetBeekeeperHeldItemVisualizer()->BeginHeldItemUseActive()` 호출
  - `ReceiveSmokerUseStarted(Context)` 호출
- `EndUse`:
  - 소독약과 동일하게 held item active 종료 호출
  - `ReceiveSmokerUseEnded(Context, bWasCanceled)` 호출
  - `Super::EndUse(Context, bWasCanceled)` 호출
- `ApplyUseEffect`:
  - `ResolveTargetBeehive(Context)` 실패 시 기본 실패 결과 반환
  - `DecreaseAmount = Max(0, AggressionDecreasePerSecond) * Max(0, DeltaTime)`
  - `Beehive->DecreaseAggression(DecreaseAmount)`
  - `Result.bSucceeded = true`
  - `Result.bConsumedItem = false` 유지
  - `StackDelta` 변경 없음

대상 resolve:

```cpp
if (ABeehive* ByEffectTarget = Cast<ABeehive>(Context.ItemUseEffectTargetObject))
{
	return ByEffectTarget;
}
return Cast<ABeehive>(Context.FocusEngagedHostActor);
```

## Gameplay Tag

`Config/DefaultGameplayTags.ini`에 아래 tag를 추가한다.

```ini
+GameplayTagList=(Tag="Item.UseArea.Beehive.Smoker",DevComment="")
```

주의:

- `USmokerUseAction`은 `RequestGameplayTag(FName(TEXT("Item.UseArea.Beehive.Smoker")), false)` 패턴을 기존 action들과 맞춘다.
- tag가 등록되지 않으면 query가 비어 의도치 않게 넓은 area와 매칭될 수 있으므로, config 추가와 검색 검증을 반드시 수행한다.
- 기존 `Disinfectant`, `BeeBrush`, `PollenPatty` tag 정책은 이번 작업에서 리팩토링하지 않는다.

## Blueprint/Unreal 수동 작업

C++ 구현은 Content asset을 수정하지 않는다. 필요한 에디터 작업은 `.md/USER_UNREAL.md` 또는 최종 보고에 명시한다.

필요 수동 작업:

- 훈연기 item definition asset 생성 또는 기존 asset 수정
  - ActionSpec에 `USmokerUseAction` 추가
  - Held presentation actor는 smoke VFX가 있는 `AVfxItemPresentationActor` 계열 BP 권장
- 벌통 BP의 기존 item-use-area component에 `Item.UseArea.Beehive.Smoker` 태그 추가
  - 소독약과 같은 영역을 쓰려면 같은 `UItemUseAreaMeshComponent.AreaTags`에 Smoker tag를 추가한다.
  - `EffectTargetPolicy`는 벌통 host를 target으로 resolve할 수 있는 기존 정책을 유지한다.

## 문서 갱신

구현 후 실제 API 이름과 일치하도록 아래 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - `ABeehive`가 공격성 수치를 소유하고 훈연기로 감소시킨다는 요약 추가
  - 공격력 계산은 이번 범위 제외임을 혼동 없게 기록
- `.md/Architecture/InventorySystem.md`
  - `USmokerUseAction` 추가
  - hold-use 중 벌통 공격성 감소, item 소비 없음 기록
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehive` aggression 상태/API 기록
  - item-use 확장에 smoker/aggression 항목 추가
- `.md/USER_UNREAL.md`
  - Smoker item definition과 use-area tag 수동 설정 절차가 필요하면 추가

## 검색 검증

```powershell
rg "SmokerUseAction|AggressionValue|MaxAggressionValue|DecreaseAggression|GetAggressionRatio" Source/BeekeepingSim .md Config
rg "Item.UseArea.Beehive.Smoker" Source/BeekeepingSim .md Config
rg "EffectiveAttackPower|BaseAttackPower|AttackPower|MinAttackMultiplier" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

확인할 것:

- `USmokerUseAction`이 `UHoldItemUseAction` 기반으로 추가되었다.
- 훈연기 action이 소독약처럼 held item active lifecycle을 호출한다.
- 훈연기 action이 `ABeehive::DecreaseAggression`만 호출하고 item 소비를 하지 않는다.
- `Item.UseArea.Beehive.Smoker`가 config와 문서에 존재한다.
- 공격력 관련 API/필드가 추가되지 않았다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 훈연기 구현에 공격력/피해 시스템 API가 필요해지는 경우
- 공격성 회복 정책을 넣지 않으면 동작을 완성할 수 없는 경우
- `ABeehive`에 aggression 상태를 추가하려면 기존 Blueprint-exposed API rename/delete가 필요해지는 경우
- gameplay tag 추가가 Config 변경만으로 부족하고 Content asset migration이 필요한 경우
- `USmokerUseAction` 추가가 기존 item action 생성/serialization 경로와 충돌하는 경우
- Content asset compile/save가 구현 완료 조건이 되는 경우
