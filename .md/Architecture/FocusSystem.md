# Focus System

## Scope

- `Source/BeekeepingSim/Public/Focus/BeekeeperFocusComponent.h`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusTargetComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusInteractable.h`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`

## Responsibilities

- PreviewFocus/EngagedFocus 전환 상태 관리
- 라인트레이스 기반 포커스 타겟 탐지 및 프롬프트/아웃라인 제어
- confirm/cancel 액션 실행과 카메라 블렌드 흐름 관리
- 크로스헤어/커서 정책 브로드캐스트

## Key Classes

- `UBeekeeperFocusComponent`: 포커스 상태 오너
- `UFocusTargetComponent`: 타겟 프롬프트/아웃라인/규칙 데이터 오너
- `UFocusActionComponent`: 상호작용 액션 베이스
- `UAnchoredFocusActionComponent`: 앵커 이동/카메라 블렌드 액션
- `UAnchoredFocusCursorActionComponent`: 앵커 액션 + 커서/UI 모드 정책
- `IFocusInteractable`: 포커스 이벤트 인터페이스

## Dependencies

- Character
- Camera
- Inventory
- UI

## Refactoring Notes

- `UFocusTargetComponent::bClearFocusOnConfirm` / `ShouldClearFocusOnConfirm()` 유지
- 위 필드는 현재 정책 필드로 보존(미사용 여부는 추후 검토 대상)
- concrete action 직접 결합 대신 `UFocusActionComponent` 경유 구조 유지

## Manual Review Points

- engaged 진입/취소 시 크로스헤어 가시성 전환 타이밍
- cancel 시 커서 복구와 입력 모드 복구 순서
- `bClearFocusOnConfirm` 실제 사용 경로 존재 여부(향후 정리 후보)
