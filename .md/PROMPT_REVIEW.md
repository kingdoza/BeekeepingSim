# 리뷰 프롬프트: Focus Prompt 위치 정책(AnchorMode) 구현

## 리뷰 목적

이번 리뷰는 `WBP_FocusPrompt` / `UFocusPromptWidget`에 추가된 위치 정책이 아키텍처 합의(`QNA_ARCHITECTURE` Focus Prompt 위치 정책 QnA 1~6 옵션 A)와 일치하는지 검증한다.

핵심 목표:
- `FFocusPromptData`가 prompt 위치 정책(`AnchorMode`)를 데이터로 전달하는지
- 일반 Focus는 `ScreenCenter`, PartFocus는 `MouseCursor`로 동작하는지
- 위치 갱신 책임이 Focus 시스템이 아니라 `UFocusPromptWidget` UI 계층에 유지되는지
- viewport clamp/DPI 좌표 변환/Canvas slot alignment 반영이 안전한지
- 문서(`0_ARCHITECTURE`, `FocusSystem`, `UISystem`)가 구현과 동기화되어 있는지

중요: 워크트리에 다른 변경이 있을 수 있으므로 **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/QNA_ARCHITECTURE.md`

필요 시:
- `.md/USER_UNREAL.md`

---

## 리뷰 범위 (우선 파일)

### Source
- `Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`
- `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`

### Content
- `Content/UI/WBP_FocusPrompt.uasset`

### 문서
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/UISystem.md`

---

## 핵심 검증 질문

1. `FFocusPromptData`에 `EFocusPromptAnchorMode`/`AnchorMode`가 additive 방식으로 추가되었는가?
2. 기본 prompt 경로(`UFocusTargetComponent::GetPromptData`)는 `ScreenCenter` 기본값을 유지하는가?
3. PartFocus override 경로(`BroadcastPartPrompt`)에서 `AnchorMode=MouseCursor`를 명시하는가?
4. `UFocusPromptWidget`에 `PromptContent` 필수 `BindWidget` 계약이 추가되었는가?
5. `SetPromptData(valid)` 직후 `UpdatePromptPosition()` 호출로 첫 프레임 위치가 즉시 반영되는가?
6. `NativeTick()`에서 visible + valid 상태일 때 위치를 매 프레임 갱신하는가?
7. viewport/mouse 좌표가 UMG 단위(DPI 보정)로 통일되어 적용되는가?
8. clamp가 `ViewportPadding + DesiredSize + Alignment`를 반영하고 역전 구간(`Max < Min`)을 안전 처리하는가?
9. Focus system이 widget 위치를 직접 조작하지 않고 prompt data source owner 역할만 유지하는가?
10. `WBP_FocusPrompt`에서 `PromptContent`, `TargetNameText`, `KeyText` 바인딩 계약이 실제로 충족되는가?

---

## 상세 체크리스트

### 1) Focus 데이터 계약
- `EFocusPromptAnchorMode { ScreenCenter, MouseCursor }` 존재
- `FFocusPromptData::AnchorMode` 기본값 `ScreenCenter`
- Core Redirect 필요 없는 additive 변경인지

### 2) PartFocus 변환 계약
- `UCursorPartFocusScopeComponent::BroadcastPartPrompt()`에서 engaged override 변환 시 `MouseCursor` 지정
- `FCursorPartFocusPromptData` 자체는 확장하지 않았는지

### 3) UI 위치 계산 책임
- `PromptContent`가 `CanvasPanelSlot`이 아닐 때 crash 없이 no-op
- `ForceLayoutPrepass()` + `PromptContent->GetDesiredSize()` 사용 여부
- `CanvasSlot->GetAlignment()` 반영 여부
- `ScreenCenterOffset`, `MouseCursorOffset`, `ViewportPadding`가 widget layout property로 존재하는지
- `UBeekeepingSimFocusSettings`에 prompt spacing 값이 추가되지 않았는지

### 4) Blueprint 계약
- `WBP_FocusPrompt` parent class가 `UFocusPromptWidget`인지
- `PromptContent` 변수 바인딩, `TargetNameText`/`KeyText` 이름 유지
- `PromptContent`가 `CanvasPanelSlot` 하위인지
- EventGraph가 prompt binding/text/visibility/position 책임을 재소유하지 않는지

### 5) 문서 동기화
- `0_ARCHITECTURE`: AnchorMode 포함 및 ScreenCenter/MouseCursor 정책 명시
- `FocusSystem`: data source owner 유지 + PartFocus 변환 정책 명시
- `UISystem`: 위치 갱신 책임/레이아웃 계약/NativeTick 역할 명시

---

## 코드 검색 기준

권장 검색:

```powershell
rg "EFocusPromptAnchorMode|AnchorMode|ScreenCenterOffset|MouseCursorOffset|ViewportPadding|PromptContent|UpdatePromptPosition" Source/BeekeepingSim .md
rg "SetEngagedFocusPromptOverride|BroadcastPartPrompt|GetPromptData" Source/BeekeepingSim/Public/Focus Source/BeekeepingSim/Private/Focus
rg "FocusPromptWidget|WBP_FocusPrompt|TargetNameText|KeyText|PromptContent" Source/BeekeepingSim Content/UI .md
```

---

## 수동 검증 포인트 (가능하면 PIE/Editor)

1. 플레이 직후 target 없음: prompt 숨김
2. 일반 Focus target 진입: 화면 중앙 근처 표시
3. 일반 Focus 이탈: 숨김
4. PartFocus hover 진입: 커서 근처 표시
5. PartFocus 유지 + 마우스 이동: prompt가 커서를 따라감
6. viewport edge 근처: prompt가 화면 밖으로 이탈하지 않음
7. invalid prompt/hover 해제: 숨김
8. PIE 종료 시 delegate 해제 관련 에러 없음

---

## 빌드/검증

- 가능하면 UBT 빌드:
  - `BeekeepingSimEditor Win64 Development`
- 빌드 실패 시:
  - 이번 변경 기인 오류 vs 기존 워크트리 오류를 분리 보고

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
