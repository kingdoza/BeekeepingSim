# 분봉 기능 구현 프롬프트

## 목표

벌통에서 외부 Blueprint 호출로 분봉을 시작하는 1차 테스트 기능을 C++로 구현한다.

분봉이 시작되면 벌통 입구에서 분봉 본진까지 spline 기반 벌떼 VFX가 생성되고, 월드에 분봉 본진 actor가 생성된다. 분봉 본진은 FocusEngaged 대상이며, `벌 운반통` 아이템을 LMB hold 상태로 본진 사용영역 위에서 빠르게 드래그할수록 본진의 `AliveRadius`가 더 빠르게 감소한다.

이번 범위는 테스트용 수동 시작까지다. 자동 분봉 발생 조건, colony simulation 반영, 포획 결과 inventory 저장은 구현하지 않는다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`

## 핵심 확정 사항

- 외부 Blueprint가 `ABeehive`의 분봉 시작 API를 호출해 테스트한다.
- 분봉 시작은 기존 벌통의 `ColonyBeeCount`, 기존 여왕벌, 소비장 벌 수/target count를 변경하지 않는다.
- 분봉 본진의 여왕벌은 기존 벌통 여왕벌을 옮기지 않고 별도로 spawn한다.
- 분봉 본진 여왕벌은 본진 중심점에 위치한다.
- `벌 운반통` 포획 결과는 item instance state로 저장하지 않는다.
- 포획 진행 source of truth는 분봉 본진 actor의 `AliveRadius`다.
- `AliveRadius` 감소는 본진 Niagara parameter `User.AliveRadius`에 즉시 반영한다.
- `Content/` asset은 수정하지 않는다. Niagara system, mesh, material, BP child 설정은 수동 작업으로 남긴다.

## 구현 대상

### WorldActors

새 파일 권장:

- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`

수정:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 필요 시 `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`

### Inventory

새 파일 권장:

- `Source/BeekeepingSim/Public/Inventory/BeeCarrierUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/BeeCarrierUseAction.cpp`

### Config

신규 gameplay tag 추가:

- `Item.UseArea.SwarmCluster.BeeCarrier`
- 선택: `Item.BeeCarrier`

기존 tag rename/redirect는 하지 않는다.

## `ABeeSwarmClusterActor`

역할:

- 분봉 본진 actor.
- FocusEngaged host.
- 본진 구형 벌떼 Niagara와 본진 여왕벌 별도 actor를 소유한다.
- `벌 운반통` item-use-area target이다.
- 포획 진행 상태(`AliveRadius`)의 source of truth다.

권장 구성:

- `Root`
- `ClusterCenter`
- `ClusterNiagara`
- `QueenBeeChildActor`
- `CaptureUseAreaMesh`
- `FocusAnchor`
- `CharacterAnchor`
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UCursorItemUseAreaScopeComponent`
- `UItemUseAreaMeshProviderComponent`

`FocusAnchor`와 `CharacterAnchor`에는 기존 anchored focus 정책과 맞게 각각 `FocusAnchor`, `CharacterAnchor` component tag를 붙인다.

`CaptureUseAreaMesh`:

- 타입: `UItemUseAreaMeshComponent`
- Area tag: `Item.UseArea.SwarmCluster.BeeCarrier`
- Effect target policy: `ComponentOwner` 또는 `ExplicitObject`로 최종 target이 `ABeeSwarmClusterActor`가 되게 한다.
- captured 상태에서는 inactive/disabled가 되도록 처리한다.

Niagara parameter 계약:

- `User.AliveRadius` float
- `User.SpawnAmount` int32
- `User.SphereRadius` float

권장 UPROPERTY:

- `AliveRadius`
- `SpawnAmount`
- `SphereRadius`
- `SwarmQueenBeeActorClass`
- `QueenCenterOffset`
- Niagara parameter name properties
  - default `User.AliveRadius`
  - default `User.SpawnAmount`
  - default `User.SphereRadius`

권장 API:

- `InitializeSwarmCluster(float InAliveRadius, int32 InSpawnAmount, float InSphereRadius)`
- `ApplyClusterNiagaraParameters()`
- `DecreaseAliveRadius(float DeltaRadius)`
- `SetAliveRadius(float NewAliveRadius)`
- `GetAliveRadius() const`
- `GetSphereRadius() const`
- `GetSpawnAmount() const`
- `IsCaptured() const`
- `GetClusterCenterComponent() const`
- `GetQueenBeeActor() const`
- `RebuildItemUseAreaDescriptors()`

권장 BP events:

- `ReceiveSwarmClusterInitialized`
- `ReceiveAliveRadiusChanged(float NewAliveRadius)`
- `ReceiveSwarmCaptured`

포획 완료:

- `AliveRadius <= 0.0f`이면 captured로 본다.
- captured 전환은 한 번만 발생한다.
- captured 시 `ClusterNiagara` parameter에는 `AliveRadius=0`을 적용한다.
- `CaptureUseAreaMesh`를 비활성화하고 item-use-area descriptor를 rebuild한다.
- actor destroy 여부, 여왕벌 숨김, 포획 완료 연출은 BP event에서 처리할 수 있게 열어둔다.

여왕벌:

- 기존 `ABeehive`의 `QueenBeeChildActor`를 재사용하거나 이동하지 않는다.
- `SwarmQueenBeeActorClass`로 별도 child actor/spawn actor를 생성한다.
- 여왕벌은 `ClusterCenter`에 attach하고 local location은 `QueenCenterOffset`만 허용한다.
- 포획 중 `AliveRadius`가 줄어도 여왕벌은 center에 유지한다.

## `ABeehiveSwarmRouteActor`

역할:

- 벌통 입구에서 분봉 본진까지 이동하는 spline 기반 벌떼 VFX actor.
- 기존 출근용 벌떼 Niagara asset을 재사용할 수 있는 parameter 계약을 유지한다.

권장 구현:

- `ABeeSplineSwarmActor`를 상속한다.
- protected `SwarmSpline`을 사용해 runtime route point를 구성한다.
- `ConfigureRoute(const FVector& StartWorldLocation, const FVector& EndWorldLocation)`를 제공한다.
- 필요하면 `RouteMidPointHeightOffset`으로 3-point spline arc를 만든다.
- 기존 `ApplyExternalSwarmParameters(const FBeeSplineSwarmAppliedParameters&)`를 사용해 spawn/speed/shape 값을 주입한다.

Spline/Niagara parameter 계약:

- `User.SwarmSpline`
- `User.SplineLength`
- `User.StartShapeExtent`
- `User.EndShapeExtent`
- `User.SpawnAmount`
- `User.SpeedMin`
- `User.SpeedMax`

권장 API:

- `ConfigureRoute(FVector StartWorldLocation, FVector EndWorldLocation)`
- `ConfigureRouteToCluster(FVector StartWorldLocation, ABeeSwarmClusterActor* ClusterActor)`
- `GetRouteEndWorldLocation() const`

도착지:

- route spline의 end는 분봉 본진 actor origin이 아니라 `ABeeSwarmClusterActor::GetClusterCenterComponent()` world location 기준으로 잡는다.

## `ABeehive` 변경

역할:

- 외부 BP 테스트용 분봉 시작 API 제공.
- 벌통 입구 start point와 분봉 본진 target point를 연결한다.
- 분봉 본진 actor와 route actor를 spawn한다.

권장 구성 추가:

- `SwarmExitPoint` scene component

권장 UPROPERTY:

- `TSubclassOf<ABeeSwarmClusterActor> SwarmClusterActorClass`
- `TSubclassOf<ABeehiveSwarmRouteActor> SwarmRouteActorClass`
- `float SwarmClusterInitialAliveRadius`
- `int32 SwarmClusterSpawnAmount`
- `float SwarmClusterSphereRadius`
- `FBeeSplineSwarmAppliedParameters SwarmRouteParameters`
- `bool bDestroyPreviousTestSwarmOnStart`
- transient `ActiveSwarmClusterActor`
- transient `ActiveSwarmRouteActor`

권장 API:

- `BeginSwarmingAtTransform(const FTransform& TargetTransform)`
- `BeginSwarmingAtActor(AActor* TargetActor)`
- `ClearActiveTestSwarm(bool bDestroyActors)`
- `GetActiveSwarmClusterActor() const`
- `GetActiveSwarmRouteActor() const`
- `GetSwarmExitPointComponent() const`

권장 BP events:

- `ReceiveSwarmingStarted(ABeeSwarmClusterActor* ClusterActor, ABeehiveSwarmRouteActor* RouteActor)`
- `ReceiveSwarmingStartFailed`

구현 규칙:

- `BeginSwarmingAtTransform`은 `SwarmClusterActorClass`와 `SwarmRouteActorClass`가 없으면 실패한다.
- `bDestroyPreviousTestSwarmOnStart=true`이면 기존 active test swarm actors를 destroy 후 새로 spawn한다.
- cluster spawn transform은 TargetTransform을 사용한다.
- cluster spawn 직후 `InitializeSwarmCluster`를 호출한다.
- route start는 `SwarmExitPoint` world location을 사용한다. 없으면 벌통 actor location으로 fallback한다.
- route end는 cluster center world location을 사용한다.
- route spawn 후 `ConfigureRoute`와 `ApplyExternalSwarmParameters(SwarmRouteParameters)`를 호출한다.
- 이번 범위에서는 기존 벌통 상태를 변경하지 않는다.
  - `ColonyBeeCount` 차감 금지
  - 기존 `QueenBeeChildActor` detach/move 금지
  - active comb bee count/target count 변경 금지
  - bucket subscription 추가 금지

## `UBeeCarrierUseAction`

역할:

- `벌 운반통` 아이템의 hold-use action.
- 분봉 본진 use-area 위에서 LMB hold 중 cursor drag speed에 비례해 `AliveRadius` 감소량을 증가시킨다.

기반:

- `UHoldItemUseAction`

use-area query:

- `Item.UseArea.SwarmCluster.BeeCarrier`

권장 설정:

- `BaseAliveRadiusDecreasePerSecond`
- `DragSpeedToAliveRadiusDecreaseScale`
- `MaxAliveRadiusDecreasePerSecond`
- `MinDragSpeedForBonus`
- `DragSpeedSmoothingAlpha`

transient state:

- `bHasLastImpactPoint`
- `LastImpactPoint`
- `SmoothedDragSpeedCmPerSecond`

구현 규칙:

- `CanBeginUse`는 super 결과, target cluster 유효성, captured 여부, item-use-area hit context를 확인한다.
- `ApplyUseEffect`는 `Context.ItemUseEffectTargetObject`를 `ABeeSwarmClusterActor`로 cast한다.
- `Context.bHasItemUseAreaHit`와 `Context.ItemUseAreaImpactPoint`를 사용한다.
- mouse deproject/trace를 action 내부에서 다시 수행하지 않는다.
- 첫 tick 또는 hit point 없음이면 base rate만 적용한다.
- 이후 tick은 `Distance(ImpactPoint, LastImpactPoint) / DeltaTime`으로 drag speed를 계산한다.
- bonus speed는 `Max(0, DragSpeed - MinDragSpeedForBonus)`에서 계산한다.
- 최종 rate:

```cpp
Rate = BaseAliveRadiusDecreasePerSecond + BonusSpeed * DragSpeedToAliveRadiusDecreaseScale;
Rate = Clamp(Rate, 0.0f, MaxAliveRadiusDecreasePerSecond);
DeltaAliveRadius = Rate * DeltaTime;
```

- `Cluster->DecreaseAliveRadius(DeltaAliveRadius)`가 실제로 값을 줄였을 때 `Result.bSucceeded=true`를 반환한다.
- item stack, durability, runtime state는 변경하지 않는다.
- `EndUse`에서 transient drag state를 reset한다.

## Blueprint/Content 경계

수동 BP/Content 작업으로 남길 것:

- `BP_BeeSwarmClusterActor` 생성 및 `ABeeSwarmClusterActor` parent 지정.
- cluster 구형 벌떼 Niagara system을 `ClusterNiagara`에 지정.
- `ClusterNiagara`가 `User.AliveRadius`, `User.SpawnAmount`, `User.SphereRadius`를 사용하도록 asset 설정.
- cluster capture use-area mesh/material 설정.
- cluster `FocusAnchor`, `CharacterAnchor`, `ClusterCenter`, queen 위치 authoring.
- `SwarmQueenBeeActorClass`에 사용할 queen BP 지정.
- route actor BP에 기존 출근용 spline Niagara asset 지정.
- `BP_Beehive`에서 `SwarmExitPoint`를 벌통 입구에 배치.
- `BP_Beehive`에서 `SwarmClusterActorClass`, `SwarmRouteActorClass` 지정.
- `벌 운반통` item definition에 `UBeeCarrierUseAction` action spec 추가.
- 외부 테스트 BP에서 `ABeehive::BeginSwarmingAtTransform` 또는 `BeginSwarmingAtActor` 호출.

`Content/` asset은 구현 에이전트가 수정/저장하지 않는다.

## 수정하면 안 되는 것

- 기존 UCLASS/USTRUCT/UENUM rename 금지.
- 기존 Blueprint API 삭제/rename 금지.
- 기존 벌통 여왕벌을 분봉 본진으로 이동시키지 않는다.
- 기존 벌통 colony population, 소비장 bee count, honey production, disease/aggression 계산을 변경하지 않는다.
- 자동 분봉 발생 조건을 추가하지 않는다.
- `벌 운반통` item instance에 포획 벌 수 state를 추가하지 않는다.
- `Content/` asset 저장 금지.
- Core Redirect 추가 금지. 이번 작업은 신규 class/API 추가 중심이어야 한다.

## 문서 반영

구현 후 구조 변경 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - Source 구조 counts
  - 시스템 간 책임 흐름
  - 벌통 분봉 테스트 시작 API, 분봉 본진, route actor, 벌 운반통 action 요약
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeeSwarmClusterActor`
  - `ABeehiveSwarmRouteActor`
  - `ABeehive` 분봉 시작 API와 no-colony-mutation 전제
- `.md/Architecture/InventorySystem.md`
  - `UBeeCarrierUseAction`
  - 포획 결과 item state 미저장 전제
- `.md/Architecture/FocusSystem.md`
  - 분봉 본진 FocusEngaged item-use-area 경로가 기존 generic scope/provider를 사용한다는 점
- `.md/Architecture/CoreSystem.md`
  - 신규 추가만이면 Core Redirect 갱신 불필요

## 검증

공백/패치 검증:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private Config .md
```

UBT 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

검색 검증:

```powershell
rg -n "BeeCarrierUseAction|BeeSwarmClusterActor|BeehiveSwarmRouteActor|Item.UseArea.SwarmCluster.BeeCarrier" Source/BeekeepingSim/Public Source/BeekeepingSim/Private Config .md
rg -n "ColonyBeeCount.*Swarm|QueenBeeChildActor.*Swarm|SetColonyBeeCount\\(|DetachFromActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

두 번째 검색은 분봉 테스트 시작 경로가 기존 벌통 상태/여왕벌을 직접 변경하지 않는지 확인하기 위한 것이다. 무관한 기존 API 선언/문서 결과는 허용되지만, 새 분봉 시작 로직에서 기존 벌통 여왕벌 이동이나 colony count 차감이 보이면 수정한다.

## 수동 PIE 확인

1. `BP_BeeSwarmClusterActor`에서 cluster Niagara와 capture use-area mesh가 보이는지 확인한다.
2. cluster Niagara에 `AliveRadius`, `SpawnAmount`, `SphereRadius` 초기값이 적용되는지 확인한다.
3. cluster center에 별도 여왕벌이 spawn/attach되는지 확인한다.
4. 외부 BP에서 벌통의 `BeginSwarmingAtTransform`을 호출하면 지정 위치에 cluster가 생성되는지 확인한다.
5. 벌통 입구 `SwarmExitPoint`에서 cluster center까지 route spline Niagara가 생성되는지 확인한다.
6. route Niagara가 기존 출근용 spline Niagara asset과 같은 parameter 계약으로 동작하는지 확인한다.
7. cluster actor를 FocusEngaged할 수 있는지 확인한다.
8. FocusEngaged 후 `벌 운반통` 선택 시 capture use-area가 표시되는지 확인한다.
9. LMB hold 상태에서 use-area 위를 천천히 드래그하면 `AliveRadius`가 base rate로 감소하는지 확인한다.
10. 빠르게 드래그하면 `AliveRadius` 감소 속도가 증가하는지 확인한다.
11. `AliveRadius <= 0`에서 captured 이벤트가 1회만 발생하고 use-area가 비활성화되는지 확인한다.
12. 분봉 시작/포획 중 기존 벌통의 `ColonyBeeCount`, 기존 여왕벌 위치, 소비장 벌 수가 변경되지 않는지 확인한다.

## 최종 보고 요구사항

- 변경 파일
- 새 UCLASS/USTRUCT/UENUM 목록
- 추가한 Blueprint/API 계약
- 추가한 Gameplay Tag
- 분봉 시작 flow 요약
- cluster Niagara parameter 이름
- route spline Niagara parameter 이름
- `벌 운반통` drag speed 계산/AliveRadius 감소 방식
- 기존 벌통 상태를 변경하지 않는 검증 내용
- 아키텍처 문서 반영 내용
- 빌드 결과 또는 미수행 사유
- 필요한 수동 BP/Content 작업 목록
