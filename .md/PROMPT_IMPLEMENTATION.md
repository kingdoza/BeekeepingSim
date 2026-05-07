# Beehive Honey System 구현 프롬프트

## 목표

벌통에 60분 주기 꿀 생산 시스템을 추가한다.

핵심 요구사항:

- 게임 시간 기준 60분마다 꿀 생산 업데이트를 수행한다.
- 벌통의 총 꿀 증가량은 현재 벌 수 기준으로 계산한다.
- 총 꿀 증가량 공식:
  - `TotalHoneyIncrease = ColonyBeeCount * HoneyProductionCoefficient`
- 같은 bucket에서 꿀 생산과 벌 수 업데이트가 동시에 발생하면 꿀 생산을 먼저 처리하고, 그 다음 벌 수 업데이트를 처리한다.
- 소비장별 꿀 증가량에는 편차가 있어야 한다.
- 단, 소비장이 가득 차지 않은 상태에서는 소비장별 증가량 총합이 벌통의 총 꿀 증가량과 같아야 한다.
- 소비장이 최대 꿀 용량에 도달해 초과분이 생기면 초과분은 버린다.
- 꿀 업데이트는 소비장 들림 여부와 무관하게 모든 active 소비장에 적용한다.
- 소비장은 내부 절대 꿀 양을 저장하고, 시각 표현에는 정규화된 `0.0~1.0` fill ratio를 사용한다.

## 참조 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_ARCHITECTURE.md`

## 주요 파일

수정 대상:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

문서 반영 대상:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

## 설계 결정사항

### 초과 생산량

- 소비장별 꿀 적용 시 `MaxHoneyPerComb`을 초과하는 양은 버린다.
- 초과분 재분배나 벌통 저장고 누적은 하지 않는다.

### 내부 단위와 표시 단위

- 소비장 내부 꿀 양은 절대값으로 저장한다.
- 표시 비율은 다음 공식으로 계산한다.
  - `HoneyFillRatio = Clamp(CurrentHoney / MaxHoneyPerComb, 0.0, 1.0)`
- 머티리얼 scalar parameter `HoneyAmount`에는 절대값이 아니라 `HoneyFillRatio`를 넣는다.

### 기본값

- `MaxHoneyPerComb = 100.0f`
- `HoneyProductionCoefficient = 0.01f`
- `HoneyDistributionDeviationRatio = 0.5f`
- `HoneyProductionBucketMinutes = 60`
- `bApplyHoneyProductionOnBeginPlayBucket = false`

### 분배 방식

소비장별 증가량은 랜덤 가중치 정규화 방식으로 계산한다.

```text
Weight = RandomRange(1.0 - HoneyDistributionDeviationRatio, 1.0 + HoneyDistributionDeviationRatio)
CombHoneyIncrease = TotalHoneyIncrease * Weight / WeightSum
```

- `HoneyDistributionDeviationRatio`는 `0.0~1.0`으로 clamp한다.
- active 소비장 수가 0이면 아무것도 하지 않는다.
- `TotalHoneyIncrease <= 0`이면 아무것도 하지 않는다.
- 소비장이 가득 차서 일부 증가량이 버려지는 경우를 제외하면 적용 증가량 총합은 `TotalHoneyIncrease`와 같아야 한다.

### bucket 처리 순서

`ABeehive::GetGameTimeBucketSubscriptions_Implementation`에서 `HoneyProduction` subscription을 `ColonyPopulation` subscription보다 먼저 추가한다.

권장 순서:

1. `BeeSwarm`
2. `QueenBeeLocation`
3. `HoneyProduction`
4. `ColonyPopulation`

결과:

- 같은 60분 경계에서 꿀 생산은 업데이트 직전 `ColonyBeeCount`를 기준으로 계산된다.
- 이후 `ApplyColonyPopulationUpdate()`가 다음 주기용 벌 수를 갱신한다.

## `ABeehive` 구현 요구사항

### 추가 UPROPERTY

`Beehive.h`에 아래 설정을 추가한다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production Time", meta = (ClampMin = "1", ClampMax = "1440"))
int32 HoneyProductionBucketMinutes = 60;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production Time")
bool bApplyHoneyProductionOnBeginPlayBucket = false;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production", meta = (ClampMin = "0.0"))
float HoneyProductionCoefficient = 0.01f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Honey Production", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float HoneyDistributionDeviationRatio = 0.5f;
```

### 추가 함수

`Beehive.h`에 BlueprintCallable/Pure API를 추가한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Honey Production")
void ApplyHoneyProductionUpdate();

UFUNCTION(BlueprintPure, Category = "Beehive|Honey Production")
float CalculateTotalHoneyIncreaseAmount() const;
```

private helper는 필요에 따라 추가한다.

```cpp
void DistributeHoneyIncreaseToCombs(float TotalHoneyIncrease);
```

### 꿀 생산 계산

```cpp
float ABeehive::CalculateTotalHoneyIncreaseAmount() const
{
    const int32 SafeBeeCount = FMath::Max(0, ColonyBeeCount);
    const float SafeCoefficient = FMath::Max(0.0f, HoneyProductionCoefficient);
    return static_cast<float>(SafeBeeCount) * SafeCoefficient;
}
```

### 꿀 생산 적용

`ApplyHoneyProductionUpdate()`는 다음 순서로 동작한다.

1. `CalculateTotalHoneyIncreaseAmount()` 호출
2. `DistributeHoneyIncreaseToCombs(TotalHoneyIncrease)` 호출

주의:

- 벌 수는 여기서 변경하지 않는다.
- 벌떼 Niagara 설정은 여기서 변경하지 않는다.
- 소비장 들림 여부는 무시하고 모든 active comb에 적용한다.

### bucket 구독 추가

`GetGameTimeBucketSubscriptions_Implementation`에 `HoneyProduction` subscription을 추가한다.

중요:

- 반드시 `ColonyPopulation` subscription보다 먼저 `OutSubscriptions.Add(HoneySubscription)` 해야 한다.

```cpp
FGameTimeBucketSubscription HoneySubscription;
HoneySubscription.BucketMinutes = FMath::Clamp(HoneyProductionBucketMinutes, 1, 1440);
HoneySubscription.bApplyImmediatelyOnBeginPlay = bApplyHoneyProductionOnBeginPlayBucket;
HoneySubscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
HoneySubscription.SubscriptionTag = FName(TEXT("HoneyProduction"));
OutSubscriptions.Add(HoneySubscription);
```

`OnGameTimeBucketEvent_Implementation`에 처리 분기를 추가한다.

```cpp
else if (Event.SubscriptionTag == FName(TEXT("HoneyProduction")))
{
    ApplyHoneyProductionUpdate();
}
```

## `ABeehiveCombActor` 구현 요구사항

### 추가 컴포넌트

`BeehiveCombActor.h`에 Front/Back 꿀 plane mesh component를 추가한다.

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UStaticMeshComponent> FrontHoneyPlane;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UStaticMeshComponent> BackHoneyPlane;
```

생성자에서 생성하고 `Root` 또는 `CombMesh`에 attach한다.

권장:

- `CombMesh`에 attach하면 소비장 mesh 기준으로 위치 조정하기 쉽다.
- 기존 Blueprint serialized component 구조를 고려해 이름은 안정적으로 유지한다.

### 꿀 상태와 설정

```cpp
UPROPERTY(EditAnywhere, Category = "Beehive|Honey", meta = (ClampMin = "0.0"))
float MaxHoneyPerComb = 100.0f;

UPROPERTY(VisibleAnywhere, Category = "Beehive|Honey", meta = (ClampMin = "0.0"))
float CurrentHoney = 0.0f;

UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
FVector FrontHoneyEmptyRelativeLocation = FVector::ZeroVector;

UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
FVector FrontHoneyFullRelativeLocation = FVector::ZeroVector;

UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
FVector BackHoneyEmptyRelativeLocation = FVector::ZeroVector;

UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
FVector BackHoneyFullRelativeLocation = FVector::ZeroVector;

UPROPERTY(EditAnywhere, Category = "Beehive|Honey")
FName HoneyMaterialParameterName = TEXT("HoneyAmount");
```

Dynamic material instance 캐시는 private transient로 둔다.

```cpp
UPROPERTY(Transient)
TObjectPtr<UMaterialInstanceDynamic> FrontHoneyMaterialInstance;

UPROPERTY(Transient)
TObjectPtr<UMaterialInstanceDynamic> BackHoneyMaterialInstance;
```

### 추가 API

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Honey")
void AddHoneyAmount(float DeltaHoney);

UFUNCTION(BlueprintCallable, Category = "Beehive|Honey")
void SetCurrentHoney(float NewHoneyAmount);

UFUNCTION(BlueprintPure, Category = "Beehive|Honey")
float GetCurrentHoney() const { return CurrentHoney; }

UFUNCTION(BlueprintPure, Category = "Beehive|Honey")
float GetHoneyFillRatio() const;
```

private helper:

```cpp
void SanitizeHoneyState();
void ApplyHoneyVisualState();
void EnsureHoneyMaterialInstances();
```

### Honey clamp 정책

```cpp
void ABeehiveCombActor::SanitizeHoneyState()
{
    MaxHoneyPerComb = FMath::Max(KINDA_SMALL_NUMBER, MaxHoneyPerComb);
    CurrentHoney = FMath::Clamp(CurrentHoney, 0.0f, MaxHoneyPerComb);
}
```

초과분은 버리므로 `AddHoneyAmount`는 clamp만 수행한다.

```cpp
void ABeehiveCombActor::AddHoneyAmount(float DeltaHoney)
{
    CurrentHoney += FMath::Max(0.0f, DeltaHoney);
    SanitizeHoneyState();
    ApplyHoneyVisualState();
}
```

### 시각 업데이트

`ApplyHoneyVisualState()`는 다음을 수행한다.

1. `HoneyFillRatio` 계산
2. Front/Back 꿀 plane relative location 보간
3. material index 0 dynamic material instance에 scalar parameter 적용

```cpp
const float FillRatio = GetHoneyFillRatio();
FrontHoneyPlane->SetRelativeLocation(FMath::Lerp(FrontHoneyEmptyRelativeLocation, FrontHoneyFullRelativeLocation, FillRatio));
BackHoneyPlane->SetRelativeLocation(FMath::Lerp(BackHoneyEmptyRelativeLocation, BackHoneyFullRelativeLocation, FillRatio));
FrontHoneyMaterialInstance->SetScalarParameterValue(HoneyMaterialParameterName, FillRatio);
BackHoneyMaterialInstance->SetScalarParameterValue(HoneyMaterialParameterName, FillRatio);
```

주의:

- material parameter에는 `CurrentHoney` 절대값이 아니라 `FillRatio`를 넣는다.
- material index 0만 대상으로 한다.
- Front/Back plane 중 하나가 null이어도 다른 쪽은 정상 적용되어야 한다.

### 적용 시점

`ABeehiveCombActor`의 다음 경로에서 꿀 상태도 sanitize/apply 한다.

- `OnConstruction`
- `BeginPlay`
- `PostEditChangeProperty`
- `SetCurrentHoney`
- `AddHoneyAmount`

기존 벌 Niagara parameter 적용과 함수 책임을 섞지 말고, 꿀 시각 적용은 별도 helper로 유지한다.

## 문서 업데이트 요구사항

`.md/0_ARCHITECTURE.md`에 요약 추가:

- `ABeehive`는 `HoneyProduction` bucket을 통해 60분마다 꿀 생산을 처리한다.
- 같은 bucket에서는 꿀 생산이 colony population보다 먼저 처리된다.
- 꿀 생산량은 업데이트 직전 `ColonyBeeCount * HoneyProductionCoefficient`다.
- `ABeehiveCombActor`는 절대 꿀 양과 정규화 fill ratio 기반 Front/Back honey plane 표현을 소유한다.

`.md/Architecture/WorldActorsSystem.md`에 상세 추가:

- `ABeehive` Honey Production 설정과 bucket 흐름
- `ABeehiveCombActor` Honey 상태, Front/Back plane, material parameter 적용
- 들림 상태와 무관하게 꿀 업데이트가 모든 active comb에 적용된다는 정책

## Blueprint / Editor 작업

구현 후 에디터에서 수행할 작업:

1. `BP_BeehiveCombActor` 또는 소비장 Blueprint에서 `FrontHoneyPlane`, `BackHoneyPlane` static mesh를 설정한다.
2. Front/Back honey plane material index 0에 `HoneyAmount` scalar parameter를 가진 material을 지정한다.
3. 각 소비장에서 아래 위치 값을 조정한다.
   - `FrontHoneyEmptyRelativeLocation`
   - `FrontHoneyFullRelativeLocation`
   - `BackHoneyEmptyRelativeLocation`
   - `BackHoneyFullRelativeLocation`
4. `BP_Beehive` 또는 배치된 벌통에서 꿀 생산 기본값을 확인한다.
   - `HoneyProductionBucketMinutes = 60`
   - `bApplyHoneyProductionOnBeginPlayBucket = false`
   - `HoneyProductionCoefficient = 0.01`
   - `HoneyDistributionDeviationRatio = 0.5`
5. Blueprint compile/save를 수행한다.

## 검증 기준

### C++ 빌드

- `BeekeepingSimEditor Win64 Development` 빌드 성공

### 기능 검증

1. 벌통에 active 소비장이 없으면 꿀 생산 업데이트가 crash 없이 no-op이어야 한다.
2. 벌 수 100, `HoneyProductionCoefficient=0.01`이면 60분마다 총 꿀 생산량은 1.0이어야 한다.
3. 소비장들이 가득 차지 않은 상태라면 소비장별 증가량 합은 1.0이어야 한다.
4. 소비장별 증가량은 `HoneyDistributionDeviationRatio=0.5` 기준으로 편차가 있어야 한다.
5. 꿀 업데이트는 들려진 소비장에도 적용되어야 한다.
6. 소비장이 최대치에 도달하면 초과분은 버려야 한다.
7. material parameter `HoneyAmount`에는 `CurrentHoney / MaxHoneyPerComb` 정규화 값이 들어가야 한다.
8. 같은 60분 bucket에서 꿀 생산이 벌 수 업데이트보다 먼저 실행되어야 한다.
9. 기존 벌떼 Niagara SpawnAmount/TargetBeeCount 동작을 변경하지 않아야 한다.

## 주의사항

- 명시 요청 없이 Content asset을 직접 수정하지 않는다.
- `ABeehiveCombActor`나 `ABeehive` class rename은 하지 않는다.
- Blueprint 노출 API를 삭제하지 않는다.
- 기존 queen bee, colony population, AttractionSwarm Niagara 재초기화 동작을 회귀시키지 않는다.
- 기존 `RefreshCombSpawnAmounts(true)`의 lifted comb skip 정책은 꿀 업데이트 경로와 분리한다.
