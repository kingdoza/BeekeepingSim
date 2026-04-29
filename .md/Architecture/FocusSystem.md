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

- PreviewFocus와 EngagedFocus 상태 관리
- 라인트레이스 기반 focus target 탐지
- prompt data, item rule, crosshair visibility 브로드캐스트
- confirm/cancel/abort 흐름에서 FocusAction 실행 위임
- focus target outline과 `IFocusInteractable` 이벤트 전달
- anchored focus camera blend 및 cursor/input mode 정책 제공

## Key Classes

- `UBeekeeperFocusComponent`: focus 상태의 단일 오너
- `UFocusTargetComponent`: prompt, item rule, outline, focus event dispatch 오너
- `UFocusActionComponent`: confirm/cancel/abort 공통 액션 베이스
- `UAnchoredFocusActionComponent`: 캐릭터 앵커 이동과 카메라 블렌드 액션
- `UAnchoredFocusCursorActionComponent`: anchored action에 cursor/input mode 정책 추가
- `IFocusInteractable`: actor-level focus 이벤트 인터페이스

## State Model

- `PreviewFocus`
  - 매 Tick 카메라 전방 trace로 갱신한다.
  - outline과 prompt만 활성화한다.
  - hotbar item rule은 적용하지 않는다.
- `EngagedFocus`
  - confirm 성공 후 target/action을 고정한다.
  - `OnFocusRuleChanged(true, Rule)`을 브로드캐스트한다.
  - action 정책에 따라 crosshair visibility와 hotbar presentation mode가 바뀐다.
  - action이 더 이상 engaged가 아니면 focus component가 정리한다.

## Crosshair Policy

- 크로스헤어 가시성의 단일 기준점은 `UBeekeeperFocusComponent`다.
- 구체 action은 `WantsCrosshairHiddenWhileEngaged()`와 `ShouldRestoreCrosshairOnCancelStart()`로 정책만 제공한다.
- UI/HUD/Blueprint는 action component를 직접 찾지 말고 `ShouldHideCrosshair()` 또는 `OnCrosshairVisibilityChanged`를 사용한다.
- cancel 시작 시 즉시 복구가 필요한 action은 `ShouldRestoreCrosshairOnCancelStart()`를 true로 반환한다.

## Dependencies

- Character
- Camera
- Inventory

## Completed Refactoring Notes

- `UFocusTargetComponent::bClearFocusOnConfirm` 제거
- `UFocusTargetComponent::ShouldClearFocusOnConfirm()` 제거
- 제거 근거: C++/Content post-migration 검사에서 참조 없음

## Design Notes

- Focus system은 widget 인스턴스를 직접 참조하지 않는다.
- Action component는 UI를 직접 제어하지 않고 정책을 반환하거나, 필요한 경우 PlayerController input mode만 적용한다.
- `UAnchoredFocusCursorActionComponent`는 cursor/input mode를 담당하지만 crosshair 최종 브로드캐스트는 Focus component가 담당한다.
- Focus target의 item rule은 Inventory/Hotbar가 구독하는 공통 정책 데이터다.

## Manual Review Points

- confirm 실패 시 preview target 복원 여부
- cancel/abort 시 crosshair, cursor, input mode, hotbar rule 복구 순서
- FocusTargetComponent가 배치된 Blueprint/level 로드 시 missing property 경고 재발 여부
