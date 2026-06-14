# BeeCarrier 포획 상태와 분봉 본진 부피 반경 구현 리뷰 프롬프트

## 리뷰 목표

이미 구현된 분봉 테스트 기능 확장이 의도대로 반영됐는지 C++/문서 기준으로 리뷰한다.

핵심 변경은 `벌 운반통` 포획 결과를 `UItemInstance` runtime state로 저장하고, 분봉 본진 포획 진행의 source of truth를 `AliveRadius`에서 벌 수(`CapturedBeeAmount`/remaining bees)로 바꾸는 것이다. `AliveRadius`는 남은 벌 수 비율에서 구 부피 공식으로 파생되어 Niagara `User.AliveRadius`에 반영되어야 한다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/CoreSystem.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/Inventory/BeeCarrierItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/Inventory/BeeCarrierUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/BeeCarrierUseAction.cpp`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`

## 기대 구현 요약

### Inventory

- `UBeeCarrierItemDefinition : UItemDefinition`이 추가되어야 한다.
- BeeCarrier definition은 `MaxStack=1` invariant를 가진다.
- `MaxCapturedBeeAmount`가 최대 수용량 source of truth다.
- `DefaultCapturedBeeAmount`는 item instance 초기 state에 clamp되어 반영된다.
- `FBeeCarrierItemState`는 `UItemInstance` optional runtime state이며 `bHasState`, `CapturedBeeAmount`만 저장한다.
- `UItemInstance`는 BeeCarrier state API를 제공해야 한다.
  - `SetBeeCarrierState`
  - `AddCapturedBees`
  - `ClearBeeCarrierState`
  - `HasBeeCarrierState`
  - `GetBeeCarrierState`
  - `GetCapturedBeeAmount`
  - `GetCapturedBeeCountRounded`
  - `GetBeeCarrierFreeCapacity`
- `SetBeeCarrierState`는 definition이 `UBeeCarrierItemDefinition`이 아니면 state를 clear해야 한다.
- `CapturedBeeAmount`는 `0..MaxCapturedBeeAmount`로 clamp되어야 한다.
- `CopyRuntimeStateFrom`은 BeeCarrier state도 복사해야 한다.
- stack helper의 max stack/compatibility 계산은 BeeCarrier를 `MaxStack=1`과 runtime-state-aware item으로 다뤄야 한다.

### WorldActors

- `ABeeSwarmClusterActor`는 `InitialAliveRadius`와 `CapturedBeeAmount`를 추가로 소유해야 한다.
- `SpawnAmount`는 분봉 본진 총 벌 수로 쓰인다.
- `CaptureBees(float RequestedBeeAmount)`는 요청량을 `0..RemainingBeeAmount`로 clamp하고 실제 포획량을 반환해야 한다.
- `CapturedBeeAmount` 변경 후 `AliveRadius`는 아래 공식으로 재계산되어야 한다.

```cpp
AliveRadius = InitialAliveRadius * Cbrt(RemainingBeeAmount / TotalBeeAmount);
```

- Unreal 구현에서는 `FMath::Pow(RemainingRatio, 1.0f / 3.0f)`를 사용해도 된다.
- `AliveRadius` 변경은 `ClusterNiagara`의 `User.AliveRadius`에 즉시 반영되어야 한다.
- captured 판정은 `CapturedBeeAmount >= TotalBeeAmount` 또는 잔여 벌 수 0 기준이어야 한다.
- captured 전환은 1회만 발생하고, `AliveRadius=0` 적용, capture use-area 비활성화, descriptor rebuild를 수행해야 한다.
- 기존 `DecreaseAliveRadius(float)` API는 삭제/rename하지 않고 legacy/manual visual wrapper로 남겨야 한다.
- BeeCarrier gameplay path가 `DecreaseAliveRadius`를 호출하면 안 된다.

### BeeCarrier Use Action

- `UBeeCarrierUseAction`은 target cluster, BeeCarrier source item, 남은 수용량, cluster 잔여 벌 수, item-use-area hit context를 확인해야 한다.
- action 내부에서 mouse deproject/trace를 다시 수행하면 안 된다.
- drag speed는 `Distance(CurrentImpactPoint, LastImpactPoint) / DeltaTime` raw 값이어야 한다.
- 첫 tick 또는 hit point 없음은 bonus 없이 base rate만 적용한다.
- capture rate는 bees/sec 단위여야 한다.

```cpp
BonusSpeed = Max(0, DragSpeedCmPerSecond - MinDragSpeedForBonus);
Rate = BaseBeeCapturePerSecond + BonusSpeed * DragSpeedToBeeCaptureScale;
Rate = Clamp(Rate, 0.0f, MaxBeeCapturePerSecond);
RequestedBeeAmount = Rate * DeltaTime;
```

- `RequestedBeeAmount`는 BeeCarrier 남은 수용량으로 clamp해야 한다.
- 실제 포획량은 `Cluster->CaptureBees(RequestedBeeAmount)` 반환값이어야 한다.
- 실제 포획량이 0보다 클 때만 `SourceItemInstance->AddCapturedBees(ActualCaptured)`를 호출하고 `Result.bSucceeded=true`여야 한다.
- item stack/durability는 변경하지 않아야 한다.

## 반드시 확인할 불변조건

- 기존 벌통 `ColonyBeeCount`, 기존 벌통 `QueenBeeChildActor`, active comb bee count/target count, bucket subscription은 이 작업에서 변경되면 안 된다.
- 새 Focus 경로나 새 actor class를 만들면 안 된다.
- `Content/` asset 수정/저장은 없어야 한다.
- Core Redirect 추가는 없어야 한다.
- 기존 `DecreaseAliveRadius`, `BeginSwarmingAtTransform`, `BeginSwarmingAtActor` 같은 Blueprint API 삭제/rename은 없어야 한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

```powershell
rg -n "FBeeCarrierItemState|UBeeCarrierItemDefinition|CaptureBees|BaseBeeCapturePerSecond|CapturedBeeAmount|InitialAliveRadius" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg -n "DecreaseAliveRadius\(" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
rg -n "ColonyBeeCount.*Swarm|QueenBeeChildActor.*Swarm|SetColonyBeeCount\(|DetachFromActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

두 번째 검색에서 `DecreaseAliveRadius` 선언/구현은 남아 있어야 한다. `UBeeCarrierUseAction` 또는 BeeCarrier 포획 경로에서 호출되면 finding으로 보고한다.

## 수동 PIE 리뷰 포인트

1. BeeCarrier item definition parent를 `UBeeCarrierItemDefinition`으로 설정하고 `MaxCapturedBeeAmount`를 지정한다.
2. 새 BeeCarrier item instance가 `DefaultCapturedBeeAmount`를 state로 가진다.
3. hotbar/storage 이동 후 `CapturedBeeAmount`가 유지된다.
4. 분봉 본진 포획 중 BeeCarrier `CapturedBeeAmount`가 증가한다.
5. 같은 drag speed에서는 포획 벌 수가 시간에 대해 거의 선형으로 증가한다.
6. `AliveRadius`는 선형이 아니라 `InitialAliveRadius * cbrt(Remaining/Total)`로 감소한다.
7. `SpawnAmount=500`, `InitialAliveRadius=20`, `CapturedBeeAmount=437.5` 근처에서 `AliveRadius`가 약 10이다.
8. BeeCarrier가 가득 차면 더 이상 포획되지 않는다.
9. 분봉 본진 잔여 벌 수가 0이면 captured event가 1회만 발생하고 use-area가 비활성화된다.
10. 기존 `DecreaseAliveRadius` Blueprint 호출은 compile/runtime 문제 없이 남아 있지만 BeeCarrier 포획에는 쓰이지 않는다.

## 리뷰 출력 형식

- Findings first. 심각도 순으로 파일/라인을 포함한다.
- 합당한 finding이 없으면 "중요 finding 없음"이라고 명시한다.
- 이후 테스트/검증 결과, 남은 수동 BP/Content 확인 항목을 짧게 정리한다.
