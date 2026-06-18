# Swarming pressure and queen cell 리뷰 프롬프트

## 리뷰 목표

site-based `ABeehive::BeginColonySwarming()` 위에 추가된 swarming pressure와 queen cell gameplay가 설계 계약을 지키는지 검토한다.

핵심 기대:

- `ABeehive`는 population-derived `SwarmingPressure`를 `SwarmingLifecycle` bucket에서 계산한다.
- pressure가 높으면 hive-wide queen cell target count를 계산하고 active comb edge surface에 runtime queen cell을 생성한다.
- queen cell은 actor가 아니며 `ABeehiveCombActor`가 소유하는 runtime component group이다.
- queen cell 제거는 item-use action으로 수행되고 pressure를 낮춘다.
- queen cell은 comb retrieval을 막지만 `FBeehiveCombItemState`에는 저장되지 않는다.
- pressure trigger가 실제 colony swarming을 시작하면 기존 target-less `BeginColonySwarming()`을 호출하고, 성공 시 pressure를 `0.0f`로 리셋한다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPlacementOccupantComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenCellSpawnAreaComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenCellSpawnAreaComponent.cpp`
- `Source/BeekeepingSim/Public/Inventory/QueenCellRemovalUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/QueenCellRemovalUseAction.cpp`
- `Config/DefaultGameplayTags.ini`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

## 중점 리뷰 항목

### Swarming pressure lifecycle

- `ABeehive`에 `SwarmingPressure`, lifecycle bucket, population ratio tuning 값, queen cell tuning 값이 Blueprint-visible 설정으로 추가되었는지 확인한다.
- `SwarmingLifecycle` bucket 기본값이 30분인지 확인한다.
- same-boundary ordering이 `HoneyProduction` -> `ColonyPopulation` -> `SwarmingLifecycle` -> `PollenPattyConsumption`이 되도록 subscription 반환 순서가 맞는지 확인한다.
- queen이 없거나 active comb가 없을 때 queen cell 생성과 colony swarming trigger가 실행되지 않는지 확인한다.
- pressure target 공식이 아래와 일치하고 invalid denominator/capacity를 안전하게 처리하는지 확인한다.

```cpp
ComfortBeeCapacity = ActiveCombCount * ComfortBeeCountPerComb;
PopulationRatio = ColonyBeeCount / ComfortBeeCapacity;
TargetPressure = (PopulationRatio - PopulationStartRatio) / (PopulationTriggerRatio - PopulationStartRatio);
```

- `SetSwarmingPressure`가 음수/비정상 값을 안전하게 정리하는지 확인한다.
- successful `BeginColonySwarming()` 경로가 `SwarmingPressure = 0.0f`로 리셋하는지 확인한다.
- `BeginSwarmingAtTransform`/`BeginSwarmingAtActor` 테스트 API가 pressure/queen cell/queen/bee count를 변경하지 않는지 확인한다.

### Queen cell target and distribution

- queen cell target count가 hive-wide이며 per-comb target으로 잘못 적용되지 않는지 확인한다.
- target 공식이 아래와 일치하는지 확인한다.

```cpp
Alpha = Clamp(
    (SwarmingPressure - QueenCellSpawnPressureThreshold)
    / (SwarmingTriggerPressure - QueenCellSpawnPressureThreshold),
    0.0f,
    1.0f);
DesiredQueenCellCount = RoundToInt(MaxQueenCellCountPerHive * Pow(Alpha, QueenCellSpawnExponent));
```

- current queen cell count가 active comb 전체 합계인지 확인한다.
- `MissingCount` 전체를 각 comb에 생성하지 않고, bucket당 `MaxQueenCellsSpawnPerBucket`까지만 생성하는지 확인한다.
- 생성 후보에서 empty slot, lifted comb, spawn area 없음, per-comb cap 도달, edge-band sample 실패 comb가 제외되는지 확인한다.
- comb 선택 weight가 `1.0f / (1.0f + CurrentQueenCellCountOnComb)`이고, cell 하나 생성 후 다음 선택에서 count/weight가 갱신되는지 확인한다.

### Queen cell spawn area

- `UQueenCellSpawnAreaComponent : UBoxComponent`가 추가되었고 `ABeehiveCombActor`에 기본 subobject로 붙었는지 확인한다.
- axis contract가 local `X` thickness/normal, `+X` front, `-X` back, `Y/Z` surface coordinates인지 확인한다.
- sampling이 front/back face 선택, edge weight 선택, edge band local YZ 샘플, inset 적용, center reject, same-face spacing reject 순서를 만족하는지 확인한다.
- placement state가 world position이 아니라 face와 area-local YZ로 저장되는지 확인한다.
- edge weight/spacing/attempt 설정이 invalid 값에 대해 안전하게 동작하는지 확인한다.

### Runtime components and item use

- queen cell이 actor로 spawn되지 않고 `QueenCellRoot_N -> QueenCellVisual -> QueenCellUseArea` runtime component group으로 생성되는지 확인한다.
- visual/use-area mesh/material이 C++ hardcoded asset이 아니라 comb Blueprint child authoring 대상인지 확인한다.
- use-area tag가 `Item.UseArea.Beehive.QueenCell`이고 `Config/DefaultGameplayTags.ini`에 추가되었는지 확인한다.
- `QueenCellUseAreaToId` mapping으로 hit use-area component에서 queen cell id를 resolve할 수 있는지 확인한다.
- queen cell add/remove 후 owning hive item-use-area descriptor rebuild가 필요한 시점에 호출되는지 확인한다.
- `UQueenCellRemovalUseAction`이 queen cell use-area query를 요구하고, hit component에서 owning `ABeehiveCombActor`와 queen cell id를 resolve한 뒤 실제 제거 성공 때만 success를 반환하는지 확인한다.
- 제거 성공 시 owning hive의 `SwarmingPressure`가 `QueenCellRemovalPressureDelta`만큼 감소하고 0 미만으로 내려가지 않는지 확인한다.

### Retrieval and persistence

- `UBeehiveCombPlacementOccupantComponent::ReceiveCanRetrievePlacementOccupant_Implementation`가 기존 조건에 `QueenCellCount == 0`을 추가했는지 확인한다.
- queen cell state가 `FBeehiveCombItemState` 또는 item instance optional state에 추가되지 않았는지 확인한다.
- `ApplyStateFromItemInstance()`가 runtime queen cell을 저장/복원하지 않고, item state 적용 시 runtime queen cell을 정리하는지 확인한다.
- comb retrieval blocked reason 표시가 현재 prompt 시스템에서 지원되지 않는다면 false 반환만으로 막는 구현인지 확인한다.

### Scope and asset safety

- 기존 `AWorldOccupancySiteActor`, `ABeeSwarmClusterSiteActor`, target-less `BeginColonySwarming()` site selection 로직을 재구현하거나 깨뜨리지 않았는지 확인한다.
- `Content/` asset이 수정되지 않았는지 확인한다.
- queen cell removal item DataAsset 연결, comb Blueprint child mesh/material 지정, spawn area authoring은 수동 Editor/BP 작업으로 남아 있는지 확인한다.
- `Config/DefaultEngine.ini` Core Redirect 변경이 없는지 확인한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "SwarmingPressure|SwarmingLifecycle|QueenCell|QueenCellSpawnArea|Item.UseArea.Beehive.QueenCell|BeginColonySwarming|CanRetrievePlacementOccupant" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory .md
```

```powershell
git status --short -- Content Config/DefaultEngine.ini
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 PIE 확인

- comb Blueprint child에서 queen cell visual mesh/material과 use-area mesh/material을 지정한다.
- queen cell spawn area box가 Blueprint/editor에서 보이고 조정 가능한지 확인한다.
- population 증가 후 `SwarmingPressure`가 상승하는지 확인한다.
- pressure threshold 아래에서는 queen cell이 생성되지 않는지 확인한다.
- queen cell이 `+X/-X` 표면의 `Y/Z` edge band에만 생성되는지 확인한다.
- queen cell이 적은 active comb가 더 자주 선택되는지 확인한다.
- lifted comb에는 새 queen cell이 생성되지 않는지 확인한다.
- queen cell removal item DataAsset/action 연결 후 hit/use가 제거에 성공하는지 확인한다.
- queen cell 제거가 pressure를 낮추는지 확인한다.
- queen cell이 있는 comb는 회수되지 않고, bees/queen/queen cell 조건이 모두 clear되면 회수되는지 확인한다.
- pressure가 trigger를 넘으면 기존 `BeginColonySwarming()`이 호출되고, 성공 시 `SwarmingPressure`가 `0.0f`가 되는지 확인한다.
