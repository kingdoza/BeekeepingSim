다음 Unreal Engine C++ 변경을 코드 리뷰해줘. 이번에는 “추가된 크로스헤어 단일 소스 구조” 중심으로 검토해줘.

대상 파일:
- Source/BeekeepingSim/Public/BeekeeperFocusComponent.h
- Source/BeekeepingSim/Private/BeekeeperFocusComponent.cpp
- Source/BeekeepingSim/Public/AnchoredFocusCursorActionComponent.h
- Source/BeekeepingSim/Private/AnchoredFocusCursorActionComponent.cpp
- Source/BeekeepingSim/Public/FocusActionComponent.h
- Source/BeekeepingSim/Private/FocusActionComponent.cpp
- Source/ARCHITECTURE.md

변경 목적:
1. `UBeekeeperFocusComponent`가 크로스헤어 Visibility의 단일 상태 소스가 되도록 변경
2. Blueprint가 더 이상 현재 Engaged 대상의 `UAnchoredFocusCursorActionComponent`를 직접 찾거나 델리게이트를 바인딩하지 않도록 단순화
3. `UAnchoredFocusCursorActionComponent`는 위젯 제어 대신 UI 정책 질의 함수만 제공

현재 의도된 동작:
- `PreviewFocus` 단계:
  - 항상 크로스헤어 표시
- `EngagedFocus` 진입:
  - 현재 `EngagedFocusAction->WantsCrosshairHiddenWhileEngaged()`가 true면 크로스헤어 숨김
- `CancelFocus` 시작:
  - 현재 `EngagedFocusAction->ShouldRestoreCrosshairOnCancelStart()`가 true면 즉시 크로스헤어 복구
- `EngagedFocus` 종료:
  - 기본적으로 크로스헤어 표시 상태 유지
- `UAnchoredFocusCursorActionComponent`는 기존처럼:
  - 커서 표시
  - `FInputModeGameAndUI`
  - 카메라 복귀 완료 후 커서 숨김 + `FInputModeGameOnly`
  는 유지

새 구조:
- `UFocusActionComponent`
  - `WantsCrosshairHiddenWhileEngaged()`
  - `ShouldRestoreCrosshairOnCancelStart()`
  - 기본값 false
- `UAnchoredFocusCursorActionComponent`
  - 두 정책 함수를 override해서 true 반환
  - 위젯 인스턴스 직접 참조 없음
- `UBeekeeperFocusComponent`
  - `bShouldHideCrosshair`
  - `ShouldHideCrosshair()`
  - `OnCrosshairVisibilityChanged(bool bVisible)`
  - Engaged 진입/Cancel 시작/Engaged 종료 시 최종 상태 갱신

집중 리뷰 포인트:
1. `UBeekeeperFocusComponent`가 실제로 크로스헤어 상태의 단일 소스 역할을 제대로 수행하는지
2. Engaged 진입, Cancel 시작, Engaged 종료 시점의 상태 갱신 순서가 요구사항과 맞는지
3. `OnCrosshairVisibilityChanged`가 중복 브로드캐스트 없이 동작하는지
4. `UAnchoredFocusCursorActionComponent`가 위젯 제어 책임을 완전히 내려놓고 정책만 제공하는지
5. 기존 커서 표시/입력 모드 전환 로직은 회귀 없이 유지되는지
6. `PreviewFocus` 단계에서 절대 크로스헤어가 숨겨지지 않는지
7. `CancelFocus` 즉시 크로스헤어 복구와 카메라 복귀 완료 후 커서/입력 모드 복구가 서로 충돌하지 않는지
8. 비로컬 플레이어에서 UI 상태 변경 의미가 없는 경로를 안전하게 처리하는지
9. Blueprint가 이제 `BeekeeperFocusComponent`만 보면 충분한 구조인지
10. 추가 테스트가 필요한 race/abort/invalid target 케이스가 있는지

가능하면 파일/라인 기준으로 severity 순으로 지적해줘.
결과는 아래 형식으로 정리해줘:
- Critical
- Major
- Minor
- 추가 테스트 필요 케이스
