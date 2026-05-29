# Beehive Comb Face Bee Count 구현 프롬프트

## 전제

이번 작업은 `ABeehiveCombActor`의 벌 수 표현을 "소비장 전체 값"과 "Front/Back face별 Niagara 주입값"으로 분리한다.

현재 C++은 단일 `SpawnAmount`/`TargetBeeCount`를 `FrontFaceBeeNiagara`와 `BackFaceBeeNiagara` 양쪽에 동일하게 넣고 있다. 이 해석은 잘못된 것으로 확정한다. 소비장 전체 `SpawnAmount`/`TargetBeeCount`를 front/back에 나눠 넣어야 한다.

사용자 지시:

- Content asset 영향은 이번 작업에서 고려하지 않는다.
- 기존 C++/Blueprint API 호환성 때문에 애매한 wrapper를 남기지 말고, 의미가 잘못된 API/UPROPERTY는 과감히 rename/remove한다.
- 단, `Content/` asset을 직접 수정/저장하지는 않는다. 필요한 Editor 수동 작업은 `.md/USER_UNREAL.md` 또는 최종 보고에 적는다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

특히 `.md/QNA_ARCHITECTURE.md`의 `소비장 면별 BeeBrush TargetBeeCount 설계 QnA` 답변을 따른다.

## 확정 정책

1. `SpawnAmount`와 `TargetBeeCount`는 소비장 전체 값이다.
2. Front/Back Niagara에는 전체값을 그대로 넣지 않고 face별 분배값을 넣는다.
3. 분배 규칙:
   - `Front = (Total + 1) / 2`
   - `Back = Total / 2`
   - 홀수면 Front가 1을 더 가진다.
4. `User.SpawnAmount`와 `User.TargetBeeCount` 모두 같은 분배 규칙으로 face별 Niagara에 주입한다.
5. `GetTargetBeeCount()`처럼 단일 값을 반환하던 API는 제거/rename하고, 새 API는 total 의미를 명확히 드러낸다.
6. 소비장 회수 조건은 양면 target 합계가 0이고 queen 미부착일 때만 만족한다.
7. BeeBrush는 visible face target만 감소한다.
8. 흔들기/legacy 전체 감소 API는 양면 target을 감소시키는 정책으로 유지하되 이름을 명확히 한다.
9. time bucket/colony 갱신으로 total spawn이 바뀔 때는 face별 target 비율을 보존한다.

## 구현 대상

주요 파일:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Private/Inventory/BeeBrushUseAction.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`
- 관련 architecture 문서

## ABeehiveCombActor 상태 변경

기존 단일 상태:

- `SpawnAmount`
- `TargetBeeCount`

권장 변경:

- `TotalSpawnAmount`
- `FrontFaceTargetBeeCount`
- `BackFaceTargetBeeCount`

`TargetBeeCount` 단일 UPROPERTY는 제거한다. `SpawnAmount`도 의미가 모호하므로 `TotalSpawnAmount`로 rename한다.

필요한 helper:

```cpp
static int32 GetFrontShareFromTotal(int32 Total);
static int32 GetBackShareFromTotal(int32 Total);
int32 GetFaceSpawnAmount(EBeehiveCombVisibleFace Face) const;
int32& GetMutableFaceTargetBeeCount(EBeehiveCombVisibleFace Face);
int32 GetFaceTargetBeeCountInternal(EBeehiveCombVisibleFace Face) const;
```

분배 규칙은 helper 한 곳에만 둔다.

## Public API 정리

기존 API 중 의미가 모호한 것은 rename/remove한다. 호출부도 같이 고친다.

권장 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ApplyCombBeeParameters(const FVector2D& InPlaneSize, int32 InTotalSpawnAmount, int32 InTotalTargetBeeCount);

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void SetTotalSpawnAmountAndResetTargetBeeCounts(const FVector2D& InPlaneSize, int32 InTotalSpawnAmount);

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void SetTotalTargetBeeCount(int32 NewTotalTargetBeeCount);

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ResetTargetBeeCountsToSpawnAmount();

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ReduceAllTargetBeeCountsByRatio(float Ratio);

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ReduceAllTargetBeeCountsByAmount(int32 Amount);

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ReduceVisibleFaceTargetBeeCountByAmount(int32 Amount);

UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void ReduceFaceTargetBeeCountByAmount(EBeehiveCombVisibleFace Face, int32 Amount);

UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
int32 GetTotalSpawnAmount() const;

UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
int32 GetTotalTargetBeeCount() const;

UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
int32 GetFaceSpawnAmount(EBeehiveCombVisibleFace Face) const;

UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
int32 GetFaceTargetBeeCount(EBeehiveCombVisibleFace Face) const;

UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
int32 GetVisibleFaceTargetBeeCount() const;
```

기존 호출부 수정 예:

- `GetSpawnAmount()` -> `GetTotalSpawnAmount()`
- `GetTargetBeeCount()` -> `GetTotalTargetBeeCount()`
- `SetSpawnAmountAndResetTargetBeeCount(...)` -> `SetTotalSpawnAmountAndResetTargetBeeCounts(...)`
- `ReduceTargetBeeCountByRatio(...)` -> `ReduceAllTargetBeeCountsByRatio(...)`
- `ReduceTargetBeeCountByAmount(...)` -> `ReduceAllTargetBeeCountsByAmount(...)` 또는 visible/face 전용 API

## Sanitize 정책

`SanitizeState()`는 다음을 보장한다.

- `TotalSpawnAmount >= 0`
- `0 <= FrontFaceTargetBeeCount <= GetFaceSpawnAmount(Front)`
- `0 <= BackFaceTargetBeeCount <= GetFaceSpawnAmount(Back)`
- `GetTotalTargetBeeCount() <= TotalSpawnAmount`

전체 target을 설정할 때는 분배 helper로 front/back target을 다시 만든다.

## Niagara 주입 변경

`ApplyNiagaraUserParameters()`는 face별 값을 넣는다.

Front:

```cpp
FrontFaceBeeNiagara->SetVariableInt("User.SpawnAmount", GetFaceSpawnAmount(Front));
FrontFaceBeeNiagara->SetVariableInt("User.TargetBeeCount", FrontFaceTargetBeeCount);
```

Back:

```cpp
BackFaceBeeNiagara->SetVariableInt("User.SpawnAmount", GetFaceSpawnAmount(Back));
BackFaceBeeNiagara->SetVariableInt("User.TargetBeeCount", BackFaceTargetBeeCount);
```

`User.PlaneSize`는 기존처럼 양면 동일 적용한다.

## BeeBrush 변경

`UBeeBrushUseAction::ApplyUseEffect()`는 visible face 전용 감소 API를 호출한다.

기존:

```cpp
CombActor->ReduceTargetBeeCountByAmount(RemoveAmount);
```

변경:

```cpp
CombActor->ReduceVisibleFaceTargetBeeCountByAmount(RemoveAmount);
```

BeeBrush는 `ColonyBeeCount`를 변경하지 않는다.

## 흔들기/전체 감소 정책

`ApplyCombShakeByRatio(...)`는 전체/양면 감소 정책으로 유지한다.

권장:

```cpp
void ABeehiveCombActor::ApplyCombShakeByRatioWithStrokeCount(float ReductionRatio, int32 StrokeCount)
{
    ReduceAllTargetBeeCountsByRatio(ClampedRatio);
    ReceiveCombShaken(...);
}
```

`ReduceAllTargetBeeCountsByRatio`는 각 face의 현재 target에 같은 ratio를 적용한다.

예:

- Front 251, Back 250, Ratio 0.1
- 각 face별로 `RoundToInt(CurrentFaceTarget * Ratio)` 감소

## Spawn 갱신 시 target 비율 보존

`ABeehive::RefreshCombSpawnAmounts(...)`는 새 total spawn을 적용할 때 face별 target 비율을 보존해야 한다.

`ABeehiveCombActor`에 전용 API를 추가한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Comb")
void SetTotalSpawnAmountPreservingTargetRatios(const FVector2D& InPlaneSize, int32 InNewTotalSpawnAmount);
```

정책:

- old face spawn이 0이면 해당 face target은 new face spawn으로 reset한다.
- old face spawn이 0보다 크면:
  - `NewFaceTarget = RoundToInt(NewFaceSpawn * OldFaceTarget / OldFaceSpawn)`
- sanitize 후 Niagara parameter를 적용한다.
- total spawn 변경으로 face spawn이 바뀌면 Niagara system reinitialize 필요 여부를 기존 `SpawnAmount != PreviousSpawnAmount` 정책에 맞춰 처리한다.

초기화/명시 reset 경로:

- `SetTotalSpawnAmountAndResetTargetBeeCounts(...)`는 face target을 새 face spawn까지 채운다.
- `RefreshCombSpawnAmounts(...)`는 확정 QnA에 따라 ratio 보존 API를 사용한다.

단, BeginPlay/OnConstruction 등 초기 상태 구성에서 target이 아직 의미 없을 때는 reset API를 사용해도 된다. 어떤 경로가 초기화인지 애매하면 호출 의도를 기준으로 명시적으로 분리한다.

## 회수 조건 수정

`UBeehiveCombPlacementOccupantComponent`의 회수 조건은 새 total API를 사용한다.

기존:

```cpp
CombActor->GetTargetBeeCount() != 0
```

변경:

```cpp
CombActor->GetTotalTargetBeeCount() != 0
```

queen attach 조건은 유지한다.

## ABeehive 호출부 수정

다음 호출부를 새 API로 맞춘다.

- `RefreshCombSpawnAmounts`
- `ReduceCombTargetBeeCountByConfiguredRatio`
- active comb spawn/target refresh 경로
- `GetTargetBeeCount`/`GetSpawnAmount` 검색 결과 전체

`RefreshCombSpawnAmounts`는 일반 갱신에서 ratio 보존 API를 사용한다.

`ApplyInitialCombSetupForBeginPlay`, construction refresh 등 초기 채움 경로는 target reset이 맞는지 확인한다. reset이 필요한 곳은 `SetTotalSpawnAmountAndResetTargetBeeCounts`, 기존 target 보존이 필요한 곳은 `SetTotalSpawnAmountPreservingTargetRatios`를 명확히 구분한다.

## 문서 갱신

구현 후 반드시 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - 기존 “양면 Niagara에 동일 값 적용” 내용을 제거하고, 전체 spawn/target을 face별 분배 주입으로 수정
  - 회수 조건을 total target 0으로 갱신
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehiveCombActor` 상태 모델 갱신
  - 분배 규칙, API 이름, BeeBrush/흔들기 정책 기록
- `.md/Architecture/InventorySystem.md`
  - BeeBrush 효과 정책을 visible face target 감소로 수정

## 검증

검색 검증:

- 기존 API명이 남아 있지 않은지 확인
  - `GetSpawnAmount(`
  - `GetTargetBeeCount(`
  - `SetSpawnAmountAndResetTargetBeeCount`
  - `ReduceTargetBeeCountBy`
  - `TargetBeeCount =`
- 의도적으로 남긴 Niagara parameter 이름 `User.TargetBeeCount`는 제외한다.

시나리오 검증:

1. `ColonyBeeCount=1000`, occupied comb 2개, `CombSpawnAmountRatio=1.0`
   - 각 소비장 total spawn 500
   - Front spawn 250, Back spawn 250
   - total target 500
   - Front target 250, Back target 250
2. total 501인 경우
   - Front 251, Back 250
3. BeeBrush
   - Front visible 상태에서 BeeBrush 사용 시 Front target만 감소
   - flip 후 BeeBrush 사용 시 Back target만 감소
4. Shake
   - 양면 target 모두 ratio만큼 감소
5. 회수
   - Front/Back target 합계가 0이어야 회수 가능

가능하면 UBT 빌드 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- Niagara system이 `User.SpawnAmount`를 total 값으로 반드시 받아야 한다는 강한 근거가 소스/문서에서 발견되는 경우
- face별 target ratio 보존이 기존 colony population/honey/queen 정책과 충돌하는 경우
- API rename 후 C++ 호출부는 정리 가능하지만 generated reflection/BP compile 단계에서 Core Redirect 없이는 빌드 자체가 불가능한 경우
- 문서의 확정 QnA와 소스 구조가 명백히 충돌하는 경우
