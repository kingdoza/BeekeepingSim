# UI System

## Scope

- `Source/BeekeepingSim/Public/UI/StorageBoxWidget.h`
- `Source/BeekeepingSim/Private/UI/StorageBoxWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemVisualWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemVisualWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotWidget.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropLibrary.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotDragDropLibrary.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotDragDropOperation.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotDragDropTypes.h`
- `Source/BeekeepingSim/Public/UI/TimeOfDayClockWidget.h`
- `Source/BeekeepingSim/Private/UI/TimeOfDayClockWidget.cpp`
- `Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`
- `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`

## Responsibilities

- hotbar/storage slot widget context와 표시 상태 유지
- full-stack/partial-stack drag payload 생성
- drop target에 따른 inventory mutation API 라우팅
- partial drag source preview와 drag visual 수량 갱신
- runtime `Hour24`를 고정 `HH:MM` 텍스트로 표시하는 clock widget 제공
- focus prompt widget의 런타임 binding/visibility/text 갱신
- Blueprint Widget이 사용할 최소 C++ API 표면 제공

## Key Classes

- `UStorageBoxWidget`: storage UI root, storage/hotbar context 초기화 진입점
- `UItemVisualWidget`: item icon/name/count 표시 base widget
- `UItemSlotWidget`: slot context, mouse input, drag 시작/drop, quick move 진입점
- `UItemSlotDragDropOperation`: drag payload와 partial move 수량 상태
- `UItemSlotDragDropLibrary`: source/target container 조합별 drop routing
- `EItemSlotContainerType`: `None`, `Hotbar`, `Storage` container 구분
- `EItemSlotDragMode`: full stack / partial stack drag 구분
- `FItemSlotMoveResult`: partial move 결과
- `UTimeOfDayClockWidget`: controller가 주입한 `Hour24`를 normalize/floor minute 변환해 `HH:MM` 표시 이벤트를 제공
- `UFocusPromptWidget`: focus component prompt delegate를 구독하고 `FFocusPromptData`를 `TargetNameText`/`KeyText`에 반영하는 base widget

## Drag/Drop Flow

1. `UItemSlotWidget::NativeOnMouseButtonDown()`이 left/right drag 버튼을 기록한다.
2. `NativeOnDragDetected()`가 `UItemSlotDragDropOperation`을 생성한다.
3. Left drag는 full stack, right drag는 partial stack으로 설정한다.
4. Partial drag는 source slot preview state와 drag visual count를 함께 갱신한다.
5. Drop 시 `UItemSlotDragDropLibrary::HandleItemSlotDrop()`이 source/target container 조합을 판정한다.
6. 실제 상태 변경은 `UBeekeeperHotbarComponent` 또는 `UStorageBoxComponent`가 수행한다.
7. Drop/cancel 후 source slot은 drag state와 active controller operation을 정리한다.

## Time Clock Flow

1. `ABeekeeperController`가 local player에서 `UTimeOfDayClockWidget`을 생성하고 viewport에 추가한다.
2. Controller가 `ITimeOfDayProvider`를 resolve하고 `AGameTimeOfDayActor::OnGameTimeOfDayChanged`를 우선 구독한다.
3. 시간이 바뀌면 controller가 `UTimeOfDayClockWidget::SetHour24()`를 호출한다.
4. Widget은 `Hour24`를 `[0, 24)`로 normalize하고 `FloorToInt(Hour24 * 60)` 기준 total minute로 변환한다.
5. 표시 minute가 이전 값과 다를 때만 `OnDisplayedTimeChanged(NewTimeText, Hour, Minute)`를 호출한다.
6. Widget은 Environment actor를 직접 검색하지 않고, bucket subsystem도 사용하지 않는다.

## Blueprint/API Contracts

현재 Blueprint 참조가 확인된 API:

- `UItemSlotWidget::InitializeSlotContext`
- `UItemSlotWidget::ShouldHideItemVisualForCurrentDrag`
- `UItemSlotWidget::IsPartialDragPreviewActive`
- `UItemSlotWidget::GetPartialDragPreviewDisplayStackCount`
- `UStorageBoxWidget::OnStorageWidgetInitialized`
- `UTimeOfDayClockWidget::SetHour24`
- `UTimeOfDayClockWidget::GetCurrentHour24`
- `UTimeOfDayClockWidget::GetFormattedTimeText`
- `UTimeOfDayClockWidget::FormatHour24AsHHMM`
- `UTimeOfDayClockWidget::OnDisplayedTimeChanged`
- `UFocusPromptWidget::BindToFocusComponent`
- `UFocusPromptWidget::UnbindFromFocusComponent`
- `UFocusPromptWidget::SetPromptData`
- `UFocusPromptWidget::ClearPrompt`
- `UFocusPromptWidget::GetCurrentPromptData`
- `UFocusPromptWidget::OnPromptDataApplied`

`ShouldHideItemVisualForCurrentDrag`는 legacy wrapper다. 새 Blueprint 로직은 가능하면 `ShouldHideItemVisualForPartialDragPreview`, `IsPartialDragPreviewActive`, `GetPartialDragPreviewDisplayStackCount` 조합을 우선 사용한다.

## C++-Only API

- `UStorageBoxWidget::InitializeStorageWidget`
- `UItemSlotWidget::RefreshPartialDragPreviewFromOperation`
- `UItemSlotWidget::RefreshDragPreviewFromOperation`

위 함수들은 현재 C++ 내부 연결을 위해 유지되며 BlueprintCallable이 아니다.

## Completed Refactoring Notes

- `UItemDragVisualWidget` 삭제
- `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation`
- `EStorageSlotContainerType` -> `EItemSlotContainerType`
- `StorageSlotDragDropTypes.h` -> `ItemSlotDragDropTypes.h`
- `UStorageBoxWidget`의 move/swap wrapper API 삭제
- `UItemSlotWidget::GetDragPreviewDisplayStackCount()` 삭제
- Core Redirect와 Blueprint compile/save 완료 상태로 기록됨

## Dependencies

- Inventory
- Character
- Environment actor를 직접 참조하지 않는다. Runtime clock은 Character/Controller가 Environment 값을 주입한다.

## Design Notes

- UI는 domain mutation을 직접 구현하지 않는다. Widget은 context를 resolve하고 Inventory API에 의도를 전달한다.
- Time clock widget은 domain time을 소유하지 않는다. 표시용 `CurrentHour24` cache와 마지막 표시 minute만 가진다.
- Time clock widget의 minute 변환은 floor 기준이다. `23.999`는 `23:59`, `24.0`은 normalize 후 `00:00`으로 표시된다.
- `UItemSlotDragDropOperation`의 UFUNCTION Category에 남아 있는 "Storage Drag Drop" 표기는 에디터 표시용 legacy naming이며 시스템 경계를 의미하지 않는다.
- Quick move target selection은 현재 UI에 남아 있는 예외적 편의 로직이다. 규칙이 복잡해지면 Inventory 쪽으로 이동한다.
- Drag visual은 별도 `UItemDragVisualWidget` 없이 `UItemVisualWidget` 계층으로 통일한다.
- `WBP_FocusPrompt`는 `UFocusPromptWidget`을 parent로 사용하고, C++이 runtime prompt binding/update를 담당한다. Blueprint는 layout/style와 선택적 `OnPromptDataApplied` 반응만 담당한다.

## Manual Review Points

- partial drag 수량 조절 중 source slot preview count와 drag visual count 일치 여부
- full stack drag cancel/drop 후 source visual 복구 여부
- storage UI 종료 시 active drag operation과 active storage context 정리 여부
- clock widget이 같은 minute 안에서 불필요하게 Blueprint 이벤트를 반복 호출하지 않는지 확인
- clock widget이 provider를 직접 resolve하지 않고, controller 주입 경로만 사용하는지 확인
- Blueprint에서 legacy wrapper를 제거하려면 먼저 `WBP_ItemSlot` 그래프를 새 API로 migration해야 한다.

## Update 2026-05-24

- `UTimeOfDayClockWidget` API is unchanged.
- Time injection source is migrated at controller side from concrete environment actor lookup to provider-based lookup (`ITimeOfDayProvider`).
