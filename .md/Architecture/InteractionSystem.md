# Interaction System

## Scope

- `Source/BeekeepingSim/Public/Interaction/PickupFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Interaction/PickupFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Interaction/StorageBoxFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`

## Responsibilities

- `UFocusActionComponent`의 도메인별 구현 제공
- pickup confirm 시 hotbar 획득 실행
- storage confirm 시 storage UI 생성, input mode 전환, active storage context 등록
- interaction 종료/cancel/abort 시 UI와 controller transient state 정리
- pickup/storage 같은 전역 Focus 상호작용 prompt entry와 availability 제공

## Key Classes

- `UPickupFocusActionComponent`: 단발성 월드 아이템 획득 액션
- `UStorageBoxFocusActionComponent`: 장기 engaged storage UI 액션

## Runtime Flow

### Pickup

1. owner가 `AWorldItemPickup`이고 pickup data가 유효한지 확인한다.
2. interacting character의 hotbar에 `TryAcquireItem()`을 호출한다.
3. 성공 또는 부분 성공이면 pickup actor를 consume/destroy한다.
4. 실패하면 actor는 유지하고 로그/온스크린 디버그 메시지만 출력한다.

### Storage

1. owner의 `UStorageBoxComponent`, player hotbar, widget class, local controller를 검증한다.
2. focus interaction input을 잠근다.
3. cursor 표시와 `FInputModeGameAndUI`를 적용한다.
4. `UStorageBoxWidget`을 생성하고 C++ 전용 `InitializeStorageWidget()`으로 컨텍스트를 주입한다.
5. `ABeekeeperController`에 active storage component를 등록한다.
6. cancel/abort/end play 시 widget 제거, active drag/storage 정리, cursor/input mode 복구, input lock 해제를 수행한다.

## Dependencies

- Focus
- Inventory
- UI
- WorldActors
- Character

## Design Notes

- Interaction system은 FocusAction의 구체 효과를 구현하되, inventory slot mutation 자체는 Inventory 컴포넌트에 위임한다.
- Storage action은 UI widget lifecycle의 owner다. Widget은 storage action이 주입한 context를 읽는다.
- Pickup action은 장기 engaged 상태 없이 동기적으로 완료될 수 있다.
- `UPickupFocusActionComponent`와 `UStorageBoxFocusActionComponent`는 `UFocusActionComponent::AppendFocusPromptEntries(...)` 경로로 표시 entry를 제공한다.
- pickup/storage action name은 `UFocusActionComponent::PromptActionText` Blueprint authoring 값을 사용하고, 표시 source는 `ResolveFocusPromptActionText()`를 따른다. 기본값은 pickup `획득`, storage `열기`다.
- pickup prompt availability는 실제 획득 실행과 같은 item/acquire spec을 사용하되, prompt 판정 시에는 hotbar `PreviewAcquireItemBySpec` dry-run을 사용한다.
- pickup 공간 부족 등 실행 불가 상태는 prompt entry 제거가 아니라 `bEnabled=false` disabled 표시로 표현할 수 있다.

## Manual Review Points

- storage interaction 종료 시 `ClearActiveItemSlotDragOperation()`과 `ClearActiveStorageComponent()` 호출 여부
- storage widget 생성 실패 시 input lock/cursor/input mode가 복구되는지 확인
- pickup 실패 시 actor가 유지되고 focus 상태가 정상 해제되는지 확인

## Update 2026-06-01 (Focus Prompt Entries)

- interaction action component가 전역 Focus prompt entry 생성 surface를 제공하는 설계를 확정했다.
- pickup/storage entry는 Focus/UI가 도메인 조건을 직접 알지 않도록 action component에서 append한다.
