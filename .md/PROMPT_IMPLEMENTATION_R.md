# 구현 수정 프롬프트: Focus LMB Click/Drag Release-Confirm 리뷰 Findings

## 우선순위

1. High: release 시점에 최종 pointer 이동거리를 재계산해 threshold 초과 click/drag 판정을 보장
2. Medium: engaged confirm press 경로가 release-confirm 정책을 우회하지 않도록 정리
3. Low: PartFocus drag 상태 API를 실제 scope/action 상태와 일치시키거나 노출 범위를 정리

## 발견 문제

### 1. release handler가 최종 pointer 위치를 반영하지 않아 같은 프레임 drag/click 취소가 누락됨

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
  - `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- 원인:
  - `UBeekeeperFocusComponent::HandleFocusPrimaryReleasedInput()`는 `bFocusClickCanceledByMovement`를 release 직전 값으로 캡처하고 바로 reset한다.
  - `UCursorPartFocusScopeComponent::HandlePartFocusPointerReleased()`도 `bPartClickCanceledByMovement`와 `bPartDragInProgress`를 release 직전 값으로 캡처하고 바로 reset한다.
  - 두 값은 Tick에서만 갱신되므로 press/release가 같은 프레임 안에서 발생하거나 마지막 Tick 이후 pointer가 threshold를 넘으면 release 위치가 판정에 포함되지 않는다.
- 영향:
  - 빠른 flick click에서 threshold를 초과해도 Focus confirm 또는 PartFocus click이 실행될 수 있다.
  - drag 가능 파츠는 release 전에 threshold를 넘었어도 drag begin/end 경로를 타지 못한다.
  - edge cancel도 threshold 조건을 실제 press-release 거리 기준으로 보장하지 못한다.
- 수정 방향:
  - release 처리 시작 시 현재 pointer 위치를 읽고 `Max*MoveDistanceSincePress`를 갱신한다.
  - threshold 초과 여부를 release handler 내부에서 다시 계산한 뒤 click/edge cancel/drag end 여부를 결정한다.
  - PartFocus는 release 시점에 threshold 초과이고 drag가 아직 시작되지 않았으면 `TryBeginPartDrag()`를 한 번 시도한 뒤, 시작 성공 시 `EndPartDrag(false)`만 실행하고 click은 실행하지 않는다. 시작 실패 시 click만 취소한다.

### 2. engaged confirm press 경로가 여전히 Started에서 action confirm hook을 호출함

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- 원인:
  - `HandleFocusPrimaryPressedInput()`는 `bIsFocusEngaged`일 때 `EngagedFocusAction->HandleConfirmInputWhileEngaged()`를 즉시 호출한다.
- 영향:
  - 현재 C++ override인 `UAnchoredFocusCursorActionComponent`는 true만 반환해 실질 동작은 없지만, Focus action 확장 시 LMB down 즉시 confirm hook이 실행될 수 있다.
  - 이번 변경의 "Started에서 즉시 begin/cancel/confirm 미실행" 기준과 어긋나는 우회 경로로 남는다.
- 수정 방향:
  - engaged 상태의 `FocusConfirmAction Started`는 gesture 후보 저장 또는 consume-only로 제한한다.
  - 실제 confirm hook 호출이 필요하면 release 확정 경로로 이동하거나, anchored cursor 전용 consume 정책을 명시적으로 분리한다.

### 3. `UCursorPartFocusActionComponent::IsPartFocusDragInProgress()`가 실제 drag 상태와 동기화되지 않음

- 대상 파일:
  - `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
  - `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- 원인:
  - action component에 `bIsPartFocusDragInProgress`가 추가되었지만 begin/end drag 경로에서 갱신하지 않는다.
  - 실제 drag 상태는 scope의 `bPartDragInProgress`만 갱신된다.
- 영향:
  - Blueprint/C++에서 action의 `IsPartFocusDragInProgress()`를 읽으면 항상 false가 될 수 있다.
- 수정 방향:
  - action-level 상태 노출이 필요하면 scope가 begin/end 성공 시 action 상태를 갱신할 수 있는 setter/internal API를 둔다.
  - 아니면 action의 상태 필드와 getter를 제거하고 scope의 `IsPartFocusDragInProgress()`만 공개 상태로 유지한다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- PIE 수동 시나리오:
  - 1. 일반 PreviewFocus 대상 위에서 LMB press 후 같은 프레임/짧은 시간 안에 threshold 이상 이동하고 release: confirm 미실행 확인
  - 2. 일반 PreviewFocus 대상 A에서 press 후 대상 B로 이동해 release: confirm 미실행 확인
  - 3. PartFocus 대상에서 press 후 threshold 이하 release: begin/cancel 1회 실행 확인
  - 4. PartFocus drag 불가 대상에서 threshold 초과 후 release: click 미실행, 추가 동작 없음 확인
  - 5. PartFocus drag 가능 테스트 subclass에서 threshold 초과: drag begin 후 release 시 drag end만 실행되고 click 미실행 확인
  - 6. edge cancel 영역에서 press/release 모두 edge이고 threshold 이하: cancel cascade 또는 host cancel 확인
  - 7. edge에서 press 후 threshold 초과 release: edge cancel 미실행 확인
  - 8. item-use-area 활성 + 선택 아이템 상태에서 LMB press/release: item-use가 소비하고 PartFocus click/drag 미시작 확인

## 문서 반영 필요 여부

- 수정이 release 판정 구현 보완이면 기존 `.md/Architecture/FocusSystem.md`의 정책과 일치시키는 작업이라 추가 문서 반영은 불필요하다.
- engaged confirm press 정책을 명시적으로 바꾸면 `.md/Architecture/FocusSystem.md` Input Notes에 consume-only/release-confirm 기준을 보강한다.
