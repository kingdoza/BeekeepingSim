# WorldActors System

## Scope

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSplineSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/WorldItemPickup.h`
- `Source/BeekeepingSim/Private/WorldActors/WorldItemPickup.cpp`
- `Source/BeekeepingSim/Public/WorldActors/StorageBox.h`
- `Source/BeekeepingSim/Private/WorldActors/StorageBox.cpp`

## Responsibilities

- 월드 배치 가능한 gameplay actor 구성
- mesh/root/focus target/action/storage 같은 컴포넌트 조립
- Blueprint native parent로서 디자이너가 asset과 연출을 붙일 수 있는 기반 제공
- Focus, Interaction, Inventory 시스템의 연결 지점 제공

## Key Classes

- `ABeehive`: anchored focus/cursor interaction 예시 actor + `ABeehiveDualSwarmActor` child 소유 및 시간/벌 수 기반 parameter 주입 지점
- `ABeehiveCombActor`: 벌통 내부 소비장 mesh + 양면 Niagara(`FrontFaceBeeNiagara`, `BackFaceBeeNiagara`)를 소유하는 actor
- `AQueenBeeActor`: 여왕벌 mesh actor, Tick마다 local yaw jitter 누적 담당
- `ABeehiveDualSwarmActor`: outgoing/ingoing Niagara 2개를 가진 벌통 전용 actor (Spline은 `ABeehive` 소유)
- `ABeeSplineSwarmActor`: 기존 단일 swarm actor로 유지되며 dual swarm 구조와 별개 (`User.SwarmSpline`/`User.SplineLength` 바인딩 유지)
- `AWorldItemPickup`: 단일 item definition 기반 pickup actor
- `AStorageBox`: storage inventory와 storage UI interaction을 가진 actor

## Composition

### `ABeehive`

- `USceneComponent` root
- `UStaticMeshComponent` body/lid mesh
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UChildActorComponent` 1개 (`BeehiveSwarmChildActor`)
- `UChildActorComponent` 1개 (`QueenBeeChildActor`)
- `USceneComponent` 1개 (`CombRackRoot`) + `MaxCombCount` 크기의 comb slot child actor component 배열
- `UCursorPartFocusScopeComponent` 1개 (`CursorPartFocusScope`)
- `UCursorPartFocusActionComponent` 1개 (`LidPartFocusAction`)
- `USplineComponent` 1개 (`SwarmSpline`)를 직접 소유하며 레벨 인스턴스별 편집 대상
- `BeeSplineSwarmActorClass` (`TSubclassOf<ABeehiveDualSwarmActor>`)로 child class 지정
- `ColonyBeeCount`, `BeeSwarmHour24`, `DualSwarmCommonSettings`, `OutgoingSwarmSettings`, `IngoingSwarmSettings`
- `IGameTimeBucketListener`를 구현해 시간 bucket 이벤트를 구독
- 기본 bucket 설정: `BeeSwarmBucketMinutes=10`, BeginPlay 즉시 적용 옵션 지원
- queen 위치 bucket 설정: `QueenBeeLocationBucketMinutes=60`, BeginPlay 즉시 적용 옵션 지원
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
- 공통 plane size source of truth는 `ABeehive::CombPlaneSize`이며 active comb actor에 일괄 주입
- cursor part focus 등록:
  - lid component part (`PersistentAction`, `ProvidedStateTags={Beehive.LidOpen}`)
  - active comb actor parts (`PersistentAction`, `RequiredStateTags={Beehive.LidOpen}`, `ExclusiveGroup={Beehive.CombLift}`)
  - preview-only component parts (prompt 없음, LMB click no-op)
  - preview key 입력(`R/F/C`)은 hover preview 대상의 action handler에서 선택적으로 처리
- Beehive/Comb의 실제 lid open-close, comb lift-restore는 action component의 owner-actor delegate(`OnPartFocusBegin/Cancel/Abort`) 또는 component 이벤트 구현 경로에서 처리한다.

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
- `UStaticMeshComponent` comb mesh
- `UNiagaraComponent` 2개 (`FrontFaceBeeNiagara`, `BackFaceBeeNiagara`)
- `UCursorPartFocusActionComponent` 1개 (`PartFocusAction`)
- `USceneComponent` 2개 (`QueenFrontAttachPoint`, `QueenBackAttachPoint`)
- 상태:
  - `SpawnAmount`는 `0` 이상 clamp
  - `TargetBeeCount`는 항상 `0..SpawnAmount` clamp
- 감소 API:
  - `ReduceTargetBeeCountByRatio(float)` (`Ratio`는 `0..1` clamp, 감소량은 `RoundToInt(CurrentTargetBeeCount * Ratio)`)
  - `ReduceTargetBeeCountByAmount(int32)`
- 파라미터 적용 시점:
  - `OnConstruction`, `BeginPlay`, `PostEditChangeProperty`, 명시 API 호출 시
- Niagara user parameter 적용:
  - `User.PlaneSize` (Vector2D)
  - `User.SpawnAmount` (Int32)
  - `User.TargetBeeCount` (Int32)

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
- 벌떼 Niagara particle 이동 로직은 Niagara 시스템에서 처리하고 C++은 spline binding/parameter 주입만 담당한다.
- `ABeehive`의 방향별 spawn 계산식:
  - `Activity = SpawnAmountByHour ? Curve(Hour24/24) : FallbackActivity`
  - `RawSpawnAmount = Activity * ColonyBeeCount * SpawnAmountScale`
  - `FinalSpawnAmount = Clamp(RawSpawnAmount, 0, MaxSpawnAmount)`
- `OutgoingNiagara.User.bIsReverse=false`, `IngoingNiagara.User.bIsReverse=true`를 C++에서 항상 명시 적용한다.
- `ABeehiveCombActor` 소유 `FrontFaceBeeNiagara`/`BackFaceBeeNiagara`도 component details에서 `OverrideParameters`를 숨기고 C++ 적용값이 source of truth다.
- `ABeehive`는 `AEnvironmentTimeOfDayActor`를 직접 참조하지 않으며, bucket listener 이벤트의 `Hour24`를 받아서만 갱신한다.
- `ABeehive`의 queen 위치 갱신도 Environment actor 직접 참조 없이 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem` 이벤트로만 수행한다.
- queen이 붙은 comb가 lifted 상태가 되면 queen은 comb attach 상태를 유지하며 함께 이동하고, 다음 위치 갱신 후보에서만 lifted slot이 제외된다.
- Pickup은 획득 성공 시 destroy되고, 실패 시 actor를 유지한다.
- StorageBox는 storage 상태를 `UStorageBoxComponent`가 소유하고, UI lifecycle은 `UStorageBoxFocusActionComponent`가 처리한다.

## Manual Review Points

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

- Blueprint child에서 component 이름 변경 시 기존 serialized component override가 유지되는지 확인
- `AWorldItemPickup::OnConstruction()` 후 prompt/mesh가 item definition과 동기화되는지 확인
- StorageBox interaction 종료 후 storage component 상태는 유지되고 UI transient state만 정리되는지 확인
## Bucket Listener Notes

- `ABeehive` registers itself to `UGameTimeBucketSubsystem` in `BeginPlay`.
- `ABeehive` unregisters itself from `UGameTimeBucketSubsystem` in `EndPlay`.
- Runtime-spawned beehives therefore receive bucket events without external manual registration.
## Beehive Attraction Swarm

- `ABeehive` now directly owns `UNiagaraComponent* AttractionSwarmNiagara` (no child actor).
- Attraction center is the `AttractionSwarmNiagara` component transform itself.
- Settings are exposed by `FBeehiveAttractionSwarmSettings` on `ABeehive`.
- Applied Niagara user parameters:
  - `User.AttractionPower` (Float)
  - `User.NoisePower` (Float)
  - `User.SpawnSphereRadius` (Float)
  - `User.SpawnAmount` (Int32, via `SetVariableInt`)
- `SpawnAmount` formula:
  - `RoundToInt(ColonyBeeCount * SpawnAmountScale)`
  - clamp to `0..MaxSpawnAmount`
- Apply points: `OnConstruction`, `BeginPlay`, `PostEditChangeProperty`, explicit apply call, and bee-count setter.
- No time/bucket-based auto update is used for attraction spawn amount.
- Existing outgoing/ingoing spline swarms remain active and unchanged.
