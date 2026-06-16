# Colony swarming API 리뷰 프롬프트

## 리뷰 목표

`ABeehive`에 추가된 실제 colony swarming API가 기존 state-neutral 수동/테스트 분봉 API를 보존하면서, 같은 route-arrival cluster presentation을 사용해 source hive state impact를 정확한 시점에 적용하는지 검토한다.

핵심 기대는 기존 `BeginSwarmingAtTransform`/`BeginSwarmingAtActor`는 queen/bee/comb state를 바꾸지 않고, 새 `BeginColonySwarmingAtTransform`/`BeginColonySwarmingAtActor`만 route start 성공 후 queen 제거와 `ColonyBeeCount` 차감을 수행하는 것이다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

## 기대 구현

### API 분리

- 기존 `ABeehive::BeginSwarmingAtTransform`와 `BeginSwarmingAtActor`가 삭제/rename되지 않아야 한다.
- 새 BlueprintCallable API가 추가되어야 한다.
  - `BeginColonySwarmingAtTransform(const FTransform& TargetTransform)`
  - `BeginColonySwarmingAtActor(AActor* TargetActor)`
- 기존 테스트 API는 `bHasQueenBee`, `ColonyBeeCount`를 검사하지 않아야 하고, `SetHasQueenBee(false)`나 `SetColonyBeeCount(...)`를 호출하지 않아야 한다.

### 새 설정

- `ABeehive`에 다음 설정이 추가되어야 한다.
  - `ColonySwarmingBeeLossRatioMin = 0.3f`
  - `ColonySwarmingBeeLossRatioMax = 0.6f`
- 값은 `0.0..1.0` 비율로 처리되어야 하며, 계산 시 clamp 후 min/max가 뒤집혀도 sort되어야 한다.

### Bee-loss 계산

정확한 계산 계약:

```cpp
const int32 PreSwarmBeeCount = FMath::Max(0, ColonyBeeCount);
const float ClampedMin = FMath::Clamp(ColonySwarmingBeeLossRatioMin, 0.0f, 1.0f);
const float ClampedMax = FMath::Clamp(ColonySwarmingBeeLossRatioMax, 0.0f, 1.0f);
const float LossRatioMin = FMath::Min(ClampedMin, ClampedMax);
const float LossRatioMax = FMath::Max(ClampedMin, ClampedMax);
const float LossRatio = FMath::FRandRange(LossRatioMin, LossRatioMax);
const int32 OutgoingBeeCount = FMath::Clamp(
    FMath::RoundToInt(static_cast<float>(PreSwarmBeeCount) * LossRatio),
    0,
    PreSwarmBeeCount);
```

- queen이 없거나 `ColonyBeeCount <= 0`이면 state mutation 없이 `NotifySwarmingStartFailed()` 후 false여야 한다.
- `OutgoingBeeCount <= 0`이면 queen을 제거하지 않고 bee count도 변경하지 않아야 한다.

### 공유 route-start flow

- route actor spawn/config/timing 로직이 테스트 API와 colony API에 중복 구현되지 않아야 한다.
- session cluster spawn amount가 있어야 한다. 기대 이름:
  - `ActiveSwarmClusterSpawnAmount`
- route emission duration은 authored `SwarmClusterSpawnAmount`가 아니라 session amount를 사용해야 한다.

```cpp
RouteEmissionDurationSeconds = static_cast<float>(SessionClusterSpawnAmount) / RouteSpawnAmount;
```

- route arrival cluster initialization도 session amount를 사용해야 한다.

```cpp
InitializeSwarmClusterFromDensityWithIntroGrowth(
    ActiveSwarmClusterSpawnAmount,
    SwarmClusterBeeDensityPerCubicMeter,
    ActiveSwarmRouteEmissionDurationSeconds);
```

### Colony impact commit timing

- colony state impact는 route actor spawn, route configure, route Niagara parameter apply, timing calculation 성공 후에만 commit되어야 한다.
- commit은 `BeginColonySwarming...`이 true를 반환하기 전에 수행되어야 한다.
- commit 구현은 source-of-truth mutation path를 써야 한다.

```cpp
SetColonyBeeCount(FMath::Max(0, ColonyBeeCount - FMath::Max(0, OutgoingBeeCount)));
SetHasQueenBee(false);
```

- `ReduceAllCombTargetBeeCountsByConfiguredRatio()`를 호출하면 double-apply risk이므로 finding이다.
- commit 이후 route arrival cluster spawn이 실패해도 queen/bee count rollback을 시도하지 않아야 한다.

### 기존 route session 정책

- `bDestroyPreviousTestSwarmOnStart=false`이고 active route session이 있으면 새 시작은 clean fail하고 기존 timers/pointers를 유지해야 한다.
- colony API도 같은 shared route-start policy를 따라야 한다.

### Queen state transfer

- 최소 계약은 source hive queen 제거와 cluster default swarm queen 생성이다.
- queen state transfer가 구현되지 않았다면 최종 보고와 문서에 deferred로 명시되어야 한다.
- transfer를 구현했다면 `AQueenBeeActor::MakeQueenCageItemState()` 값(class, base egg laying power, disease)을 cluster queen에 적용하는 API가 작고 명확해야 한다.

### Blueprint/API/Core Redirect

- Blueprint API addition만 있어야 한다.
- 기존 UCLASS/USTRUCT/UENUM/UFUNCTION/UPROPERTY rename/delete가 없어야 한다.
- Core Redirect는 필요하지 않아야 하며 `Config/DefaultEngine.ini`가 수정되지 않아야 한다.
- 기존 Blueprint가 real colony impact를 원하면 새 `BeginColonySwarming...` API로 전환해야 한다는 migration note가 있어야 한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "BeginSwarmingAtTransform|BeginSwarmingAtActor|BeginColonySwarming|ColonySwarmingBeeLossRatio|ActiveSwarmClusterSpawnAmount|ApplyColonySwarmingImpact|SwarmClusterSpawnAmount" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
rg -n "SetHasQueenBee\\(false\\)|SetColonyBeeCount\\(|ReduceAllCombTargetBeeCountsByConfiguredRatio|NotifySwarmingStartFailed|ReceiveSwarmingStarted" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## Manual PIE 확인

### 테스트 API

1. 기존 `BeginSwarmingAtTransform`을 호출한다.
2. route/cluster presentation이 기존처럼 동작하는지 확인한다.
3. hive queen이 유지되는지 확인한다.
4. `ColonyBeeCount`가 변하지 않는지 확인한다.
5. active comb spawn/target state가 기존 presentation-only 범위를 벗어나 변경되지 않는지 확인한다.

### Colony API

1. `ColonyBeeCount`를 known value로 설정하고 `bHasQueenBee=true`로 둔다.
2. `ColonySwarmingBeeLossRatioMin`과 `Max`를 같은 값, 예: `0.5`로 설정한다.
3. `BeginColonySwarmingAtTransform`을 호출한다.
4. route start 후 hive queen이 제거되는지 확인한다.
5. `ColonyBeeCount`가 예상 비율만큼 감소하는지 확인한다.
6. route emission duration이 removed bee count / route spawn amount인지 확인한다.
7. spawned cluster `SpawnAmount`가 removed bee count인지 확인한다.
8. active comb spawn amount가 `SetColonyBeeCount()` 경로로 갱신되는지 확인한다.
9. queen이 없는 상태에서 colony API가 실패하고 bee count를 변경하지 않는지 확인한다.
10. zero colony bees 상태에서 colony API가 실패하고 queen을 제거하지 않는지 확인한다.
11. 기존 swarm cluster capture flow가 계속 동작하는지 확인한다.

## 리뷰 결론 요구

- 승인 가능 / 수정 후 재검토 / 설계 재검토 필요 중 하나로 결론을 낸다.
- finding이 있으면 파일/라인과 함께 원인, 영향, 수정 방향을 제시한다.
- 추가 구현 프롬프트가 필요하면 `.md/PROMPT_IMPLEMENTATION_R.md`를 작성한다.
