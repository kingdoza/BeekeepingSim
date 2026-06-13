# 분봉 테스트 기능 리뷰 프롬프트

## 리뷰 목표

이번 리뷰는 외부 Blueprint 호출로 벌통 분봉 테스트를 시작하고, 분봉 본진에서 벌 운반통 hold-use로 `AliveRadius`를 감소시키는 C++ 구현만 검토한다.

이번 범위 제외:

- 자동 분봉 발생 조건
- colony simulation 반영
- 포획 결과 inventory 저장
- `Content/` asset 수정/저장

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`

## 리뷰 대상 파일

Source:

- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/Inventory/BeeCarrierUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/BeeCarrierUseAction.cpp`

Config/docs:

- `Config/DefaultGameplayTags.ini`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/PROMPT_REVIEW.md`

## 기대 구현

### 1. 분봉 본진 actor

- `ABeeSwarmClusterActor`는 FocusEngaged host여야 한다.
- `UFocusTargetComponent`, `UAnchoredFocusCursorActionComponent`, `UCursorItemUseAreaScopeComponent`, `UItemUseAreaMeshProviderComponent`를 사용해야 한다.
- `CaptureUseAreaMesh`는 `UItemUseAreaMeshComponent`이고 tag는 `Item.UseArea.SwarmCluster.BeeCarrier`여야 한다.
- effect target은 `ABeeSwarmClusterActor`로 resolve되어야 한다.
- `AliveRadius`가 포획 진행 source of truth여야 한다.
- `AliveRadius`, `SpawnAmount`, `SphereRadius`가 Niagara parameter에 즉시 반영되어야 한다.
- `AliveRadius <= 0` captured 전환은 1회만 발생해야 한다.
- captured 후 use-area는 비활성화되고 descriptors가 rebuild되어야 한다.
- 기존 벌통 여왕벌을 이동/재사용하지 않고, 분봉 본진이 별도 queen child actor를 소유해야 한다.

### 2. route actor

- `ABeehiveSwarmRouteActor`는 `ABeeSplineSwarmActor`를 상속해야 한다.
- runtime spline은 start/mid/end 3-point arc로 구성되어야 한다.
- route end는 cluster actor origin이 아니라 `GetClusterCenterComponent()` world location이어야 한다.
- 기존 spline Niagara parameter 계약을 유지해야 한다.

### 3. 벌통 테스트 시작 API

- `ABeehive::BeginSwarmingAtTransform`와 `BeginSwarmingAtActor`가 BlueprintCallable이어야 한다.
- class/world/spawn 실패 시 false 반환과 `ReceiveSwarmingStartFailed` event가 있어야 한다.
- 성공 시 cluster spawn, initialize, route spawn, configure, external parameter 적용 후 `ReceiveSwarmingStarted` event가 호출되어야 한다.
- `bDestroyPreviousTestSwarmOnStart=true`면 이전 active test swarm actors를 destroy해야 한다.
- `ClearActiveTestSwarm`은 destroy 여부를 명시적으로 처리해야 한다.
- `EndPlay`에서 active test swarm cleanup이 누락되지 않았는지 확인한다.

### 4. 기존 벌통 상태 무변경

분봉 테스트 시작 경로에서 아래가 없어야 한다.

- `ColonyBeeCount` 차감 또는 `SetColonyBeeCount` 호출
- 기존 `QueenBeeChildActor` detach/move/reparent
- active comb spawn/target bee count 변경
- 새 time bucket subscription
- 자동 분봉 조건 추가

### 5. 벌 운반통 action

- `UBeeCarrierUseAction`은 `UHoldItemUseAction` 기반이어야 한다.
- use-area query는 `Item.UseArea.SwarmCluster.BeeCarrier`여야 한다.
- `CanBeginUse`는 super, target cluster 유효성, captured 여부, item-use-area hit context를 확인해야 한다.
- action 내부에서 mouse deproject/trace를 다시 수행하지 않아야 한다.
- drag speed는 `Distance(CurrentImpactPoint, LastImpactPoint) / DeltaTime`이어야 한다.
- 첫 tick 또는 hit point 없음은 base rate만 적용해야 한다.
- rate 공식:

```cpp
Rate = BaseAliveRadiusDecreasePerSecond + BonusSpeed * DragSpeedToAliveRadiusDecreaseScale;
Rate = Clamp(Rate, 0.0f, MaxAliveRadiusDecreasePerSecond);
DeltaAliveRadius = Rate * DeltaTime;
```

- `DecreaseAliveRadius`가 실제 감소를 만든 tick에만 `bSucceeded=true`여야 한다.
- item stack, durability, item runtime state를 변경하지 않아야 한다.
- `EndUse`에서 drag transient state를 reset해야 한다.

## Gameplay Tag

신규 tag만 추가되어야 한다.

- `Item.UseArea.SwarmCluster.BeeCarrier`
- `Item.BeeCarrier`

기존 tag rename/redirect 또는 Core Redirect 추가가 있으면 문제로 본다.

## 권장 검색

```powershell
rg -n "BeeCarrierUseAction|BeeSwarmClusterActor|BeehiveSwarmRouteActor|Item.UseArea.SwarmCluster.BeeCarrier" Source/BeekeepingSim/Public Source/BeekeepingSim/Private Config .md
rg -n "ColonyBeeCount.*Swarm|QueenBeeChildActor.*Swarm|SetColonyBeeCount\\(|DetachFromActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg -n "CoreRedirects|SwarmCluster|BeeCarrier" Config .md/Architecture/CoreSystem.md
```

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private Config .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 PIE 확인

- `BP_BeeSwarmClusterActor`에 cluster Niagara, capture use-area mesh/material, queen BP class를 설정한다.
- `BP_Beehive`에 `SwarmExitPoint`, `SwarmClusterActorClass`, `SwarmRouteActorClass`를 설정한다.
- route actor BP에 기존 spline swarm Niagara asset을 설정한다.
- 벌 운반통 item definition에 `UBeeCarrierUseAction` action spec을 추가한다.
- 외부 BP에서 벌통 `BeginSwarmingAtTransform` 또는 `BeginSwarmingAtActor`를 호출한다.
- cluster 생성, route Niagara, FocusEngaged, use-area 표시, slow/fast drag rate 차이, captured 1회 전환을 확인한다.
- 분봉 시작/포획 중 기존 벌통 `ColonyBeeCount`, 기존 여왕벌 위치, 소비장 벌 수가 변하지 않는지 확인한다.

## 리뷰 결과 작성 형식

리뷰 결과는 `.md/AGENT_REVIEW.md` 기준으로 작성한다.

- Findings first: severity, file/line, 문제, 영향, 수정 방향
- blocking/major issue가 없으면 "검토 범위에서 발견된 blocking/major issue 없음"을 명확히 적는다.
- 남은 리스크는 UBT/PIE/BP 수동 확인 여부와 연결해서 적는다.
- 구현 요약은 findings 이후에 짧게 둔다.
