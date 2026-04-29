# Interaction System

## Scope

- `Source/BeekeepingSim/Public/Interaction/PickupFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Interaction/PickupFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Interaction/StorageBoxFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`

## Responsibilities

- 포커스 confirm 시 아이템 획득/보관함 UI 진입 액션 실행
- 액션별 입력 잠금/커서/입력 모드 정책 적용
- 단발성 pickup 액션과 장기 engaged UI 액션 분리

## Key Classes

- `UPickupFocusActionComponent`: 월드 아이템 획득 액션
- `UStorageBoxFocusActionComponent`: 보관함 UI 중심 액션

## Dependencies

- Focus
- Inventory
- UI
- WorldActors

## Refactoring Notes

- 액션 클래스 이름과 public API 유지
- storage action의 widget 표시/해제 및 active storage 등록 흐름 유지
- 카메라 블렌드 없는 UI 액션(Storage) vs 단발성 pickup 액션 분리 유지

## Manual Review Points

- confirm/cancel/abort 시 입력 잠금/커서 상태 복구
- pickup 실패 시 actor 유지와 메시지 처리
- storage action 중 hotbar 입력 차단 정책 유지 여부
