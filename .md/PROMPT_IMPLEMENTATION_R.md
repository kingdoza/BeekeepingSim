# 구현 수정 프롬프트: Beehive Comb Drag Flip/Shake 리뷰 Findings

## 우선순위

1. High: drag 진행 중에도 매 Tick pointer delta cache를 갱신하고 action update 전에 최신 delta를 제공
2. High: release 시점에 새로 시작된 drag session도 final delta로 한 번 해석한 뒤 end 처리
3. Medium: 초기화/명시 set 경로에서 flip BP event가 불필요하게 발동하지 않도록 flip event와 state apply를 분리

## 발견 문제

### 1. drag 시작 후 delta cache가 멈춰 comb flip/shake threshold를 정상 감지하지 못함

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- 원인:
  - `UpdatePartPointerGestureState()`가 `bPartDragInProgress`이면 즉시 return한다.
  - delta cache(`CachedPartDragDeltaFromPress`, `CachedPartDragDeltaSinceLastUpdate`) 갱신도 이 함수 안에만 있다.
  - Tick 순서는 `UpdatePartPointerGestureState()` 후 `UpdatePartDrag()`인데, drag 시작 이후에는 action update 전에 최신 mouse delta가 들어가지 않는다.
- 영향:
  - drag는 click cancel threshold(기본 12px)를 넘는 순간 시작되지만, comb flip threshold(기본 120px)까지 이동해도 delta가 12px 근처 값에 고정될 수 있다.
  - 일반적인 느린 좌우 drag에서는 `UBeehiveCombPartFocusActionComponent`가 flip 조건을 감지하지 못한다.
  - shake도 실제 Y 방향 반전이 delta에 반영되지 않아 stroke count가 정상 증가하지 않는다.
- 수정 방향:
  - pointer down 상태에서는 drag 진행 여부와 무관하게 mouse position과 delta cache를 갱신한다.
  - click cancel/drag begin 시도만 `!bPartDragInProgress` 조건으로 제한한다.
  - `UpdatePartDrag()` 호출 직전에 항상 최신 `CachedPartDragDeltaFromPress` / `CachedPartDragDeltaSinceLastUpdate`가 준비되도록 helper를 분리한다.

### 2. release에서 처음 threshold를 넘긴 drag는 update 없이 바로 end되어 flip/shake가 실행되지 않음

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- 원인:
  - `HandlePartFocusPointerReleased()`는 release 위치를 반영한 뒤 `TryBeginPartDrag()`가 성공하면 `bDragWasInProgress = true`로 만들고 바로 `EndPartDrag(false)`를 호출한다.
  - 이 경로에서는 `UpdatePartDrag()`가 한 번도 호출되지 않는다.
- 영향:
  - 빠른 drag-release 또는 같은 프레임 release에서 `DeltaFromPress`가 flip threshold를 만족해도 comb action이 해석할 기회가 없다.
  - 요구된 release-confirm/drag 충돌 방지 정책은 만족하지만 실제 flip/shake 동작이 누락된다.
- 수정 방향:
  - release에서 `TryBeginPartDrag()`가 새로 성공한 경우, final delta cache로 `UpdatePartDrag(0.0f)`를 1회 호출한 뒤 `EndPartDrag(false)`를 호출한다.
  - 이미 drag 중인 경우도 release final delta를 반영한 후 마지막 `UpdatePartDrag(0.0f)`를 호출할지 정책을 명시하고, comb shake 마지막 반전 누락 여부를 PIE로 검증한다.

### 3. `SetVisibleCombFace()`와 construction path가 `ReceiveCombFlipped`를 호출해 load/construction 때 flip 연출이 발동할 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- 원인:
  - `OnConstruction()`, `BeginPlay()`, `PostEditChangeProperty()`가 `SetVisibleCombFace(VisibleCombFace)`를 호출한다.
  - `SetVisibleCombFace()`는 상태 적용과 동시에 `ReceiveCombFlipped(VisibleCombFace)`를 호출한다.
- 영향:
  - Blueprint에서 `Receive Comb Flipped`를 flip 애니메이션/사운드 트리거로 구현하면 actor construction, BeginPlay, property edit 때도 연출이 실행될 수 있다.
  - 기존 `ReceiveCombFlipped(NewVisibleFace)` 호환 경로가 "실제 flip 발생" 이벤트라는 의미를 잃을 수 있다.
- 수정 방향:
  - 내부 helper 예: `ApplyVisibleCombFaceTransform()`을 분리해 construction/BeginPlay/PostEdit에서는 transform만 적용한다.
  - `FlipCombFaceWithDirection()`은 state toggle 후 기존 `ReceiveCombFlipped(NewVisibleFace)`와 신규 `ReceiveCombFlippedWithDirection(NewVisibleFace, Direction)`을 호출한다.
  - `SetVisibleCombFace()`가 public API로 이벤트를 발생시킬지 여부를 명확히 정한다. 호환 안정성을 우선하면 explicit set은 transform/state만 적용하고 flip wrapper만 event를 발생시키는 쪽이 안전하다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- PIE 수동 시나리오:
  - 1. lifted comb에서 천천히 오른쪽으로 120px 이상 drag: `Right` flip 1회 발생 확인
  - 2. lifted comb에서 천천히 왼쪽으로 120px 이상 drag: `Left` flip 1회 발생 확인
  - 3. press 후 빠르게 120px 이상 이동하고 즉시 release: flip 1회 발생 확인
  - 4. 상하 반복 drag에서 실제 방향 반전마다 stroke count가 증가하고 required count 도달 시 bee count가 1회 감소하는지 확인
  - 5. drag 시작 후 마우스를 멈춘 상태에서 shake stroke count가 임의 증가하지 않는지 확인
  - 6. actor load/BeginPlay/property edit 시 `ReceiveCombFlipped` 기반 flip 연출이 자동 발동하지 않는지 확인

## 문서 반영 필요 여부

- delta 갱신/release final update 보완은 기존 `.md/Architecture/FocusSystem.md`의 drag delta contract를 구현과 일치시키는 수정이므로 추가 문서 반영은 불필요하다.
- `SetVisibleCombFace()` 이벤트 정책을 바꾸면 `.md/Architecture/WorldActorsSystem.md`의 flip API 설명에 "state apply"와 "flip event" 차이를 명시한다.
