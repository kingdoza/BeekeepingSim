# PROMPT_REVIEW — Focus LMB Click/Drag Release-Confirm 전환 리뷰

## 리뷰 목표

이번 변경의 핵심은 다음이다.

1. 전체 Focus confirm 실행 시점을 `LMB Down 즉시`에서 `LMB Up click 확정 시점`으로 변경
2. FocusEngaged 내부 PartFocus click 실행 시점을 `Down 즉시`에서 `Release 확정`으로 변경
3. 향후 drag 확장을 위해 press/release 사이 gesture state를 분리
4. item-use-area hold-use는 기존 press begin / release end 모델 유지

리뷰는 **동작 회귀, 책임 경계 준수, API 호환성, 입력 우선순위 충돌** 중심으로 수행한다.

---

## 변경 파일(리뷰 대상)

### Source
- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `Source/BeekeepingSim/Public/Focus/BeekeeperFocusComponent.h`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/BeekeepingSimFocusSettings.h`

### 문서
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/CharacterSystem.md`

---

## 필수 검증 포인트

### 1) 입력 바인딩/라우팅
- `FocusConfirmAction`이 `Started + Completed` 모두 바인딩되는지
- `PartFocusClickAction`이 `Started + Completed` 모두 바인딩되는지
- `Started`에서 즉시 begin/cancel/confirm가 실행되지 않는지
- 기존 함수(`FocusConfirmInput`, `PartFocusClickInput`, `PartFocusClickReleaseInput`)가 삭제/rename 없이 wrapper로 유지되는지

### 2) 전체 Focus click 확정 정책
- `UBeekeeperFocusComponent`가 pending click state를 소유하는지
- release 시점에만 `ConfirmFocus()`가 호출되는지
- `press target == release target` + 이동거리 threshold 이하에서만 실행되는지
- threshold 초과 시 confirm이 취소되는지

### 3) PartFocus click/drag 정책
- `UCursorPartFocusScopeComponent`가 pending click/drag state를 소유하는지
- press에서는 후보 저장만 하고 실행하지 않는지
- release에서만 click 확정 실행되는지
- threshold 초과 시 click 취소되는지
- drag 불가능 대상에서 threshold 초과 시 무동작인지
- drag 가능 대상에서 threshold 초과 시 drag begin 시도하는지
- drag 진행 중 release 시 drag end만 실행되고 click은 실행되지 않는지

### 4) item-use-area 우선순위
- `UAnchoredFocusCursorActionComponent`에서 item-use-area press/release 소비 우선이 유지되는지
- item-use가 소비한 프레임에서 PartFocus gesture가 시작되지 않는지(press/release 모두)

### 5) API/계약 호환성
- 기존 BlueprintCallable API 삭제/rename 없는지
- UCLASS/USTRUCT/UENUM rename 없는지
- Core Redirect 필요 변경이 없는지
- `ClickCancelThresholdPixels`가 `UBeekeepingSimFocusSettings`에 추가되었는지

### 6) edge cancel 정책
- edge cancel이 down 시점이 아니라 release 확정 시점에서만 발동하는지
- press/release 모두 edge 영역 + threshold 조건에서만 취소 경로가 동작하는지

---

## 기대되는 리뷰 결과 형식

1. **Findings (Severity 순)**
   - `[High|Medium|Low]` 문제 요약
   - 파일/라인 근거
   - 실제 런타임 영향
2. **Open Questions / Assumptions**
3. **Regression Risk 요약**
4. **필요 시 추가 테스트 제안**
   - PIE 수동 시나리오 번호 기준으로 명시

---

## 참고 기준 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/CharacterSystem.md`
- `.md/QNA_ARCHITECTURE.md`의 **Focus LMB Click/Drag QnA**
