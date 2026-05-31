# 구현 수정 프롬프트: Focus Prompt AnchorMode 리뷰 Findings

## 우선순위

1. High: `WBP_FocusPrompt` asset에 필수 `PromptContent` BindWidget 계약 반영
2. 참고: UBT 빌드 실패는 이번 FocusPrompt 변경 파일 기인이 아닌 기존 unity build 충돌로 분리

## 발견 문제

### 1. `WBP_FocusPrompt` asset에 `PromptContent`가 없어 위치 정책이 실제 적용되지 않음

- 대상 파일:
  - `Content/UI/WBP_FocusPrompt.uasset`
  - `Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`
  - `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`
- 확인 결과:
  - `UFocusPromptWidget`는 `PromptContent`를 필수 `BindWidget`으로 선언한다.
  - `UFocusPromptWidget::UpdatePromptPosition()`은 `PromptContent`의 `UCanvasPanelSlot`에 `SetPosition(...)`을 적용한다.
  - `rg -a "PromptContent" Content/UI/WBP_FocusPrompt.uasset` 결과가 0건이다.
  - 같은 asset에서 `NativeParentClass=/Script/BeekeepingSim.FocusPromptWidget`, `TargetNameText`, `KeyText`는 확인된다.
- 원인:
  - C++ 위치 정책은 추가됐지만 Blueprint designer tree의 전체 prompt 컨테이너 이름 변경/variable 노출/compile-save가 최종 asset에 반영되지 않았다.
- 영향:
  - `PromptContent` 필수 binding 계약을 충족하지 못한다.
  - widget compile/load 단계에서 required BindWidget 오류 또는 runtime null binding이 발생할 수 있다.
  - `UpdatePromptPosition()`이 `PromptContent == nullptr`에서 반환하므로 `ScreenCenter`/`MouseCursor` 위치 정책, DPI 변환, viewport clamp가 실제 WBP에 적용되지 않는다.
- 수정 방향:
  - Unreal Editor에서 `Content/UI/WBP_FocusPrompt`를 연다.
  - prompt 전체를 감싸는 컨테이너를 `PromptContent`로 rename한다.
  - `PromptContent`를 variable로 노출한다.
  - `PromptContent`가 `CanvasPanel` direct child인지 확인한다.
  - `PromptContent` slot을 runtime 위치 제어에 맞춰 `CanvasPanelSlot`으로 유지하고, 필요 시 `Auto Size=true`, `Alignment=(0, 0.5)`를 적용한다.
  - `TargetNameText`, `KeyText` 이름은 유지한다.
  - Compile/Save 후 `rg -a "PromptContent|TargetNameText|KeyText|NativeParentClass" Content/UI/WBP_FocusPrompt.uasset`로 재확인한다.

## 검증 방법

- 검색:
  - `rg "EFocusPromptAnchorMode|AnchorMode|ScreenCenterOffset|MouseCursorOffset|ViewportPadding|PromptContent|UpdatePromptPosition" Source/BeekeepingSim .md`
  - `rg -a "PromptContent|TargetNameText|KeyText|NativeParentClass|FocusPromptWidget" Content/UI/WBP_FocusPrompt.uasset`
- UBT:
  - `BeekeepingSimEditor Win64 Development`
  - 현재 빌드 실패 원인:
    - `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`
    - `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
    - 두 파일의 anonymous namespace `CenterMouseCursorInViewport` 함수명이 unity build에서 충돌
  - 위 빌드 blocker는 이번 FocusPrompt 변경 파일 기인이 아니지만, 최종 검증 전 별도 수정이 필요하다.
- Editor/PIE:
  - 플레이 직후 target 없음: prompt 숨김
  - 일반 Focus target 진입: 화면 중앙 근처 표시
  - 일반 Focus 이탈: 숨김
  - PartFocus hover 진입: 커서 근처 표시
  - PartFocus 유지 + 마우스 이동: prompt가 커서를 따라감
  - viewport edge 근처: prompt가 화면 밖으로 이탈하지 않음
  - invalid prompt/hover 해제: 숨김
  - PIE 종료 시 delegate 해제 관련 오류 없음

## 아키텍처 문서 반영 필요 여부

- 현재 `.md/0_ARCHITECTURE.md`, `.md/Architecture/FocusSystem.md`, `.md/Architecture/UISystem.md`에는 AnchorMode 위치 정책이 반영되어 있다.
- 위 수정은 Content asset 계약 반영이므로 추가 문서 변경은 불필요하다.

