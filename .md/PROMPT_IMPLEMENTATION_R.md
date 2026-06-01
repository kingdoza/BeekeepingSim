# 구현 수정 프롬프트: Focus Prompt Multi-Entry 리뷰 Findings

## 우선순위

1. High: `UPickupFocusActionComponent::AppendFocusPromptEntries(...)` 구현 누락으로 UBT 링크 실패
2. High: pickup/retrieve prompt availability가 `PreviewAcquireItemBySpec` dry-run 기반으로 구현되지 않음
3. Medium: storage prompt entry가 `StorageOpen`/`열기` 계약으로 제공되지 않음
4. Medium: `WBP_FocusPrompt`의 `OnPromptEntriesApplied` row 렌더링 구현 확인/반영 필요

## 발견 문제

### 1. Pickup append override 선언만 있고 구현이 없어 빌드가 실패함

- 대상 파일:
  - `Source/BeekeepingSim/Public/Interaction/PickupFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/Interaction/PickupFocusActionComponent.cpp`
- 확인 결과:
  - header에 `AppendFocusPromptEntries(...) override`가 선언되어 있다.
  - cpp에는 해당 함수 정의가 없다.
  - UBT 결과: `LNK2001 unresolved external UPickupFocusActionComponent::AppendFocusPromptEntries(...)`
- 영향:
  - `BeekeepingSimEditor Win64 Development` 빌드가 실패한다.
  - Focus Prompt Multi-Entry 구현은 현재 통합 불가능 상태다.
- 수정 방향:
  - cpp에 override 구현을 추가한다.
  - `EntryId=Pickup`, `ActionText=획득`, `SortPriority=0` 계약을 지킨다.
  - enabled 판정은 hotbar mutation이 아니라 `PreviewAcquireItemBySpec`로 수행한다.

### 2. Pickup/retrieve availability가 hotbar dry-run 기반으로 연결되지 않음

- 대상 파일:
  - `Source/BeekeepingSim/Private/Interaction/PickupFocusActionComponent.cpp`
  - `Source/BeekeepingSim/Public/WorldActors/PlacementSlotRetrievePartFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/WorldActors/PlacementSlotRetrievePartFocusActionComponent.cpp`
  - `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPartFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- 확인 결과:
  - `PreviewAcquireItemBySpec`는 Inventory에 존재하고 dry-run 경로 자체는 mutation 없이 동작한다.
  - pickup prompt append 구현이 없어 dry-run availability도 없다.
  - `UPlacementSlotRetrievePartFocusActionComponent`는 `AppendPartFocusPromptEntries(...)`를 override하지 않는다.
  - `CanRetrievePlacementOccupant(...)`는 occupant/slot 조건까지만 확인하고 hotbar 수용 가능성은 확인하지 않는다.
  - 실제 회수는 `TryAcquireItemBySpec(...)`에서 실패할 수 있으므로 prompt enabled 상태와 실행 결과가 어긋날 수 있다.
  - `UBeehiveCombPartFocusActionComponent`도 retrieve prompt append override가 없고, bridge availability는 generic retrieve action의 hotbar dry-run 결과를 포함하지 않는다.
- 영향:
  - `[RMB] 회수` row가 enabled/disabled 상태로 유지 표시되지 않는다.
  - hotbar 공간 부족/stack compatibility 실패 시 disabled row 대신 row 누락 또는 enabled처럼 보이는 불일치가 발생할 수 있다.
  - 리뷰 요구사항 6, 7, PartFocus retrieve 계약을 충족하지 못한다.
- 수정 방향:
  - retrieve action에 `AppendPartFocusPromptEntries(...)` override를 추가한다.
  - 회수 조건 helper를 domain 조건 + `PreviewAcquireItemBySpec` 결과로 분리한다.
  - 실제 실행은 같은 `FItemAcquireSpec` 구성 helper를 사용해 `TryAcquireItemBySpec`를 호출한다.
  - `EntryId=Retrieve`, `KeyText=RMB`, `ActionText=회수`, `SortPriority=50`을 적용한다.
  - 실패 사유는 가능하면 `DisabledReason`에 넣는다.
  - comb bridge는 generic retrieve action의 availability helper를 재사용하거나 동일한 dry-run helper를 공유한다.

### 3. Storage entry가 전용 계약으로 제공되지 않음

- 대상 파일:
  - `Source/BeekeepingSim/Public/Interaction/StorageBoxFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`
- 확인 결과:
  - `UStorageBoxFocusActionComponent`는 `AppendFocusPromptEntries(...)`를 override하지 않는다.
  - 현재는 base fallback이 `EntryId=Primary`, empty `ActionText`로 row를 만들 수 있을 뿐이다.
- 영향:
  - 요구된 `EntryId=StorageOpen`, `ActionText=열기`, `SortPriority=0` 계약을 충족하지 못한다.
  - UI row가 다중 entry 모델의 의미 있는 action text를 표시할 수 없다.
- 수정 방향:
  - storage action에 `AppendFocusPromptEntries(...)` override를 추가한다.
  - `KeyText=Context.BasePromptData.InteractionKeyText`, `ActionText=열기`, `bEnabled=CanBeginFocusAction(Context.InteractingCharacter)`로 구성한다.

### 4. `WBP_FocusPrompt` row 렌더링 구현 반영이 확인되지 않음

- 대상 파일:
  - `Content/UI/WBP_FocusPrompt.uasset`
- 확인 결과:
  - `PromptContent`, `TargetNameText`, `KeyText`, native parent 문자열은 확인된다.
  - `OnPromptEntriesApplied` 또는 `Entries` 문자열은 검색되지 않았다.
- 영향:
  - C++이 `OnPromptEntriesApplied(...)`를 호출해도 Blueprint가 row rebuild/수직 정렬/disabled alpha를 수행하지 않을 수 있다.
- 수정 방향:
  - Unreal Editor에서 `WBP_FocusPrompt`를 열고 `OnPromptEntriesApplied` 구현을 추가/확인한다.
  - `Entries` 배열 기준으로 row를 rebuild한다.
  - `bEnabled=false` row는 disabled alpha/스타일을 적용한다.
  - invalid 호출 시 기존 rows를 clear한다.

## 검증 방법

- 검색:
  - `rg "AppendFocusPromptEntries|AppendPartFocusPromptEntries|PreviewAcquireItemBySpec|EntryId|ActionText|DisabledReason" Source/BeekeepingSim/Public Source/BeekeepingSim/Private`
  - `rg -a "PromptContent|TargetNameText|KeyText|OnPromptEntriesApplied|Entries" Content/UI/WBP_FocusPrompt.uasset`
- UBT:
  - `BeekeepingSimEditor Win64 Development`
- PIE:
  - pickup hover: `[키] 획득` enabled
  - hotbar full pickup: `[키] 획득` disabled
  - storage hover: `[키] 열기`
  - retrieve 가능: `[RMB] 회수` enabled
  - retrieve 불가 또는 hotbar full: `[RMB] 회수` disabled
  - invalid prompt 전환 시 rows clear
  - PartFocus prompt 위치는 기존처럼 cursor follow 유지

## 문서 반영 필요 여부

- 현재 문서는 설계 의도를 이미 반영하고 있다.
- 위 수정이 기존 설계 범위 안에서 구현 누락을 채우는 작업이면 추가 문서 변경은 불필요하다.

