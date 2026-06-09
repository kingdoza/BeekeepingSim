# 작업대 소비장 BP 이동 Hook 구현 프롬프트

## 목표

밀도작업대에서 소비장 PartFocus `잡기/놓기` 상태에 맞춰 `BP_UncappingTableCombSlot`에서 소비장을 실제로 들어 올리고 원위치로 내리는 Blueprint 애니메이션을 구현할 수 있도록 C++ hook을 추가한다.

이번 C++ 작업의 목표는 이동 애니메이션 자체를 C++로 구현하는 것이 아니라, Blueprint가 안정적으로 받을 수 있는 이벤트를 제공하는 것이다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

## 수정 대상

- `Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp`

필요 시 문서:

- `.md/USER_UNREAL.md`

`Content/` 에셋은 수정하지 않는다. BP 작업은 최종 보고에서 수동 작업으로 안내한다.

## 현재 구조

- `AUncappingTable`은 `CombSlotChildActor`로 `AUncappingTableCombSlot`을 둔다.
- `AUncappingTableCombSlot`은 `UCombUncappingPartFocusActionComponent`를 가지고, 놓인 소비장 PartFocus descriptor의 action handler로 사용한다.
- `AItemPlacementSlotActor::TryPlaceItem_Implementation()`은 배치된 소비장을 slot의 `AttachComponent`에 attach한다.
- 현재 작업대 PartFocus는 `잡기/놓기` engaged 상태와 드래그 뒤집기만 처리하며, 벌통의 `UBeehiveCombLiftComponent` 같은 실질 transform 이동 컴포넌트는 없다.

## 구현 요구

### 1. Blueprint 이벤트 추가

`AUncappingTableCombSlot` protected 영역에 Blueprint hook을 추가한다.

권장:

```cpp
UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Grabbed"))
void ReceiveCombGrabbed(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Released"))
void ReceiveCombReleased(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);

UFUNCTION(BlueprintImplementableEvent, Category = "Uncapping Table|Comb Slot", meta = (DisplayName = "Receive Comb Grab Aborted"))
void ReceiveCombGrabAborted(ABeehiveCombActor* CombActor, ABeekeeperCharacter* InteractingCharacter);
```

의미:

- `ReceiveCombGrabbed`: 작업대 소비장 PartFocus `잡기` 시작
- `ReceiveCombReleased`: `놓기` 또는 정상 cancel
- `ReceiveCombGrabAborted`: focus scope 비활성화 등 강제 종료

### 2. 기존 delegate handler에서 BP 이벤트 호출

`AUncappingTableCombSlot`에는 이미 PartFocus delegate handler가 있다.

- `HandleCombPartFocusBegin`
- `HandleCombPartFocusCancel`
- `HandleCombPartFocusAbort`

각 handler에서 `GetPlacedCombActor()`로 현재 소비장을 구하고, 상태 갱신 후 BP 이벤트를 호출한다.

권장 순서:

```cpp
SetCombPartFocusEngaged(true);
ReceiveCombGrabbed(GetPlacedCombActor(), InteractingCharacter);
```

```cpp
SetCombPartFocusEngaged(false);
ReceiveCombReleased(GetPlacedCombActor(), InteractingCharacter);
```

```cpp
SetCombPartFocusEngaged(false);
ReceiveCombGrabAborted(GetPlacedCombActor(), InteractingCharacter);
```

주의:

- `ActionComponent->GetOwner()`를 신뢰하기보다 `GetPlacedCombActor()`를 사용하는 편이 작업대 슬롯 맥락에 더 직접적이다.
- `GetPlacedCombActor()`가 null이어도 이벤트 호출은 생략해도 된다. 권장은 null이면 호출하지 않는 것이다.
- `SetCombPartFocusEngaged()`는 기존 밀도 use-area 차단/descriptor rebuild와 연결되어 있으므로 유지한다.

### 3. Blueprint에서 움직일 대상 명확화

Blueprint 수동 작업은 `BP_UncappingTable`보다 `BP_UncappingTableCombSlot`에서 하는 것을 권장한다.

이유:

- 실제 소비장은 slot의 `AttachComponent`에 attach된다.
- slot BP는 `AttachComponent`, `CombPartFocusAction`, `GetPlacedCombActor()`, 새 `ReceiveComb...` 이벤트에 접근하기 쉽다.

권장 BP 구현:

- `ReceiveCombGrabbed`
  - 현재 `AttachComponent` relative transform을 rest transform으로 저장한다.
  - Timeline 또는 Lerp로 `AttachComponent`를 들어 올린 relative transform으로 이동한다.
- `ReceiveCombReleased`
  - Timeline 또는 Lerp로 `AttachComponent`를 저장한 rest transform으로 되돌린다.
- `ReceiveCombGrabAborted`
  - 애니메이션 중이면 중단하고 즉시 또는 짧은 보간으로 rest transform으로 되돌린다.

주의:

- 소비장 actor 자체를 world transform으로 직접 움직이면 slot attach 관계, 재배치, descriptor rebuild와 어긋날 수 있다.
- 우선 `AttachComponent` relative transform을 움직이는 방식을 추천한다.
- 이동 중에도 소비장 뒤집기 드래그가 작동해야 하므로 `UCombUncappingPartFocusActionComponent`의 drag flip 로직은 변경하지 않는다.

### 4. C++에서 구현하지 말아야 할 것

- 벌통 `UBeehiveCombLiftComponent`를 작업대에 재사용하지 않는다.
- 작업대용 새 lift component를 이번 범위에서 만들지 않는다.
- 소비장 transform 이동 애니메이션을 C++ Tick으로 구현하지 않는다.
- `Content/` BP asset을 저장하지 않는다.
- `ABeehiveCombActor::SetVisibleCombFace`, `ApplyStateFromItemInstance`, brush/mask 로직은 건드리지 않는다.
- 벌통 `ABeehive` 소비장 lift 로직은 건드리지 않는다.

## Beehive 영향 제한

이번 변경은 `AUncappingTableCombSlot`의 Blueprint 이벤트 hook 추가다.

Beehive에 영향이 없어야 하는 이유:

- 벌통 소비장 PartFocus는 `ABeehive`가 `UBeehiveCombPartFocusActionComponent` delegate를 받아 `UBeehiveCombLiftComponent`로 처리한다.
- 작업대 슬롯 클래스 `AUncappingTableCombSlot`은 벌통 슬롯 `ABeehiveCombSlotActor`와 별개다.
- 이번 BP 이벤트는 작업대 slot actor에서만 호출된다.

## 검증

공백/패치 검증:

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp
```

UBT 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

PIE 수동 확인:

1. `BP_UncappingTableCombSlot`에서 새 `Receive Comb Grabbed`, `Receive Comb Released`, `Receive Comb Grab Aborted` 이벤트가 보이는지 확인한다.
2. `Receive Comb Grabbed`에서 `AttachComponent`를 들어 올리는 Timeline을 연결한다.
3. `Receive Comb Released`에서 원위치 복귀 Timeline을 연결한다.
4. 작업대에 소비장을 놓고 `잡기` 시 소비장이 이동하는지 확인한다.
5. `놓기` 시 원위치로 돌아오는지 확인한다.
6. 잡은 상태에서 horizontal drag flip이 기존처럼 동작하는지 확인한다.
7. 벌통의 소비장 들기/내리기 동작이 기존처럼 유지되는지 확인한다.

## 최종 보고 요구사항

- 변경 파일
- 추가한 Blueprint 이벤트 이름
- 이벤트 호출 시점
- BP에서 움직일 권장 대상
- Beehive 영향 없음 판단
- UBT 빌드 결과 또는 미수행 사유
- 필요한 수동 BP 작업 목록
