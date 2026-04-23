다음 문서를 먼저 읽고 그 규칙과 현재 구조를 기준으로 작업해:
- Source/CODEX_ARCHITECTURE.md
- Source/ARCHITECTURE.md
- Source/QNA_ARCHITECTURE.md

다음 작업을 수행해.

[목표]
현재 포커스 시스템에서 크로스헤어 Visibility 제어를 단순화한다.
핵심 목표는 `BeekeeperCharacter` Blueprint가 현재 Engaged 대상의 `UAnchoredFocusCursorActionComponent`를 직접 찾고 델리게이트를 바인딩하지 않도록 만드는 것이다.

구체적으로는:
- `UBeekeeperFocusComponent`가 “현재 크로스헤어를 숨겨야 하는지”를 직접 계산/노출하도록 구조를 바꾼다
- Blueprint UI는 `UBeekeeperFocusComponent` 하나만 보고 크로스헤어 WBP를 숨김/표시할 수 있어야 한다
- 기존 `PreviewFocus` / `EngagedFocus` 구조는 유지한다
- `UAnchoredFocusCursorActionComponent`는 위젯을 직접 참조하지 않고, 위젯 제어 정책만 제공한다

[작업 대상 범위]
오직 아래 경로의 실제 C++ 파일만 수정 또는 생성 대상으로 고려한다.
- Source/BeekeepingSim/Public
- Source/BeekeepingSim/Private

[설계 기준]
- `ABeekeeperCharacter`는 `UBeekeeperFocusComponent`를 소유한다
- `UBeekeeperFocusComponent`는 `PreviewFocus` / `EngagedFocus` 상태를 관리한다
- `UAnchoredFocusCursorActionComponent`는 위젯을 직접 건드리지 않고 “Engaged 중 크로스헤어를 숨겨야 하는지” 정책만 제공한다
- `PreviewFocus` 단계에서는 항상 크로스헤어를 유지한다
- `EngagedFocus` 단계에서만 크로스헤어 숨김이 발동한다
- Cancel 시작 시 즉시 크로스헤어를 복구한다
- 커서 숨김과 입력 모드 복구는 카메라 복귀 완료 후 수행한다

[대상 파일]
- Source/BeekeepingSim/Public/BeekeeperFocusComponent.h
- Source/BeekeepingSim/Private/BeekeeperFocusComponent.cpp
- Source/BeekeepingSim/Public/AnchoredFocusCursorActionComponent.h
- Source/BeekeepingSim/Private/AnchoredFocusCursorActionComponent.cpp
- Source/BeekeepingSim/Public/FocusActionComponent.h
- Source/BeekeepingSim/Private/FocusActionComponent.cpp

[구현 요구사항]
1. `UBeekeeperFocusComponent`가 크로스헤어 Visibility의 단일 상태 소스가 되어야 한다
2. Blueprint는 더 이상 Engaged 대상 액터나 액션 컴포넌트를 직접 찾지 않아야 한다
3. `UAnchoredFocusCursorActionComponent`는 위젯 인스턴스를 직접 참조하지 않아야 한다
4. `UAnchoredFocusCursorActionComponent`는 정책 질의 함수만 제공해야 한다
   - `WantsCrosshairHiddenWhileEngaged()`
   - `ShouldRestoreCrosshairOnCancelStart()`
5. `UBeekeeperFocusComponent`는 아래를 제공해야 한다
   - `bShouldHideCrosshair`
   - `ShouldHideCrosshair()`
   - `OnCrosshairVisibilityChanged(bool bVisible)` 또는 동등 구조
6. `UBeekeeperFocusComponent`는 아래 시점에 최종 크로스헤어 상태를 갱신해야 한다
   - Engaged 시작 직후
   - Cancel 시작 직후
   - Engaged 종료 시
7. 중복 브로드캐스트 방지 로직을 포함해야 한다
8. 비로컬 플레이어 UI 상태를 바꾸지 않도록 안전하게 처리해야 한다

[구현 방향]
- `UFocusActionComponent`에 기본 UI 정책 질의 함수 추가
  - 기본값은 false
- `UAnchoredFocusCursorActionComponent`는 위 함수를 override해서:
  - Engaged 중 숨김 필요 = true
  - Cancel 시작 시 즉시 복구 필요 = true
- `UBeekeeperFocusComponent`는 현재 `EngagedFocusAction`의 정책을 읽어 최종 `bShouldHideCrosshair`를 계산한다
- Character Blueprint는 앞으로:
  - `Get BeekeeperFocus`
  - `Bind Event to OnCrosshairVisibilityChanged`
  - 또는 `ShouldHideCrosshair()` 조회
  만 수행하면 된다

[Unreal 관련 제약 조건]
- UObject 라이프사이클과 GC를 고려한다
- 핵심 상태 계산은 C++로 구현한다
- Blueprint는 UI 반영과 단순 바인딩에 한정한다
- Tick 추가는 피하고, 기존 상태 전이 지점에서만 계산한다

[출력 요구사항]
- [상태] 완료
- [요약] 수행 내용
- [영향 파일] 변경 파일 목록
- [ARCHITECTURE.md 반영 여부] 반영 내용 요약
- [다음] 추가 지시 대기
