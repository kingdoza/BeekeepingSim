# WorldActors System

## Scope

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSplineSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
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
- `USplineComponent` 1개 (`SwarmSpline`)를 직접 소유하며 레벨 인스턴스별 편집 대상
- `BeeSplineSwarmActorClass` (`TSubclassOf<ABeehiveDualSwarmActor>`)로 child class 지정
- `ColonyBeeCount`, `BeeSwarmHour24`, `DualSwarmCommonSettings`, `OutgoingSwarmSettings`, `IngoingSwarmSettings`
- `IGameTimeBucketListener`를 구현해 시간 bucket 이벤트를 구독
- 기본 bucket 설정: `BeeSwarmBucketMinutes=10`, BeginPlay 즉시 적용 옵션 지원
- `ApplyBeeSwarmSettings()`에서 child actor 재생성 없이 기존 instance에 계산된 DTO를 재주입
- 시간 갱신(`ApplyBeeSwarmHour24`) 경로에서도 class 변경 없이 parameter만 갱신

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
- `ABeehive`는 `AEnvironmentTimeOfDayActor`를 직접 참조하지 않으며, bucket listener 이벤트의 `Hour24`를 받아서만 갱신한다.
- Pickup은 획득 성공 시 destroy되고, 실패 시 actor를 유지한다.
- StorageBox는 storage 상태를 `UStorageBoxComponent`가 소유하고, UI lifecycle은 `UStorageBoxFocusActionComponent`가 처리한다.

## Manual Review Points

- Blueprint child에서 component 이름 변경 시 기존 serialized component override가 유지되는지 확인
- `AWorldItemPickup::OnConstruction()` 후 prompt/mesh가 item definition과 동기화되는지 확인
- StorageBox interaction 종료 후 storage component 상태는 유지되고 UI transient state만 정리되는지 확인
## Bucket Listener Notes

- `ABeehive` registers itself to `UGameTimeBucketSubsystem` in `BeginPlay`.
- `ABeehive` unregisters itself from `UGameTimeBucketSubsystem` in `EndPlay`.
- Runtime-spawned beehives therefore receive bucket events without external manual registration.
