# Unreal Editor 수동 작업 목록 - 분봉 테스트 포획 / Colony Swarming Site Selection

이 문서는 C++ 구현 후 Unreal Editor에서 직접 설정, Compile/Save, PIE 검증해야 하는 항목만 정리한다.

`Content/` asset은 Codex가 수정하지 않는다. 아래 작업은 Editor에서 수동으로 수행한다.

## 0. Colony Swarming Site Selection 수동 migration

실제 colony swarming C++ API는 target parameter가 없는 `ABeehive::BeginColonySwarming()`만 남아 있다.

제거된 Blueprint node:

- `BeginColonySwarmingAtTransform`
- `BeginColonySwarmingAtActor`

`Content/Beehive/BP_Beehive.uasset`에는 기존 `BeginColonySwarmingAtActor` node 참조가 남아 있으므로 Editor에서 수동 교체한다.

### `BP_Beehive` node 교체

1. Unreal Editor에서 `Content/Beehive/BP_Beehive`를 연다.
2. missing/unknown 상태가 된 `BeginColonySwarmingAtActor` node 또는 해당 call site를 찾는다.
3. 해당 node를 `BeginColonySwarming` node로 교체한다.
4. 새 node에는 target actor/transform 입력을 연결하지 않는다. 실제 목적지는 C++가 world의 swarm cluster site 중에서 선택한다.
5. 기존 target actor 탐색, comb slot transform 조회, 임시 target actor wiring이 이 call 전용이었다면 정리한다.
6. `BP_Beehive`를 Compile/Save한다.

### Swarm cluster site 배치

실제 colony swarming은 selectable site가 없으면 실패하므로, 테스트 레벨에 여러 `ABeeSwarmClusterSiteActor` 또는 해당 BP child를 배치한다.

각 site에서 확인한다.

- `OccupantSpawnPoint`가 최종 swarm cluster 생성 위치/회전을 가리키는지 확인한다.
- `bEnabled = true`인지 확인한다.
- `bAutoReleaseWhenOccupantDestroyed = true`를 권장한다.
- `AcceptedOccupantClass`는 기본 `ABeeSwarmClusterActor`를 사용한다. BP child로 제한해야 할 때만 명시적으로 변경한다.
- `SelectionWeightMultiplier` 기본값은 `1.0`이다.
- `DistanceWeightScaleCm` 기본값은 `3000.0`이다.
- `DistanceWeightExponent` 기본값은 `2.0`이다.
- `bUse2DDistanceForSelection = true`를 기본으로 둔다.
- `MaxSelectionDistanceCm = 0.0`이면 거리 제한이 없다.

여러 site를 서로 다른 거리로 배치해 가까운 site가 더 자주 선택되는지 반복 PIE에서 확인한다.

### `BP_Beehive` colony swarming 설정

기존 `BP_Beehive` 또는 `ABeehive` 기반 벌통 Blueprint에서 확인한다.

- `SwarmExitPoint`가 벌통 입구 위치에 배치되어 있는지 확인한다.
- `SwarmClusterActorClass`에 `BP_BeeSwarmClusterActor` 또는 사용할 cluster BP를 지정한다.
- `SwarmRouteActorClass`에 route actor BP를 지정한다.
- `SwarmRouteParameters.SpawnAmount`, `SpeedMin`, `SpeedMax`가 0보다 큰 유효 값인지 확인한다.
- `ColonySwarmingBeeLossRatioMin`, `ColonySwarmingBeeLossRatioMax`를 의도한 실제 분봉 손실 비율로 설정한다.
- 외부 테스트 Blueprint나 UI 버튼은 `BeginColonySwarming`만 호출한다.

Compile/Save:

- `BP_Beehive`
- swarm cluster site BP child를 만들었다면 해당 BP
- 테스트 Level Blueprint 또는 실제 호출 Blueprint
- 테스트 레벨

### Colony swarming PIE 검증

1. selectable site가 없는 상태에서 `BeginColonySwarming`을 호출하면 실패하고 queen/`ColonyBeeCount`가 변경되지 않는지 확인한다.
2. 여러 site를 배치한 상태에서 `BeginColonySwarming`을 호출하면 한 site가 reserved 상태가 되는지 확인한다.
3. 가까운 site가 반복 호출에서 더 자주 선택되는지 확인한다.
4. route가 선택 site의 `OccupantSpawnPoint` 위치로 끝나는지 확인한다.
5. route actor spawn/config/timing 실패 조건을 만들면 pending reservation이 release되고 queen/`ColonyBeeCount`가 변경되지 않는지 확인한다.
6. route arrival 후 cluster spawn이 성공하면 site가 occupied 상태가 되는지 확인한다.
7. route arrival 후 site occupation 실패 조건에서는 site가 release되고 실패 event가 발생하며, 이미 commit된 queen/bee count rollback은 없는지 확인한다.
8. cluster 최종 포획 후 actor destroy가 발생하면 site가 auto-release되어 다시 available이 되는지 확인한다.
9. `ClearActiveTestSwarm(true)` 또는 벌통 EndPlay 시 pending reservation/active occupation이 남지 않는지 확인한다.
10. 기존 `BeginSwarmingAtTransform`/`BeginSwarmingAtActor` 테스트 API는 queen/`ColonyBeeCount`/site state를 변경하지 않는지 확인한다.

## 1. Gameplay Tag 확인

`Config/DefaultGameplayTags.ini`에 추가된 tag가 Editor에서 보이는지 확인한다.

- `Item.UseArea.SwarmCluster.BeeCarrier`
- `Item.BeeCarrier`

기존 tag rename, Gameplay Tag Redirect, Core Redirect 작업은 하지 않는다.

## 2. `BP_BeeSwarmClusterActor` 생성

`ABeeSwarmClusterActor` 기반 Blueprint를 생성한다.

native component 확인:

- `Root`
- `ClusterCenter`
- `ClusterNiagara`
- `QueenBeeChildActor`
- `CaptureUseAreaMesh`
- `FocusAnchor`
- `CharacterAnchor`
- `FocusTarget`
- `FocusAction`
- `ItemUseAreaScope`
- `ItemUseAreaMeshProvider`

설정:

- `ClusterNiagara`에 분봉 본진용 구형 벌떼 Niagara System을 지정한다.
- Niagara System이 아래 user parameter를 사용하도록 설정한다.
  - `User.AliveRadius` float
  - `User.SpawnAmount` int
  - `User.SphereRadius` float
- `CaptureUseAreaMesh`에 포획 use-area mesh와 표시 material을 지정한다.
- `CaptureUseAreaMesh.AreaTags`에 `Item.UseArea.SwarmCluster.BeeCarrier`가 들어 있는지 확인한다.
- `CaptureUseAreaMesh.EffectTargetPolicy`는 `ComponentOwner`로 둔다.
- `ClusterCenter`를 본진 중심점에 배치한다.
- `FocusAnchor`를 FocusEngaged 카메라 기준 위치에 배치한다.
- `CharacterAnchor`를 플레이어 고정 위치에 배치한다.
- `FocusAnchor` component tag가 `FocusAnchor`인지 확인한다.
- `CharacterAnchor` component tag가 `CharacterAnchor`인지 확인한다.
- `SwarmQueenBeeActorClass`에 사용할 queen BP를 지정한다.
- `QueenCenterOffset`으로 여왕벌이 cluster center에 보이도록 조정한다.

Compile/Save:

- `BP_BeeSwarmClusterActor`
- cluster Niagara System
- capture use-area material/material instance

## 3. Route Actor Blueprint 설정

`ABeehiveSwarmRouteActor` 기반 Blueprint를 생성한다.

native component 확인:

- `Root`
- `SwarmSpline`
- `SwarmNiagara`

설정:

- `SwarmNiagara`에 기존 출근용 spline swarm Niagara System 또는 동일 parameter 계약을 쓰는 Niagara System을 지정한다.
- Niagara System이 아래 user parameter를 사용하도록 확인한다.
  - `User.SwarmSpline`
  - `User.SplineLength`
  - `User.StartShapeExtent`
  - `User.EndShapeExtent`
  - `User.SpawnAmount`
  - `User.SpeedMin`
  - `User.SpeedMax`
- `RouteMidPointHeightOffset`을 route arc 높이에 맞게 조정한다.

Compile/Save:

- route actor BP
- route Niagara System

## 4. `BP_Beehive` 분봉 테스트 설정

기존 `BP_Beehive` 또는 `ABeehive` 기반 벌통 Blueprint를 연다.

native component 확인:

- `SwarmExitPoint`

설정:

- `SwarmExitPoint`를 벌통 입구 위치에 배치한다.
- `SwarmClusterActorClass`에 `BP_BeeSwarmClusterActor`를 지정한다.
- `SwarmRouteActorClass`에 route actor BP를 지정한다.
- `SwarmClusterSpawnAmount`를 cluster Niagara 규모에 맞게 설정한다.
- `SwarmClusterBeeDensityPerCubicMeter`를 포획 난이도/cluster 크기에 맞게 설정한다. 기본값은 `8000.0 bees/m^3`이며, `SpawnAmount=500`이면 radius는 약 `24.63cm`다.
- `SwarmRouteParameters`를 route Niagara에 맞게 설정한다.
  - `StartShapeExtent`
  - `EndShapeExtent`
  - `SpawnAmount`
  - `SpeedMin`
  - `SpeedMax`
- 반복 테스트가 필요하면 `bDestroyPreviousTestSwarmOnStart = true`를 권장한다.

주의:

- 기존 `QueenBeeChildActor`는 분봉 본진으로 이동시키지 않는다.
- 기존 벌통 `ColonyBeeCount`, 소비장 벌 수/target count를 분봉 테스트 BP에서 직접 수정하지 않는다.

Compile/Save:

- `BP_Beehive`
- 테스트 레벨

## 5. 벌 운반통 Item Definition 설정

벌 운반통 item definition 또는 관련 DataAsset을 연다.

설정:

- `GameplayTags`에 필요하면 `Item.BeeCarrier`를 추가한다.
- action spec에 `UBeeCarrierUseAction`을 추가한다.
- action tag는 프로젝트의 기존 item action authoring 규칙에 맞춰 지정한다.
- `UBeeCarrierUseAction` tuning 값을 테스트 의도에 맞게 조정한다.
  - `BaseAliveRadiusDecreasePerSecond`
  - `DragSpeedToAliveRadiusDecreaseScale`
  - `MaxAliveRadiusDecreasePerSecond`
  - `MinDragSpeedForBonus`

계산 기준:

```cpp
DragSpeed = Distance(CurrentImpactPoint, LastImpactPoint) / DeltaTime;
BonusSpeed = Max(0, DragSpeed - MinDragSpeedForBonus);
Rate = BaseAliveRadiusDecreasePerSecond + BonusSpeed * DragSpeedToAliveRadiusDecreaseScale;
Rate = Clamp(Rate, 0.0f, MaxAliveRadiusDecreasePerSecond);
DeltaAliveRadius = Rate * DeltaTime;
```

Compile/Save:

- 벌 운반통 item definition/DataAsset

## 6. 외부 테스트 Blueprint 작성

테스트용 Level Blueprint, debug actor, 또는 임시 버튼 BP에서 벌통 분봉 시작 API를 호출한다.

호출 방식:

- `ABeehive::BeginSwarmingAtTransform`
- 또는 `ABeehive::BeginSwarmingAtActor`

권장 테스트:

- 목표 위치 actor를 하나 배치하고 `BeginSwarmingAtActor(TargetActor)`를 호출한다.
- 실패 event 확인이 필요하면 `SwarmClusterActorClass` 또는 `SwarmRouteActorClass`를 비운 상태도 테스트한다.

Compile/Save:

- 테스트 Blueprint
- 테스트 레벨

## 7. PIE 검증 체크리스트

1. 외부 BP 호출 시 지정 위치에 `BP_BeeSwarmClusterActor`가 생성되는지 확인한다.
2. cluster Niagara에 `AliveRadius`, `SpawnAmount`, `SphereRadius` 초기값이 적용되는지 확인한다.
3. cluster center에 기존 벌통 여왕벌이 아니라 별도 여왕벌이 생성/attach되는지 확인한다.
4. 벌통 입구 `SwarmExitPoint`에서 cluster center까지 route actor가 생성되는지 확인한다.
5. route Niagara가 spline을 따라 재생되는지 확인한다.
6. route end가 cluster actor origin이 아니라 `ClusterCenter` 위치와 맞는지 확인한다.
7. cluster actor에 FocusConfirm으로 FocusEngaged 진입되는지 확인한다.
8. FocusEngaged 후 벌 운반통을 선택하면 capture use-area가 표시되는지 확인한다.
9. LMB hold 첫 tick은 base rate만 적용되는지 확인한다.
10. use-area 위를 천천히 드래그하면 `AliveRadius`가 base rate 중심으로 감소하는지 확인한다.
11. 같은 `DeltaTime`에서 더 긴 drag distance가 더 큰 `AliveRadius` 감소량을 만드는지 확인한다.
12. 빠르게 드래그하면 `MinDragSpeedForBonus` 초과분만 bonus rate로 더해지는지 확인한다.
13. `AliveRadius`가 실제 감소한 tick에만 action result success가 true인지 로그/디버그로 확인한다.
14. `AliveRadius <= 0`에서 captured event가 1회만 발생하는지 확인한다.
15. captured 후 capture use-area가 비활성화되어 hover/use 대상에서 빠지는지 확인한다.
16. captured 후 cluster Niagara에는 `User.AliveRadius = 0`이 적용되는지 확인한다.
17. 분봉 시작 중 기존 벌통 `ColonyBeeCount`가 변하지 않는지 확인한다.
18. 분봉 시작 중 기존 벌통 `QueenBeeChildActor` 위치/attach parent가 변하지 않는지 확인한다.
19. 분봉 시작/포획 중 active comb bee count/target count가 변하지 않는지 확인한다.
20. `bDestroyPreviousTestSwarmOnStart = true` 상태에서 재호출 시 이전 cluster/route actor가 destroy되는지 확인한다.

## 8. Compile / Save 대상

작업 후 아래 asset을 Compile/Save한다.

- `BP_BeeSwarmClusterActor`
- cluster Niagara System
- capture use-area material/material instance
- route actor BP
- route Niagara System
- `BP_Beehive`
- 벌 운반통 item definition/DataAsset
- 외부 테스트 Blueprint
- 테스트 레벨

Editor 재시작 후 다시 열어 component, class reference, gameplay tag, Niagara parameter, item action 설정이 유지되는지 확인한다.

## 9. 이번 범위에서 하지 않는 작업

- Codex/C++가 `Content/` asset을 직접 수정하거나 저장하지 않는다.
- 자동 분봉 발생 조건을 만들지 않는다.
- colony simulation, honey production, disease/aggression 계산에 분봉을 연결하지 않는다.
- 분봉 테스트 경로에서는 기존 벌통 `ColonyBeeCount`를 차감하지 않는다. 실제 colony swarming은 `BeginColonySwarming` route start 성공 후 C++가 차감한다.
- 기존 벌통 `QueenBeeChildActor`를 분봉 본진으로 이동시키지 않는다.
- 소비장 bee count/target count를 분봉 시작으로 변경하지 않는다.
- 벌 운반통 item instance에 포획 결과 state를 저장하지 않는다.
- Core Redirect를 추가하지 않는다.
