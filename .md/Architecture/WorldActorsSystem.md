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
- `USceneComponent` 1개 (`CombRackRoot`) + `MaxCombCount` 크기의 comb slot child actor component 배열
- `USplineComponent` 1개 (`SwarmSpline`)를 직접 소유하며 레벨 인스턴스별 편집 대상
- `BeeSplineSwarmActorClass` (`TSubclassOf<ABeehiveDualSwarmActor>`)로 child class 지정
- `ColonyBeeCount`, `BeeSwarmHour24`, `DualSwarmCommonSettings`, `OutgoingSwarmSettings`, `IngoingSwarmSettings`
- `IGameTimeBucketListener`를 구현해 시간 bucket 이벤트를 구독
- 기본 bucket 설정: `BeeSwarmBucketMinutes=10`, BeginPlay 즉시 적용 옵션 지원
- `ApplyBeeSwarmSettings()`에서 child actor 재생성 없이 기존 instance에 계산된 DTO를 재주입
- 시간 갱신(`ApplyBeeSwarmHour24`) 경로에서도 class 변경 없이 parameter만 갱신
- `CombSlotSpacing` 기준 local `-Y` 배치: slot `i`는 `FVector(0, -i * CombSlotSpacing, 0)`
- `CurrentCombCount` 정책: `MaxCombCount` 변경 시 강제 초기화 없이 `0..MaxCombCount` clamp만 수행
- `CurrentCombCount <= 0`이면 활성 comb actor가 없고 comb spawn amount는 0
- comb spawn amount 계산식: `RoundToInt(ColonyBeeCount * Clamp01(CombSpawnAmountRatio) / CurrentCombCount)`
- `SetColonyBeeCount` 포함 spawn amount 갱신 경로에서 active comb의 target bee count를 spawn amount로 리셋
- 공통 plane size source of truth는 `ABeehive::CombPlaneSize`이며 active comb actor에 일괄 주입

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
