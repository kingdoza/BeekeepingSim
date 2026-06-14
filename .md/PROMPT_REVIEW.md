# 왕롱 여왕벌 포획과 분봉 본진 최종 완료 조건 리뷰 프롬프트

## 리뷰 목표

이미 구현된 분봉/BeeCarrier 포획 구조 위에 왕롱 여왕벌 포획 기능과 분봉 본진 최종 완료 조건 분리가 정확히 반영됐는지 C++/문서 기준으로 리뷰한다.

핵심 기대는 왕롱 포획 결과가 `UItemInstance` optional runtime state로 저장되고, 벌통 여왕벌과 분봉 본진 여왕벌이 모두 `AQueenBeeActor` use-area + `IQueenBeeCaptureSource` host 경로로 포획되는 것이다. 분봉 본진은 벌 전량 포획과 여왕벌 포획이 모두 완료되어야 최종 `ReceiveSwarmCaptured`를 호출해야 한다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/CoreSystem.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/Inventory/QueenCageItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/QueenCageUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/QueenCageUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeCaptureSource.h`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Private/Inventory/BeeCarrierUseAction.cpp`
- `Config/DefaultGameplayTags.ini`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`

## 기대 구현 요약

### Inventory

- `UQueenCageItemDefinition : UItemDefinition`이 추가되어야 한다.
- 왕롱은 항상 여왕벌 1마리 capacity이며 `MaxStack=1` invariant를 가진다.
- `FQueenCageItemState`는 `UItemInstance` optional runtime state여야 한다.
- 저장 필드는 최소 `bHasState`, `bHasQueen`, `CapturedQueenBeeClass`, `BaseEggLayingPower`, `DiseaseValue`여야 한다.
- 왕롱 state에는 world actor reference가 저장되면 안 된다.
- `UItemInstance`는 QueenCage state API를 제공해야 한다.
  - `SetQueenCageEmptyState`
  - `SetQueenCageState`
  - `SetCapturedQueenBeeState`
  - `ClearQueenCageState`
  - `HasQueenCageState`
  - `GetQueenCageState`
  - `HasCapturedQueen`
  - `CanAcceptQueenBee`
- `InitializeFromDefinition()`은 `UQueenCageItemDefinition`이면 empty state로 초기화해야 한다.
- `CopyRuntimeStateFrom()`은 QueenCage state를 복사해야 한다.
- `SetStackCount()`와 `ItemStackMoveUtils::ResolveMaxStack()`은 QueenCage를 `MaxStack=1`로 다뤄야 한다.
- runtime state compatibility 비교에 QueenCage state가 포함되어야 한다.

### Queen Bee Actor

- `AQueenBeeActor`가 `IItemUseAreaActivationProvider`를 구현해야 한다.
- `QueenCageUseAreaMesh`는 `UItemUseAreaMeshComponent`이고 `QueenBeeMesh` 하위에 attach되어야 한다.
- area tag는 `Item.UseArea.QueenBee.QueenCage`여야 한다.
- effect target policy는 `ComponentOwner`여야 하며, item action context target이 `AQueenBeeActor`가 되어야 한다.
- captured queen은 use-area가 inactive이고 collision/hit 대상에서 빠져야 한다.
- `MakeQueenCageItemState()` 또는 동등 API가 queen class, 산란력, 질병값을 export해야 한다.
- `AQueenBeeActor`가 자신의 owner를 cast해서 벌통/분봉 본진 상태를 직접 바꾸면 안 된다.

### Capture Source

- `IQueenBeeCaptureSource` interface가 추가되어야 한다.
- `CanCaptureQueenBee(AQueenBeeActor*)`와 `CaptureQueenBee(AQueenBeeActor*, FQueenCageItemState&)`가 BlueprintNativeEvent/Callable surface로 제공되어야 한다.
- `ABeehive`와 `ABeeSwarmClusterActor`가 interface를 구현해야 한다.
- 실제 포획 가능성 판단, queen child 제거/재생성 차단, descriptor rebuild는 host 구현체가 담당해야 한다.

### QueenCage Use Action

- `UQueenCageUseAction : UHoldItemUseAction`이 추가되어야 한다.
- use-area query tag는 `Item.UseArea.QueenBee.QueenCage`여야 한다.
- 시작/적용 조건은 source item이 `UQueenCageItemDefinition` 기반이고 `CanAcceptQueenBee()`가 true여야 한다.
- target은 `Context.ItemUseEffectTargetObject`의 `AQueenBeeActor`여야 한다.
- host는 `Context.FocusEngagedHostActor`의 `IQueenBeeCaptureSource`여야 한다.
- action 내부에서 drag speed, progress, mouse deproject/trace를 사용하면 안 된다.
- 성공 시 `CaptureQueenBee` 반환 state를 source item instance에 저장하고 `Result.bSucceeded=true`여야 한다.
- item stack/durability는 변경하면 안 된다.
- 이미 여왕벌이 든 왕롱은 추가 포획을 시작/적용할 수 없어야 한다.

### Beehive

- `ABeehive`는 `bHasQueenBee`를 보유해야 한다.
- `GetQueenBeeActor()`는 `bHasQueenBee=false`이면 null을 반환해야 한다.
- `EnsureQueenBeeChildActorClass()`는 `bHasQueenBee=false`에서 queen child를 재생성하면 안 된다.
- queen location update, `IsQueenBeeAttachedToComb`, `TryBrushQueenBeeFromCombVisibleFace`는 queen 없음 상태에서 no-op/false여야 한다.
- `CalculateBeeIncreaseAmount()`는 queen 없음 상태에서 0이어야 한다.
- 벌통 queen capture 성공 시 `bHasQueenBee=false`, queen child 제거/비활성화, item-use-area descriptor rebuild가 수행되어야 한다.
- queen capture가 `ColonyBeeCount`, active comb bee count/target count, honey state를 즉시 변경하면 안 된다.

### Swarm Cluster

- `ABeeSwarmClusterActor`는 `bBeesCaptured`, `bQueenCaptured`, 기존 `bCaptured`를 분리해야 한다.
- `bBeesCaptured`: BeeCarrier로 벌이 모두 포획되었거나 총 벌 수가 0인 상태
- `bQueenCaptured`: 왕롱으로 분봉 본진 여왕벌이 포획된 상태
- `bCaptured`: 최종 완료이며 반드시 `bBeesCaptured && bQueenCaptured`
- `CaptureBees()`는 `bCaptured`가 아니라 `bBeesCaptured`를 기준으로 포획 가능성을 판단해야 한다.
- 벌만 모두 포획되면 `AliveRadius=0`, Niagara parameter 적용, BeeCarrier use-area 비활성화, descriptor rebuild만 수행해야 한다.
- 벌만 모두 포획된 시점에 `ReceiveSwarmCaptured`를 호출하면 finding이다.
- 분봉 queen capture 성공 시 `bQueenCaptured=true`, queen child 제거/재생성 차단, descriptor rebuild를 수행해야 한다.
- `ReceiveSwarmCaptured`는 `bBeesCaptured && bQueenCaptured && !bCaptured` 조건에서 1회만 호출되어야 한다.
- 기존 `DecreaseAliveRadius` API는 삭제/rename하면 안 되며, BeeCarrier 벌 포획량/부피 공식은 변경하면 안 된다.

### Tags and Docs

- `Config/DefaultGameplayTags.ini`에 `Item.UseArea.QueenBee.QueenCage`가 추가되어야 한다.
- 선택적으로 `Item.QueenCage`가 추가될 수 있다.
- 아키텍처 문서에는 QueenCage item state, `AQueenBeeActor` use-area, `IQueenBeeCaptureSource`, 벌통 queen capture 영향 범위, 분봉 final captured 조건이 반영되어야 한다.

## 반드시 확인할 불변조건

- 기존 UCLASS/USTRUCT/UENUM rename/delete는 없어야 한다.
- 기존 BlueprintCallable API 삭제/rename은 없어야 한다.
- 새 Focus 시스템/입력 경로가 추가되면 안 된다.
- `Content/` asset 수정/저장은 없어야 한다.
- Core Redirect 추가는 없어야 한다.
- 왕롱 포획 시 `ColonyBeeCount`, active comb bee count/target count를 즉시 변경하면 안 된다.
- 자동 분봉 발생 조건, 여왕벌 방출/재배치/왕롱 교체 기능이 추가되면 안 된다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private Config .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

```powershell
rg -n "FQueenCageItemState|UQueenCageItemDefinition|UQueenCageUseAction|IQueenBeeCaptureSource|QueenCageUseAreaMesh|Item.UseArea.QueenBee.QueenCage|bHasQueenBee|bBeesCaptured|bQueenCaptured" Source/BeekeepingSim/Public Source/BeekeepingSim/Private Config .md
rg -n "ReceiveSwarmCaptured|HandleCapturedIfNeeded|IsCaptured\(|SetCaptureUseAreaActive|CaptureBees" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg -n "ColonyBeeCount.*Queen|SetColonyBeeCount\(|TotalTargetBeeCount.*Queen|DetachFromActor" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

두 번째 검색은 분봉 본진의 벌 포획 완료와 최종 완료 조건이 분리되었는지 확인하기 위한 것이다.

세 번째 검색은 왕롱 포획 경로가 벌통 colony/comb 상태를 즉시 바꾸거나 queen actor를 기존 방식으로 detach하지 않는지 확인하기 위한 것이다. 무관한 기존 API 선언/문서 결과는 허용되지만, 새 왕롱 capture implementation에서 population/comb count를 변경하면 finding으로 보고한다.

## 수동 PIE 리뷰 포인트

1. 왕롱 DataAsset parent를 `UQueenCageItemDefinition`으로 설정하고 `UQueenCageUseAction`을 action spec에 추가한다.
2. 왕롱에 필요한 gameplay tag가 있으면 `Item.QueenCage`를 부여한다.
3. `AQueenBeeActor` BP child에서 `QueenCageUseAreaMesh` mesh/material/relative transform이 여왕벌 주변을 덮는지 확인한다.
4. 벌통 FocusEngaged 상태에서 왕롱으로 벌통 여왕벌을 포획하면 왕롱 `bHasQueen=true`가 되고 벌통 `bHasQueenBee=false`가 되는지 확인한다.
5. 벌통 여왕벌 포획 후 colony population 증가량이 0이 되는지 확인한다.
6. 벌통 여왕벌 포획 후 queen location bucket/update가 여왕벌을 재생성하지 않는지 확인한다.
7. 이미 여왕벌이 든 왕롱으로 다른 여왕벌을 포획할 수 없는지 확인한다.
8. 분봉 본진에서 BeeCarrier로 벌을 모두 포획해도 `ReceiveSwarmCaptured`가 아직 호출되지 않는지 확인한다.
9. 분봉 본진에서 벌이 모두 포획된 뒤 왕롱으로 여왕벌을 포획하면 그때 `ReceiveSwarmCaptured`가 1회만 호출되는지 확인한다.
10. 분봉 본진에서 여왕벌을 먼저 포획하고 나중에 벌을 모두 포획해도 최종 완료가 1회만 발생하는지 확인한다.
11. hotbar/storage 이동 후 왕롱의 `FQueenCageItemState`가 유지되는지 확인한다.

## 리뷰 출력 형식

- Findings first. 심각도 순으로 파일/라인을 포함한다.
- 합당한 finding이 없으면 "중요 finding 없음"이라고 명시한다.
- 이후 테스트/검증 결과, 남은 수동 BP/Content 확인 항목을 짧게 정리한다.
