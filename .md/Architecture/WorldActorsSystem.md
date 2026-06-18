# WorldActors System

## Scope

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveLidPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveLidPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombLiftComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombLiftComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSplineSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/WorldOccupancySiteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/WorldOccupancySiteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterSiteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterSiteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeCaptureSource.h`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActorCustomization.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActorCustomization.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenCellSpawnAreaComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenCellSpawnAreaComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/UncappingTable.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTable.cpp`
- `Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp`
- `Source/BeekeepingSim/Public/WorldActors/CombUncappingPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/CombUncappingPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerActor.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerSlotActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyTransferComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyTransferComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyNozzlePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyNozzlePartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerRetrievePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerRetrievePartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyDecantingTable.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyDecantingTable.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/WorldItemPickup.h`
- `Source/BeekeepingSim/Private/WorldActors/WorldItemPickup.cpp`
- `Source/BeekeepingSim/Public/WorldActors/StorageBox.h`
- `Source/BeekeepingSim/Private/WorldActors/StorageBox.cpp`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlot.h`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemActor.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacementOccupantComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacementOccupantComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacementSlotRetrievePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacementSlotRetrievePartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemRetrievePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemRetrievePartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemRetrieveFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemRetrieveFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemRemainingComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemRemainingComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemRemainingVisualComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemRemainingVisualComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemAreaScaleRemainingVisualComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemAreaScaleRemainingVisualComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombSlotActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPlacementOccupantComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`

## Responsibilities

- 월드 배치 가능한 gameplay actor 구성
- mesh/root/focus target/action/storage 같은 컴포넌트 조립
- Blueprint native parent로서 디자이너가 asset과 연출을 붙일 수 있는 기반 제공
- Focus, Interaction, Inventory 시스템의 연결 지점 제공
- 벌통 내부 파츠 FocusAction policy preset과 소비장 lift movement 제공
- 벌통/소비장/벌떼 Niagara editor details에서 C++ source-of-truth parameter를 숨기는 editor-only customization 제공

## Key Classes

- `ABeehive`: anchored focus/cursor interaction 예시 actor + item-use-area first host(provider/scope) + `ABeehiveDualSwarmActor` child 소유 및 시간/벌 수 parameter 주입 + 위생 질병 VFX owner + 외부 BP 수동 분봉 테스트 시작 API owner + 벌통 여왕벌 capture source + swarming pressure/queen cell lifecycle owner
- `ABeeSwarmClusterActor`: 분봉 본진 actor. 포획/잔여 벌 수 source of truth, 벌 수와 밀도에서 파생한 `InitialAliveRadius`/`AliveRadius`/`SphereRadius` cluster Niagara parameter, native preview focus hit proxy `FocusCollision`, 별도 queen child actor, FocusEngaged bee-carrier/queen-cage use-area를 소유한다. Route arrival spawn 직후에는 intro growth 동안 `AliveRadius`만 `0`에서 target radius로 성장하고, 최종 captured는 벌 전량 포획과 여왕벌 포획이 모두 끝난 상태다.
- `AWorldOccupancySiteActor`: reusable world occupancy site. `Available`/`Reserved`/`Occupied` state, 예약자, 점유자, occupant spawn point, accepted occupant class, occupant destroyed auto-release 옵션을 소유한다.
- `ABeeSwarmClusterSiteActor`: 실제 colony swarming 목적지 site. `ABeeSwarmClusterActor`를 기본 occupant로 수용하며 벌통 `SwarmExitPoint` 위치 기준 거리 weight를 계산한다.
- `ABeehiveSwarmRouteActor`: 벌통 입구에서 요청 target까지 runtime spline route를 구성하는 `ABeeSplineSwarmActor` subclass. 기존 spline swarm Niagara parameter 계약을 재사용하며, cluster 생성은 소유하지 않는다.
- `ABeehiveCombActor`: 벌통/작업대 소비장 mesh + 양면 Niagara(`FrontFaceBeeNiagara`, `BackFaceBeeNiagara`) + 꿀 양/숙성도 상태 + face별 wax capping mask/use-area/visual state + runtime queen cell component group을 소유하는 actor (`BeeDiseaseValue` legacy API 유지)
- `UQueenCellSpawnAreaComponent`: comb-local `+X/-X` 표면과 `Y/Z` edge band를 샘플링하는 `UBoxComponent` subclass. queen cell placement를 world position이 아니라 face와 area-local YZ로 생성한다.
- `AUncappingTable`: anchored cursor FocusEngaged, PartFocus scope/provider, item-use-area scope/provider, 단일 comb slot child actor를 조합하는 밀도 작업대 native WorldActor
- `AUncappingTableCombSlot`: 작업대 전용 comb slot. `ABeehiveCombActor`만 accept하고, place 후 item instance state를 적용하며, occupied comb PartFocus descriptor를 작업대 전용 action으로 직접 제공한다. 작업대 comb PartFocus 잡기 상태 source of truth(`bCombPartFocusEngaged`)를 소유하고, 잡기/놓기/강제 종료 Blueprint hook을 제공한다.
- `UCombUncappingPartFocusActionComponent`: 작업대 소비장 horizontal drag flip과 PartFocus secondary retrieve bridge를 담당하는 action component. 벌통 lift/shake/lid tag 정책은 포함하지 않는다.
- `AHoneyContainerActor`: 배치된 꿀 용기 1개를 대표한다. volume/density/ripeness runtime state, honey visual mesh/material 갱신, source 배출용 `HoneyStreamNiagara`, nozzle PartFocus action, retrieve PartFocus action을 소유한다.
- `AHoneyContainerSlotActor`: reusable 꿀 용기 placement slot. `EHoneyContainerSlotRole::Source/Target`과 accepted gameplay tag query로 수용 item을 판정한다.
- `UHoneyTransferComponent`: source/target slot 검증, source container `HoneyStreamNiagara` 제어, DropLength grow phase, 실제 꿀 이송/혼합, auto stop, Niagara parameter 주입을 소유하는 reusable component다.
- `UHoneyNozzlePartFocusActionComponent`: source container nozzle primary click을 owning slot host의 `UHoneyTransferComponent` toggle로 라우팅한다. concrete 작업대 class에 의존하지 않는다.
- `UHoneyContainerRetrievePartFocusActionComponent`: generic placement retrieve acquire 성공 후 꿀 용기 state를 acquired item instance에 write-back하고 slot clear를 수행한다.
- `AHoneyDecantingTable`: source/target 꿀 용기 slot child actor, FocusEngaged cursor scopes, `UHoneyTransferComponent`를 조립하는 소분 작업대 native WorldActor다.
- `UBeehiveCombLiftComponent`: active comb slot의 child actor component relative transform을 보간해 소비장 들기/내리기를 수행하는 component
- `UBeehiveLidPartFocusActionComponent`: lid open part action policy preset (`PersistentAction`, `ProvidedStateTags={Beehive.LidOpen}`)
- `UBeehiveCombPartFocusActionComponent`: comb lift part action policy preset (`PersistentAction`, `RequiredStateTags={Beehive.LidOpen}`, `ExclusiveGroup={Beehive.CombLift}`) + comb drag gesture 해석 owner
- `AQueenBeeActor`: 여왕벌 mesh actor, Tick마다 local yaw jitter 누적 + `BaseEggLayingPower` 보유 + 왕롱 use-area/state export + legacy disease material API 유지
- `IQueenBeeCaptureSource`: `AQueenBeeActor` 포획 가능 여부와 host 상태 변경을 벌통/분봉 본진 쪽으로 위임하는 interface
- `ABeehiveDualSwarmActor`: outgoing/ingoing Niagara 2개를 가진 벌통 전용 actor (Spline은 `ABeehive` 소유)
- `ABeeSplineSwarmActor`: 기존 단일 swarm actor로 유지되며 dual swarm 구조와 별개 (`User.SwarmSpline`/`User.SplineLength` 바인딩 유지)
- `AWorldItemPickup`: 단일 item definition 기반 pickup actor
- `AStorageBox`: storage inventory와 storage UI interaction을 가진 actor
- `IItemPlacementSlot`: item placement action이 concrete actor를 몰라도 배치를 요청할 수 있는 슬롯 계약
- `AItemPlacementSlotActor`: `IItemUseAreaActivationProvider` + `IItemPlacementSlot` + `ICursorPartFocusProvider` + `IItemUseAreaMeshSource`를 구현하는 generic 배치 슬롯 actor
  - 구성: `Root`, `SlotMeshComponent`, `AttachComponent`
  - `SlotMeshComponent`는 `UItemUseAreaMeshComponent` 기반 hit + visual component다.
  - `SlotMeshAsset`으로 슬롯 인스턴스별 영역 mesh를 지정할 수 있다.
  - `SlotMeshMaterial`로 슬롯 인스턴스별 item-use-area 표시 material을 지정할 수 있다.
  - `SlotMeshRelativeTransform`으로 hit/visual mesh의 local transform을 조정한다.
  - `AttachRelativeTransform`으로 placed actor attach point의 local transform을 조정한다.
- `UPlacementOccupantComponent`: 배치 점유 actor의 회수 정보(return item definition, owning slot)와 회수 가능 조건 hook을 제공한다.
- `UPlacementSlotRetrievePartFocusActionComponent`: PartFocus secondary 입력에서 state-aware acquire(`TryAcquireItemBySpec`) + slot clear를 수행한다.
- `UPlacedItemRemainingComponent`: 배치 아이템 durability 잔량 런타임 상태 owner.
- `UPlacedItemRemainingVisualComponent`: 배치 잔량 비주얼 반영용 base component.
- `UPlacedItemAreaScaleRemainingVisualComponent`: 화분떡용 XY 면적 scale 비주얼 component (`sqrt(Ratio), sqrt(Ratio), 1`).
- `ABeehiveCombSlotActor`: comb 전용 `AItemPlacementSlotActor` subclass. comb class 검증, beehive refresh 요청, comb slot 정책을 캡슐화한다.
- `UBeehiveCombPlacementOccupantComponent`: comb 회수 가능 조건(`TotalTargetBeeCount`, queen attach, queen cell 없음)을 구현하는 occupant subclass다.
- `FBeehiveDualSwarmActorCustomization` / `FBeehiveDualSwarmNiagaraComponentCustomization`: editor-only details customization. `OverrideParameters` 같은 C++ 적용값의 details 노출을 숨긴다.

## Composition

### `ABeehive`

- `USceneComponent` root
- `UStaticMeshComponent` body/lid mesh
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UChildActorComponent` 1개 (`BeehiveSwarmChildActor`)
- `UChildActorComponent` 1개 (`QueenBeeChildActor`)
- `bHasQueenBee`: 벌통 queen child actor 보유 여부. false이면 queen child 재생성/위치 갱신/산란 증가량 계산을 비활성화한다.
- `USceneComponent` 1개 (`CombRackRoot`) + `MaxCombCount` 크기의 comb slot child actor component 배열
- `USceneComponent` 1개 (`CombLiftTargetRoot`)
- `USceneComponent` 1개 (`SwarmExitPoint`)를 분봉 테스트 route start point로 사용한다.
- `UBeehiveCombLiftComponent` 1개 (`CombLiftComponent`)
- `UCursorPartFocusScopeComponent` 1개 (`CursorPartFocusScope`)
- `UCursorPartFocusActionComponent` 1개 (`LidPartFocusAction`)
- `UCursorItemUseAreaScopeComponent` 1개 (`ItemUseAreaScope`)
- `USplineComponent` 1개 (`SwarmSpline`)를 직접 소유하며 레벨 인스턴스별 편집 대상
- `UNiagaraComponent` 1개 (`AttractionSwarmNiagara`)
- `UNiagaraComponent` 1개 (`DiseaseVfxNiagara`)를 직접 소유하며 Niagara System/transform은 BP/Details에서 authoring
- `BeeSplineSwarmActorClass` (`TSubclassOf<ABeehiveDualSwarmActor>`)로 child class 지정
- `SwarmClusterActorClass`, `SwarmRouteActorClass`, `SwarmClusterSpawnAmount`, `SwarmClusterBeeDensityPerCubicMeter`(기본 `8000.0 bees/m^3`), `SwarmRouteParameters`, `bDestroyPreviousTestSwarmOnStart`
- colony-impact swarming 설정: `ColonySwarmingBeeLossRatioMin=0.3`, `ColonySwarmingBeeLossRatioMax=0.6`
- swarming pressure 설정: `SwarmingPressure=0.0`, `SwarmingLifecycleBucketMinutes=30`, `bApplySwarmingLifecycleOnBeginPlayBucket=false`, `ComfortBeeCountPerComb=100.0`, `PopulationStartRatio=0.7`, `PopulationTriggerRatio=1.1`, `QueenCellSpawnPressureThreshold=0.7`, `SwarmingTriggerPressure=1.0`, `QueenCellRemovalPressureDelta=0.1`
- queen cell hive-wide target 설정: `MaxQueenCellCountPerHive=10`, `QueenCellSpawnExponent=1.5`, `MaxQueenCellsSpawnPerBucket=2`
- transient `ActiveSwarmClusterActor`, `ActiveSwarmRouteActor`, `PendingSwarmClusterSite`, `ActiveSwarmClusterSite`, `ActiveSwarmClusterSpawnAmount`, pending cluster transform, active route arrival/emission timer state
- 분봉 테스트 API:
  - `BeginSwarmingAtTransform(const FTransform& TargetTransform)`
  - `BeginSwarmingAtActor(AActor* TargetActor)`
  - `ClearActiveTestSwarm(bool bDestroyActors)`
  - `GetActiveSwarmClusterActor()`, `GetActiveSwarmRouteActor()`, `GetSwarmExitPointComponent()`
- colony-impact 분봉 API:
  - `BeginColonySwarming()`
- 공통 route 시작 실패 조건: world 없음, `SwarmClusterActorClass` 없음, `SwarmRouteActorClass` 없음, route `SpawnAmount <= 0`, 평균 route speed <= 0, route spawn 실패, `bDestroyPreviousTestSwarmOnStart=false` 상태의 active route session 존재
- colony-impact 분봉 추가 실패 조건: queen 없음, `ColonyBeeCount <= 0`, loss ratio 계산 결과 `OutgoingBeeCount <= 0`, available/positive weight `ABeeSwarmClusterSiteActor` 없음, 선택 site 예약 실패
- 분봉 route 시작 성공 flow: 이전 test swarm session/timer 정리 옵션 처리(`bDestroyPreviousTestSwarmOnStart=false`이면 active route session을 덮어쓰지 않고 실패) -> session cluster spawn amount 결정 -> route spawn at `SwarmExitPoint` -> `ConfigureRoute(SwarmExitPoint, TargetTransform.Location)` -> `ApplyExternalSwarmParameters(SwarmRouteParameters)` -> route arrival/emission stop/destroy timer 설정 -> true 반환
- colony-impact site selection flow: world의 `ABeeSwarmClusterSiteActor` 중 `IsAvailable()`이고 `CalculateSelectionWeightForHive(this) > 0`인 후보를 weighted random으로 선택한다. 선택 site는 route start 전에 `TryReserve(this)`로 예약하고, route start 실패 시 commit 없이 reservation을 release한다.
- colony-impact commit timing: route actor spawn/config/parameter/timing 계산 성공 후 `SetColonyBeeCount(Max(0, ColonyBeeCount - OutgoingBeeCount))`와 `SetHasQueenBee(false)`를 호출한다. Commit 이후 route arrival cluster spawn 또는 site occupation 실패가 발생해도 queen/bee count rollback은 수행하지 않는다.
- colony-impact swarming 성공 시 `SwarmingPressure`는 `0.0f`로 리셋된다.
- route emission duration: `ActiveSwarmClusterSpawnAmount / SwarmRouteParameters.SpawnAmount`. 테스트 세션은 authored `SwarmClusterSpawnAmount`, colony-impact 세션은 차감된 `OutgoingBeeCount`를 사용한다.
- route arrival flow: pending target transform에 cluster spawn -> `InitializeSwarmClusterFromDensityWithIntroGrowth(ActiveSwarmClusterSpawnAmount, SwarmClusterBeeDensityPerCubicMeter, RouteEmissionDurationSeconds)` -> pending site가 있으면 `TryOccupy(this, ClusterActor)` 후 active site로 이동 -> `ReceiveSwarmingStarted`
- `GetActiveSwarmClusterActor()`는 route arrival 전까지 null이며, `ReceiveSwarmingStarted`는 cluster가 실제 생성된 뒤 호출된다.
- 기존 분봉 테스트 시작 API는 `ColonyBeeCount`, `QueenBeeChildActor`, active comb bee count/target count, bucket subscription을 변경하지 않는다.
- 기존 분봉 테스트 시작 API는 `SwarmingPressure`와 queen cell state도 변경하지 않는다.
- colony-impact 분봉은 `SetColonyBeeCount()`를 bee-count mutation path로 사용해 dual swarm, attraction swarm, active comb spawn amount를 갱신하고 target ratio를 보존한다. `ReduceAllCombTargetBeeCountsByConfiguredRatio()`는 호출하지 않는다.
- 왕롱 포획 API:
  - `HasQueenBee()`
  - `SetHasQueenBee(bool bNewHasQueenBee)`
  - `CanCaptureQueenBee(AQueenBeeActor* QueenBee)`
  - `CaptureQueenBee(AQueenBeeActor* QueenBee, FQueenCageItemState& OutCapturedState)`
- 벌통 여왕벌 포획은 현재 `QueenBeeChildActor`의 child actor만 허용하고, 성공 시 `bHasQueenBee=false`로 전환해 child actor를 제거/재생성 차단한 뒤 item-use-area descriptor를 rebuild한다.
- 벌통 여왕벌 포획은 `ColonyBeeCount`, active comb bee count/target count, honey state를 즉시 변경하지 않는다.
- `ColonyBeeCount`, `BeeSwarmHour24`, `DualSwarmCommonSettings`, `OutgoingSwarmSettings`, `IngoingSwarmSettings`
- `IGameTimeBucketListener`를 구현해 시간 bucket 이벤트를 구독
- 기본 bucket 설정: `BeeSwarmBucketMinutes=10`, BeginPlay 즉시 적용 옵션 지원
- queen 위치 bucket 설정: `QueenBeeLocationBucketMinutes=60`, BeginPlay 즉시 적용 옵션 지원
- colony population bucket 설정: `ColonyPopulationBucketMinutes=60`, `bApplyColonyPopulationOnBeginPlayBucket=false` 기본
- colony population 계수 설정: `BeeIncreaseCoefficient`, `BeeDecreaseCoefficient`, `BeeDecreaseAbsoluteAmountPerBucket`
- honey production bucket 설정: `HoneyProductionBucketMinutes=60`, `bApplyHoneyProductionOnBeginPlayBucket=false` 기본
- honey production 계수/분배 설정: `HoneyProductionCoefficient`, `HoneyDistributionDeviationRatio`
- honey ripeness 설정/API: `HoneyRipenessIncreasePerBucket`, `ApplyHoneyRipenessUpdate()`
- pollen patty consumption bucket 설정: `PollenPattyConsumptionBucketMinutes=60`, `bApplyPollenPattyConsumptionOnBeginPlayBucket=false` 기본
- pollen patty consumption 설정: `PollenPattyConsumptionAmountPerBucket`, `PollenPattyConsumptionSide(Leftmost/Rightmost)`, `PollenPattyConsumptionAreaTags`
- colony population의 `ItemEggLayingBonus`는 선택된 active 화분떡 1개(`UPollenPattyItemDefinition`)의 `EggLayingMultiplier`를 사용한다.
- queen 위치 갱신 규칙:
  - `bHasQueenBee=false`이면 위치 갱신은 no-op이다.
  - active comb 후보에서 현재 lifted comb slot 제외
  - 중앙 slot일수록 높은 가중치로 weighted random 선택
  - 선택된 comb의 front/back attach point를 50:50로 선택
  - attach point 기준 회전에 `0..360` 랜덤 yaw를 relative rotation으로 추가
- BeeBrush queen relocation:
  - `ABeehive::TryBrushQueenBeeFromCombVisibleFace(CombActor)`가 여왕벌 이동의 owner다.
  - 대상 소비장의 현재 visible face attach point에 여왕벌이 붙어 있을 때만 털어냄이 성공한다.
  - 성공 시 같은 소비장을 제외한 다른 active comb 중 하나를 균등 랜덤으로 선택하고, front/back attach point를 50:50로 골라 재부착한다.
  - 다른 소비장이 없거나 여왕벌이 해당 face에 붙어 있지 않으면 이동하지 않는다.
- `ApplyBeeSwarmSettings()`에서 child actor 재생성 없이 기존 instance에 계산된 DTO를 재주입
- 시간 갱신(`ApplyBeeSwarmHour24`) 경로에서도 class 변경 없이 parameter만 갱신
- `CombSlotSpacing` 기준 local `Y` 중앙 정렬 배치:
  - `HalfSpan = (MaxCombCount - 1) * 0.5 * CombSlotSpacing`
  - `SlotY = -HalfSpan + (i * CombSlotSpacing)`
  - 전체 slot 배열 중심은 `CombRackRoot` origin
- `InitialCombCount`는 에디터 authoring 값이며 `0..MaxCombCount`로 clamp된다.
- `CurrentCombCount`는 외부 setter가 없는 내부 캐시이며, 실제 slot occupancy(`GetPlacedCombActor()`) 기준으로 갱신된다.
- PIE/game world에서는 `InitialCombCount`, `MaxCombCount`, `CombActorClass`, `CombSlotActorClass`, `CombSlotSpacing` details 편집을 차단한다.
- occupied comb 수가 0이면 comb spawn amount는 0이다.
- comb spawn amount 계산식: `RoundToInt(ColonyBeeCount * Clamp01(CombSpawnAmountRatio) / OccupiedCombCount)`
- `SetColonyBeeCount`, `ApplyColonyPopulationUpdate()` 등 일반 spawn amount 갱신 경로에서는 active comb의 face target 비율을 보존하며 total spawn만 갱신한다.
- 초기 배치/초기 채움 경로(`RefreshCombLayoutAndParameters`, `ApplyInitialCombSetupForBeginPlay`)에서는 active comb target을 face spawn값으로 명시 reset한다.
- `ApplyColonyPopulationUpdate()` 경로에서는 lifted comb slot을 제외한 active comb에만 spawn/target 갱신을 적용한다.
- `HoneyProduction` bucket event에서는 `ApplyHoneyRipenessUpdate()` 후 `ApplyHoneyProductionUpdate()` 순서로 처리한다.
- `ApplyHoneyRipenessUpdate()`는 들림 여부와 무관하게 이미 full 상태인 모든 active comb의 숙성도만 증가시킨다.
- `ApplyHoneyProductionUpdate()` 직접 호출 경로에서는 들림 여부와 무관하게 모든 active comb에 꿀 증가량만 적용한다.
- wax capping 자동 재생성은 벌통 honey ripeness/production 업데이트에서 active comb mutation 직후 `TryRegenerateWaxCapping()`으로만 수행한다.
- 공통 plane size source of truth는 `ABeehive::CombPlaneSize`이며 active comb actor에 일괄 주입
- cursor part focus 등록:
  - lid component part (`PersistentAction`, `ProvidedStateTags={Beehive.LidOpen}`)
  - active comb actor parts (`PersistentAction`, `RequiredStateTags={Beehive.LidOpen}`, `ExclusiveGroup={Beehive.CombLift}`)
  - preview-only component parts (prompt 없음, LMB click no-op)
  - preview key 입력(`R/F/C`)은 hover preview 대상의 action handler에서 선택적으로 처리
- 현재 native `ABeehive` 기본 subobject(`LidPartFocusAction`)는 공통 `UCursorPartFocusActionComponent`를 사용하고, `ABeehiveCombActor` 기본 subobject(`PartFocusAction`)는 `UBeehiveCombPartFocusActionComponent`를 사용한다.
- Beehive/Comb의 실제 lid open-close, comb lift-restore는 action component의 owner-actor delegate(`OnPartFocusBegin/Cancel/Abort`) 또는 component 이벤트 구현 경로에서 처리한다.
- `ABeehive`는 `UItemUseAreaMeshProviderComponent`를 통해 item-use-area descriptor를 수집한다.
- descriptor 기본 tag:
  - lid: `Beehive.UseArea.Lid`
  - comb: `Beehive.UseArea.Comb`
- `ABeehive`는 sanitation과 별도로 aggression 상태를 소유한다.
  - `MaxAggressionValue=100`, `AggressionValue=100` 기본값으로 시작한다.
  - `DecreaseAggression`, `SetAggressionValue`, `GetAggressionRatio` API를 제공한다.
- aggression은 훈연기 hold-use 효과로만 감소하고, 현재 범위에서는 자동 회복/공격력 계산과 연결하지 않는다.
- item-use 확장:
  - sanitation 상태: `SanitationValue`, `MaxSanitationValue`, `IncreaseSanitation`, `SetSanitationValue`, `GetSanitationRatio`
  - sanitation disease 설정/API: `SanitationDiseaseThreshold`, `MaxSanitationBeeDecreaseMultiplier`, `GetSanitationDiseaseRatio()`, `GetSanitationBeeDecreaseMultiplier()`
  - `SetSanitationValue()`는 위생값 clamp 후 `RefreshHiveDiseaseVisuals()`로 질병 시각값을 즉시 갱신한다.
  - 질병 시각값 source of truth는 항상 `ABeehive::GetSanitationDiseaseRatio()`이며, 현재 활성 시각 출력 대상은 `ABeehive::DiseaseVfxNiagara.User.Disease` 단일 경로다.
  - attraction swarm, dual swarm outgoing/ingoing, active comb front/back Niagara, queen material의 직접 `Disease` 적용 코드는 legacy path로 유지하되 주석처리되어 있다.
  - `ABeehive`는 pollen slot 상태를 직접 소유하지 않는다.
  - pollen slot은 `AItemPlacementSlotActor` child actor로 authoring하며, occupied 상태는 slot actor의 `PlacedActor`가 소유한다.
  - slot actor는 `IItemUseAreaActivationProvider`로 occupied 여부를 판단한다.
  - empty slot: descriptor active(`AreaTags` 유지)
  - occupied slot: descriptor inactive(`AreaTags` 비움)
  - host provider는 child actor 내부 `UItemUseAreaMeshComponent`를 수집한다.

### `AUncappingTable`

- `USceneComponent` root
- `UStaticMeshComponent` `TableMesh`
- `USceneComponent` 2개 (`FocusAnchor`, `CharacterAnchor`)
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UCursorPartFocusScopeComponent`
- `UCursorPartFocusRegistrationComponent`
- `UChildCursorPartFocusProviderComponent`
- `UCursorItemUseAreaScopeComponent`
- `UItemUseAreaMeshProviderComponent`
- `USceneComponent` `CombSlotRoot`
- `UChildActorComponent` `CombSlotChildActor`
- `CombSlotActorClass` 기본값은 `AUncappingTableCombSlot::StaticClass()`
- `CombSlotChildActor`는 `PartFocusChild` component tag를 가져 child provider가 slot descriptor를 수집한다.
- `RebuildCursorPartFocusDescriptors()`는 scope clear 후 registration append를 수행한다.
- `RebuildItemUseAreaDescriptors()`는 item-use-area scope rebuild를 수행한다.
- FocusEngaged 진입/취소, hotbar 선택 clear, cursor/input mode는 `UAnchoredFocusCursorActionComponent` 기존 정책을 따른다.

### `AUncappingTableCombSlot`

- `AItemPlacementSlotActor` subclass이며 `ABeehiveCombActor`만 accept한다.
- empty slot item-use-area tag는 `Item.UseArea.UncappingTable`이다.
- place 성공 후 `ABeehiveCombActor::ApplyStateFromItemInstance(SourceItemInstance)`를 호출한다.
- `CombPartFocusAction`의 begin/cancel/abort delegate로 `bCombPartFocusEngaged`를 갱신하고, 변경 시 owning `AUncappingTable`의 PartFocus/item-use-area descriptors를 rebuild한다.
- 잡기/놓기/강제 종료 BP hook:
  - `ReceiveCombGrabbed(CombActor, InteractingCharacter)`
  - `ReceiveCombReleased(CombActor, InteractingCharacter)`
  - `ReceiveCombGrabAborted(CombActor, InteractingCharacter)`
- 작업대 소비장 들어올림/원위치 애니메이션은 C++ Tick/component가 아니라 `BP_UncappingTableCombSlot`에서 `AttachComponent` relative transform을 움직이는 방식으로 구현한다.
- occupied comb descriptor:
  - `PartId = "UncappingTable.Comb"`
  - `OwnerActor = CombActor`
  - `HitComponent = CombActor->GetCombMeshComponent()`
  - `OutlineComponents = { CombMesh }`
  - `ActionHandler = CombPartFocusAction`
- place/clear/BeginPlay 후 owning `AUncappingTable`의 PartFocus와 item-use-area descriptors를 rebuild한다.

### `AHoneyContainerActor`

- `USceneComponent` root
- `UStaticMeshComponent` `ContainerMesh`
- `UStaticMeshComponent` `HoneyVisualMesh`
- `UStaticMeshComponent` `NozzleHitComponent`
- `USceneComponent` `NozzleOrigin`
- `USceneComponent` `PourTarget`
- `UNiagaraComponent` `HoneyStreamNiagara`
- `UPlacementOccupantComponent`
- `UHoneyContainerRetrievePartFocusActionComponent`
- `UHoneyNozzlePartFocusActionComponent`
- `ApplyStateFromItemInstance`는 `UHoneyContainerItemDefinition` default 또는 item instance `FHoneyContainerItemState`를 적용한다.
- `WriteHoneyContainerStateToItemInstance`는 회수 성공 후 acquired item instance에 volume/density/ripeness를 저장한다.
- honey visual fill ratio는 `CurrentVolumeMl / MaxVolumeMl`이며 `HoneyVisualMesh` Z scale에 반영된다.
- honey material scalar parameter:
  - `HoneyDensity`
  - `HoneyRipeness`

### `AHoneyContainerSlotActor`

- `AItemPlacementSlotActor` subclass이며 `AHoneyContainerActor`만 accept한다.
- role enum은 reusable `Source`, `Target`이다.
- 수용 가능한 용기 조합은 `AcceptedItemTagQuery`와 source item definition gameplay tags로 판정한다.
- place 성공 후 `AHoneyContainerActor::ApplyStateFromItemInstance(SourceItemInstance)`를 호출한다.
- occupied 상태에서 retrieve descriptor를 제공하고, role이 `Source`이면 container nozzle descriptor도 제공한다.
- place/clear/BeginPlay 후 owning host의 PartFocus/item-use-area descriptors를 rebuild한다.

### `UHoneyTransferComponent`

- 설정:
  - `TransferRateMlPerSecond`
  - `DropLengthGrowSpeedCmPerSecond`
  - `DefaultDropLengthCm`
  - Niagara parameter names: `User.HoneyDensity`, `User.HoneyRipeness`, `User.DropLength`
- state:
  - `Idle`
  - `GrowingDrop`
  - `Transferring`
- `StartTransfer`는 source/target slot role, placed container, source volume, target free volume을 검증한다.
- `GrowingDrop`에서는 source container `HoneyStreamNiagara` world Z와 target container/slot `PourTarget` world Z 차이만큼 `DropLength`를 증가시키며 volume을 이동하지 않는다.
- `Transferring`에서는 `min(rate * DeltaTime, source volume, target free volume)`만큼 이동한다.
- target density/ripeness는 기존 내용물과 유입량을 volume-weighted average로 혼합한다. 혼합 결과 density가 1.0 미만이면 ripeness는 0.0으로 정규화한다.
- source/target missing, slot occupant 변경, source empty, target full, owner EndPlay, explicit stop에서 Niagara를 즉시 정지한다.

### `AHoneyDecantingTable`

- `USceneComponent` root
- `UStaticMeshComponent` `TableMesh`
- `USceneComponent` 2개 (`FocusAnchor`, `CharacterAnchor`)
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UCursorPartFocusScopeComponent`
- `UCursorPartFocusRegistrationComponent`
- `UChildCursorPartFocusProviderComponent`
- `UCursorItemUseAreaScopeComponent`
- `UItemUseAreaMeshProviderComponent`
- source/target `UChildActorComponent` 2개
- `UHoneyTransferComponent`
- `UNiagaraComponent` `HoneyStreamNiagara`는 legacy 호환용 component이며, 신규 이송 VFX의 기본 source는 source container의 `HoneyStreamNiagara`다.
- source slot role은 `Source`, target slot role은 `Target`으로 runtime 설정된다.
- `RebuildCursorPartFocusDescriptors()`와 `RebuildItemUseAreaDescriptors()`는 `AUncappingTable`과 같은 host rebuild 패턴을 따른다.
- `BeginPlay`/`OnConstruction`에서 transfer component에 source/target slot을 연결한다. table `HoneyStreamNiagara`는 기존 serialized Blueprint 호환을 위한 fallback으로만 전달된다.

### `ABeehiveDualSwarmActor`

- `USceneComponent` root
- `UNiagaraComponent` outgoing/ingoing
- `ApplySwarmSpline(USplineComponent*)`로 Beehive 소유 spline reference를 주입받는다.
- `ApplySplineBindings()`에서 주입받은 spline으로 `User.SwarmSpline`, `User.SplineLength`, `User.bIsReverse`를 양쪽 Niagara에 명시 적용
- `ApplyDualSwarmParameters()`에서 공통 shape + 방향별 spawn/speed 적용
- `FBeehiveDualSwarmNiagaraParameters::Disease` field는 legacy API로 유지하지만, outgoing/ingoing `User.Disease` 직접 적용 코드는 주석처리되어 있다.
- 자체적으로 spawn/speed/shape를 계산하지 않으며 source of truth는 `ABeehive`
- `IFocusInteractable` 구현

### `ABeeSplineSwarmActor`

- `USceneComponent` root
- `USplineComponent` swarm spline
- `UNiagaraComponent` swarm niagara
- `ControlMode` 기본값은 `Standalone`
- `Standalone` source of truth는 Niagara Component User Parameter이며, C++은 `User.SwarmSpline`과 `User.SplineLength`만 적용
- `ExternalControlled` source of truth는 상위 actor 주입값이며, C++은 전달받은 5개 parameter와 spline 길이를 Niagara에 적용
- Niagara Spline Data Interface는 `User.SwarmSpline` Object parameter를 통해 Beehive가 전달한 `SwarmSpline`을 참조한다.
- `ABeeSplineSwarmActor`는 기존 단일 actor 워크플로우를 유지한다.
- Spline point 자동 생성/삭제/재배치 기능은 제공하지 않음

### `ABeeSwarmClusterActor`

- 구성:
  - `Root`
  - `ClusterCenter`
  - `ClusterNiagara`
  - `FocusCollision` (`USphereComponent`)
  - `QueenBeeChildActor`
  - `CaptureUseAreaMesh`
  - `FocusAnchor`, `CharacterAnchor` (`ComponentTags`: `FocusAnchor`, `CharacterAnchor`)
  - `UFocusTargetComponent`
  - `USwarmClusterFocusActionComponent` (`UAnchoredFocusCursorActionComponent` subclass)
  - `UCursorItemUseAreaScopeComponent`
  - `UItemUseAreaMeshProviderComponent`
- `CaptureUseAreaMesh`는 `UItemUseAreaMeshComponent`이며 area tag는 `Item.UseArea.SwarmCluster.BeeCarrier`다.
- `CaptureUseAreaMesh` effect target은 `ComponentOwner` 정책으로 `ABeeSwarmClusterActor`를 `Context.ItemUseEffectTargetObject`에 전달한다.
- `FocusCollision`은 preview focus hit testing용 native sphere component다.
  - collision enabled: `QueryOnly`
  - object type: `WorldDynamic`
  - response: all channels ignore, `ECC_Visibility` block
  - overlap/physics/navigation 영향 없음
  - radius: `Max(0, AliveRadius) + 5`
  - actor가 final captured 상태이거나 FocusEngaged 중이면 disabled
  - intro growth 중에는 collision을 유지하고, `USwarmClusterFocusActionComponent::CanBeginFocusAction`으로 FocusEngaged 시작만 막는다.
  - BP-authored legacy `SphereCollision`은 제거하거나 `NoCollision`으로 바꿔야 하며, native `FocusCollision`만 preview focus hit proxy로 사용한다.
- `SpawnAmount`, `BeeDensityPerCubicMeter`, `CapturedBeeAmount`가 포획 진행의 source of truth다.
- `InitialAliveRadius`와 `SphereRadius`는 `SpawnAmount / BeeDensityPerCubicMeter` 부피에서 파생되는 runtime 값이다. 계산식은 `RadiusCm = cbrt((3 * (SpawnAmount / BeeDensityPerCubicMeter)) / (4 * PI)) * 100`이며, density 단위는 `bees/m^3`, Unreal 거리 단위는 cm다.
- `AliveRadius`는 `InitialAliveRadius * cbrt(RemainingBeeAmount / TotalBeeAmount)`로 계산되는 Niagara 표시 반경이다.
- route arrival spawn 직후 intro growth:
  - `SpawnAmount`, `InitialAliveRadius`, `SphereRadius`는 즉시 target 값이다.
  - Niagara `User.SpawnAmount`와 `User.SphereRadius`도 즉시 target 값이다.
  - `AliveRadius`만 `0`에서 시작해 `RouteEmissionDurationSeconds` 동안 성장한다.
  - route-arrival 초기화는 `ReceiveSwarmClusterInitialized`/`ReceiveAliveRadiusChanged` 전에 intro 시작 상태를 적용해 Blueprint event와 Niagara가 full `AliveRadius`를 먼저 관측하지 않게 한다.
  - `IntroVisualBeeAmount = SpawnAmount * Clamp(Elapsed / Duration, 0..1)`
  - `AliveRadius = InitialAliveRadius * cbrt(IntroVisualBeeAmount / SpawnAmount)`
  - intro tick은 growth 중에만 켜고 완료 즉시 비활성화한다.
- `USwarmClusterFocusActionComponent::CanBeginFocusAction`은 intro growth 중 false를 반환해 host FocusEngaged 진입을 막는다. BeeCarrier/QueenCage use-area는 FocusEngaged 내부에서만 의미가 있으므로 별도 intro gate를 두지 않는다.
- `CaptureBees(RequestedBeeAmount)`는 요청량을 잔여 벌 수로 clamp하고, 실제 포획량만 `CapturedBeeAmount`에 누적한 뒤 `AliveRadius`를 재계산한다.
- intro growth 중 `CaptureBees`, `SetCapturedBeeAmount`, `RefreshAliveRadiusFromBeeAmounts`가 호출되면 intro를 종료하고 기존 bee amount 기반 capture math를 적용한다.
- `DecreaseAliveRadius`/`SetAliveRadius`는 삭제하지 않고 legacy/manual visual adjustment API로 유지한다. BeeCarrier 포획 gameplay는 이 API를 사용하지 않는다.
- cluster Niagara parameter 계약:
  - `User.AliveRadius` (Float, density-derived `InitialAliveRadius`와 잔여 벌 수에서 파생)
  - `User.SpawnAmount` (Int32)
  - `User.SphereRadius` (Float, density-derived `InitialAliveRadius`와 동일)
- `CapturedBeeAmount >= SpawnAmount` 또는 잔여 벌 수가 0이면 captured로 전환한다. `SpawnAmount <= 0`인 본진도 포획 불가/완료 상태로 처리한다. 전환은 1회만 발생하고, `AliveRadius=0`을 Niagara에 적용한 뒤 `CaptureUseAreaMesh`를 비활성화하고 item-use-area descriptor를 rebuild한다.
- 최종 captured 이후 `ReceiveSwarmCaptured` Blueprint event를 호출한 뒤 C++가 다음 tick에 분봉 본진 actor destroy를 예약한다. 완료 연출이 필요하면 event 안에서 즉시 처리한다.
- 여왕벌은 `SwarmQueenBeeActorClass` child actor로 별도 생성하며 기존 벌통 `QueenBeeChildActor`를 이동하거나 재사용하지 않는다.
- 여왕벌 위치는 `ClusterCenter`에 attach된 `QueenBeeChildActor` relative location `QueenCenterOffset`을 사용한다. Density 기반 초기화 시점에 child actor relative rotation의 pitch/yaw/roll 세 축을 1회 랜덤화하며, `ApplyQueenBeeTransform()` 재호출은 위치만 보정하고 회전을 다시 뽑지 않는다.
- Blueprint events:
  - `ReceiveSwarmClusterInitialized`
  - `ReceiveAliveRadiusChanged(float NewAliveRadius)`
  - `ReceiveSwarmCaptured`

### `AWorldOccupancySiteActor`

- reusable world occupancy site base이며 inventory placement API나 `AItemPlacementSlotActor`에 의존하지 않는다.
- 구성:
  - `Root`
  - `OccupantSpawnPoint`
- authoring/state:
  - `bEnabled=true`
  - `bAutoReleaseWhenOccupantDestroyed=true`
  - `SiteTags`
  - `AcceptedOccupantClass`
  - transient `ReservedByActor`
  - transient `OccupyingActor`
- site state:
  - `Available`: enabled이고 유효 예약자/점유자가 없다.
  - `Reserved`: 유효 예약자가 있고 점유자는 없다.
  - `Occupied`: 유효 점유자가 있다.
- `TryReserve(RequestedBy)`는 available일 때만 성공한다.
- `TryOccupy(RequestedBy, Occupant)`는 available이거나 같은 requester가 예약한 경우에만 성공하고, 반드시 `CanAcceptOccupant(Occupant)`를 통과해야 한다.
- `ClearOccupant(Occupant)`는 현재 점유 actor와 일치할 때만 점유를 해제한다.
- `bAutoReleaseWhenOccupantDestroyed=true`이면 occupant `OnDestroyed`에 바인딩해 점유를 자동 해제한다.
- `GetOccupantSpawnTransform()`은 `OccupantSpawnPoint` world transform을 반환한다.

### `ABeeSwarmClusterSiteActor`

- `AWorldOccupancySiteActor` subclass이며 실제 colony swarming destination이다.
- constructor에서 `AcceptedOccupantClass` 기본값을 `ABeeSwarmClusterActor`로 설정한다.
- selection 설정:
  - `SelectionWeightMultiplier=1.0`
  - `DistanceWeightScaleCm=3000.0`
  - `DistanceWeightExponent=2.0`
  - `bUse2DDistanceForSelection=true`
  - `MaxSelectionDistanceCm=0.0` (`0`은 unlimited)
- `CalculateSelectionWeightForHive(Hive)`는 hive `SwarmExitPoint` 위치를 우선 사용하고 없으면 hive actor location을 사용한다.
- weight 공식:
  - `Weight = SelectionWeightMultiplier / Pow(1.0f + Distance / DistanceWeightScaleCm, DistanceWeightExponent)`
- `MaxSelectionDistanceCm > 0`이고 distance가 초과되면 `0`을 반환한다.
- scale/exponent/multiplier invalid 값은 safe positive/default 값으로 보정하며, `Weight <= 0` candidate는 실제 colony swarming 선택에서 제외된다.

### `ABeehiveSwarmRouteActor`

- `ABeeSplineSwarmActor` subclass다.
- `ConfigureRoute(StartWorldLocation, EndWorldLocation)`는 actor location을 start로 맞추고 `Start -> ForwardLeadPoint -> 자동 중간점들 -> End` spline arc를 구성한다.
- `ForwardLeadPoint`는 route actor forward 방향으로 `ForwardLeadDistance`만큼 앞선 점이며, `ABeehive`는 route actor spawn rotation을 `SwarmExitPoint` rotation으로 맞춘다.
- 자동 중간점 수는 `AutoMiddlePointSpacing` 기반 segment count에서 계산하고, `MaxAutoMiddlePointCount`로 clamp한 뒤 항상 홀수로 보정한다.
- 자동 중간점 높이는 `Sin(Alpha * PI) * RouteMidPointHeightOffset`으로 계산한다. 홀수 중간점의 중앙 index는 `Alpha == 0.5`라서 중앙점 1개만 `RouteMidPointHeightOffset`에 도달한다.
- `ConfigureRouteToCluster`는 legacy/helper API로 유지되지만, manual swarming test의 기본 시작 flow는 cluster를 아직 만들지 않으므로 target transform location을 `ConfigureRoute` end로 직접 사용한다.
- route actor는 cluster creation을 소유하지 않는다. `ABeehive` route-arrival handler가 pending target transform에 cluster를 spawn한다.
- route emission stop은 `ABeeSplineSwarmActor::StopExternalSwarmEmission()`으로 마지막 external parameter set을 재적용하되 `User.SpawnAmount=0`만 바꾼다.
- route actor cleanup은 `RouteArrivalDelaySeconds + RouteEmissionDurationSeconds` 뒤 수행해 이미 방출된 route bee가 target에 도달할 시간을 남긴다.
- route timing 공식:
  - `RouteArrivalDelaySeconds = GetSplineLength() / ((Max(0, SpeedMin) + Max(0, SpeedMax)) * 0.5)`
  - `RouteEmissionDurationSeconds = SessionClusterSpawnAmount / Max(0, SwarmRouteParameters.SpawnAmount)`
  - `RouteDestroyDelaySeconds = RouteArrivalDelaySeconds + RouteEmissionDurationSeconds`
- route Niagara parameter 계약은 기존 `ABeeSplineSwarmActor::ApplyExternalSwarmParameters`와 같다.
  - `User.SwarmSpline`
  - `User.SplineLength`
  - `User.StartShapeExtent`
  - `User.EndShapeExtent`
  - `User.SpawnAmount`
  - `User.SpeedMin`
  - `User.SpeedMax`

### `ABeehiveCombActor`

- `USceneComponent` root
- `USceneComponent` `CombPivotRoot` (소비장 내부 visual pivot)
- `UStaticMeshComponent` comb mesh
- `UNiagaraComponent` 2개 (`FrontFaceBeeNiagara`, `BackFaceBeeNiagara`)
- `UBeehiveCombPartFocusActionComponent` 1개 (`PartFocusAction`)
- `USceneComponent` 2개 (`QueenFrontAttachPoint`, `QueenBackAttachPoint`)
- `UStaticMeshComponent` 2개 (`FrontHoneyPlane`, `BackHoneyPlane`)
- `UStaticMeshComponent` 2개 (`FrontWaxCappingPlane`, `BackWaxCappingPlane`)
- `UItemUseAreaMeshComponent` 3개 (`BeeBrushUseAreaMesh`, `FrontWaxCappingUseAreaMesh`, `BackWaxCappingUseAreaMesh`)
- `UQueenCellSpawnAreaComponent` 1개 (`QueenCellSpawnArea`)
- visible face 상태: `EBeehiveCombVisibleFace` (`Front`/`Back`)
- flip API: `FlipCombFace`, `SetVisibleCombFace`, `GetVisibleCombFace`
- 방향 포함 flip API: `FlipCombFaceWithDirection(EBeehiveCombFlipDirection)`
- 방향 포함 BP 이벤트: `ReceiveCombFlippedWithDirection(NewVisibleFace, FlipDirection)`
- `SetVisibleCombFace(...)`는 visible face state만 저장한다. 실제 yaw/flip 회전 연출은 Blueprint event에서 구현한다.
- shake API: `ApplyCombShakeByRatio` (`ReduceAllTargetBeeCountsByRatio` 수행)
- flip/shake 시각 애니메이션은 C++ 보간 대신 Blueprint event(`ReceiveCombFlipped`, `ReceiveCombShaken`) 위임
- 상태:
  - `TotalSpawnAmount`는 `0` 이상 clamp
  - `FrontFaceTargetBeeCount`는 `0..FrontFaceSpawnAmount` clamp
  - `BackFaceTargetBeeCount`는 `0..BackFaceSpawnAmount` clamp
  - `TotalTargetBeeCount(Front+Back)`는 항상 `0..TotalSpawnAmount` 범위
  - `CurrentHoney`는 `0..MaxHoneyPerComb` clamp (초과분 폐기)
  - `CurrentHoneyRipeness`는 `0..MaxHoneyRipeness` clamp
  - face별 capping mask는 `RuntimeFrontWaxCappingMask`/`RuntimeBackWaxCappingMask` transient runtime byte buffer가 actor-local source of truth다.
  - inventory/item 이동 사이의 capping mask persistence path는 `FBeehiveCombItemState`이며, Blueprint class defaults는 큰 face mask 배열을 직렬화하지 않는다.
  - 기존 Blueprint class default에 저장된 `FrontWaxCappingMask`/`BackWaxCappingMask` payload와 연결되지 않도록 actor runtime property는 Core Redirect 없이 `Runtime*` 이름을 사용한다.
  - capping mask `255`는 밀랍 남음, `0`은 제거됨이다.
  - face 완료 기준은 `GetWaxCappingRemainingRatio(Face) <= UncappedThreshold`다.
  - capping plane 표시 조건은 `IsHoneyFull() && !IsWaxCappingFaceComplete(Face)`다.
  - `TryRegenerateWaxCapping()`은 `WaxCappingRegenerationRipenessThreshold` 이상으로 숙성된 full honey 소비장의 제거된 face mask를 face 단위로 `255` 복원한다.
  - `BeeDiseaseValue`는 `0..1` clamp된 legacy 런타임 시각값이며, front/back Niagara 직접 주입 경로는 주석처리되어 있다.
  - queen cell placement는 `FQueenCellPlacement` 배열로 runtime에만 보관한다. 저장 필드는 cell id, `EBeehiveCombVisibleFace`, spawn area local YZ, local rotation, scale이다.
  - queen cell runtime 표현은 `QueenCellRoot_N -> QueenCellVisual -> QueenCellUseArea` component group이며 actor가 아니다.
  - queen cell visual mesh/material과 use-area mesh/material은 comb Blueprint child에서 지정한다. `QueenCellUseAreaMesh`가 없으면 visual mesh fallback을 사용할 수 있다.
  - queen cell use-area tag는 `Item.UseArea.Beehive.QueenCell`이고 effect target은 component owner인 `ABeehiveCombActor`다.
  - queen cell은 `FBeehiveCombItemState`에 저장하지 않는다. `ApplyStateFromItemInstance()`는 runtime queen cell component/state를 비워 item state와 섞이지 않게 한다.
- `UQueenCellSpawnAreaComponent` axis contract:
  - local `X`: comb thickness/normal
  - local `+X`: front surface
  - local `-X`: back surface
  - local `Y/Z`: rectangular surface coordinates
  - bottom/left/right/top edge weight로 edge band를 고르고, center 영역과 `MinQueenCellSpacingCm` 위반 위치를 거부한다.
- 분배 규칙:
  - `FrontShare = (Total + 1) / 2`
  - `BackShare = Total / 2`
  - 홀수면 front가 1 더 가진다.
- 감소 API:
  - `ReduceAllTargetBeeCountsByRatio(float)` (`Ratio`는 `0..1` clamp, face별 감소량은 `RoundToInt(CurrentFaceTarget * Ratio)`)
  - `ReduceAllTargetBeeCountsByAmount(int32)` (양면 감소)
  - `ReduceVisibleFaceTargetBeeCountByAmount(int32)` (visible face 전용 감소)
  - `ReduceFaceTargetBeeCountByAmount(EBeehiveCombVisibleFace, int32)`
- 파라미터 적용 시점:
  - `OnConstruction`, `BeginPlay`, `PostEditChangeProperty`, 명시 API 호출 시
  - `OnConstruction`/`PostEditChangeProperty`는 editor-visible scalar state sanitize, Niagara parameter, honey transform/material-safe visual update만 수행하고, capping mask state/texture refresh는 game world context에서만 수행한다.
  - `BeginPlay`, bee parameter 변경 API, `ApplyWaxCappingBrush`, `TryRegenerateWaxCapping`, `ApplyStateFromItemInstance`는 runtime capping mask state/texture refresh 경로를 유지한다.
- Niagara user parameter 적용:
  - `User.PlaneSize` (Vector2D)
  - `User.SpawnAmount` (Int32, face별 분배값)
  - `User.TargetBeeCount` (Int32, face별 분배값)
  - legacy `User.Disease` 직접 적용 경로는 주석처리되어 있다.
- honey visual 적용:
  - fill ratio: `Clamp(CurrentHoney/MaxHoneyPerComb, 0..1)`
  - ripeness ratio: `Clamp(CurrentHoneyRipeness/MaxHoneyRipeness, 0..1)`
  - front/back plane relative location을 empty/full 위치 사이에서 보간
  - runtime wax/capping material에는 `HoneyRipeness` scalar와 transient texture parameter `WaxCappingMask`를 주입한다.
  - transient texture는 face별 byte mask에서 생성/갱신하며 `UPROPERTY(Transient)`로 GC 보호한다. Editor construction/details 변경 경로에서는 capping material dynamic instance가 없으면 `WaxCappingMask` texture를 생성하지 않는다.
  - material index 0 scalar parameter(`HoneyAmount`)에 fill ratio 적용
  - `FrontHoneyPlane`/`BackHoneyPlane` material index 0 scalar parameter(`HoneyRipeness`)에 ripeness ratio 적용
  - `FrontWaxCappingPlane`/`BackWaxCappingPlane` material index 0 scalar parameter(`HoneyRipeness`)에도 같은 ripeness ratio 적용
  - `FrontWaxCappingPlane`/`BackWaxCappingPlane`은 Editor/Blueprint authoring 중에는 보이게 두고, runtime에서는 `IsHoneyFull()` 기반 hidden-in-game 상태로 제어

### `AQueenBeeActor`

- `USceneComponent` root
- `UStaticMeshComponent` queen bee mesh
- `BaseEggLayingPower`는 colony population 증가량 계산의 기본 산란력이다.
- `DiseaseMaterialParameterName`, `DiseaseValue`, `SetDiseaseValue(...)`는 legacy API로 유지되며, `SetDiseaseValue(...)`는 값을 `0..1`로 clamp한다.
- queen mesh material scalar `Disease` 직접 적용 경로는 주석처리되어 있고, disease 시각 출력은 `ABeehive::DiseaseVfxNiagara`가 담당한다.
- Tick yaw jitter 정책은 disease 시각값과 분리되어 유지된다.

### `AWorldItemPickup`

- `USceneComponent` root
- `UStaticMeshComponent` pickup mesh
- `UFocusTargetComponent`
- `UPickupFocusActionComponent`
- `UItemDefinition` reference

`OnConstruction()`에서 item definition의 `WorldMesh`와 `DisplayName`을 mesh/prompt에 반영한다.

### `AStorageBox`

- `USceneComponent` root
- `UStaticMeshComponent` box mesh
- `UFocusTargetComponent`
- `UStorageBoxComponent`
- `UStorageBoxFocusActionComponent`

## Dependencies

- Focus
- Interaction
- Inventory
- UI

## Design Notes

- WorldActors는 상태 로직보다 component composition에 집중한다.
- 꿀 용기 간 이송 규칙은 concrete 작업대가 아니라 `UHoneyTransferComponent`에 둔다. 작업대 actor는 slot 조립 host이고, 꿀 줄기 VFX component는 source container가 소유한다.
- 꿀 용기 material parameter 정본은 `HoneyDensity`, `HoneyRipeness`다.
- 꿀 줄기 Niagara parameter 정본은 `User.HoneyDensity`, `User.HoneyRipeness`, `User.DropLength`다.
- Actor 이름과 native parent 이름은 Blueprint 참조가 있으므로 rename 시 Core Redirect와 Blueprint migration이 필요하다.
- Editor details customization은 editor-only 보조 기능이다. Runtime gameplay source of truth는 각 actor/component의 C++ parameter application 경로다.
- FocusEngaged item-use area는 벌통 전용 기능이 아니라 generic host-provider 구조로 다룬다.
- FocusEngaged host actor는 host에 부착된 `UItemUseAreaMeshProviderComponent`가 owner/direct child actor의 `UItemUseAreaMeshComponent`를 수집해 `FItemUseAreaDescriptor`를 구성하는 경로를 사용한다.
- `ABeehive`는 generic item-use-area 구조의 첫 구현 host로 본다.
- 사용영역 mesh는 기존 gameplay mesh component, 반투명 가상 mesh component, child actor 내부 mesh component를 모두 허용하되 최종적으로 descriptor의 `HitComponent`, `VisualComponents`, `EffectTargetObject`로 정규화한다.
- 벌떼 Niagara particle 이동 로직은 Niagara 시스템에서 처리하고 C++은 spline binding/parameter 주입만 담당한다.
- `ABeehive`의 방향별 spawn 계산식:
  - `Activity = SpawnAmountByHour ? Curve(Hour24/24) : FallbackActivity`
  - `RawSpawnAmount = Activity * ColonyBeeCount * SpawnAmountScale`
  - `FinalSpawnAmount = Clamp(RawSpawnAmount, 0, MaxSpawnAmount)`
- `OutgoingNiagara.User.bIsReverse=false`, `IngoingNiagara.User.bIsReverse=true`를 C++에서 항상 명시 적용한다.
- `ABeehiveCombActor` 소유 `FrontFaceBeeNiagara`/`BackFaceBeeNiagara`도 component details에서 `OverrideParameters`를 숨기고 C++ 적용값이 source of truth다.
- `ABeehive`는 `AEnvironmentTimeOfDayActor`를 직접 참조하지 않으며, bucket listener 이벤트의 `Hour24`를 받아서만 갱신한다.
- `ABeehive`의 queen 위치 갱신도 Environment actor 직접 참조 없이 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트로만 수행한다.
- `ABeehive`의 colony population 갱신도 동일하게 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트(`ColonyPopulation`)로만 수행한다.
- `ABeehive`의 honey ripeness + production 갱신도 동일하게 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트(`HoneyProduction`)로만 수행한다.
- `ABeehive`의 swarming pressure/queen cell lifecycle도 동일하게 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트(`SwarmingLifecycle`, 기본 30분)로만 수행한다.
- `ABeehive`의 pollen patty 고정 소모도 동일하게 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트(`PollenPattyConsumption`)로 수행한다.
- lifecycle 관련 같은 시간 경계 처리 순서는 `HoneyProduction` -> `ColonyPopulation` -> `SwarmingLifecycle` -> `PollenPattyConsumption`이다. 따라서 같은 boundary에서는 population 변화가 swarming pressure 계산에 먼저 반영된다.
- 같은 60분 경계에서는 subscription 순서를 `HoneyProduction` 먼저, `ColonyPopulation` 다음으로 두어 꿀 생산이 기존 벌 수 기준으로 선행 처리된다.
- 같은 경계에서 `ColonyPopulation`과 `PollenPattyConsumption`이 함께 발생하면 population update가 먼저 실행되고, 이후 consumption이 실행된다.
- pollen patty 소모량은 `PollenPattyConsumptionAmountPerBucket` 고정값이며, bucket 길이/이벤트 횟수/벌 수/온도에 따라 스케일하지 않는다.
- pollen patty 소모 대상 탐색은 direct child `AItemPlacementSlotActor` 수집 경로를 사용하며 provider descriptor `AreaTags`는 사용하지 않는다.
  - slot 매칭: `AItemPlacementSlotActor::GetSlotAreaTags().HasAll(PollenPattyConsumptionAreaTags)`
  - 후보 조건: occupied actor에 active `UPlacedItemRemainingComponent`가 있고 `CurrentAmount > 0`
  - 후보 선택: 벌통 local Y 기준 `Leftmost`(최소 Y) 또는 `Rightmost`(최대 Y), tie는 먼저 수집된 slot 유지
  - 소모 실행: 선택된 1개에만 `ConsumeAmount(...)` 호출, 같은 bucket spillover 없음
- colony population 계산식:
  - `Increase = QueenBaseEggLayingPower * ItemEggLayingBonus * TemperatureScore * BeeIncreaseCoefficient`
  - `EffectiveThreshold = Clamp(SanitationDiseaseThreshold, 0, MaxSanitationValue)`
  - `DiseaseRatio = 0 if EffectiveThreshold <= 0 or SanitationValue >= EffectiveThreshold, else Clamp((EffectiveThreshold - SanitationValue) / EffectiveThreshold, 0..1)`
  - `SanitationDecreaseMultiplier = Lerp(1.0, MaxSanitationBeeDecreaseMultiplier, DiseaseRatio)`
  - `ProportionalDecrease = ColonyBeeCount * BeeDecreaseCoefficient`
  - `AbsoluteDecrease = BeeDecreaseAbsoluteAmountPerBucket`
  - `Decrease = Min(ColonyBeeCount, ((ProportionalDecrease + AbsoluteDecrease) / ItemLifespanBonus / TemperatureScore) * SanitationDecreaseMultiplier)`
  - 최종 적용은 `RoundToInt`를 마지막 단계에서만 수행하고 최소 0으로 clamp한다.
  - `ItemEggLayingBonus`는 selected active pollen patty가 `UPollenPattyItemDefinition`이면 `Max(1.0, EggLayingMultiplier)`, 아니면 `1.0`이다.
  - `BeeDecreaseCoefficient`는 현재 벌 수에 비례하는 감소율이고, `BeeDecreaseAbsoluteAmountPerBucket`는 population bucket 1회당 고정 감소량이며 기본값 `0.0f`로 기존 동작을 유지한다.
  - `ItemEggLayingBonus`는 증가 항 전용이며, `ItemLifespanBonus`와 `TemperatureScore`는 비례 감소량과 절대 감소량을 더한 전체 감소 항에 적용된다.
  - `SanitationDiseaseThreshold=0.0f`이면 `DiseaseRatio=0`이고, `MaxSanitationBeeDecreaseMultiplier=1.0f`이면 위생 기반 추가 감소가 없다.
  - 감소량은 bucket 시작 시점의 기존 `ColonyBeeCount`를 초과하지 않으며, `ItemLifespanBonus`와 `TemperatureScore`는 기존 `1.0f` placeholder 정책을 유지한다.
- swarming pressure 계산식:
  - `ComfortBeeCapacity = ActiveCombCount * ComfortBeeCountPerComb`
  - `PopulationRatio = ColonyBeeCount / ComfortBeeCapacity`
  - `TargetPressure = Max(0, (PopulationRatio - PopulationStartRatio) / Max(0.0001, PopulationTriggerRatio - PopulationStartRatio))`
  - queen이 없거나 active comb가 없거나 capacity가 유효하지 않으면 target은 0이고 queen cell 생성/colony swarming trigger는 실행하지 않는다.
- queen cell target count 계산식:
  - `Alpha = Clamp((SwarmingPressure - QueenCellSpawnPressureThreshold) / Max(0.0001, SwarmingTriggerPressure - QueenCellSpawnPressureThreshold), 0..1)`
  - `DesiredQueenCellCount = RoundToInt(MaxQueenCellCountPerHive * Pow(Alpha, QueenCellSpawnExponent))`
  - target은 hive-wide count이며 comb별 target이 아니다.
  - 현재 count는 active comb 전체의 `GetQueenCellCount()` 합계다.
  - 부족분은 bucket당 `MaxQueenCellsSpawnPerBucket`까지만 생성한다.
  - 생성 후보 comb weight는 `1 / (1 + CurrentQueenCellCountOnComb)`이며, queen cell이 적은 comb가 더 자주 선택된다.
  - empty slot, lifted comb, spawn area가 없는 comb, per-comb cap에 도달한 comb, edge-band sample을 찾지 못하는 comb는 생성 후보에서 제외된다.
- queen이 붙은 comb가 lifted 상태가 되면 queen은 comb attach 상태를 유지하며 함께 이동하고, 다음 위치 갱신 후보에서만 lifted slot이 제외된다.
- honey ripeness는 `HoneyProduction` bucket에서 생산 전에 이미 full 상태였던 comb에만 증가한다. 같은 bucket에서 production으로 처음 full이 된 comb는 다음 bucket부터 숙성된다.
- honey 분배는 랜덤 가중치 정규화(`Weight / WeightSum`)를 사용하며, comb가 최대 꿀량에 도달해 생긴 초과분은 재분배하지 않고 버린다.
- Pickup은 획득 성공 시 destroy되고, 실패 시 actor를 유지한다.
- StorageBox는 storage 상태를 `UStorageBoxComponent`가 소유하고, UI lifecycle은 `UStorageBoxFocusActionComponent`가 처리한다.
- placement/comb 회수 prompt availability는 실제 회수 경로의 조건 helper를 공유한다. 예: `CanRetrievePlacementOccupant`, comb 회수 조건(`TotalTargetBeeCount == 0`, queen 미부착, `QueenCellCount == 0`), hotbar acquire dry-run 결과를 조합한다.
- 회수 조건 실패는 prompt entry 제거가 아니라 `FFocusPromptEntry::bEnabled=false` disabled 표시 대상이 될 수 있다.
- 뚜껑/소비장 같은 토글형 PartFocus primary action name은 공통 `UCursorPartFocusActionComponent`의 `PrimaryPromptActionText`/`EngagedPrimaryPromptActionText`를 사용하며, 표시 source는 `ResolvePrimaryPromptActionText()`를 따른다.
  - 뚜껑 기본 authoring: `열기` / `닫기`
  - 소비장 기본 authoring: `들기` / `넣기`

## Beehive Comb Delegate Ownership

- `ABeehive`는 자신이 현재 active slot으로 관리 중인 `ABeehiveCombActor`의 `PartFocusAction` delegate에만 바인딩한다.
- 바인딩은 comb part descriptor 등록 시점에 수행하며, 동일 handler 중복 등록을 막기 위해 `RemoveDynamic` 후 `AddDynamic`으로 재바인딩한다.
- handler는 `ActionComponent->GetOwner()`를 `ABeehiveCombActor`로 해석한 뒤, 현재 active comb membership을 다시 검증한다.
- membership 검증 실패 시 이벤트를 무시하므로 독립 배치된 comb actor는 `ABeehive` 이벤트 위임 경로에 들어오지 않는다.
- `ABeehive`는 검증 통과 시 `ReceiveCombPartFocusBegin/Cancel/Abort` Blueprint 이벤트로 위임한다.

## Beehive Comb Lift Component

- 소비장 들기/내리기 이동은 `UBeehiveCombLiftComponent`가 담당한다.
- 소비장 actor를 detach하지 않고, 소비장을 담는 slot `UChildActorComponent`의 relative transform을 이동/회전 보간한다.
- `ABeehive`는 `CombLiftTargetRoot`를 공통 목표 기준점으로 소유하며, `UBeehiveCombLiftComponent`는 이를 사용해 lifted target transform(위치/회전)을 계산한다.
- lifted target transform은 `CombLiftTargetRoot` world transform을 `CombRackRoot` 기준 relative transform으로 변환해 사용한다.
- scale은 `CombLiftTargetRoot` 값 대신 slot rest relative transform의 scale을 유지한다.
- 카메라 방향 기반 회전 계산, yaw 보정, rotation delta는 사용하지 않는다.
- layout refresh 후 재적용 시에도 최신 `CombLiftTargetRoot` transform을 다시 relative 변환해 즉시 적용한다.
- `CombLiftMoveDuration`은 들기/내리기 공통 보간 시간이다.
- layout refresh 정책:
  1. `RefreshCombSlotTransforms()`가 모든 slot rest transform을 먼저 갱신
  2. 직후 `ReapplyLiftedCombTransformAfterLayoutRefresh()`가 lifted 상태를 즉시 재적용

## Bucket Listener Notes

- `ABeehive`는 `BeginPlay`에서 자신을 `UGameTimeBucketSubsystem`에 등록한다.
- `ABeehive`는 `EndPlay`에서 자신을 `UGameTimeBucketSubsystem`에서 해제한다.
- Runtime spawn된 beehive도 외부 수동 등록 없이 bucket event를 받는다.

## Beehive Attraction Swarm

- `ABeehive`는 `AttractionSwarmNiagara`를 child actor 없이 직접 소유한다.
- Attraction center는 `AttractionSwarmNiagara` component transform 자체다.
- 설정은 `ABeehive`의 `FBeehiveAttractionSwarmSettings`로 노출된다.
- 적용되는 Niagara user parameter:
  - `User.AttractionPower` (Float)
  - `User.NoisePower` (Float)
  - `User.SpawnSphereRadius` (Float)
  - `User.SpawnAmount` (Int32, via `SetVariableInt`)
- legacy `User.Disease` 직접 적용 경로는 주석처리되어 있으며, disease 시각 출력은 `DiseaseVfxNiagara.User.Disease` 단일 경로를 사용한다.
- `SpawnAmount` 계산식:
  - `RoundToInt(ColonyBeeCount * SpawnAmountScale)`
  - clamp to `0..MaxSpawnAmount`
- 적용 시점: `OnConstruction`, `BeginPlay`, `PostEditChangeProperty`, 명시 apply call, bee-count setter.
- Attraction spawn amount에는 time/bucket 기반 auto update를 사용하지 않는다.
- 기존 outgoing/ingoing spline swarm은 유지된다.

## Manual Review Points

- Blueprint child에서 component 이름 변경 시 기존 serialized component override가 유지되는지 확인
- `AWorldItemPickup::OnConstruction()` 후 prompt/mesh가 item definition과 동기화되는지 확인
- StorageBox interaction 종료 후 storage component 상태는 유지되고 UI transient state만 정리되는지 확인
- `BeehiveDualSwarmActorCustomization`이 unrelated NiagaraComponent details를 숨기지 않고 벌통/소비장 관련 Niagara에만 적용되는지 확인

## Update 2026-05-24

- WorldActors bucket listener integration now consumes provider-backed bucket events (`UGameTimeBucketSubsystem` bound to `ITimeOfDayProvider`).
- No direct `AGameTimeOfDayActor` dependency is introduced in WorldActors gameplay actors.

## Update 2026-05-27

- `APlacedItemActor`를 추가했다.
  - 배치 아이템 1개를 대표하는 generic world actor
  - `InitializePlacedItem(UItemInstance*, AActor*)`로 source item definition과 owning placement slot owner를 초기화한다.
  - 기본 구성: `Root`, `ItemMesh`, `UPlacementOccupantComponent`, `UPlacedItemRetrievePartFocusActionComponent`(generic `UPlacementSlotRetrievePartFocusActionComponent` wrapper)
- `UPlacedItemRetrieveFocusActionComponent`를 추가했다.
  - preview hover + secondary input에서 회수를 수행한다.
  - 현재는 generic retrieve component(`UPlacementSlotRetrievePartFocusActionComponent`) 호출을 위임하는 compatibility wrapper로 유지한다.
  - hotbar acquire가 완전 성공(`AddedQuantity == 1`)일 때만 회수 성공 처리한다.
  - 성공 시 owning slot이 `IItemPlacementSlot`이면 `Execute_ClearPlacedItem`으로 slot/actor 제거를 위임한다.
  - slot 정보가 없으면 fallback으로 placed actor만 destroy한다.
- `AItemPlacementSlotActor::TryPlaceItem_Implementation`은 spawn/attach 성공 후, spawned actor가 `APlacedItemActor`일 때 `InitializePlacedItem(SourceItemInstance, this)`를 호출한다.

## Update 2026-05-27 (PartFocus Retrieve)

- `AItemPlacementSlotActor`는 `ICursorPartFocusProvider`를 구현한다.
  - empty 상태: item-use-area descriptor만 제공
  - occupied 상태: item-use-area descriptor는 제공하지 않고 placed item PartFocus descriptor를 제공
- placed item descriptor는 `APlacedItemActor`의 hit component/action component를 사용한다.
- `APlacedItemActor`는 host 내부 PartFocus 대상 actor이며 global focus target이 아니다.
  - 기본 구성: `Root`, `ItemMesh`, `UPlacedItemRetrievePartFocusActionComponent`(generic wrapper)
- 현재 secondary PartFocus 회수 로직의 source of truth는 `UPlacementSlotRetrievePartFocusActionComponent`다.
- `UPlacedItemRetrievePartFocusActionComponent`는 secondary PartFocus action 경로를 generic component로 위임하는 compatibility wrapper다.
  - acquire 완전 성공 + `AddedQuantity == 1`일 때만 성공
  - 성공 시 slot `ClearPlacedItem` 호출, 실패 시 actor/slot 유지
- `ABeehive::RebuildCursorPartFocusDescriptors()`는 기존 lid/comb 등록 후 `CursorPartFocusRegistration` append를 호출해 provider 기반 part를 추가 등록한다.

## Update 2026-05-27 (BeeBrush Lifted Comb UseArea)

- `ABeehiveCombActor`에 `BeeBrushUseAreaMesh`를 추가했다.
  - 타입: `UItemUseAreaMeshComponent`
  - 소비장 전용 BeeBrush item-use-area hit/visual 통합 component다.
  - `AreaTags`/`VisualSettings`/`EffectTargetPolicy`는 component 디테일에서 설정한다.
- lifted comb active 조건은 `ABeehiveCombActor::IsItemUseAreaMeshActive`에서 판정한다.
- comb lift begin/cancel/abort 후 `ItemUseAreaScope->RebuildItemUseAreaDescriptors()`를 호출해 descriptor를 즉시 갱신한다.

## Update 2026-05-27 (ItemUseAreaMesh Provider Integration)

- `ABeehive` item-use-area 등록 경로를 actor-level override에서 component 기반으로 전환했다.
  - `UItemUseAreaMeshProviderComponent`를 기본 subobject로 소유한다.
- `ABeehiveCombActor::BeeBrushUseAreaMesh`
  - 타입: `UItemUseAreaMeshComponent`
  - 부착: `CombMesh` 하위
  - active 조건: `IItemUseAreaActivationProvider` 구현으로 lifted comb일 때만 true
- `AItemPlacementSlotActor::SlotMeshComponent`
  - 타입: `UItemUseAreaMeshComponent`
  - active 조건: slot empty일 때만 true
  - occupied actor의 `UItemUseAreaMeshComponent`는 `IItemUseAreaMeshSource` 경로로 host provider에 노출한다.
  - place/clear 이후 host `UCursorItemUseAreaScopeComponent::RebuildItemUseAreaDescriptors()`를 호출해 즉시 반영
- `EffectTargetObject`는 각 use-area mesh의 `EffectTargetPolicy`로 결정한다.

## Update 2026-05-28 (Generic Placement Occupant + Beehive Comb Slot)

- 새 generic placement 타입:
  - `UPlacementOccupantComponent`
    - `RuntimeReturnItemDefinition`, `AuthoredReturnItemDefinition`, `OwningPlacementSlotActor`
    - `CanRetrievePlacementOccupant`, `PreClearPlacementOccupant`, BP native hook
  - `UPlacementSlotRetrievePartFocusActionComponent`
    - hovered PartFocus secondary 입력에서 회수 수행
    - 성공 조건: state-aware acquire(`TryAcquireItemBySpec`)의 `bSuccess && AddedQuantity == 1`
    - 성공 시 owning slot `ClearPlacedItem` 실행
- `AItemPlacementSlotActor` 확장:
  - `InitialOccupantActor` preplaced claim 지원 (BeginPlay)
  - `bAttachInitialOccupantToSlot`, `bSnapInitialOccupantToAttachPoint` 옵션 추가
  - spawned/preplaced actor가 `UPlacementOccupantComponent`를 가지면 slot ownership 주입
  - occupied actor의 `UItemUseAreaMeshComponent`를 provider source로 노출
  - clear 시 occupant `PreClearPlacementOccupant` 호출 후 destroy
- `APlacedItemActor` migration:
  - 기본 구성: `UPlacementOccupantComponent` + `UPlacedItemRetrievePartFocusActionComponent`(=`UPlacementSlotRetrievePartFocusActionComponent` wrapper) + `UPlacedItemRemainingComponent`
  - 기존 `InitializePlacedItem`, getter API는 deprecated wrapper 성격으로 유지
  - `UPlacedItemRetrievePartFocusActionComponent`/`UPlacedItemRetrieveFocusActionComponent`는 generic component wrapper로 축소
- 벌통 소비장 slot 구조:
  - `ABeehiveCombSlotActor : AItemPlacementSlotActor`
  - comb actor class 검증 후 배치 허용
  - place/clear/BeginPlay 이후 owning `ABeehive`에 `RefreshCombStateFromSlots()` 요청
  - comb slot은 occupied descriptor를 직접 등록하지 않고, comb part action의 secondary bridge를 사용
- 벌통 comb 관리 전환:
  - `ABeehive`는 `CombSlotActorClass`(기본 `ABeehiveCombSlotActor`)를 통해 slot child actor를 유지
  - BeginPlay에서만 `InitialCombCount`만큼 초기 comb를 slot에 배치
  - active comb는 slot의 `GetPlacedCombActor()`로 조회
  - honey/colony/queen/lift 관련 comb 순회는 placed comb 기준으로 동작
- 소비장 회수 조건:
- `UBeehiveCombPlacementOccupantComponent`가 회수 가능 정책을 구현
  - `TotalTargetBeeCount == 0`, `ABeehive::IsQueenBeeAttachedToComb(...) == false`, `QueenCellCount == 0`일 때만 회수 가능
  - prompt availability와 실제 secondary retrieve는 같은 회수 가능 조건을 공유해야 한다.

## Update 2026-05-31 (Placed Item Durability Remaining)

- `APlacedItemActor` 구성에 placed remaining 시스템을 추가했다.
  - 기본 subobject: `UPlacedItemRemainingComponent`
  - runtime visual: item definition spec의 `VisualComponentClass`를 검증 후 생성
- `UPlacedItemRemainingComponent` 정책:
  - `UItemDefinition::PlacedRemainingSpec.bUseDurabilityAsPlacedRemaining=true`일 때만 active
  - source item instance가 있으면 durability 복사
  - source item instance가 없는 preplaced actor는 full durability로 초기화
  - invalid config(`bUsesDurability=false` 또는 `MaxDurability<=0`)는 warning log + 비활성 처리
- 회수 경로:
  - `UPlacementSlotRetrievePartFocusActionComponent`가 remaining durability를 acquire spec으로 전달한다.
  - acquire 성공 후 `LastModifiedItemInstance`에 durability write-back을 수행하고 slot clear를 진행한다.
  - acquire 실패 시 placed actor/remaining 상태를 유지한다.
- 화분떡 전용 actor 경로 제거:
  - `APollenPattyActor`를 제거하고 `APlacedItemActor` 공용 경로를 사용한다.
  - 화분떡 scale visual은 `UPlacedItemAreaScaleRemainingVisualComponent`로 처리한다.

## Update 2026-05-31 (Pollen Patty Fixed Consumption)

- `ABeehive`에 화분떡 고정 소모 API/설정을 추가했다.
  - `ApplyPollenPattyConsumptionUpdate()`
  - `PollenPattyConsumptionBucketMinutes`, `bApplyPollenPattyConsumptionOnBeginPlayBucket`
  - `PollenPattyConsumptionAmountPerBucket`, `PollenPattyConsumptionSide`, `PollenPattyConsumptionAreaTags`
- bucket 구독 `SubscriptionTag="PollenPattyConsumption"`에서 선택된 slot 하나만 소모한다.
- slot tag 조회는 `AItemPlacementSlotActor::GetSlotAreaTags()`를 사용한다.
- 런타임 로직에 `Item.UseArea.Beehive.PollenPatty` 문자열 하드코딩을 두지 않고, `PollenPattyConsumptionAreaTags` authored 값으로만 판정한다.

## Update 2026-05-31 (Pollen Patty Population Bonus)

- `ABeehive::GetItemEggLayingBonus()`는 화분떡 소모 대상 선택 helper(`FindPollenPattyConsumptionTargetSlot`)를 재사용한다.
- bonus 대상 식별은 `PollenPattyConsumptionAreaTags` + active `UPlacedItemRemainingComponent` 기준을 그대로 재사용한다.
- 선택된 occupied actor의 item definition이 `UPollenPattyItemDefinition`일 때만 `EggLayingMultiplier`를 적용한다.
- 여러 active 화분떡이 있어도 bonus는 중첩하지 않고, selected 1개만 적용한다.

## Update 2026-06-01 (Retrieve Prompt Availability)

- 배치 아이템/소비장 회수 prompt는 PartFocus action의 `AppendPartFocusPromptEntries(...)` 경로에서 제공한다.
- 회수 entry는 회수 조건 또는 hotbar 수용 가능성 중 하나라도 실패하면 disabled 상태로 표시할 수 있다.
- disabled 표시 판정과 실제 `HandleSecondaryPartFocusAction` 실행 판정은 같은 helper/API를 공유해야 한다.

## Update 2026-06-01 (PartFocus Toggle Action Text)

- PartFocus primary prompt action text는 common action component에서 engaged 상태 기준으로 시작/해제 텍스트를 전환한다.
- `ABeehive` lid action은 별도 subclass 없이 common `UCursorPartFocusActionComponent`의 authored text로 `열기`/`닫기`를 표시할 수 있다.
- `UBeehiveCombPartFocusActionComponent`는 common resolver 기본 정책으로 소비장 `들기`/`넣기` primary prompt를 제공한다.

## Update 2026-06-02 (Honey Ripeness)

- `ABeehiveCombActor`에 꿀 숙성도 상태를 추가했다.
  - `MaxHoneyRipeness`
  - `CurrentHoneyRipeness`
  - `HoneyRipenessMaterialParameterName`
  - `AddHoneyRipeness`
  - `SetCurrentHoneyRipeness`
  - `GetCurrentHoneyRipeness`
  - `GetHoneyRipenessRatio`
  - `IsHoneyFull`
- `ApplyHoneyVisualState()`는 기존 `HoneyAmount`와 함께 `HoneyRipeness` scalar parameter를 front/back honey material instance 양쪽에 주입한다.
- `ABeehive`에 `HoneyRipenessIncreasePerBucket`와 `ApplyHoneyRipenessUpdate()`를 추가했다.
- `HoneyProduction` bucket branch는 `ApplyHoneyRipenessUpdate()` 후 `ApplyHoneyProductionUpdate()` 순서로 처리한다.
- `GetGameTimeBucketSubscriptions_Implementation()`에는 별도 `HoneyRipeness` subscription을 추가하지 않는다.
- `ApplyHoneyProductionUpdate()` 직접 호출은 숙성을 수행하지 않고 꿀 생산만 수행한다.

## Update 2026-06-03 (Sanitation Disease Visuals + Population Decrease)

- `ABeehive`가 sanitation disease의 source of truth다.
  - `GetSanitationDiseaseRatio()`: `SanitationDiseaseThreshold` 이하 구간에서 `0..1` disease ratio 계산
  - `GetSanitationBeeDecreaseMultiplier()`: disease ratio로 `1.0..MaxSanitationBeeDecreaseMultiplier` 감소 배율 계산
- colony population 감소 항은 비례 감소 + 절대 감소에 `ItemLifespanBonus`/`TemperatureScore`를 적용한 뒤 `SanitationDecreaseMultiplier`를 곱하고, 기존 `ColonyBeeCount`로 clamp한다.
- `SetSanitationValue()`는 `RefreshHiveDiseaseVisuals()`를 통해 disease 시각값을 즉시 전파한다.
- disease 시각값 적용 대상은 `ABeehive::DiseaseVfxNiagara.User.Disease` 단일 경로다.
- attraction swarm, dual swarm outgoing/ingoing, active comb front/back Niagara, queen material의 직접 `Disease` 적용 코드는 legacy path로 남아 있으나 주석처리되어 있다.
- disease 전용 Tick/subsystem/bucket subscription은 추가하지 않는다.

## Update 2026-06-08 (Uncapping Table + Comb Wax Capping Mask)

- `AUncappingTable`을 추가했다.
  - C++ native WorldActor
  - `UFocusTargetComponent` + `UAnchoredFocusCursorActionComponent`로 FocusConfirm 진입
  - `UCursorPartFocusScopeComponent`/registration/child provider로 placed comb PartFocus descriptor 수집
  - `UCursorItemUseAreaScopeComponent` + `UItemUseAreaMeshProviderComponent`로 empty slot placement area와 occupied comb capping use-area 수집
- `AUncappingTableCombSlot`을 추가했다.
  - `AItemPlacementSlotActor` subclass
  - `ABeehiveCombActor`만 accept
  - place 후 `ApplyStateFromItemInstance`, retrieve 전 `WriteStateToItemInstance`
  - occupied descriptor는 generic occupied descriptor 대신 `UCombUncappingPartFocusActionComponent`를 직접 노출
- `UCombUncappingPartFocusActionComponent`를 추가했다.
  - 작업대 소비장의 horizontal drag flip만 담당
  - PartFocus secondary retrieve는 comb의 `PlacementRetrieveAction`을 bridge한다.
  - 기존 `UBeehiveCombPartFocusActionComponent`를 상속/직접 재사용하지 않는다.
- `ABeehiveCombActor` capping mask/use-area/visual state를 확장했다.
  - `FrontWaxCappingUseAreaMesh`, `BackWaxCappingUseAreaMesh`
  - `CappingMaskLongSideResolution`, `UncappedThreshold`
  - transient runtime `RuntimeCappingMaskWidth`, `RuntimeCappingMaskHeight`, `RuntimeFrontWaxCappingMask`, `RuntimeBackWaxCappingMask`
  - transient `FrontWaxCappingMaskTexture`, `BackWaxCappingMaskTexture`
  - `ApplyWaxCappingBrush(...)`, `GetWaxCappingRemainingRatio(...)`, `IsWaxCappingFaceComplete(...)`, `IsWaxCappingComplete()`
- face mask 배열과 dimension은 runtime actor state이며 Blueprint class defaults에 직렬화하지 않는다. 이전 serialized mask property와의 연결을 끊기 위해 actor runtime property 이름은 Core Redirect 없이 `Runtime*` prefix를 사용한다. `FBeehiveCombItemState`는 소비장 capping mask state의 inventory/item persistence 계약으로 유지한다.
- capping mask dimension은 `PlaneSize` 비율과 long-side resolution에서 계산하며, invalid 저장 mask는 full mask fallback이다.
- capping visual은 game world runtime에서 byte buffer에서 transient `UTexture2D`를 갱신해 material parameter `WaxCappingMask`로 주입한다. Editor construction/details 변경 경로는 transient capping texture allocation/update를 피한다.
- 작업대 capping use-area active 조건:
  - host가 `AUncappingTable`
  - `AUncappingTableCombSlot::IsCombPartFocusEngaged()`가 false
  - `IsHoneyFull()`
  - component face가 현재 visible face
  - face remaining ratio가 `UncappedThreshold`보다 큼
- 이번 범위에서 벌통 소비장 lift/shake 정책, honey production/ripeness bucket, 채밀/수확/꿀 아이템 생산은 변경하지 않는다.

## Update 2026-06-09 (Wax Capping Regeneration)

- `ABeehiveCombActor`에 `TryRegenerateWaxCapping()`과 `WaxCappingRegenerationRipenessThreshold`를 추가했다.
- 재생성 조건은 `IsHoneyFull()`이고 `GetHoneyRipenessRatio() >= WaxCappingRegenerationRipenessThreshold`인 경우다.
- `ABeehive::ApplyHoneyRipenessUpdate()`와 `ABeehive::DistributeHoneyIncreaseToCombs()`만 자동 재생성을 호출한다. 작업대 배치, `SetCurrentHoney*`, `ApplyStateFromItemInstance()` 경로는 capping mask를 자동 복원하지 않는다.

## Update 2026-06-10 (Honey Container + Decanting Table)

- `AHoneyContainerActor`를 추가했다.
  - 꿀 용기 runtime state와 visual update owner
  - source 배출용 `HoneyStreamNiagara` owner
  - nozzle/retrieve PartFocus action owner
  - 회수 state write-back API 제공
- `AHoneyContainerSlotActor`를 추가했다.
  - `Source`/`Target` role
  - accepted gameplay tag query 기반 배치 판정
  - occupied retrieve/nozzle descriptor 제공
- `UHoneyTransferComponent`를 추가했다.
  - DropLength grow 후 transfer phase 진입
  - active source container `HoneyStreamNiagara`를 제어하고 target length를 `Max(0, SourceStream.Z - TargetPourTarget.Z)`로 계산
  - source volume 감소, target weighted-average 혼합
  - invalid slot/container, source empty, target full에서 auto stop
  - transfer state 변경 delegate를 제공하며, 소분 작업대는 이를 구독해 hover 유지 중인 노즐 PartFocus prompt를 즉시 재평가한다.
- `UHoneyNozzlePartFocusActionComponent`와 `UHoneyContainerRetrievePartFocusActionComponent`를 추가했다.
- `AHoneyDecantingTable`을 추가했다.
  - native anchored cursor FocusEngaged 작업대
  - source/target slot child actor를 transfer component에 연결
- 이번 범위에서 Content/Niagara asset은 수정하지 않는다. BP child에서 mesh/material/Niagara system/tag query를 authoring한다.

## Update 2026-06-13 (Swarming Test Cluster + Route)

- `ABeeSwarmClusterActor`를 추가했다.
  - 분봉 본진 actor이며 FocusEngaged host다.
  - `SpawnAmount`, `BeeDensityPerCubicMeter`, `CapturedBeeAmount`가 포획 진행 source of truth이고, 파생 `InitialAliveRadius`/`AliveRadius`/`SphereRadius`가 Niagara에 즉시 반영된다.
  - 별도 `SwarmQueenBeeActorClass` child actor를 cluster center에 유지한다.
  - `Item.UseArea.SwarmCluster.BeeCarrier` capture use-area를 generic mesh provider 경로로 제공한다.
  - child 여왕벌의 `Item.UseArea.QueenBee.QueenCage` use-area도 같은 generic mesh provider 경로로 수집된다.
- `ABeehiveSwarmRouteActor`를 추가했다.
  - `ABeeSplineSwarmActor`를 상속하고 runtime 거리 기반 자동 중간점 spline route를 구성한다.
  - route end는 route-arrival cluster spawn 이전에는 pending target transform location이고, legacy `ConfigureRouteToCluster` helper에서는 cluster center component world location이다.
- `ABeehive`에 외부 Blueprint 테스트용 분봉 시작 API를 추가했다.
  - `BeginSwarmingAtTransform`, `BeginSwarmingAtActor`, `ClearActiveTestSwarm`
  - 시작 성공 event: `ReceiveSwarmingStarted`
  - 시작 실패 event: `ReceiveSwarmingStartFailed`
- 분봉 테스트 시작은 기존 벌통 상태를 변경하지 않는다.
  - `ColonyBeeCount` 차감 없음
  - 기존 `QueenBeeChildActor` detach/move 없음
  - active comb bee count/target count 변경 없음
  - 자동 분봉 조건 및 bucket subscription 추가 없음

## Update 2026-06-14 (BeeCarrier Capture Amount + Volume Radius)

- `ABeeSwarmClusterActor`에 `CaptureBees`, `SetCapturedBeeAmount`, captured/remaining/total bee amount query API를 추가했다.
- 분봉 본진 포획 진행은 벌 수 기준이다. `AliveRadius`는 `InitialAliveRadius * cbrt(RemainingBeeAmount / TotalBeeAmount)`로 파생해 `User.AliveRadius`에 주입한다.
- `DecreaseAliveRadius`는 기존 Blueprint API 호환을 위해 남기며, BeeCarrier 포획 gameplay에서는 사용하지 않는다.
- `CaptureBees`에서 잔여 벌 수가 0이 되면 `bBeesCaptured=true`로 전환하고 BeeCarrier use-area를 비활성화하지만, `ReceiveSwarmCaptured`는 호출하지 않는다.

## Update 2026-06-14 (Queen Cage Capture + Swarm Final Completion)

- `AQueenBeeActor`에 `QueenCageUseAreaMesh`를 추가했다.
  - 부착: `QueenBeeMesh` 하위
  - area tag: `Item.UseArea.QueenBee.QueenCage`
  - effect target: `ComponentOwner`, 즉 `Context.ItemUseEffectTargetObject`는 `AQueenBeeActor`
  - `IsItemUseAreaMeshActive`는 captured queen에 대해 false를 반환한다.
- `AQueenBeeActor::MakeQueenCageItemState()`는 왕롱 저장용 state를 export한다.
  - 저장 값: queen class, `BaseEggLayingPower`, `DiseaseValue`
  - world actor reference는 저장하지 않는다.
- `IQueenBeeCaptureSource`를 추가했고, `ABeehive`와 `ABeeSwarmClusterActor`가 구현한다.
- `ABeehive`는 `bHasQueenBee`를 여왕벌 보유 source of truth로 사용한다.
  - `bHasQueenBee=false`이면 `GetQueenBeeActor()`는 null을 반환한다.
  - queen location bucket/update, `IsQueenBeeAttachedToComb`, `TryBrushQueenBeeFromCombVisibleFace`는 no-op/false가 된다.
  - `CalculateBeeIncreaseAmount()`는 queen이 없으면 0을 반환한다.
  - 포획 성공 시 child actor를 제거하고 item-use-area descriptor를 rebuild한다.
- `ABeeSwarmClusterActor`의 capture state는 3단계로 분리된다.
  - `bBeesCaptured`: BeeCarrier로 벌이 모두 포획되었거나 총 벌 수가 0인 상태
  - `bQueenCaptured`: 왕롱으로 분봉 본진 여왕벌이 포획된 상태
  - `bCaptured`: 최종 완료. `bBeesCaptured && bQueenCaptured`일 때만 true
- `ABeeSwarmClusterActor`의 density 기반 초기화는 분봉 본진 여왕벌 child actor 배치 후 pitch/yaw/roll 세 축을 1회 랜덤 회전한다. 이후 transform 보정은 위치만 갱신하고 랜덤 회전을 다시 적용하지 않는다.
- 분봉 본진에서 벌만 모두 포획되면 `AliveRadius=0`, cluster Niagara parameter 갱신, BeeCarrier use-area 비활성화, descriptor rebuild만 수행한다.
- 분봉 본진에서 여왕벌 포획 성공 시 queen child actor를 제거/재생성 차단하고 descriptor를 rebuild한다.
- `ReceiveSwarmCaptured`는 최종 완료 이벤트로 유지되며, `bBeesCaptured && bQueenCaptured && !bCaptured` 조건에서 1회만 호출된다. 이벤트 호출 후 C++가 다음 tick에 분봉 본진 actor를 destroy해 focus 대상에서도 제거한다.

## Update 2026-06-15 (Swarm Cluster Density Radius)

- `ABeehive` 분봉 테스트 cluster radius authoring 입력(`SwarmClusterInitialAliveRadius`, `SwarmClusterSphereRadius`)을 제거했다.
- 분봉 본진 생성 입력은 `SwarmClusterSpawnAmount`와 `SwarmClusterBeeDensityPerCubicMeter`이며, density 기본값은 `8000.0 bees/m^3`다.
- `ABeeSwarmClusterActor`는 density 기반 초기화에서 `InitialAliveRadius`와 `SphereRadius`를 밀도 공식으로 cm 단위 산출하고, 일반 초기화의 `AliveRadius`는 기존 잔여 벌 수 cube-root 공식으로 갱신한다. Route-arrival intro 초기화는 초기 event/Niagara 전에 `AliveRadius=0`을 적용한 뒤 intro 동안 성장시킨다.
- Niagara parameter 이름 `User.AliveRadius`, `User.SpawnAmount`, `User.SphereRadius`는 유지한다.
- 기존 radius 기반 `InitializeSwarmCluster(float, int32, float)` BlueprintCallable API는 제거 대상이며, 기존 Blueprint node 사용 asset은 수동 migration이 필요하다.

## Update 2026-06-15 (Route Arrival Cluster Spawn)

- `ABeehive::BeginSwarmingAtTransform`은 cluster를 즉시 spawn하지 않고 route actor만 시작한다.
- route spline은 `SwarmExitPoint`에서 요청 target transform location까지 구성된다.
- cluster actor는 `RouteArrivalDelaySeconds = RouteSplineLength / Avg(SpeedMin, SpeedMax)` 이후 pending target transform에 spawn된다.
- route emission duration은 `ActiveSwarmClusterSpawnAmount / SwarmRouteParameters.SpawnAmount`이고, 이 시점에 `StopExternalSwarmEmission()`이 `User.SpawnAmount=0`을 적용한다. 테스트 세션은 authored `SwarmClusterSpawnAmount`, colony-impact 세션은 `OutgoingBeeCount`를 사용한다.
- route actor destroy delay는 `RouteArrivalDelaySeconds + RouteEmissionDurationSeconds`이며 cluster actor는 route cleanup 이후에도 유지된다.
- cluster spawn 시 `InitializeSwarmClusterFromDensityWithIntroGrowth`가 cluster bees와 separate swarm queen을 함께 생성하고 intro 초기 `AliveRadius`를 적용한다. `ReceiveSwarmingStarted`는 이 시점에 호출된다.
- cluster intro growth는 `RouteEmissionDurationSeconds` 동안 `AliveRadius`만 `0`에서 target radius로 성장시킨다. `SpawnAmount`와 `SphereRadius`는 spawn 즉시 target Niagara 값이다.
- `USwarmClusterFocusActionComponent`가 intro growth 중 `CanBeginFocusAction=false`를 반환하므로 swarm cluster host는 intro 완료 전 FocusEngaged에 진입할 수 없다.
- BeeCarrier/QueenCage use-area에는 별도 intro gate를 추가하지 않는다. 해당 use-area는 FocusEngaged 안에서만 사용되며, intro 이후에는 기존 captured-state rule을 따른다.

## Update 2026-06-15 (Swarm Cluster Native FocusCollision)

- `ABeeSwarmClusterActor`가 native `USphereComponent FocusCollision`을 소유한다.
- `FocusCollision`은 preview focus trace용 hit proxy이며 `ECC_Visibility`만 block한다.
- `FocusCollision` radius는 `Max(0, AliveRadius) + 5`로 `ApplyClusterNiagaraParameters()` 경로에서 `AliveRadius` 변경과 함께 동기화한다.
- `USwarmClusterFocusActionComponent`는 FocusEngaged start 시 `FocusCollision`을 suppress/disable하고, return completed 또는 abort 시 suppression을 해제해 preview focus hit proxy를 복구한다.
- FocusEngaged 중 native preview hit proxy를 꺼야 BeeCarrier/QueenCage item-use-area cursor trace가 내부 use-area component를 hit할 수 있다.
- 기존 swarm cluster Blueprint의 authored `SphereCollision`은 제거하거나 `NoCollision`으로 변경한 뒤 compile/save해야 한다. C++ 구현은 임의의 Blueprint component를 이름으로 찾아 mutation하지 않는다.

## Update 2026-06-17 (Colony Swarming Site Selection)

- 기존 `ABeehive::BeginSwarmingAtTransform`/`BeginSwarmingAtActor`는 state-neutral 테스트/프레젠테이션 API로 유지한다.
- 실제 colony swarming API는 target parameter가 없는 `ABeehive::BeginColonySwarming()`이다. 제거된 `BeginColonySwarmingAtTransform`/`BeginColonySwarmingAtActor` Blueprint node는 `BeginColonySwarming`으로 수동 migration해야 한다.
- `BeginColonySwarming()`은 queen 보유와 양수 `ColonyBeeCount`를 요구한다.
- 새 설정 `ColonySwarmingBeeLossRatioMin=0.3`, `ColonySwarmingBeeLossRatioMax=0.6`은 `0.0..1.0` 비율이며, 계산 시 clamp/sort 후 `OutgoingBeeCount = Clamp(RoundToInt(PreSwarmBeeCount * LossRatio), 0, PreSwarmBeeCount)`를 산출한다.
- `AWorldOccupancySiteActor`를 추가해 reusable available/reserved/occupied world occupancy site 모델을 제공한다. 점유 actor destroy 시 자동 해제 옵션을 포함하며 inventory placement API와 분리된다.
- `ABeeSwarmClusterSiteActor`를 추가해 실제 colony swarming destination을 표현한다. 기본 occupant class는 `ABeeSwarmClusterActor`이고, hive 거리 기반 `CalculateSelectionWeightForHive()`를 제공한다.
- 실제 colony swarming은 world의 available `ABeeSwarmClusterSiteActor` 중 positive weight 후보를 weighted random으로 선택하고, 선택 site를 `TryReserve(this)`로 예약한 뒤 site occupant spawn transform으로 route를 시작한다.
- 실제 colony swarming은 route actor spawn/config/parameter/timing 계산 성공 후 `SetColonyBeeCount()`와 `SetHasQueenBee(false)`로 source hive state를 변경한다. `SetColonyBeeCount()`가 comb spawn amount refresh를 담당하므로 `ReduceAllCombTargetBeeCountsByConfiguredRatio()`는 호출하지 않는다.
- `ActiveSwarmClusterSpawnAmount`가 route timing과 arrival cluster initialization의 session bee count다. 테스트 세션은 `SwarmClusterSpawnAmount`, colony-impact 세션은 `OutgoingBeeCount`를 저장한다.
- route arrival cluster spawn 성공 후 pending site는 `TryOccupy(this, ClusterActor)`로 occupied가 되고 `ActiveSwarmClusterSite`로 이동한다. route start 실패는 commit 전에 reservation을 release하고, commit 이후 cluster spawn/occupation 실패는 reservation을 release하되 queen/bee count rollback은 수행하지 않는다.
- 이번 구현은 source hive queen state transfer를 보류한다. 실제 colony swarming으로 source queen은 제거되지만, spawned swarm queen은 기존 `SwarmQueenBeeActorClass` defaults로 생성된다.

## Update 2026-06-18 (Swarming Pressure + Queen Cells)

- `ABeehive`에 population-derived `SwarmingPressure`와 `SwarmingLifecycle` bucket을 추가했다. 기본 bucket은 30분이고, same-boundary lifecycle 순서는 `HoneyProduction` -> `ColonyPopulation` -> `SwarmingLifecycle` -> `PollenPattyConsumption`이다.
- pressure target은 `ComfortBeeCapacity = ActiveCombCount * ComfortBeeCountPerComb`, `PopulationRatio = ColonyBeeCount / ComfortBeeCapacity`, `TargetPressure = Max(0, (PopulationRatio - PopulationStartRatio) / Max(0.0001, PopulationTriggerRatio - PopulationStartRatio))`로 계산한다.
- queen cell target은 hive-wide count이며 `Alpha = Clamp((SwarmingPressure - QueenCellSpawnPressureThreshold) / Max(0.0001, SwarmingTriggerPressure - QueenCellSpawnPressureThreshold), 0..1)`, `DesiredQueenCellCount = RoundToInt(MaxQueenCellCountPerHive * Pow(Alpha, QueenCellSpawnExponent))`를 사용한다.
- missing queen cell은 bucket당 `MaxQueenCellsSpawnPerBucket`까지만 생성한다. 후보 active comb는 lifted 상태, per-comb cap, spawn area/sample 불가 조건을 제외하고, weight `1 / (1 + CurrentQueenCellCountOnComb)`로 weighted random 선택한다.
- `UQueenCellSpawnAreaComponent : UBoxComponent`를 추가했다. local `+X/-X`를 front/back surface로, `Y/Z`를 rectangular surface coordinates로 사용하고 edge-band 샘플만 허용한다.
- queen cell은 `ABeehiveCombActor` runtime component group이지 actor가 아니다. visual/use-area mesh/material은 comb Blueprint child에서 지정한다.
- `UQueenCellRemovalUseAction`은 `Item.UseArea.Beehive.QueenCell` hit component에서 cell id를 resolve해 제거하고, 제거 성공 시 owning hive pressure를 `QueenCellRemovalPressureDelta`만큼 낮춘다. 실제 item DataAsset 연결은 BP/DataAsset 작업이다.
- queen cell은 `FBeehiveCombItemState`에 저장하지 않으며, 존재하는 동안 comb retrieval을 막는다.
- 실제 `BeginColonySwarming()` 성공 시 `SwarmingPressure`는 `0.0f`로 리셋된다. `BeginSwarmingAtTransform`/`BeginSwarmingAtActor` 테스트 API는 state-neutral 상태를 유지한다.
