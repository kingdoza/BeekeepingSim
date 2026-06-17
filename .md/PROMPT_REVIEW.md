# Colony swarming site selection 리뷰 프롬프트

## 리뷰 목표

`ABeehive`의 실제 colony swarming이 더 이상 외부 target을 받지 않고, 월드의 available `ABeeSwarmClusterSiteActor` 중 하나를 거리 가중 random으로 선택/예약/점유하는지 검토한다.

핵심 기대:

- `BeginSwarmingAtTransform`/`BeginSwarmingAtActor`는 state-neutral 테스트/프레젠테이션 API로 유지된다.
- 실제 colony swarming API는 `BeginColonySwarming()` 하나이며 target parameter가 없다.
- 신규 `AWorldOccupancySiteActor`가 inventory placement와 분리된 reusable reservation/occupation 모델을 제공한다.
- 신규 `ABeeSwarmClusterSiteActor`가 swarm cluster 목적지와 거리 기반 selection weight를 제공한다.
- 제거된 `BeginColonySwarmingAtTransform`/`BeginColonySwarmingAtActor` Blueprint node는 수동 migration 대상이다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/WorldOccupancySiteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/WorldOccupancySiteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterSiteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterSiteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

## 중점 리뷰 항목

### API 계약

- `ABeehive::BeginSwarmingAtTransform`와 `BeginSwarmingAtActor`가 삭제/rename되지 않았는지 확인한다.
- test API가 `bHasQueenBee`, `ColonyBeeCount`를 검사하지 않고, site 예약/점유와 queen/bee count mutation을 수행하지 않는지 확인한다.
- `ABeehive::BeginColonySwarming()`만 BlueprintCallable colony API로 남았는지 확인한다.
- `BeginColonySwarmingAtTransform`/`BeginColonySwarmingAtActor` C++ API가 제거되었는지 확인한다.

### Site state 모델

- `EWorldOccupancySiteState`가 `Available`, `Reserved`, `Occupied`를 제공하는지 확인한다.
- `AWorldOccupancySiteActor::IsAvailable()`이 enabled, no valid reservation, no valid occupant 조건을 모두 요구하는지 확인한다.
- `TryReserve`는 available site에서만 성공해야 한다.
- `TryOccupy`는 available이거나 같은 requester가 예약한 site에서만 성공해야 하며, 반드시 `CanAcceptOccupant`를 통과해야 한다.
- `ClearOccupant`와 occupant `OnDestroyed` auto-release가 site state를 leak 없이 해제하는지 확인한다.
- 이 모델이 `AItemPlacementSlotActor` 또는 inventory placement API에 의존하지 않는지 확인한다.

### Swarm site selection

- `ABeeSwarmClusterSiteActor` 기본 accepted occupant가 `ABeeSwarmClusterActor`인지 확인한다.
- `CalculateSelectionWeightForHive`가 hive `SwarmExitPoint` 위치를 우선 사용하고 없으면 hive actor location을 사용하는지 확인한다.
- distance는 기본 2D이며, `MaxSelectionDistanceCm > 0` 초과 후보는 weight 0인지 확인한다.
- invalid scale/exponent/multiplier가 안전한 값으로 보정되는지 확인한다.
- 공식이 아래와 일치하는지 확인한다.

```cpp
Weight = SelectionWeightMultiplier / Pow(1.0f + Distance / DistanceWeightScaleCm, DistanceWeightExponent);
```

### Colony swarming flow

- `BeginColonySwarming()`은 queen 없음, `ColonyBeeCount <= 0`, outgoing bee count 0, selectable site 없음, reservation 실패에서 hive state mutation 없이 실패해야 한다.
- outgoing bee count 계산은 기존 min/max loss ratio clamp/sort/random/round/clamp 계약을 유지해야 한다.
- 후보 site는 world의 `ABeeSwarmClusterSiteActor` 중 available이고 positive finite weight인 actor만 포함해야 한다.
- weighted random은 total weight를 합산하고 `[0, TotalWeight]` threshold를 누적 weight와 비교해 선택해야 한다.
- 선택 site는 route start 전에 `TryReserve(this)`로 예약해야 한다.
- route start 실패는 pending reservation을 release하고 queen/bee count를 변경하지 않아야 한다.
- route actor spawn/config/parameter/timing 성공 후에만 `SetColonyBeeCount(...)`와 `SetHasQueenBee(false)`가 호출되어야 한다.
- route arrival cluster spawn 성공 후 pending site가 `TryOccupy(this, ClusterActor)`로 occupied가 되고 `ActiveSwarmClusterSite`로 이동하는지 확인한다.
- post-commit cluster spawn/occupation 실패는 site를 release하고 failure event를 호출하되 queen/bee count rollback을 수행하지 않아야 한다.
- `ClearActiveTestSwarm(true)`와 `EndPlay`가 pending reservation 또는 active occupation을 leak하지 않는지 확인한다.

### Blueprint/Core Redirect 영향

- 신규 `UCLASS`/`UENUM` 추가만 있고 rename은 없으므로 `Config/DefaultEngine.ini` Core Redirect 수정이 없어야 한다.
- 제거 API 참조 검색 결과 `Content/Beehive/BP_Beehive.uasset`에 `BeginColonySwarmingAtActor` 참조가 남아 있으므로, Editor에서 해당 Blueprint node를 `BeginColonySwarming`으로 수동 교체/compile/save해야 한다.
- 이번 구현 pass에서 `Content/` asset이 수정되지 않았는지 확인한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "BeginSwarmingAtTransform|BeginSwarmingAtActor|BeginColonySwarming|WorldOccupancySite|BeeSwarmClusterSite|PendingSwarmClusterSite|ActiveSwarmClusterSite" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
rg -a -n "BeginColonySwarmingAtTransform|BeginColonySwarmingAtActor" Source Content Config .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 PIE 확인

- `BeginSwarmingAtTransform`/`BeginSwarmingAtActor` 호출 시 queen, `ColonyBeeCount`, site state가 변경되지 않는지 확인한다.
- 여러 `ABeeSwarmClusterSiteActor`를 배치하고 `BeginColonySwarming()` 반복 호출 시 가까운 site가 더 자주 선택되는지 확인한다.
- 선택 site가 route 시작 후 reserved, route arrival 후 occupied로 바뀌는지 확인한다.
- cluster 최종 capture/destroy 후 site가 available로 해제되는지 확인한다.
- selectable site가 없거나 route start가 실패하면 queen/bee count가 변경되지 않는지 확인한다.
- post-commit cluster spawn/occupation 실패 조건에서는 site가 release되고 queen/bee count rollback이 없는지 확인한다.
