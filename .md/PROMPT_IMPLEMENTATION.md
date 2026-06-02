# 꿀 숙성도 시스템 구현 프롬프트

## 목표

벌통의 `HoneyProduction` 시간 bucket에서 꿀 생산 전에 가득 찬 소비장의 꿀 숙성도를 증가시키는 시스템을 구현한다.

이번 설계의 핵심은 다음과 같다.

- 꿀 숙성도 상태 owner는 `ABeehiveCombActor`다.
- 벌통의 시간 bucket 처리 owner는 기존처럼 `ABeehive`다.
- 숙성도 내부값은 꿀 양과 동일하게 절대값으로 저장한다.
  - `CurrentHoney`: `0..MaxHoneyPerComb`, 기본 최대 `100.0f`
  - `CurrentHoneyRipeness`: `0..MaxHoneyRipeness`, 기본 최대 `100.0f`
- 머터리얼에는 정규화된 `0..1` 값을 전달한다.
  - `HoneyAmount = CurrentHoney / MaxHoneyPerComb`
  - `HoneyRipeness = CurrentHoneyRipeness / MaxHoneyRipeness`
- 숙성은 `ApplyHoneyProductionUpdate()` 호출 전에 수행한다.
- `ApplyHoneyProductionUpdate()`를 직접 호출하면 꿀 생산만 수행해야 한다. 숙성+생산 순서는 bucket event handler가 책임진다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

필요 시 함께 확인한다.

- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

## 구현 기준

`HoneyProduction` bucket event 처리 순서는 반드시 아래와 같다.

```text
OnGameTimeBucketEvent("HoneyProduction")
1. ApplyHoneyRipenessUpdate()
2. ApplyHoneyProductionUpdate()
```

의미:

- 숙성 판정은 이번 꿀 생산 전 시점에 이미 꿀이 가득 찬 소비장 기준이다.
- 이번 bucket에서 꿀이 차서 full이 된 소비장은 같은 bucket에서 바로 숙성하지 않는다.
- 그 소비장은 다음 `HoneyProduction` bucket부터 숙성 대상이다.
- 같은 업데이트 시기에 하나의 소비장에서 꿀 증가와 숙성 증가가 동시에 일어나지 않아야 한다.
- 직접 `ABeehive::ApplyHoneyProductionUpdate()`를 호출하는 경로는 기존 의미대로 꿀 생산만 수행한다.

숙성 대상:

- 벌통이 관리하는 active comb 전체
- lifted comb 포함
- empty slot 제외
- 꿀이 full 상태인 comb만 대상

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- 필요 시 `.md/USER_UNREAL.md`
- 필요 시 `.md/PROMPT_REVIEW.md`

수정 금지:

- Content `.uasset` 직접 수정/저장
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect가 필요한 rename
- `ApplyHoneyProductionUpdate()`에 숙성 처리를 섞는 방식
- 별도 `HoneyRipeness` bucket subscription 추가

## `ABeehiveCombActor` 변경

꿀 숙성도 상태를 추가한다.

권장 property:

```cpp
UPROPERTY(EditAnywhere, Category = "Beehive|Honey Ripeness", meta = (ClampMin = "0.0"))
float MaxHoneyRipeness = 100.0f;

UPROPERTY(VisibleAnywhere, Category = "Beehive|Honey Ripeness", meta = (ClampMin = "0.0"))
float CurrentHoneyRipeness = 0.0f;

UPROPERTY(EditAnywhere, Category = "Beehive|Honey Ripeness")
FName HoneyRipenessMaterialParameterName = TEXT("HoneyRipeness");
```

권장 public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Ripeness")
void AddHoneyRipeness(float DeltaRipeness);

UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Ripeness")
void SetCurrentHoneyRipeness(float NewHoneyRipeness);

UFUNCTION(BlueprintPure, Category = "Beehive|Honey Ripeness")
float GetCurrentHoneyRipeness() const;

UFUNCTION(BlueprintPure, Category = "Beehive|Honey Ripeness")
float GetHoneyRipenessRatio() const;

UFUNCTION(BlueprintPure, Category = "Beehive|Honey")
bool IsHoneyFull() const;
```

권장 private helper:

```cpp
void SanitizeHoneyRipenessState();
```

동작:

- `MaxHoneyRipeness`는 `KINDA_SMALL_NUMBER` 이상으로 sanitize한다.
- `CurrentHoneyRipeness`는 `0..MaxHoneyRipeness`로 clamp한다.
- `AddHoneyRipeness`는 음수 입력을 무시하거나 `Max(0)` 처리한다.
- `GetHoneyRipenessRatio()`는 `Clamp(CurrentHoneyRipeness / MaxHoneyRipeness, 0..1)`을 반환한다.
- `IsHoneyFull()`은 `GetHoneyFillRatio() >= 1.0f - KINDA_SMALL_NUMBER` 기준으로 판단한다.

기존 honey visual 적용 경로에 숙성도 머터리얼 파라미터를 추가한다.

- `ApplyHoneyVisualState()`에서 `EnsureHoneyMaterialInstances()` 후 front/back MID 양쪽에 적용
- 기존 `HoneyAmount` 적용은 유지
- 신규 scalar parameter:

```cpp
FrontHoneyMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, GetHoneyRipenessRatio());
BackHoneyMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, GetHoneyRipenessRatio());
```

주의:

- 머터리얼에 `HoneyRipeness` scalar parameter가 없으면 시각 변화가 없을 수 있다. C++은 parameter 주입만 담당한다.
- `FrontHoneyPlane`과 `BackHoneyPlane` 둘 다 같은 숙성도 ratio를 받는다.
- 숙성도는 face별 상태가 아니라 소비장 전체 상태다.

## `ABeehive` 변경

벌통에 숙성 증가량 설정과 update API를 추가한다.

권장 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Ripeness", meta = (ClampMin = "0.0"))
float HoneyRipenessIncreasePerBucket = 1.0f;
```

권장 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Ripeness")
void ApplyHoneyRipenessUpdate();
```

동작:

- `HoneyRipenessIncreasePerBucket <= 0`이면 no-op
- active comb 전체를 순회한다.
- `CombActor->IsHoneyFull()`인 comb만 `AddHoneyRipeness(HoneyRipenessIncreasePerBucket)` 호출
- empty slot은 제외
- lifted comb도 포함
- 숙성도가 이미 max이면 clamp로 유지

`OnGameTimeBucketEvent_Implementation`의 `HoneyProduction` branch를 수정한다.

```cpp
else if (Event.SubscriptionTag == FName(TEXT("HoneyProduction")))
{
	ApplyHoneyRipenessUpdate();
	ApplyHoneyProductionUpdate();
}
```

주의:

- `GetGameTimeBucketSubscriptions_Implementation`에는 새 subscription을 추가하지 않는다.
- `HoneyProductionBucketMinutes`와 `bApplyHoneyProductionOnBeginPlayBucket`의 기존 subscription을 그대로 사용한다.
- 같은 60분 경계에서 기존 순서인 `HoneyProduction` before `ColonyPopulation`은 유지한다.
- `ApplyHoneyProductionUpdate()` 내부에는 `ApplyHoneyRipenessUpdate()` 호출을 넣지 않는다.

## `FBeehiveCombItemState` 변경

소비장 회수/재배치 시 숙성도도 보존한다.

권장 field:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0.0"))
float HoneyRipeness = 0.0f;
```

기존 API는 삭제하거나 시그니처 변경하지 않는다.

기존 유지:

```cpp
void SetBeehiveCombState(float HoneyAmount, bool bIsFrontFaceVisible);
```

신규 추가:

```cpp
UFUNCTION(BlueprintCallable, Category = "Item|Beehive Comb")
void SetBeehiveCombStateWithRipeness(float HoneyAmount, float HoneyRipeness, bool bIsFrontFaceVisible);
```

권장 동작:

- 기존 `SetBeehiveCombState(HoneyAmount, bIsFrontFaceVisible)`는 `HoneyRipeness=0.0f`로 새 helper를 호출한다.
- `SetBeehiveCombStateWithRipeness(...)`는 `HoneyAmount`, `HoneyRipeness`를 각각 `Max(0.0f, Value)`로 저장한다.
- `ClearBeehiveCombState()`는 기존처럼 struct 전체를 기본값으로 reset한다.

`ABeehiveCombActor`의 item state 입출력을 갱신한다.

- `ApplyStateFromItemInstance(...)`
  - `SetCurrentHoney(CombState.HoneyAmount)`
  - `SetCurrentHoneyRipeness(CombState.HoneyRipeness)`
  - visible face 복원 유지
- `WriteStateToItemInstance(...)`
  - `SetBeehiveCombStateWithRipeness(CurrentHoney, CurrentHoneyRipeness, bIsFrontFaceVisible)` 호출

주의:

- 저장되는 `HoneyRipeness`는 정규화값이 아니라 내부 절대값이다.
- 기존 소비장 state 보존 invariant인 `MaxStack == 1` 정책은 유지한다.

## 문서 갱신

구현 후 실제 API 이름과 일치하도록 아래 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - honey production bucket에서 숙성 후 생산 순서 반영
  - 숙성도 내부값/머터리얼 정규화 정책 요약
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehiveCombActor` honey ripeness 상태/API/material parameter
  - `ABeehive::ApplyHoneyRipenessUpdate`
  - `HoneyProduction` bucket branch 순서
- `.md/Architecture/InventorySystem.md`
  - `FBeehiveCombItemState::HoneyRipeness`
  - 회수/재배치 state 보존 범위에 숙성도 추가
- `.md/USER_UNREAL.md` 필요 시
  - honey material에 scalar parameter `HoneyRipeness` 필요
  - `BP_HoneyComb` / honey material 수동 확인 항목

문서 갱신은 구현 범위에 포함한다. 단, Content asset은 직접 수정하지 않는다.

## 검색 검증

```powershell
rg "HoneyRipeness|CurrentHoneyRipeness|MaxHoneyRipeness|ApplyHoneyRipenessUpdate|SetBeehiveCombStateWithRipeness" Source/BeekeepingSim .md
rg "HoneyProduction|ApplyHoneyProductionUpdate|OnGameTimeBucketEvent" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors .md
rg "FBeehiveCombItemState|SetBeehiveCombState|ApplyStateFromItemInstance|WriteStateToItemInstance" Source/BeekeepingSim .md
```

확인할 것:

- `ApplyHoneyProductionUpdate()` 직접 호출 경로는 숙성을 수행하지 않는다.
- `HoneyProduction` bucket branch에서만 `ApplyHoneyRipenessUpdate()` 후 `ApplyHoneyProductionUpdate()` 순서가 실행된다.
- 이번 bucket에서 처음 full이 된 소비장은 같은 bucket에서 숙성되지 않는다.
- full 상태였던 소비장은 bucket마다 `HoneyRipenessIncreasePerBucket`만큼 숙성된다.
- `HoneyRipeness` material parameter에는 `0..1` 정규화값이 전달된다.
- item state에는 절대값 `HoneyRipeness`가 저장된다.
- 기존 `SetBeehiveCombState(float, bool)` 시그니처가 유지된다.
- UCLASS/USTRUCT/UENUM rename이 없다.
- Core Redirect가 필요하지 않다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 수동 검증 항목

Editor/PIE에서 확인할 항목:

- 꿀이 가득 차지 않은 소비장이 이번 bucket에서 full이 되어도 같은 bucket에서는 숙성도가 증가하지 않는다.
- 이미 full이었던 소비장은 다음 `HoneyProduction` bucket에서 숙성도가 증가한다.
- `ApplyHoneyProductionUpdate()`를 수동 호출하면 꿀 생산만 수행되고 숙성도는 증가하지 않는다.
- 소비장 회수 후 재배치 시 `HoneyAmount`, `HoneyRipeness`, visible face가 모두 복원된다.
- `M_Honey` 또는 honey plane material에 `HoneyRipeness` scalar parameter가 있고, 정규화값에 따라 시각 변화가 발생한다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `ApplyHoneyProductionUpdate()`에 숙성을 섞지 않으면 구현이 불가능한 경우
- 기존 `SetBeehiveCombState(float, bool)` 시그니처 변경이 필요해지는 경우
- `UCLASS`/`USTRUCT`/`UENUM` rename이 필요해지는 경우
- Core Redirect 필요 여부가 불명확한 경우
- Content asset 수정/저장이 C++ 구현 완료 조건이 되는 경우
- 숙성도를 face별로 나누지 않으면 구현이 불가능한 경우
- 별도 `HoneyRipeness` bucket subscription이 필요하다고 판단되는 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Content 수동 작업 목록
- Core Redirect 불필요 여부
- `ApplyHoneyProductionUpdate()` 직접 호출은 꿀 생산만 수행한다는 확인
