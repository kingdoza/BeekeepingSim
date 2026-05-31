# 리뷰 프롬프트: WBP_FocusPrompt C++ 이관

## 리뷰 목적

이번 리뷰는 `WBP_FocusPrompt`의 런타임 책임이 Blueprint EventGraph에서 C++ base widget(`UFocusPromptWidget`)으로 안전하게 이관되었는지 검증한다.

핵심 목표:
- `BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름이 유지되었는지
- `WBP_FocusPrompt`가 layout/style 위주로 남고, binding/text/visibility 갱신은 C++이 담당하는지
- delegate lifecycle(bind/unbind)과 null-safe 처리로 런타임 안정성이 확보되었는지
- 아키텍처 문서(`0_ARCHITECTURE`, `UISystem`, `FocusSystem`)가 변경 사항과 일치하는지

중요: 워크트리에 다른 변경이 있을 수 있으므로 **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/QNA_REVIEW.md`
- `.md/QNA_IMPLEMENTATION.md`

필요 시:
- `.md/USER_UNREAL.md`

---

## 리뷰 범위 (우선 파일)

### Source
- `Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`
- `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`

### Content
- `Content/UI/WBP_FocusPrompt.uasset`

### 문서
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/FocusSystem.md`

---

## 핵심 검증 질문

1. `UFocusPromptWidget`가 `UUserWidget` 기반으로 추가되었고 공개 API가 요구사항과 일치하는가?
2. `NativePreConstruct()`에서 design-time에 `Visible`을 적용하는가?
3. `NativeConstruct()` 시작 시 런타임 기본 상태 `Collapsed`를 보장하는가?
4. `GetOwningPlayerPawn() -> ABeekeeperCharacter -> GetBeekeeperFocus()` 자동 바인딩이 정상인가?
5. `BindToFocusComponent()`에서 기존 바인딩 정리 후 `AddUniqueDynamic` + `GetCurrentPromptData()` 즉시 반영을 수행하는가?
6. `NativeDestruct()`/`UnbindFromFocusComponent()`에서 delegate 해제가 보장되는가?
7. `SetPromptData()`가 `bIsValid` false 시 `Collapsed`, true 시 text + `Visible`을 적용하는가?
8. `TargetNameText`, `KeyText`가 `BindWidget` 이름으로 유지되는가?
9. `WBP_FocusPrompt` parent class가 `UFocusPromptWidget`으로 변경되었는가?
10. `WBP_FocusPrompt` EventGraph가 prompt binding/update 책임을 더 이상 가지지 않는가?

---

## 상세 체크리스트

### 1) C++ 계약 및 책임
- `BindToFocusComponent`, `UnbindFromFocusComponent`, `SetPromptData`, `ClearPrompt`, `HandleFocusPromptChanged` 구현 유무
- `CurrentPromptData` 캐시와 `OnPromptDataApplied(PromptData, bVisible)` 이벤트 호출 시점 적절성
- `UTextBlock*` null guard 처리 여부

### 2) 라이프사이클/안정성
- Construct/Destruct 시점 중복 바인딩, dangling delegate 가능성 없는지
- owning pawn/focus component 미해결 시 crash 없이 숨김 유지되는지
- local player 전용 흐름에서 안전한지

### 3) Blueprint 계약
- `WBP_FocusPrompt` parent: `UFocusPromptWidget`
- `TargetNameText`, `KeyText` Designer 이름 유지
- 기존 EventGraph 바인딩/갱신 노드 제거(또는 비활성) 여부

### 4) 아키텍처 일치성
- Focus는 prompt data source owner(`OnFocusPromptChanged`, `GetCurrentPromptData`) 유지
- UI는 표시 책임(`UFocusPromptWidget`)으로 분리
- Character의 widget 생성/viewport 추가 책임 유지

---

## 코드 검색 기준

권장 검색:

```powershell
rg "UFocusPromptWidget|FocusPromptWidget|OnPromptDataApplied|BindToFocusComponent|HandleFocusPromptChanged" Source/BeekeepingSim .md
rg "OnFocusPromptChanged|UpdateFocusPrompt|TargetNameText|KeyText" Source/BeekeepingSim Content/UI .md
```

확인 포인트:
- 새 클래스가 `Public/UI`, `Private/UI`에 존재
- `WBP_FocusPrompt` 관련 런타임 업데이트가 C++ 경로로 이동

---

## 수동 검증 포인트 (가능하면 PIE/Editor)

1. 플레이 직후 focus target 없음: prompt `Collapsed`
2. focus target 진입: `TargetNameText`, `KeyText` 갱신 + `Visible`
3. focus 이탈/invalid prompt: `Collapsed`
4. engaged prompt override: override 텍스트 반영
5. 위젯 제거/PIE 종료 시 delegate 해제 관련 에러 없음

---

## 빌드/검증

- 가능하면 UBT 빌드 결과 확인:
  - `BeekeepingSimEditor Win64 Development`
- 빌드 실패 시:
  - 변경분 기인 오류 vs 기존 워크트리/환경 오류를 분리해 보고

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순서로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- Findings 이후:
  - 가정/불확실성
  - 테스트 공백
  - 문서 동기화 누락 여부
