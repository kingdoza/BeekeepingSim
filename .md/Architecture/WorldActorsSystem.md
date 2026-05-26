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
- `Source/BeekeepingSim/Public/WorldActors/PollenPattyActor.h`
- `Source/BeekeepingSim/Private/WorldActors/PollenPattyActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/WorldItemPickup.h`
- `Source/BeekeepingSim/Private/WorldActors/WorldItemPickup.cpp`
- `Source/BeekeepingSim/Public/WorldActors/StorageBox.h`
- `Source/BeekeepingSim/Private/WorldActors/StorageBox.cpp`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlot.h`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`

## Responsibilities

- 월드 배치 가능한 gameplay actor 구성
- mesh/root/focus target/action/storage 같은 컴포넌트 조립
- Blueprint native parent로서 디자이너가 asset과 연출을 붙일 수 있는 기반 제공
- Focus, Interaction, Inventory 시스템의 연결 지점 제공
- 벌통 내부 파츠 FocusAction policy preset과 소비장 lift movement 제공
- 벌통/소비장/벌떼 Niagara editor details에서 C++ source-of-truth parameter를 숨기는 editor-only customization 제공

## Key Classes

- `ABeehive`: anchored focus/cursor interaction 예시 actor + item-use-area first host(provider/scope) + `ABeehiveDualSwarmActor` child 소유 및 시간/벌 수 기반 parameter 주입 지점
- `APollenPattyActor`: 벌통 slot에 부착되는 pollen patty 월드 표시 actor
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
- `AItemPlacementSlotActor`: `IItemUseAreaProvider` + `IItemPlacementSlot`를 구현하는 generic 배치 슬롯 actor
  - 구성: `Root`, `SlotMeshComponent`, `AttachComponent`
  - `SlotMeshComponent`는 hit + visual component를 겸용한다.
  - `SlotMeshAsset`으로 슬롯 인스턴스별 영역 mesh를 지정할 수 있다.
  - `SlotMeshMaterial`로 슬롯 인스턴스별 item-use-area 표시 material을 지정할 수 있다.
  - `SlotMeshRelativeTransform`으로 hit/visual mesh의 local transform을 조정한다.
  - `AttachRelativeTransform`으로 placed actor attach point의 local transform을 조정한다.
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
- queen 위치 갱신 규칙:
  - active comb 후보에서 현재 lifted comb slot 제외
  - 중앙 slot일수록 높은 가중치로 weighted random 선택
  - 선택된 comb의 front/back attach point를 50:50로 선택
  - attach point 기준 회전에 `0..360` 랜덤 yaw를 relative rotation으로 추가
- `ApplyBeeSwarmSettings()`에서 child actor 재생성 없이 기존 instance에 계산된 DTO를 재주입
- 시간 갱신(`ApplyBeeSwarmHour24`) 경로에서도 class 변경 없이 parameter만 갱신
- `CombSlotSpacing` 기준 local `Y` 중앙 정렬 배치:
  - `HalfSpan = (MaxCombCount - 1) * 0.5 * CombSlotSpacing`
  - `SlotY = -HalfSpan + (i * CombSlotSpacing)`
  - 전체 slot 배열 중심은 `CombRackRoot` origin
- `CurrentCombCount` 정책: `MaxCombCount` 변경 시 강제 초기화 없이 `0..MaxCombCount` clamp만 수행
- `CurrentCombCount <= 0`이면 활성 comb actor가 없고 comb spawn amount는 0
- comb spawn amount 계산식: `RoundToInt(ColonyBeeCount * Clamp01(CombSpawnAmountRatio) / CurrentCombCount)`
- `SetColonyBeeCount` 포함 spawn amount 갱신 경로에서 active comb의 target bee count를 spawn amount로 리셋
- `ApplyColonyPopulationUpdate()` 경로에서는 lifted comb slot을 제외한 active comb에만 spawn/target 갱신을 적용
- `ApplyHoneyProductionUpdate()` 경로에서는 들림 여부와 무관하게 모든 active comb에 꿀 증가량을 적용
- 공통 plane size source of truth는 `ABeehive::CombPlaneSize`이며 active comb actor에 일괄 주입
- cursor part focus 등록:
  - lid component part (`PersistentAction`, `ProvidedStateTags={Beehive.LidOpen}`)
  - active comb actor parts (`PersistentAction`, `RequiredStateTags={Beehive.LidOpen}`, `ExclusiveGroup={Beehive.CombLift}`)
  - preview-only component parts (prompt 없음, LMB click no-op)
  - preview key 입력(`R/F/C`)은 hover preview 대상의 action handler에서 선택적으로 처리
- 현재 native `ABeehive` 기본 subobject(`LidPartFocusAction`)는 공통 `UCursorPartFocusActionComponent`를 사용하고, `ABeehiveCombActor` 기본 subobject(`PartFocusAction`)는 `UBeehiveCombPartFocusActionComponent`를 사용한다.
- Beehive/Comb의 실제 lid open-close, comb lift-restore는 action component의 owner-actor delegate(`OnPartFocusBegin/Cancel/Abort`) 또는 component 이벤트 구현 경로에서 처리한다.
- `ABeehive`는 `IItemUseAreaProvider`를 구현해 lid/comb descriptor(`FItemUseAreaDescriptor`)를 제공한다.
- descriptor 기본 tag:
  - lid: `Beehive.UseArea.Lid`
  - comb: `Beehive.UseArea.Comb`
- item-use 확장:
  - sanitation 상태: `SanitationValue`, `MaxSanitationValue`, `IncreaseSanitation`, `SetSanitationValue`, `GetSanitationRatio`
  - `ABeehive`는 pollen slot 상태를 직접 소유하지 않는다.
  - pollen slot은 `AItemPlacementSlotActor` child actor로 authoring하며, occupied 상태는 slot actor의 `PlacedActor`가 소유한다.
  - slot actor는 empty일 때만 pollen descriptor(`Item.UseArea.Beehive.PollenPatty`)를 provider로 반환한다.
  - 벌통 포함 모든 host actor는 필요 시 `UChildItemUseAreaProviderComponent`를 붙여 child slot provider를 `Component Tags`/class 조건으로 노출한다.
  - disinfectant descriptor(`Item.UseArea.Beehive.Disinfectant`)는 lid/comb에서 계속 제공

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
- shake API: `ApplyCombShakeByRatio` (`ReduceTargetBeeCountByRatio`만 수행)
- flip/shake 시각 애니메이션은 C++ 보간 대신 Blueprint event(`ReceiveCombFlipped`, `ReceiveCombShaken`) 위임
- 상태:
  - `SpawnAmount`는 `0` 이상 clamp
  - `TargetBeeCount`는 항상 `0..SpawnAmount` clamp
  - `CurrentHoney`는 `0..MaxHoneyPerComb` clamp (초과분 폐기)
- 감소 API:
  - `ReduceTargetBeeCountByRatio(float)` (`Ratio`는 `0..1` clamp, 감소량은 `RoundToInt(CurrentTargetBeeCount * Ratio)`)
  - `ReduceTargetBeeCountByAmount(int32)`
- 파라미터 적용 시점:
  - `OnConstruction`, `BeginPlay`, `PostEditChangeProperty`, 명시 API 호출 시
- Niagara user parameter 적용:
  - `User.PlaneSize` (Vector2D)
  - `User.SpawnAmount` (Int32)
  - `User.TargetBeeCount` (Int32)
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
- FocusEngaged host actor는 자신의 provider 구현 또는 host에 붙은 provider component를 통해 필요한 `FItemUseAreaDescriptor`를 구성한다.
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
- 같은 60분 경계에서는 subscription 순서를 `HoneyProduction` 먼저, `ColonyPopulation` 다음으로 두어 꿀 생산이 기존 벌 수 기준으로 선행 처리된다.
- colony population 계산식:
  - `Increase = QueenBaseEggLayingPower * ItemEggLayingBonus * TemperatureScore * BeeIncreaseCoefficient`
  - `Decrease = ColonyBeeCount * BeeDecreaseCoefficient / ItemLifespanBonus / TemperatureScore`
  - 최종 적용은 `RoundToInt`를 마지막 단계에서만 수행하고 최소 0으로 clamp한다.
  - 1차 구현에서는 item/temperature bonus를 각각 `1.0f`로 고정한다.
- queen이 붙은 comb가 lifted 상태가 되면 queen은 comb attach 상태를 유지하며 함께 이동하고, 다음 위치 갱신 후보에서만 lifted slot이 제외된다.
- honey 분배는 랜덤 가중치 정규화(`Weight / WeightSum`)를 사용하며, comb가 최대 꿀량에 도달해 생긴 초과분은 재분배하지 않고 버린다.
- Pickup은 획득 성공 시 destroy되고, 실패 시 actor를 유지한다.
- StorageBox는 storage 상태를 `UStorageBoxComponent`가 소유하고, UI lifecycle은 `UStorageBoxFocusActionComponent`가 처리한다.

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
  - 기본 구성: `Root`, `ItemMesh`, `FocusTarget`, `UPlacedItemRetrieveFocusActionComponent`
- `UPlacedItemRetrieveFocusActionComponent`를 추가했다.
  - preview hover + secondary input에서 회수를 수행한다.
  - hotbar `TryAcquireItem(ItemDefinition, 1)`이 완전 성공(`AddedQuantity == 1`)일 때만 회수 성공 처리한다.
  - 성공 시 owning slot이 `IItemPlacementSlot`이면 `Execute_ClearPlacedItem`으로 slot/actor 제거를 위임한다.
  - slot 정보가 없으면 fallback으로 placed actor만 destroy한다.
- `AItemPlacementSlotActor::TryPlaceItem_Implementation`은 spawn/attach 성공 후, spawned actor가 `APlacedItemActor`일 때 `InitializePlacedItem(SourceItemInstance, this)`를 호출한다.
- `APollenPattyActor` native class/file/UCLASS rename은 수행하지 않는다.

## Update 2026-05-27 (PartFocus Retrieve)

- `AItemPlacementSlotActor`는 `ICursorPartFocusProvider`를 구현한다.
  - empty 상태: item-use-area descriptor만 제공
  - occupied 상태: item-use-area descriptor는 제공하지 않고 placed item PartFocus descriptor를 제공
- placed item descriptor는 `APlacedItemActor`의 hit component/action component를 사용한다.
- `APlacedItemActor`는 host 내부 PartFocus 대상 actor이며 global focus target이 아니다.
  - 기본 구성: `Root`, `ItemMesh`, `UPlacedItemRetrievePartFocusActionComponent`
- `UPlacedItemRetrievePartFocusActionComponent`는 secondary PartFocus action으로 회수를 처리한다.
  - `TryAcquireItem(ItemDefinition, 1)` 완전 성공 + `AddedQuantity == 1`일 때만 성공
  - 성공 시 slot `ClearPlacedItem` 호출, 실패 시 actor/slot 유지
- `ABeehive::RebuildCursorPartFocusDescriptors()`는 기존 lid/comb 등록 후 `CursorPartFocusRegistration` append를 호출해 provider 기반 part를 추가 등록한다.
