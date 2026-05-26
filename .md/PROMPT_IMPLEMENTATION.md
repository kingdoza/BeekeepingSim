# Hotbar Middle Click Selection Toggle 구현 프롬프트

## 전제

이번 작업은 마우스 휠 클릭(Middle Mouse Button)으로 hotbar 선택 상태를 토글하는 기능을 추가한다.

확정 요구사항:

- 슬롯 선택 상태에서 휠 클릭:
  - 전체 슬롯 미선택 상태로 전환한다.
  - 해제 직전 선택 슬롯을 마지막 선택 슬롯으로 기억한다.
- 슬롯 미선택 상태에서 휠 클릭:
  - 마지막으로 선택된 슬롯을 다시 선택한다.
  - 마지막 선택 슬롯 기본값은 1번째 슬롯이다. C++ index 기준 `0`.
- 마지막 선택 슬롯이 유효하지 않거나 focus rule로 disabled이면 fallback 선택을 사용한다.
- 기능 상태 owner는 `UBeekeeperHotbarComponent`다.
- `ABeekeeperCharacter`는 input action을 받아 hotbar API로 라우팅만 한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/CharacterSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/USER_UNREAL.md`
- `.md/QNA_IMPLEMENTATION.md`

## 목표

1. `UBeekeeperHotbarComponent`에 마지막 선택 슬롯 상태를 추가한다.
2. hotbar 선택/해제/강제 해제 시 마지막 선택 슬롯을 일관되게 갱신한다.
3. middle click용 hotbar toggle API를 추가한다.
4. `ABeekeeperCharacter`에 middle click input action property와 input handler를 추가한다.
5. Editor input mapping 작업을 `.md/USER_UNREAL.md`에 기록한다.

## 수정 대상

- `Source/BeekeepingSim/Public/Inventory/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/CharacterSystem.md`
- `.md/USER_UNREAL.md`

필요 시:

- `.md/0_ARCHITECTURE.md`

## 설계 원칙

- Hotbar 선택 상태는 Inventory system의 `UBeekeeperHotbarComponent`가 소유한다.
- Character는 input routing만 담당한다.
- UI widget은 선택/해제 mutation을 직접 구현하지 않는다.
- FocusEngaged 중 hotbar slot input이 막히는 상황에서는 middle click toggle도 막는다.
- `LastSelectedIndex`는 runtime selection memory다. 저장/세이브 데이터로 다루지 않는다.

## Hotbar 구현 요구

### 상태 추가

`UBeekeeperHotbarComponent` private 상태:

```cpp
int32 SelectedIndex = INDEX_NONE;
int32 LastSelectedIndex = 0;
```

`LastSelectedIndex` 기본값은 `0`이다.

### Public API 추가

```cpp
UFUNCTION(BlueprintCallable, Category = "Hotbar")
void ToggleSelectionFromLastSelectedSlot();

UFUNCTION(BlueprintPure, Category = "Hotbar")
int32 GetLastSelectedIndex() const { return LastSelectedIndex; }
```

### Private helper 추가

권장:

```cpp
void RememberSelectedIndex();
int32 ResolveToggleFallbackSelectionIndex() const;
```

`RememberSelectedIndex()`:

- `SelectedIndex`가 유효하면 `LastSelectedIndex = SelectedIndex`

`ResolveToggleFallbackSelectionIndex()`:

1. `LastSelectedIndex`가 valid이고 slot enabled이면 반환
2. `0`번 slot이 valid이고 enabled이면 반환
3. 첫 번째 enabled slot 반환
4. 없으면 `INDEX_NONE`

빈 slot도 선택 가능한 기존 hotbar 정책을 유지한다. 즉 item 존재 여부가 아니라 `IsSlotEnabled(Index)`만 본다.

### ToggleSelectionFromLastSelectedSlot()

동작:

```cpp
if (bIsEngagedFocusActive && ActiveFocusAction && ActiveFocusAction->ShouldBlockHotbarSlotInputWhileEngaged())
{
	return;
}

if (SelectedIndex != INDEX_NONE)
{
	RememberSelectedIndex();
	ClearSelection();
	return;
}

const int32 TargetIndex = ResolveToggleFallbackSelectionIndex();
if (TargetIndex != INDEX_NONE)
{
	SelectSlot(TargetIndex);
}
```

주의:

- `ClearSelection()`이 자체적으로 `RememberSelectedIndex()`를 호출하게 할 경우 중복 호출은 문제 없지만, 의도를 명확히 유지한다.
- `SelectSlot()`이 성공하면 `LastSelectedIndex`도 갱신한다.

### SelectSlot(Index)

기존 성공 경로에 추가:

```cpp
SelectedIndex = Index;
LastSelectedIndex = Index;
BroadcastHotbarChanged();
```

### ClearSelection()

기존 clear 전에:

```cpp
RememberSelectedIndex();
SelectedIndex = INDEX_NONE;
BroadcastHotbarChanged();
```

### ApplyFocusRule(...)

FocusEngaged 진입 등으로 선택을 강제 clear하는 경우에도 마지막 선택 슬롯을 보존해야 한다.

현재 코드에서:

```cpp
if (!bWasEngaged && bIsEngagedFocusActive && bShouldClearSelectionOnEngage && SelectedIndex != INDEX_NONE)
{
	SelectedIndex = INDEX_NONE;
	bSelectionChanged = true;
}
```

변경:

```cpp
RememberSelectedIndex();
SelectedIndex = INDEX_NONE;
```

### ReevaluateSlotsInternal()

`ShouldClearSelectedSlot()` 때문에 선택이 clear되는 경우에도 마지막 선택 슬롯을 기억한다.

```cpp
if (ShouldClearSelectedSlot())
{
	RememberSelectedIndex();
	SelectedIndex = INDEX_NONE;
	bHasChanged = true;
}
```

### ApplySelectedItemStackDelta(...)

stack count가 0 이하가 되어 선택 슬롯이 비워지고 `SelectedIndex = INDEX_NONE`이 되는 경우에도 마지막 선택 슬롯을 기억한다.

현재 선택 슬롯이 item use로 소모되어 비워질 때:

```cpp
RememberSelectedIndex();
Slots[SelectedIndex].ItemInstance = nullptr;
SelectedIndex = INDEX_NONE;
```

### InitializeSlots()

`LastSelectedIndex`가 invalid이면 `0`으로 복구하거나 fallback을 사용한다.

권장:

```cpp
if (!IsIndexValid(LastSelectedIndex))
{
	LastSelectedIndex = 0;
}
```

## Character input 구현 요구

### Header

`ABeekeeperCharacter` input properties에 추가:

```cpp
UPROPERTY(EditDefaultsOnly, Category = "Inputs")
TObjectPtr<UInputAction> HotbarToggleSelectionAction;
```

input handler 추가:

```cpp
void HotbarToggleSelectionInput();
```

### CPP binding

`SetupPlayerInputComponent`에서:

```cpp
if (HotbarToggleSelectionAction)
{
	EnhancedInputComponent->BindAction(
		HotbarToggleSelectionAction,
		ETriggerEvent::Started,
		this,
		&ABeekeeperCharacter::HotbarToggleSelectionInput);
}
```

### Handler

```cpp
void ABeekeeperCharacter::HotbarToggleSelectionInput()
{
	if (!BeekeeperHotbar)
	{
		return;
	}

	BeekeeperHotbar->ToggleSelectionFromLastSelectedSlot();
}
```

Character에서는 focus input lock을 직접 판단하지 않는다. Hotbar component의 block 정책을 기준으로 한다.

## Editor 작업 문서화

`.md/USER_UNREAL.md`에 추가:

1. `IA_HotbarToggleSelection` 생성
2. Value Type은 Digital/Boolean 계열로 설정
3. Input Mapping Context에서 Middle Mouse Button 또는 Mouse Wheel Button에 매핑
4. `BP_BeekeeperCharacter`에서 `HotbarToggleSelectionAction`에 `IA_HotbarToggleSelection` 할당

## 검증 기준

### 코드 검색

있어야 함:

- `LastSelectedIndex`
- `ToggleSelectionFromLastSelectedSlot`
- `GetLastSelectedIndex`
- `ResolveToggleFallbackSelectionIndex`
- `RememberSelectedIndex`
- `HotbarToggleSelectionAction`
- `HotbarToggleSelectionInput`

### 동작 확인

1. 1번 슬롯 선택 상태에서 middle click
   - 전체 슬롯 미선택
2. 다시 middle click
   - 1번 슬롯 선택
3. 3번 슬롯 선택 후 middle click
   - 미선택
4. 다시 middle click
   - 3번 슬롯 선택
5. 선택 아이템을 사용해서 stack이 0이 되어 선택 해제된 뒤 middle click
   - 마지막 선택 슬롯 index를 기준으로 재선택 시도
6. 마지막 선택 슬롯이 focus rule로 disabled인 상태에서 middle click
   - 0번 슬롯 또는 첫 enabled 슬롯 fallback
7. Focus action이 hotbar slot input을 block하는 상태에서 middle click
   - 선택 상태가 바뀌지 않음

### 빌드

가능하면 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 문서 갱신

구현 후 갱신:

- `.md/Architecture/InventorySystem.md`
  - hotbar selection memory와 middle click toggle 정책 추가
- `.md/Architecture/CharacterSystem.md`
  - `HotbarToggleSelectionAction` input binding 추가
- `.md/USER_UNREAL.md`
  - input action/mapping/BP assignment 절차 추가

`.md/0_ARCHITECTURE.md`는 전체 시스템 책임 흐름이 바뀌지 않으면 필수는 아니다. 다만 Character/Inventory 입력 요약에 이미 hotbar input 항목이 있으면 짧게 반영한다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- middle click이 기존 Enhanced Input mapping에서 다른 gameplay action과 충돌하는 경우
- FocusEngaged 중에도 강제로 빈손 toggle을 허용해야 한다는 요구가 확인되는 경우
- 마지막 선택 슬롯이 disabled일 때 fallback 없이 아무것도 하지 않아야 한다는 요구가 확인되는 경우
- 빈 슬롯을 선택하지 못하게 해야 한다는 요구가 확인되는 경우
- UI Blueprint가 `SelectedIndex == INDEX_NONE` 외에 별도 selected state를 직접 관리하고 있어 C++ 변경만으로 동작이 일관되지 않는 경우
