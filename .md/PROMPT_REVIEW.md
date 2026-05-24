# PROMPT_REVIEW — Beehive Comb Drag Flip/Shake + Flip Direction 전달

## 리뷰 목표

이번 변경의 목적은 다음 2가지다.

1. FocusEngaged 상태에서 lifted comb(`ABeehiveCombActor`)를 LMB drag로 해석
   - 좌우 drag: flip(visible face toggle)
   - 상하 반복 drag: shake(bee count 감소)
2. flip 유발 drag의 X 방향(left/right)을 C++ API와 Blueprint event까지 전달
   - 기존 flip/shake 정책, no-op 정책, click/drag gesture 충돌 방지 정책은 유지

리뷰는 **동작 정확성, 기존 계약 호환성, 회귀 위험** 중심으로 수행한다.

---

## 리뷰 대상 파일

### Source
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

### 문서
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

---

## 핵심 검증 항목

### High 1) Focus drag lifecycle 및 delta 계약
- `UCursorPartFocusActionComponent` 기존 begin/cancel/abort API가 유지되는지
- drag lifecycle API가 유지되는지:
  - `CanBeginPartFocusDrag`
  - `BeginPartFocusDrag`
  - `UpdatePartFocusDrag`
  - `EndPartFocusDrag`
  - `IsPartFocusDragInProgress`
- `UCursorPartFocusScopeComponent`가 drag delta를 제공하는지:
  - `GetPartFocusDragDeltaFromPress`
  - `GetPartFocusDragDeltaSinceLastUpdate`
- drag update 전에 delta 캐시가 갱신되는지

### High 2) comb drag 가능 조건과 mode lock
- `UBeehiveCombPartFocusActionComponent::CanBeginPartFocusDrag`가
  - action engaged 상태에서만 허용되는지
  - comb owner 유효성 검증하는지
- flip/shake mode lock이 유지되는지
  - 한 drag session에서 둘 다 실행되지 않는지
  - mode 미확정 release 시 no-op인지

### High 3) flip/shake 판정 정확성
- flip 조건:
  - `Abs(X) >= CombFlipDragThresholdPixels`
  - `Abs(X) > Abs(Y) * HorizontalDominanceRatio`
- shake 조건:
  - vertical dominance/threshold 기반 mode 확정
  - Y 누적 + 방향 반전으로 stroke 카운트 증가
  - `RequiredShakeStrokeCount` 도달 시 1회 실행
- shake 효과가 1차 범위대로 `ReduceTargetBeeCountByRatio` 경로만 쓰는지

### High 4) flip 방향 전달 확장
- `EBeehiveCombFlipDirection` enum 추가 여부
- 기존 `FlipCombFace()`가 삭제되지 않고 wrapper로 유지되는지
- 방향 포함 API 추가 여부:
  - `FlipCombFaceWithDirection(EBeehiveCombFlipDirection)`
- 방향 포함 BP 이벤트 추가 여부:
  - `ReceiveCombFlippedWithDirection(NewVisibleFace, FlipDirection)`
- `UBeehiveCombPartFocusActionComponent`에서 `DeltaFromPress.X` 부호로 방향 결정하는지
  - `>=0 -> Right`, `<0 -> Left`
- 기존 `ReceiveCombFlipped(NewVisibleFace)` 경로가 유지되어 BP 호환이 깨지지 않는지

### Medium 1) comb actor 구조/상태
- `ABeehiveCombActor`에 `CombPivotRoot`가 추가되고, mesh/niagara가 pivot 하위로 붙었는지
- visible face 상태(`Front/Back`)와 flip API가 일관되게 동작하는지
- front/back 데이터 이름/의미를 swap하지 않는지

### Medium 2) 회귀 방지
- lid open/close, comb lift/restore 기존 delegate 흐름 회귀 없는지
- item-use-area 입력 우선 정책 회귀 없는지
- public API rename/delete 없는지

---

## 코드 검색 체크

- `rg "GetPartFocusDragDeltaFromPress|GetPartFocusDragDeltaSinceLastUpdate" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
- `rg "CanBeginPartFocusDrag|BeginPartFocusDrag|UpdatePartFocusDrag|EndPartFocusDrag" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`
- `rg "CombFlipDragThresholdPixels|HorizontalDominanceRatio|CombShakeStrokeThresholdPixels|RequiredShakeStrokeCount|ShakeBeeReductionRatio|VerticalDominanceRatio" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`
- `rg "EBeehiveCombFlipDirection|FlipCombFaceWithDirection|ReceiveCombFlippedWithDirection|FlipCombFace\\(" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`

---

## PIE 수동 검증 시나리오

1. lifted comb에서 오른쪽 drag flip 시 방향 이벤트 `Right` 확인
2. lifted comb에서 왼쪽 drag flip 시 방향 이벤트 `Left` 확인
3. flip 후 visible face toggle 결과가 기존과 동일한지 확인
4. shake 제스처로 stroke count 충족 시 bee count 감소 확인
5. 대각선/애매한 drag는 no-op인지 확인
6. 한 drag session에서 flip과 shake 동시 실행되지 않는지 확인
7. drag 불가 상태에서는 threshold 초과 시 click 취소 + no-op인지 확인
8. 기존 `ReceiveCombFlipped` 기반 BP 연출이 깨지지 않는지 확인
9. lid open/close, comb lift/restore, item-use-area hold-use 회귀 없는지 확인

---

## 기대 출력 형식

1. Findings (High/Medium/Low, 심각도 순)
2. Open Questions / Assumptions
3. Regression Risk 요약
4. 최종 판단: Pass / Conditional Pass / Fail
