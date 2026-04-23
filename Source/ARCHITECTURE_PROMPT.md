다음 문서를 먼저 읽고 그 규칙과 현재 구조를 기준으로 설계 제안을 해줘:
- Source/CODEX_ARCHITECTURE.md
- Source/ARCHITECTURE.md
- Source/QNA_ARCHITECTURE.md

현재 프로젝트 상황:
- `ABeekeeperCharacter`는 `UBeekeeperFocusComponent`를 소유하고 있다.
- 포커스 시스템은 `PreviewFocus` / `EngagedFocus`로 나뉘어 있다.
- `UAnchoredFocusActionComponent`는 Confirm 시 캐릭터를 `CharacterAnchorTag`로 이동시키고, 카메라를 `FocusAnchorTag`로 블렌드한다.
- `UAnchoredFocusCursorActionComponent`는 `UAnchoredFocusActionComponent`를 상속해서:
  - Engaged 시작 시 크로스헤어 숨김 요청 델리게이트 브로드캐스트
  - 마우스 커서 표시
  - `FInputModeGameAndUI` 적용
  - Cancel 시작 시 크로스헤어 복구 요청 델리게이트 브로드캐스트
  - 카메라 복귀 완료 후 커서 숨김 및 `FInputModeGameOnly` 복구
  를 처리한다.
- 현재 크로스헤어 위젯은 `BeekeeperCharacter` Blueprint에서 생성하고 소유하고 있다.

문제:
- 현재 구조에서 크로스헤어 위젯을 숨기고 다시 표시하려면,
  `BeekeeperCharacter` 쪽에서 현재 Engaged 대상의 `UAnchoredFocusCursorActionComponent`를 찾아서
  `OnCrosshairVisibilityRequested(bool)`에 바인딩해야 한다.
- 하지만 이 바인딩 흐름이 Blueprint 기준으로 너무 복잡하다.
- 특히 아래 과정이 번거롭다:
  - `BeekeeperFocusComponent.OnFocusRuleChanged`를 감지
  - `GetEngagedFocusTarget()`
  - Owner Actor 찾기
  - `UAnchoredFocusCursorActionComponent` 찾기
  - 델리게이트 바인딩/언바인딩 관리
- 이 방식보다 더 단순하고 유지보수하기 쉬운 설계를 찾고 싶다.

원하는 것:
- 지금 구조를 바탕으로, “크로스헤어 Visibility를 Engage 상태에 따라 제어하는 더 나은 설계”를 제안해줘.
- 굳이 이벤트 바인딩 방식이 아니어도 된다.
- 핵심은:
  - Blueprint 연결이 단순해야 한다
  - UI 책임 분리가 적절해야 한다
  - 현재 `PreviewFocus` / `EngagedFocus` 구조를 깨지 않아야 한다
  - 재사용 가능한 포커스 액션 구조는 유지해야 한다
  - 특정 액터(벌통) 전용 하드코딩이 아니어야 한다

설계 검토 대상:
1. 현재 방식 유지 + 단순화
   - 예: `BeekeeperFocusComponent` 또는 Character가 더 직접적인 상태 신호를 제공
2. 액션 컴포넌트 델리게이트 구조 개선
   - 예: HUD/Controller가 바인딩하기 쉽게 상위 공통 컴포넌트에서 통합 브로드캐스트
3. 포커스 시스템 중심 설계
   - 예: `UBeekeeperFocusComponent`가 “크로스헤어 숨김 필요 여부”를 직접 계산/노출
4. UI 소유자 변경
   - Character 대신 `PlayerController` 또는 HUD가 크로스헤어 위젯을 소유
5. 바인딩 없는 상태 조회 방식
   - 예: UI가 매 프레임/이벤트 기반으로 단순 bool 상태를 읽는 방식
6. 그 외 더 나은 구조가 있으면 자유롭게 제안

비교해줬으면 하는 관점:
- Blueprint 구현 난이도
- C++ 책임 분리의 명확성
- 포커스 시스템 재사용성
- UI 시스템과의 결합도
- 포제션 변경 대응
- 멀티플레이 로컬 플레이어 안전성
- 장기 유지보수성

중요 제약:
- 위젯 인스턴스를 액션 컴포넌트가 직접 참조하는 방식은 가급적 피하고 싶다
- 하지만 현재 델리게이트 바인딩 방식보다 훨씬 낫다면 장단점을 비교해줘
- `PreviewFocus` 단계에서는 크로스헤어를 유지하고,
  `EngagedFocus` 단계에서만 숨김/복구가 일어나야 한다
- Cancel 시작 시 즉시 크로스헤어 복구, 커서/입력모드는 카메라 복귀 완료 후 복구라는 기존 UX는 유지하고 싶다

원하는 출력 형식:
1. 먼저 “현재 구조에서 가장 큰 문제점” 요약
2. 그 다음 가능한 대안 2~4개 제시
3. 각 대안의 장단점 비교
4. 최종 추천안 1개 제시
5. 추천안 기준으로:
   - 어떤 C++ 클래스가 어떤 책임을 가지는지
   - Blueprint에서는 무엇만 하면 되는지
   - 왜 지금보다 단순해지는지
   를 설명

가능하면 아래 수준으로 구체적으로 설명해줘:
- 어떤 클래스에 bool 상태나 델리게이트를 추가할지
- `BeekeeperFocusComponent`, `AnchoredFocusActionComponent`, `PlayerController`, HUD 중 어디가 중심이 될지
- 크로스헤어 위젯 소유자는 Character 유지가 나은지, Controller/HUD 이동이 나은지

핵심 목표:
- “현재 설계 기반에서, 크로스헤어 Visibility 제어를 더 단순하게 만들 수 있는 아키텍처 대안”을 얻고 싶다.
