### Focus Prompt 위치 정책 QnA

1. Prompt 위치 정책을 `FFocusPromptData`에 싣는 방식으로 확정할지?
- 질문 내용: 전체 Focus prompt는 화면 중앙 근처, PartFocus prompt는 마우스 커서 근처에 표시해야 한다. 이 위치 정책을 `FFocusPromptData`에 `AnchorMode` enum으로 추가해 UI에 전달할지 결정이 필요하다.
- 필요한 이유: UI가 `IsFocusEngaged()`나 PartFocus 내부 상태를 추론하지 않고, prompt data만 보고 위치를 정할 수 있어야 시스템 경계가 단순해진다.
- 선택지
  - 옵션 A: `FFocusPromptData`에 `EFocusPromptAnchorMode { ScreenCenter, MouseCursor }`를 추가한다.
  - 옵션 B: `UFocusPromptWidget`이 focus component 상태를 직접 조회해서 전체 Focus/PartFocus를 추론한다.
  - 옵션 C: PartFocus 전용 delegate를 UI가 별도로 구독해서 위치 정책을 분리한다.
- 권장 옵션: 옵션 A. prompt data가 표시 텍스트와 위치 정책을 함께 전달하고, UI는 표시만 담당한다.
- 딥변 : 옵션A

2. 전체 Focus prompt의 기준점을 화면 중앙 고정점으로 둘지?
- 질문 내용: 전체 Focus 대상은 카메라 중앙 trace로 잡히므로 prompt를 world hit point에 투영하지 않고 viewport 중앙 근처에 고정할지 결정이 필요하다.
- 필요한 이유: world hit point projection을 쓰면 대상 표면/거리/카메라 회전에 따라 prompt가 흔들릴 수 있고, 현재 요구사항의 "화면중앙의 근처지점"과 다르게 동작할 수 있다.
- 선택지
  - 옵션 A: viewport center + `ScreenCenterOffset`에 표시한다.
  - 옵션 B: focus target actor/component의 world location을 screen projection해서 그 근처에 표시한다.
  - 옵션 C: 기존 WBP의 center anchor 위치를 그대로 유지하고 C++ 위치 갱신은 PartFocus에만 적용한다.
- 권장 옵션: 옵션 A. 전체 Focus는 카메라 중앙 판정과 같은 화면 기준으로 안정적으로 표시한다.
- 딥변 : 옵션A

3. PartFocus prompt는 마우스 커서를 매 프레임 따라가게 할지?
- 질문 내용: PartFocus hover 대상이 유지되는 동안 prompt data가 다시 브로드캐스트되지 않아도 마우스 위치는 계속 변한다. 따라서 `UFocusPromptWidget`이 visible 상태에서 위치를 반복 갱신할지 결정이 필요하다.
- 필요한 이유: `SetPromptData()` 시점에만 위치를 잡으면 cursor 이동 중 prompt가 따라오지 않는다.
- 선택지
  - 옵션 A: `UFocusPromptWidget::NativeTick()`에서 visible prompt의 위치를 매 프레임 갱신한다.
  - 옵션 B: PartFocus scope가 마우스 이동마다 prompt data를 다시 브로드캐스트한다.
  - 옵션 C: PartFocus prompt는 hover 변경 시점에만 위치를 갱신한다.
- 권장 옵션: 옵션 A. 위치 갱신은 UI 표시 책임이고, Focus/PartFocus의 prompt broadcast 빈도를 늘리지 않는다.
- 딥변 : 옵션A

4. Prompt 전체 컨테이너를 C++ `BindWidget` 필수 요소로 추가할지?
- 질문 내용: C++에서 위치를 바꾸려면 `TargetNameText`, `KeyText` 외에 프롬프트 전체 묶음 위젯을 참조해야 한다. `WBP_FocusPrompt`의 현재 프롬프트 root 위젯을 `PromptContent`로 이름 지정하고 필수 `BindWidget`으로 잡을지 결정이 필요하다.
- 필요한 이유: 텍스트만 참조하면 prompt 전체를 Canvas 좌표로 이동할 안정적인 대상이 없다.
- 선택지
  - 옵션 A: `PromptContent`를 필수 `BindWidget`으로 추가하고 WBP designer tree에서 해당 컨테이너 이름을 맞춘다.
  - 옵션 B: `BindWidgetOptional`로 두고 없으면 widget self 또는 root slot을 fallback으로 사용한다.
  - 옵션 C: C++에서 이름 검색으로 기존 `VerticalBox_52`를 찾는다.
- 권장 옵션: 옵션 A. Blueprint 구조 계약을 명확히 하고, 이름 검색이나 fallback에 의존하지 않는다.
- 딥변 : 옵션A

5. 화면 밖 이탈 방지 clamp를 기본 적용할지?
- 질문 내용: PartFocus prompt가 커서 근처를 따라갈 때 화면 오른쪽/아래쪽 가장자리에서 prompt가 viewport 밖으로 나갈 수 있다. `ViewportPadding` 기준 clamp를 기본 적용할지 결정이 필요하다.
- 필요한 이유: cursor 기반 UI는 edge에서 잘리기 쉽고, prompt의 가독성과 조작 피드백이 떨어질 수 있다.
- 선택지
  - 옵션 A: `ViewportPadding`을 둬 prompt content가 viewport 밖으로 나가지 않게 clamp한다.
  - 옵션 B: clamp하지 않고 cursor + offset 위치를 그대로 사용한다.
  - 옵션 C: edge 근처에서는 offset 방향을 반전해 cursor 반대편에 표시한다.
- 권장 옵션: 옵션 A. 구현이 단순하고 모든 해상도에서 잘림을 방지한다.
- 딥변 : 옵션A

6. 위치 튜닝값의 소속을 `UFocusPromptWidget`으로 둘지?
- 질문 내용: `ScreenCenterOffset`, `MouseCursorOffset`, `ViewportPadding` 같은 값의 소속을 정해야 한다.
- 필요한 이유: 이 값들은 Focus 판정이 아니라 UI 표시 간격이므로, Focus settings에 넣을지 Widget 기본값으로 둘지 경계를 확정해야 한다.
- 선택지
  - 옵션 A: `UFocusPromptWidget`의 `EditAnywhere, BlueprintReadOnly` layout property로 둔다.
  - 옵션 B: `UBeekeepingSimFocusSettings`에 둔다.
  - 옵션 C: `WBP_FocusPrompt` EventGraph 변수로 둔다.
- 권장 옵션: 옵션 A. UI 표시 튜닝값은 UI widget 소유가 맞고, WBP 에셋별 기본값 조정도 가능하다.
- 딥변 : 옵션A
