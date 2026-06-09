# 작업대 소비장 BP 이동 Hook 리뷰 프롬프트

## 리뷰 목표

밀도작업대 소비장 PartFocus `잡기/놓기` 상태를 `BP_UncappingTableCombSlot`에서 안정적으로 받아 실제 이동 애니메이션을 구현할 수 있도록 추가한 C++ Blueprint hook이 요구사항대로 좁게 구현됐는지 검토한다.

이번 변경은 C++에서 이동 애니메이션을 구현하는 작업이 아니다. C++는 Blueprint 이벤트 제공과 호출 시점 보장만 담당해야 한다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/QNA_REVIEW.md`
- 필요 시 `.md/QNA_IMPLEMENTATION.md`
- 필요 시 `.md/USER_UNREAL.md`

## 리뷰 대상

- `Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

`Content/` 에셋은 수정 대상이 아니다.

## 기대 구현 요약

### 1. Blueprint 이벤트

`AUncappingTableCombSlot`에 아래 이벤트가 추가되어야 한다.

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Grabbed"))
void ReceiveCombGrabbed(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Released"))
void ReceiveCombReleased(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Grab Aborted"))
void ReceiveCombGrabAborted(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);
```

검토 포인트:

- 이벤트는 `BlueprintImplementableEvent`여야 한다.
- 이벤트는 `AUncappingTableCombSlot` 쪽 hook이어야 하며, `AUncappingTable`이나 `ABeehiveCombActor`에 둘 필요가 없다.
- `CombActor`와 `InteractingCharacter`를 Blueprint로 넘겨야 한다.
- UHT 빌드가 가능하도록 forward declaration/include가 충분해야 한다.

### 2. 이벤트 호출 시점

기존 PartFocus delegate handler에서 호출되어야 한다.

- `HandleCombPartFocusBegin`: `SetCombPartFocusEngaged(true)` 이후 `ReceiveCombGrabbed(...)`
- `HandleCombPartFocusCancel`: `SetCombPartFocusEngaged(false)` 이후 `ReceiveCombReleased(...)`
- `HandleCombPartFocusAbort`: `SetCombPartFocusEngaged(false)` 이후 `ReceiveCombGrabAborted(...)`

검토 포인트:

- 상태 갱신과 descriptor refresh가 먼저 일어나야 한다.
- 소비장 조회는 `ActionComponent->GetOwner()`보다 슬롯 맥락의 `GetPlacedCombActor()`를 사용해야 한다.
- `GetPlacedCombActor()`가 `nullptr`이면 이벤트 호출을 생략하는 것이 적절하다.
- `InteractingCharacter`는 이벤트로 그대로 전달되어야 한다.
- 기존 `bCombPartFocusEngaged` source of truth와 `RequestOwningUncappingTableRefresh()` 흐름을 깨지 않아야 한다.

### 3. 범위 제한

검토 포인트:

- C++ Tick, Timeline 대체 로직, 새 lift component, transform 보간 로직이 추가되지 않아야 한다.
- 벌통의 `UBeehiveCombLiftComponent`, `ABeehive`, `ABeehiveCombSlotActor` 동작을 건드리지 않아야 한다.
- `UCombUncappingPartFocusActionComponent`의 horizontal drag flip 로직을 변경하지 않아야 한다.
- `ABeehiveCombActor::SetVisibleCombFace`, `ApplyStateFromItemInstance`, brush/mask 로직을 변경하지 않아야 한다.
- `Content/` 에셋을 수정하거나 저장하지 않아야 한다.
- Core Redirect가 필요한 rename이 없어야 한다.

### 4. 문서 반영

검토 포인트:

- `.md/0_ARCHITECTURE.md`에는 작업대 slot BP hook의 존재와 실제 이동이 BP 책임이라는 요지가 반영되어야 한다.
- `.md/Architecture/WorldActorsSystem.md`에는 `AUncappingTableCombSlot`의 이벤트 hook과 BP 구현 책임이 반영되어야 한다.
- `.md/USER_UNREAL.md`에는 `BP_UncappingTableCombSlot`에서 수동으로 해야 할 작업이 명확히 안내되어야 한다.
- 문서가 C++에서 이동을 구현한 것처럼 오해를 만들지 않아야 한다.

## 권장 검색

```powershell
rg -n "ReceiveCombGrabbed|ReceiveCombReleased|ReceiveCombGrabAborted|HandleCombPartFocus|SetCombPartFocusEngaged|GetPlacedCombActor" Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp
rg -n "BP_UncappingTableCombSlot|Receive Comb Grabbed|AttachComponent|잡기/놓기 애니메이션 Hook" .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md .md/USER_UNREAL.md
```

## 정적 확인

- Begin/Cancel/Abort handler에서 이벤트 호출 순서가 상태 갱신 이후인지 확인한다.
- 이벤트 호출이 현재 슬롯의 placed comb 기준인지 확인한다.
- null comb에서 Blueprint 이벤트가 호출되지 않는지 확인한다.
- `InteractingCharacter`가 dead parameter로 남지 않고 BP 이벤트에 전달되는지 확인한다.
- 작업대 slot 외 클래스의 불필요한 변경이 없는지 확인한다.
- `Content/` 변경이 없는지 확인한다.

## 검증 명령

공백/패치 검증:

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md .md/USER_UNREAL.md .md/PROMPT_REVIEW.md
```

UBT 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## PIE 수동 확인

1. `BP_UncappingTableCombSlot`에서 `Receive Comb Grabbed`, `Receive Comb Released`, `Receive Comb Grab Aborted` 이벤트가 보이는지 확인한다.
2. `Receive Comb Grabbed`에서 `AttachComponent` relative transform을 저장하고 들어 올리는 Timeline/Lerp를 연결한다.
3. `Receive Comb Released`에서 저장한 rest transform으로 되돌리는 Timeline/Lerp를 연결한다.
4. `Receive Comb Grab Aborted`에서 진행 중 애니메이션을 중단하고 rest transform으로 복귀시키는지 확인한다.
5. 작업대에 소비장을 놓고 `잡기` 시 소비장이 이동하는지 확인한다.
6. `놓기` 시 원위치로 돌아오는지 확인한다.
7. 잡은 상태에서 horizontal drag flip이 기존처럼 동작하는지 확인한다.
8. 벌통의 소비장 들기/내리기 동작이 기존처럼 유지되는지 확인한다.

## 리뷰 결과 작성 형식

리뷰 결과는 `.md/AGENT_REVIEW.md` 기준으로 작성한다.

- Findings first: severity, file/line, 문제, 영향, 수정 방향
- 문제가 없으면 "검토 범위에서 발견된 blocking/major issue 없음"을 명확히 적는다.
- 남은 리스크는 UBT/PIE/BP 수동 확인 여부와 연결해서 적는다.
- 구현 요약은 findings 이후에 짧게 둔다.
