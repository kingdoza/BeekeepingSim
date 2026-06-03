# 벌통 위생 기반 질병/벌 수 감소 구현 프롬프트

## 목표

벌통의 위생도(`SanitationValue`)가 임계값보다 낮아졌을 때 두 가지 효과를 적용한다.

1. colony population 감소량에 위생 기반 감소 배율을 반영한다.
2. 위생도 기반 질병값(`Disease`, `0..1`)을 벌통/벌떼/소비장/여왕벌 시각 파라미터에 즉시 반영한다.

이번 기능의 source of truth는 `ABeehive`의 위생 상태다.

- 위생 상태 owner: `ABeehive`
- 질병값 계산 owner: `ABeehive`
- 벌 수 감소 계산 owner: `ABeehive::CalculateBeeDecreaseAmount()`
- Disease 시각값 전파 owner: `ABeehive`
- 소비장 front/back Niagara 적용 owner: `ABeehiveCombActor`
- outgoing/ingoing Niagara 적용 owner: `ABeehiveDualSwarmActor`
- 여왕벌 material 적용 owner: `AQueenBeeActor`

## 확정 설계

위생 임계값과 질병값 매핑:

```text
EffectiveThreshold = Clamp(SanitationDiseaseThreshold, 0, MaxSanitationValue)

if EffectiveThreshold <= 0:
  DiseaseRatio = 0
else if SanitationValue >= EffectiveThreshold:
  DiseaseRatio = 0
else:
  DiseaseRatio = Clamp((EffectiveThreshold - SanitationValue) / EffectiveThreshold, 0, 1)
```

의미:

- `SanitationValue >= SanitationDiseaseThreshold`이면 질병값은 `0`
- `SanitationValue == 0`이면 질병값은 `1`
- 임계값과 0 사이에서는 선형 보간

벌 수 감소 배율:

```text
SanitationDecreaseMultiplier =
  Lerp(1.0, MaxSanitationBeeDecreaseMultiplier, DiseaseRatio)
```

colony population 감소식:

```text
ProportionalDecrease = ColonyBeeCount * BeeDecreaseCoefficient
AbsoluteDecrease = BeeDecreaseAbsoluteAmountPerBucket

BaseDecrease =
  ((ProportionalDecrease + AbsoluteDecrease)
    / ItemLifespanBonus
    / TemperatureScore)
  * SanitationDecreaseMultiplier

Decrease = Min(ColonyBeeCount, BaseDecrease)
```

최종 벌 수 적용은 기존처럼 `ApplyColonyPopulationUpdate()`에서 마지막 단계에만 반올림한다.

```text
NewBeeCount = Max(0, RoundToInt(ColonyBeeCount + Increase - Decrease))
```

주의:

- `SanitationDecreaseMultiplier`는 `BaseDecrease` 계산 단계에서 반영한다.
- `Decrease = Min(ColonyBeeCount, BaseDecrease)` clamp는 유지한다.
  - 같은 bucket에서 새로 증가한 벌까지 자연감소가 먹지 않게 하기 위해서다.
- `ItemEggLayingBonus`는 기존처럼 증가 항에만 적용한다.
- `ItemLifespanBonus`와 `TemperatureScore`의 현재 `1.0f` placeholder 정책은 유지한다.

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

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/USER_UNREAL.md`
- 필요 시 `.md/PROMPT_REVIEW.md`

수정 금지:

- `Content/` asset 직접 수정/저장
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect가 필요한 rename
- `UGameTimeBucketSubsystem` 또는 Environment 시간 bucket 구조 변경
- 별도 disease tick actor/subsystem/component 추가
- 별도 disease bucket subscription 추가

## `ABeehive` 변경

### 새 설정값

`Beehive|Sanitation Disease` 카테고리에 추가한다.

권장 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Sanitation Disease", meta = (ClampMin = "0.0"))
float SanitationDiseaseThreshold = 0.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Sanitation Disease", meta = (ClampMin = "1.0"))
float MaxSanitationBeeDecreaseMultiplier = 1.0f;
```

기본값 의미:

- `SanitationDiseaseThreshold = 0.0f`: 기존 동작 유지, disease ratio 항상 0
- `MaxSanitationBeeDecreaseMultiplier = 1.0f`: 감소 배율 변화 없음

### 새 public API

권장 API:

```cpp
UFUNCTION(BlueprintPure, Category = "Beehive|Sanitation Disease")
float GetSanitationDiseaseRatio() const;

UFUNCTION(BlueprintPure, Category = "Beehive|Sanitation Disease")
float GetSanitationBeeDecreaseMultiplier() const;
```

동작:

- `GetSanitationDiseaseRatio()`는 위 확정 설계의 `DiseaseRatio`를 반환한다.
- `GetSanitationBeeDecreaseMultiplier()`는 `Lerp(1.0f, MaxSanitationBeeDecreaseMultiplier, DiseaseRatio)`를 반환한다.
- `MaxSanitationBeeDecreaseMultiplier`는 최소 `1.0f`로 sanitize해서 사용한다.

### 감소식 변경

`CalculateBeeDecreaseAmount()`는 기존 비례/절대 감소량 계산 후 `SanitationDecreaseMultiplier`를 `BaseDecrease` 계산 단계에서 반영한다.

권장 구현 형태:

```cpp
float ABeehive::CalculateBeeDecreaseAmount() const
{
	const float CurrentBeeCount = static_cast<float>(FMath::Max(0, ColonyBeeCount));
	const float DecreaseCoefficient = FMath::Max(0.0f, BeeDecreaseCoefficient);
	const float AbsoluteDecreaseAmount = FMath::Max(0.0f, BeeDecreaseAbsoluteAmountPerBucket);
	const float ItemLifespanBonus = FMath::Max(KINDA_SMALL_NUMBER, GetItemLifespanBonus());
	const float TemperatureScore = FMath::Max(KINDA_SMALL_NUMBER, GetTemperatureScore());
	const float SanitationDecreaseMultiplier = GetSanitationBeeDecreaseMultiplier();

	const float BaseDecrease = (((CurrentBeeCount * DecreaseCoefficient) + AbsoluteDecreaseAmount)
		/ ItemLifespanBonus
		/ TemperatureScore)
		* SanitationDecreaseMultiplier;

	return FMath::Min(CurrentBeeCount, FMath::Max(0.0f, BaseDecrease));
}
```

유지할 것:

- `ApplyColonyPopulationUpdate()`의 최종 `RoundToInt` 정책
- `NewBeeCount` 최소 0 clamp
- `ColonyPopulation` bucket subscription과 `LatestOnly` catch-up policy
- `CalculateBeeIncreaseAmount()` 계산식
- `GetItemEggLayingBonus()` 정책
- `GetItemLifespanBonus()` / `GetTemperatureScore()`의 현재 `1.0f` 반환

### Disease 시각값 전파

위생값 변경 시 질병 표현이 즉시 반영되어야 한다.

흐름:

```text
IncreaseSanitation(...)
  -> SetSanitationValue(...)
    -> SanitationValue clamp
    -> RefreshHiveDiseaseVisuals()
```

권장 private helper:

```cpp
void RefreshHiveDiseaseVisuals();
void ApplyDiseaseToCombActors(float DiseaseRatio);
void ApplyDiseaseToQueenBee(float DiseaseRatio);
```

`RefreshHiveDiseaseVisuals()`는 다음을 수행한다.

1. `DiseaseRatio = GetSanitationDiseaseRatio()`
2. `AttractionSwarmNiagara->SetVariableFloat(TEXT("User.Disease"), DiseaseRatio)`
3. `ApplySettingsToDualSwarmChildActor()` 또는 child actor 직접 API를 통해 outgoing/ingoing disease 갱신
4. active comb 전체에 `CombActor->SetBeeDiseaseValue(DiseaseRatio)` 호출
5. `AQueenBeeActor::SetDiseaseValue(DiseaseRatio)` 호출

호출 시점:

- `SetSanitationValue()`에서 값 clamp 후 즉시 호출
- `OnConstruction()`에서 `SetSanitationValue(SanitationValue)` 또는 `RefreshHiveDiseaseVisuals()` 호출
- `BeginPlay()`에서 `SetSanitationValue(SanitationValue)` 또는 `RefreshHiveDiseaseVisuals()` 호출
- `PostEditChangeProperty()` 후 refresh 경로에서 호출
- `RefreshCombStateFromSlots()` 또는 comb 배치/회수 후 새 active comb에 disease가 반영되도록 호출

주의:

- 위생값 변경 때마다 spawn amount를 불필요하게 재계산하지 않도록 가능하면 disease 전용 전파 helper를 둔다.
- 단, `FBeehiveDualSwarmNiagaraParameters`에 `Disease`를 포함하는 방식이면 `ApplySettingsToDualSwarmChildActor()` 재호출도 허용한다.
- 어떤 방식을 택하든 위생값 변경 시 outgoing/ingoing disease 값이 즉시 갱신되어야 한다.

## `BeeSwarmTypes` / `ABeehiveDualSwarmActor` 변경

`FBeehiveDualSwarmNiagaraParameters`에 Disease 값을 추가한다.

권장 field:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bee Swarm", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float Disease = 0.0f;
```

`ABeehive::BuildDualSwarmParameters()`에서 설정한다.

```cpp
Parameters.Disease = GetSanitationDiseaseRatio();
```

`ABeehiveDualSwarmActor.cpp` namespace에 parameter name을 추가한다.

```cpp
static const FName Disease(TEXT("User.Disease"));
```

`ApplyDualSwarmParameters(...)`에서 outgoing/ingoing 양쪽에 적용한다.

```cpp
const float DiseaseValue = FMath::Clamp(Parameters.Disease, 0.0f, 1.0f);
OutgoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::Disease, DiseaseValue);
IngoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::Disease, DiseaseValue);
```

## `ABeehiveCombActor` 변경

소비장 front/back 벌떼 Niagara에 `User.Disease`를 적용한다.

권장 public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Comb Disease")
void SetBeeDiseaseValue(float NewDiseaseValue);

UFUNCTION(BlueprintPure, Category = "Beehive|Comb Disease")
float GetBeeDiseaseValue() const { return BeeDiseaseValue; }
```

권장 private/state:

```cpp
UPROPERTY(VisibleAnywhere, Category = "Beehive|Comb Disease", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float BeeDiseaseValue = 0.0f;
```

`BeehiveCombActorNames`에 추가한다.

```cpp
static const FName Disease(TEXT("User.Disease"));
```

`ApplyNiagaraUserParameters()`에서 front/back 양쪽에 적용한다.

```cpp
FrontFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, BeeDiseaseValue);
BackFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, BeeDiseaseValue);
```

`SetBeeDiseaseValue(...)`는 `0..1` clamp 후 `ApplyNiagaraUserParameters()`를 호출한다.

주의:

- 기존 `ApplyCombBeeParameters(...)` 시그니처는 변경하지 않는다.
- 기존 `SetTotalSpawnAmount...` API 시그니처도 변경하지 않는다.
- Disease는 소비장 상태 보존 item state에 저장하지 않는다. 벌통 위생 상태에서 다시 계산되는 시각값이다.
- 독립 배치된 comb actor는 기본 disease `0`으로 동작한다.

## `AQueenBeeActor` 변경

여왕벌 mesh material에 `Disease` scalar parameter를 적용한다.

권장 public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Queen Bee|Disease")
void SetDiseaseValue(float NewDiseaseValue);

UFUNCTION(BlueprintPure, Category = "Queen Bee|Disease")
float GetDiseaseValue() const { return DiseaseValue; }
```

권장 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Queen Bee|Disease")
FName DiseaseMaterialParameterName = TEXT("Disease");

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Queen Bee|Disease", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float DiseaseValue = 0.0f;
```

권장 private helper:

```cpp
void EnsureDiseaseMaterialInstances();
void ApplyDiseaseMaterialParameter();
```

구현 기준:

- `SetDiseaseValue(...)`는 `0..1` clamp 후 material parameter를 적용한다.
- `QueenBeeMesh`의 material slot 전체에 `UMaterialInstanceDynamic`을 생성/유지한다.
- 모든 material slot에 `DiseaseMaterialParameterName` scalar를 주입한다.
- `BeginPlay()`와 `OnConstruction()`에서도 현재 `DiseaseValue`를 적용한다.
- material에 `Disease` scalar parameter가 없으면 시각 변화가 없을 수 있으나 C++은 주입만 담당한다.

주의:

- `AQueenBeeActor` rename 금지
- `QueenBeeMesh` component rename 금지
- 기존 `GetBaseEggLayingPower()` 정책 변경 금지
- Tick yaw jitter 정책 변경 금지

## `ABeehive` 적용 세부 사항

`SetSanitationValue()`는 위생값 clamp의 단일 진입점이어야 한다.

권장 형태:

```cpp
void ABeehive::SetSanitationValue(float NewValue)
{
	const float SafeMax = FMath::Max(0.0f, MaxSanitationValue);
	SanitationValue = FMath::Clamp(NewValue, 0.0f, SafeMax);
	RefreshHiveDiseaseVisuals();
}
```

`IncreaseSanitation()`은 기존처럼 `SetSanitationValue(SanitationValue + Delta)`를 호출한다.

`ApplyAttractionSwarmSettings()`에 `User.Disease` 적용을 추가한다.

```cpp
AttractionSwarmNiagara->SetVariableFloat(TEXT("User.Disease"), GetSanitationDiseaseRatio());
```

`RefreshCombSpawnAmounts(...)`는 spawn/target 갱신 후 해당 comb에 현재 disease도 주입해야 한다.

```cpp
CombActor->SetBeeDiseaseValue(GetSanitationDiseaseRatio());
```

또는 `RefreshHiveDiseaseVisuals()`가 comb 전체를 순회하도록 하고, comb 배치/회수/초기화 경로 끝에서 호출한다.

중복 호출은 허용되지만 source of truth는 항상 `GetSanitationDiseaseRatio()`여야 한다.

## 계산 예시

예시 1: 질병 없음

```text
MaxSanitationValue = 100
SanitationDiseaseThreshold = 50
SanitationValue = 80
DiseaseRatio = 0
SanitationDecreaseMultiplier = 1
```

예시 2: 절반 질병

```text
MaxSanitationValue = 100
SanitationDiseaseThreshold = 50
SanitationValue = 25
MaxSanitationBeeDecreaseMultiplier = 3

DiseaseRatio = (50 - 25) / 50 = 0.5
SanitationDecreaseMultiplier = Lerp(1, 3, 0.5) = 2
```

예시 3: 최대 질병

```text
MaxSanitationValue = 100
SanitationDiseaseThreshold = 50
SanitationValue = 0
MaxSanitationBeeDecreaseMultiplier = 3

DiseaseRatio = 1
SanitationDecreaseMultiplier = 3
```

예시 4: 감소량 clamp

```text
ColonyBeeCount = 3
Increase = 8
BaseDecrease = 10
Decrease = Min(3, 10) = 3
NewBeeCount = RoundToInt(3 + 8 - 3) = 8
```

## 문서 갱신

구현 후 실제 API 이름과 일치하도록 아래 문서를 갱신한다.

`.md/0_ARCHITECTURE.md`

- sanitation disease ratio 계산식 추가
- colony population 감소식에 `SanitationDecreaseMultiplier` 반영
- Disease 시각값 source of truth가 `ABeehive::GetSanitationDiseaseRatio()`임을 명시
- Disease가 attraction/outgoing/ingoing/comb front/back/queen material에 적용됨을 요약

`.md/Architecture/WorldActorsSystem.md`

- `ABeehive` sanitation disease 설정/API 추가
- `ABeehiveDualSwarmActor` `User.Disease` 적용 추가
- `ABeehiveCombActor` front/back `User.Disease` 적용 추가
- `AQueenBeeActor` material `Disease` scalar parameter 적용 추가
- 위생값 변경 시 `RefreshHiveDiseaseVisuals()` 경로로 즉시 반영됨을 명시

`.md/USER_UNREAL.md` 필요 시

- 벌통 attraction Niagara, outgoing Niagara, ingoing Niagara에 `User.Disease` float user parameter 필요
- 소비장 front/back bee Niagara에 `User.Disease` float user parameter 필요
- 여왕벌 material에 `Disease` scalar parameter 필요
- C++은 파라미터를 주입하지만, asset에 파라미터가 없으면 시각 변화가 없을 수 있음

문서 갱신은 구현 범위에 포함한다. 단, Content asset은 직접 수정하지 않는다.

## 검색 검증

```powershell
rg "SanitationDiseaseThreshold|MaxSanitationBeeDecreaseMultiplier|GetSanitationDiseaseRatio|GetSanitationBeeDecreaseMultiplier|RefreshHiveDiseaseVisuals" Source/BeekeepingSim .md
rg "User.Disease|DiseaseMaterialParameterName|SetDiseaseValue|BeeDiseaseValue|SetBeeDiseaseValue" Source/BeekeepingSim .md
rg "CalculateBeeDecreaseAmount|SanitationDecreaseMultiplier|BaseDecrease|BeeDecreaseAbsoluteAmountPerBucket" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors .md
```

확인할 것:

- 기존 BlueprintCallable/Public API 삭제 또는 rename이 없다.
- `ApplyCombBeeParameters(...)` 시그니처가 유지된다.
- `FBeehiveDualSwarmNiagaraParameters`는 field 추가만 수행한다.
- `DiseaseRatio`는 `0..1` 범위로 clamp된다.
- `MaxSanitationBeeDecreaseMultiplier`는 최소 `1.0`으로 처리된다.
- `SanitationDiseaseThreshold = 0`이면 disease ratio가 0이고 기존 감소식과 동일하다.
- 위생값 변경 시 `RefreshHiveDiseaseVisuals()`가 호출된다.
- attraction/outgoing/ingoing/comb front/back/queen 모두 같은 `DiseaseRatio`를 받는다.
- Core Redirect가 필요한 rename/delete가 없다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 수동 검증 항목

Editor/PIE에서 확인할 항목:

- `SanitationValue >= SanitationDiseaseThreshold`이면 모든 Disease 표현값이 0이다.
- `SanitationValue`가 임계값 아래로 내려가면 Disease 표현값이 `0..1`로 증가한다.
- 소독약 hold-use로 `SanitationValue`가 증가하면 Disease 표현값이 즉시 낮아진다.
- `SanitationValue = 0`이면 Disease 표현값이 1이다.
- `MaxSanitationBeeDecreaseMultiplier > 1`일 때 colony population bucket 감소량이 증가한다.
- 감소량이 커져도 `Decrease`는 기존 `ColonyBeeCount`를 초과하지 않는다.
- 벌통 attraction swarm, outgoing, ingoing, 소비장 front/back Niagara가 `User.Disease`를 받는다.
- 여왕벌 material이 `Disease` scalar parameter를 받는다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 기존 `CalculateBeeDecreaseAmount()` API 삭제/rename이 필요해지는 경우
- `ApplyCombBeeParameters(...)` 시그니처 변경이 필요해지는 경우
- `ABeehive`가 아닌 별도 disease owner가 필요하다고 판단되는 경우
- disease 전용 Tick/subsystem/bucket 없이는 구현이 불가능하다고 판단되는 경우
- `ItemLifespanBonus` 또는 `TemperatureScore` 의미 변경이 필요해지는 경우
- `AQueenBeeActor` component rename이 필요해지는 경우
- Content asset 수정/저장이 C++ 구현 완료 조건이 되는 경우
- UCLASS/USTRUCT/UENUM rename 또는 Core Redirect 필요성이 생기는 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Core Redirect 불필요 여부
- Blueprint/API 영향
- Content 수동 작업 목록
- 위생값 변경 시 Disease 표현이 즉시 갱신되는지 확인
- `SanitationDecreaseMultiplier`가 `BaseDecrease` 계산 단계에서 반영된다는 확인
- `Decrease = Min(ColonyBeeCount, BaseDecrease)` clamp 유지 확인
