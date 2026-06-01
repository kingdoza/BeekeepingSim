# 리뷰 프롬프트: Focus Prompt ActionName Blueprint 노출

## 리뷰 목적

이번 리뷰는 Focus Prompt Multi-Entry에서 사용할 ActionName source를 Blueprint/Details에서 authoring 가능하도록 노출한 C++ API가 설계 의도와 일치하는지 검증한다.

이번 리뷰 범위는 **ActionName 노출 API**만 대상으로 한다.

제외:
- prompt row 렌더링/정렬/스타일 (`WBP_FocusPrompt`)
- 위치 정책 (`AnchorMode`, `PromptContent`, `UpdatePromptPosition`)
- availability 계산 로직 개선
- 추가 데이터 모델 설계 변경

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

---

## 리뷰 범위 파일

- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Private/Interaction/PickupFocusActionComponent.cpp`
- `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

---

## 핵심 검증 질문

1. `UFocusActionComponent`에 다음 API가 Blueprint 노출 형태로 추가되었는가?
   - `PromptActionText`, `EngagedPromptActionText`
   - setter/getter 4종
   - `ResolveFocusPromptActionText()`
2. `ResolveFocusPromptActionText()` 동작이 정책과 일치하는가?
   - engaged && engaged text not empty -> engaged text
   - else -> prompt text
3. `UCursorPartFocusActionComponent`에 다음 API가 추가되었는가?
   - `PrimaryPromptActionText`, `EngagedPrimaryPromptActionText`
   - setter/getter 4종
   - `ResolvePrimaryPromptActionText()`
4. `ResolvePrimaryPromptActionText()` 동작이 정책과 일치하는가?
5. 기본값 authoring이 반영되었는가?
   - pickup: `획득`
   - storage: `열기`
   - beehive lid: `열기` / `닫기`
   - beehive comb: `들기` / `넣기`
6. 기존 API 삭제/rename 없이 additive 변경인가?
7. 이번 작업 범위 바깥(UI row/위치 정책/DTO 구조) 변경이 섞이지 않았는가?

---

## 검색 검증

```powershell
rg "PromptActionText|EngagedPromptActionText|SetPromptActionText|GetPromptActionText|SetEngagedPromptActionText|GetEngagedPromptActionText|ResolveFocusPromptActionText" Source/BeekeepingSim .md
rg "PrimaryPromptActionText|EngagedPrimaryPromptActionText|SetPrimaryPromptActionText|GetPrimaryPromptActionText|SetEngagedPrimaryPromptActionText|GetEngagedPrimaryPromptActionText|ResolvePrimaryPromptActionText" Source/BeekeepingSim .md
rg "획득|열기|닫기|들기|넣기" Source/BeekeepingSim/Private/Interaction Source/BeekeepingSim/Private/WorldActors .md
```

---

## 빌드 검증

- 가능하면 UBT 수행:
  - `BeekeepingSimEditor Win64 Development`
- 빌드 실패 시:
  - 이번 변경 기인 오류인지 기존 워크트리/unity 충돌인지 분리 보고

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순서로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- 이슈가 없으면:
  - `No blocking issues found.` 명시
  - 남은 검증 공백(PIE/Blueprint 수동확인 등)만 간단히 기재
