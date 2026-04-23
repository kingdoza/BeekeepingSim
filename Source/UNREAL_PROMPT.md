다음은 현재 C++ 구현 기준으로, Unreal Editor 내장 AI 어시스턴트에게 그대로 붙여넣어 사용할 프롬프트다.

---

현재 프로젝트 상황:
- 크로스헤어 위젯은 현재 `BeekeeperCharacter` Blueprint에서 생성하고 소유하고 있다.
- 새 C++ 컴포넌트 `UAnchoredFocusCursorActionComponent` 가 추가되어 있다.
- 이 컴포넌트는 `OnCrosshairVisibilityRequested(bool bVisible)` 델리게이트를 브로드캐스트한다.
- 의도된 동작은:
  - Engaged 시작 시 `false` 브로드캐스트 -> 크로스헤어 숨김
  - FocusCancel 시작 시 `true` 브로드캐스트 -> 크로스헤어 즉시 복구
- 이 컴포넌트는 위젯 인스턴스를 직접 참조하지 않는다.
- 따라서 Blueprint 쪽에서 델리게이트를 구독해서 현재 크로스헤어 위젯의 표시/숨김을 처리해야 한다.

원하는 작업:
1. 현재 `BeekeeperCharacter`가 생성/소유 중인 크로스헤어 위젯을 유지하는 방식으로 먼저 구현해줘.
2. `UAnchoredFocusCursorActionComponent::OnCrosshairVisibilityRequested` 이벤트를 받아 크로스헤어 위젯을 활성/비활성화하는 Blueprint 연결 절차를 구체적으로 안내해줘.
3. 가능하면 아래 내용을 순서대로 자세히 설명해줘:
   - `BeekeeperCharacter` Blueprint에서 현재 생성 중인 크로스헤어 위젯 참조 변수 확인 또는 생성
   - 위젯을 `BeginPlay` 또는 기존 생성 시점에 `Add to Viewport` 하는 구조 확인
   - 현재 포커스 대상 액터의 `FocusAction` 컴포넌트가 `UAnchoredFocusCursorActionComponent`인지 확인하는 방법
   - `OnCrosshairVisibilityRequested(bool bVisible)`에 Blueprint에서 바인딩하는 방법
   - `bVisible` 값에 따라 현재 크로스헤어 위젯을 `Set Visibility` 또는 `Add/Remove From Parent` 중 어떤 방식으로 처리하는 게 나은지 추천
   - 포커스 대상이 벌통(`ABeehive`)일 때 실제로 이 델리게이트가 동작하도록 연결하는 방법
4. 특히 “크로스헤어 위젯은 Character가 소유하고, 액션 컴포넌트는 대상 액터가 소유”하는 구조에서 이벤트를 어떻게 찾아서 바인딩해야 하는지 Blueprint 노드 흐름으로 설명해줘.
5. 가능하면 아래 두 가지 구현안을 비교해서 추천해줘:
   - 옵션 A: `BeekeeperCharacter`가 계속 크로스헤어 위젯을 소유하고, 현재 Engaged 대상의 `UAnchoredFocusCursorActionComponent`에 바인딩
   - 옵션 B: 크로스헤어 위젯 소유를 `PlayerController` 또는 HUD로 옮기고, 거기서 액션 컴포넌트 델리게이트를 바인딩
6. 위 두 옵션 중 현재 구조에서 더 적절한 쪽을 하나 추천하고, 이유를 설명해줘.

추천 방향 관련 추가 요구:
- 현재 구조에서는 `BeekeeperCharacter`가 위젯을 소유하는 것도 동작 가능하다고 본다.
- 하지만 더 장기적으로 `PlayerController` 또는 HUD가 소유하는 게 낫다면 그 이유를 설명해줘.
- 특히 다음 관점으로 비교해줘:
  - 포제션 변경 대응
  - UI 책임 분리
  - 재사용성
  - 현재 프로젝트 구조와의 충돌 여부

출력 형식 요구:
1. 먼저 “현재 구조 유지 시 구현 방법”을 단계별로 설명
2. 그 다음 “위젯 소유자를 바꾸는 게 더 좋은지” 판단
3. 마지막에 추천안 1개를 제시

추가로 원함:
- 가능하면 실제 Blueprint 노드 이름 기준으로 설명해줘
- 예:
  - `Create Widget`
  - `Add to Viewport`
  - `Bind Event to OnCrosshairVisibilityRequested`
  - `Set Visibility`
  - `Cast To AnchoredFocusCursorActionComponent`
- 위젯을 완전히 제거하기보다 `Visibility` 전환이 더 적절하면 그 이유도 같이 설명해줘

핵심 목표:
- 포커스 상호작용 중에는 크로스헤어가 숨겨지고
- Cancel 시작 시 즉시 다시 보이게 하며
- 현재 C++ 구조(`UAnchoredFocusCursorActionComponent` 델리게이트 기반)를 그대로 활용하는 Blueprint 연결 절차를 만들고 싶다.
