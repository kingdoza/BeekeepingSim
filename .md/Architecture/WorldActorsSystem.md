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
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActorCustomization.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActorCustomization.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
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

- `ABeehive`: anchored focus/cursor interaction 예시 actor + item-use-area first host(provider/scope) + `ABeehiveDualSwarmActor` child 소유 및 시간/벌 수 기반 parameter 주입 지점
- `ABeehiveCombActor`: 벌통 내부 소비장 mesh + 양면 Niagara(`FrontFaceBeeNiagara`, `BackFaceBeeNiagara`)를 소유하는 actor
- `UBeehiveCombLiftComponent`: active comb slot의 child actor component relative transform을 보간해 소비장 들기/내리기를 수행하는 component
- `UBeehiveLidPartFocusActionComponent`: lid open part action policy preset (`PersistentAction`, `ProvidedStateTags={Beehive.LidOpen}`)
- `UBeehiveCombPartFocusActionComponent`: comb lift part action policy preset (`PersistentAction`, `RequiredStateTags={Beehive.LidOpen}`, `ExclusiveGroup={Beehive.CombLift}`) + comb drag gesture 해석 owner
- `AQueenBeeActor`: 여왕벌 mesh actor, Tick마다 local yaw jitter 누적 + `BaseEggLayingPower` 보유
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
- `UBeehiveCombPlacementOccupantComponent`: comb 회수 가능 조건(`TotalTargetBeeCount`, queen attach)을 구현하는 occupant subclass다.
- `FBeehiveDualSwarmActorCustomization` / `FBeehiveDualSwarmNiagaraComponentCustomization`: editor-only details customization. `OverrideParameters` 같은 C++ 적용값의 details 노출을 숨긴다.

## Composition

### `ABeehive`

- `USceneComponent` root
- `UStaticMeshComponent` body/lid mesh
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UChildActorComponent` 1개 (`BeehiveSwarmChildActor`)
- `UChildActorComponent` 1개 (`QueenBeeChildActor`)
- `USceneComponent` 1개 (`CombRackRoot`) + `MaxCombCount` 크기의 comb slot child actor component 배열
- `USceneComponent` 1개 (`CombLiftTargetRoot`)
- `UBeehiveCombLiftComponent` 1개 (`CombLiftComponent`)
- `UCursorPartFocusScopeComponent` 1개 (`CursorPartFocusScope`)
- `UCursorPartFocusActionComponent` 1개 (`LidPartFocusAction`)
- `UCursorItemUseAreaScopeComponent` 1개 (`ItemUseAreaScope`)
- `USplineComponent` 1개 (`SwarmSpline`)를 직접 소유하며 레벨 인스턴스별 편집 대상
- `UNiagaraComponent` 1개 (`AttractionSwarmNiagara`)
- `BeeSplineSwarmActorClass` (`TSubclassOf<ABeehiveDualSwarmActor>`)로 child class 지정
- `ColonyBeeCount`, `BeeSwarmHour24`, `DualSwarmCommonSettings`, `OutgoingSwarmSettings`, `IngoingSwarmSettings`
- `IGameTimeBucketListener`를 구현해 시간 bucket 이벤트를 구독
- 기본 bucket 설정: `BeeSwarmBucketMinutes=10`, BeginPlay 즉시 적용 옵션 지원
- queen 위치 bucket 설정: `QueenBeeLocationBucketMinutes=60`, BeginPlay 즉시 적용 옵션 지원
- colony population bucket 설정: `ColonyPopulationBucketMinutes=60`, `bApplyColonyPopulationOnBeginPlayBucket=false` 기본
- colony population 계수 설정: `BeeIncreaseCoefficient`, `BeeDecreaseCoefficient`
- honey production bucket 설정: `HoneyProductionBucketMinutes=60`, `bApplyHoneyProductionOnBeginPlayBucket=false` 기본
- honey production 계수/분배 설정: `HoneyProductionCoefficient`, `HoneyDistributionDeviationRatio`
- pollen patty consumption bucket 설정: `PollenPattyConsumptionBucketMinutes=60`, `bApplyPollenPattyConsumptionOnBeginPlayBucket=false` 기본
- pollen patty consumption 설정: `PollenPattyConsumptionAmountPerBucket`, `PollenPattyConsumptionSide(Leftmost/Rightmost)`, `PollenPattyConsumptionAreaTags`
- colony population의 `ItemEggLayingBonus`는 선택된 active 화분떡 1개(`UPollenPattyItemDefinition`)의 `EggLayingMultiplier`를 사용한다.
- queen 위치 갱신 규칙:
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
- `ApplyHoneyProductionUpdate()` 경로에서는 들림 여부와 무관하게 모든 active comb에 꿀 증가량을 적용
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
- `ABeehive`는 sanitation과 별도로 aggression 상태(`MaxAggressionValue`, `AggressionValue`)를 소유하며, `DecreaseAggression`, `SetAggressionValue`, `GetAggressionRatio` API를 제공한다.
- aggression은 훈연기 hold-use 효과로만 감소하고, 현재 범위에서는 자동 회복/공격력 계산과 연결하지 않는다.
- item-use 확장:
  - sanitation 상태: `SanitationValue`, `MaxSanitationValue`, `IncreaseSanitation`, `SetSanitationValue`, `GetSanitationRatio`
  - `ABeehive`는 pollen slot 상태를 직접 소유하지 않는다.
  - pollen slot은 `AItemPlacementSlotActor` child actor로 authoring하며, occupied 상태는 slot actor의 `PlacedActor`가 소유한다.
  - slot actor는 `IItemUseAreaActivationProvider`로 occupied 여부를 판단한다.
  - empty slot: descriptor active(`AreaTags` 유지)
  - occupied slot: descriptor inactive(`AreaTags` 비움)
  - host provider는 child actor 내부 `UItemUseAreaMeshComponent`를 수집한다.

### `ABeehiveDualSwarmActor`

- `USceneComponent` root
- `UNiagaraComponent` outgoing/ingoing
- `ApplySwarmSpline(USplineComponent*)`로 Beehive 소유 spline reference를 주입받는다.
- `ApplySplineBindings()`에서 주입받은 spline으로 `User.SwarmSpline`, `User.SplineLength`, `User.bIsReverse`를 양쪽 Niagara에 명시 적용
- `ApplyDualSwarmParameters()`에서 공통 shape + 방향별 spawn/speed 적용
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

### `ABeehiveCombActor`

- `USceneComponent` root
- `USceneComponent` `CombPivotRoot` (소비장 내부 visual pivot)
- `UStaticMeshComponent` comb mesh
- `UNiagaraComponent` 2개 (`FrontFaceBeeNiagara`, `BackFaceBeeNiagara`)
- `UBeehiveCombPartFocusActionComponent` 1개 (`PartFocusAction`)
- `USceneComponent` 2개 (`QueenFrontAttachPoint`, `QueenBackAttachPoint`)
- `UStaticMeshComponent` 2개 (`FrontHoneyPlane`, `BackHoneyPlane`)
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
- Niagara user parameter 적용:
  - `User.PlaneSize` (Vector2D)
  - `User.SpawnAmount` (Int32, face별 분배값)
  - `User.TargetBeeCount` (Int32, face별 분배값)
- honey visual 적용:
  - fill ratio: `Clamp(CurrentHoney/MaxHoneyPerComb, 0..1)`
  - front/back plane relative location을 empty/full 위치 사이에서 보간
  - material index 0 scalar parameter(`HoneyAmount`)에 fill ratio 적용

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
- `ABeehive`의 honey production 갱신도 동일하게 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트(`HoneyProduction`)로만 수행한다.
- `ABeehive`의 pollen patty 고정 소모도 동일하게 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트(`PollenPattyConsumption`)로 수행한다.
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
  - `Decrease = ColonyBeeCount * BeeDecreaseCoefficient / ItemLifespanBonus / TemperatureScore`
  - 최종 적용은 `RoundToInt`를 마지막 단계에서만 수행하고 최소 0으로 clamp한다.
  - `ItemEggLayingBonus`는 selected active pollen patty가 `UPollenPattyItemDefinition`이면 `Max(1.0, EggLayingMultiplier)`, 아니면 `1.0`이다.
  - `ItemLifespanBonus`/`Decrease`는 화분떡 bonus와 분리되어 기존 `1.0f` 정책을 유지한다.
- queen이 붙은 comb가 lifted 상태가 되면 queen은 comb attach 상태를 유지하며 함께 이동하고, 다음 위치 갱신 후보에서만 lifted slot이 제외된다.
- honey 분배는 랜덤 가중치 정규화(`Weight / WeightSum`)를 사용하며, comb가 최대 꿀량에 도달해 생긴 초과분은 재분배하지 않고 버린다.
- Pickup은 획득 성공 시 destroy되고, 실패 시 actor를 유지한다.
- StorageBox는 storage 상태를 `UStorageBoxComponent`가 소유하고, UI lifecycle은 `UStorageBoxFocusActionComponent`가 처리한다.
- placement/comb 회수 prompt availability는 실제 회수 경로의 조건 helper를 공유한다. 예: `CanRetrievePlacementOccupant`, comb 회수 조건(`TotalTargetBeeCount == 0`, queen 미부착), hotbar acquire dry-run 결과를 조합한다.
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
  - `TotalTargetBeeCount == 0` 및 `ABeehive::IsQueenBeeAttachedToComb(...) == false`일 때만 회수 가능
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
