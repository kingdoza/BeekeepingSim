### [FocusPrompt Asset 반영]

1. `WBP_FocusPrompt` PromptContent 바인딩 반영 방식
- 질문 내용
  - `UFocusPromptWidget`는 `PromptContent`를 필수 `BindWidget`으로 사용하도록 확정되어 있다.
  - 그러나 현재 환경의 Unreal Python API(커맨드렛/풀 에디터 실행 모두)에서는 `WidgetBlueprint` 디자이너 트리 접근이 노출되지 않아 `PromptContent` rename/variable 노출/slot 설정을 자동 반영할 수 없었다.
  - 따라서 `Content/UI/WBP_FocusPrompt.uasset`의 compile/save 반영은 에디터 수동 작업이 필요하다.
- 필요한 이유
  - 코드 계약(`BindWidget`)과 에셋 바인딩을 일치시켜야 런타임 위치 정책이 정상 적용된다.
- 선택지
  - 옵션 A: Unreal Editor에서 `WBP_FocusPrompt`를 수동 수정/Compile/Save한다. (권장)
  - 옵션 B: C++를 `BindWidgetOptional` + fallback으로 다시 완화한다.
  - 옵션 C: Editor Utility/플러그인 추가로 별도 자동화 파이프라인을 구축한다.
- 권장 옵션: 옵션 A
