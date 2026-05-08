# Focus System

## Scope

- `Source/BeekeepingSim/Public/Focus/BeekeeperFocusComponent.h`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusTargetComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusInteractable.h`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaProvider.h`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`

## Responsibilities

- PreviewFocus와 EngagedFocus 상태 관리
- 라인트레이스 기반 focus target 탐지
- prompt data, item rule, crosshair visibility 브로드캐스트
- confirm/cancel/abort 흐름에서 FocusAction 실행 위임
- focus target outline과 `IFocusInteractable` 이벤트 전달
- anchored focus camera blend 및 cursor/input mode 정책 제공
- Host 내부 파츠 hover/click용 cursor part focus scope 제공

## Key Classes

- `UBeekeeperFocusComponent`: focus 상태의 단일 오너
- `UFocusTargetComponent`: prompt, item rule, outline, focus event dispatch 오너
- `UFocusActionComponent`: confirm/cancel/abort 공통 액션 베이스
- `UAnchoredFocusActionComponent`: 캐릭터 앵커 이동과 카메라 블렌드 액션
- `UAnchoredFocusCursorActionComponent`: anchored action에 cursor/input mode 정책 추가
- `UCursorPartFocusScopeComponent`: FocusEngaged Host 내부 파츠 hover/confirm/cancel/outline/prompt 스코프
- `UCursorPartFocusActionComponent`: 파츠별 begin/cancel/abort lifecycle + tag/group 정책
- `UCursorItemUseAreaScopeComponent`: FocusEngaged host 내부 item-use-area 수집/표시/hover/LMB hold-use scope
- `IItemUseAreaProvider`: host/child actor가 `FItemUseAreaDescriptor`를 제공하는 인터페이스
- `IFocusInteractable`: actor-level focus 이벤트 인터페이스

## State Model

- `PreviewFocus`
  - 매 Tick 카메라 전방 trace로 갱신한다.
  - outline과 prompt만 활성화한다.
  - hotbar item rule은 적용하지 않는다.
- `EngagedFocus`
  - confirm 성공 후 target/action을 고정한다.
  - `OnFocusRuleChanged(true, Rule)`을 브로드캐스트한다.
  - action 정책에 따라 crosshair visibility와 hotbar presentation mode가 바뀐다.
  - action이 더 이상 engaged가 아니면 focus component가 정리한다.
  - engaged 중 confirm/cancel 입력은 action이 선택적으로 우선 소비할 수 있다.

## Cursor Part Focus Scope

- 전역 focus(`UBeekeeperFocusComponent`)와 host 내부 part focus를 분리한다.
- `UBeekeeperFocusComponent`는 world actor의 Preview/Engaged 단일 오너를 유지한다.
- `UCursorPartFocusScopeComponent`는 Host가 engaged일 때만 활성화된다.
- scope 책임:
  - 마우스 기반 trace/hover part resolve
  - `RequiredStateTags` 만족 파츠만 preview 허용
  - hover outline 적용(기존 `UFocusTargetComponent`와 동일한 CustomDepth 정책)
  - confirm/cancel 시 part action stack 처리
  - 화면 외곽 취소 영역(기본 64px) 처리
- 취소 우선순위:
  - active part action stack 역순 cancel cascade
  - stack 비어 있으면 host focus cancel로 폴백

## FocusEngaged Item Use Area Design

- Item-use area는 벌통 전용이 아니라 FocusEngaged host actor가 선택적으로 제공하는 generic 기능으로 설계한다.
- Generic naming 기준:
  - `UCursorItemUseAreaScopeComponent`
  - `FItemUseAreaDescriptor`
  - `IItemUseAreaProvider` 또는 `UItemUseAreaProviderComponent`
- `UCursorItemUseAreaScopeComponent`는 FocusEngaged host 내부에서 선택 아이템 기반 사용영역 표시, 커서 hover 판정, LMB hold item-use session, 실질 효과 routing을 담당한다.
- FocusEngaged host가 item-use-area scope/provider를 지원하고 선택 아이템이 있으면 LMB는 item-use action으로 처리한다.
- FocusEngaged host가 item-use-area를 지원하지 않거나 선택 아이템이 없으면 기존 FocusAction/PartFocus 입력 정책을 따른다.
- Anchored cursor FocusEngaged 진입 시 hotbar 선택은 비워진다. item-use area는 engaged 이후 hotbar에서 대상 아이템을 다시 선택했을 때 활성화된다.
- 사용영역 표시/점멸은 LMB와 무관하며, host가 item-use-area를 지원하고 대상 아이템이 선택된 동안 대응 영역을 표시한다.
- item-use area 활성 중에는 PartFocus outline보다 item-use area 표시를 우선하며, 결정된 정책 기준으로 선택 아이템이 있을 때 PartFocus outline은 숨긴다.
- 커서 trace는 기존 visibility trace를 사용하되 active `FItemUseAreaDescriptor`에 등록된 component인지 추가 검증한다.
- 여러 사용영역이 겹치면 trace hit result에서 가장 가까운 active area component 1개를 hover/effect 대상으로 사용한다.

## Part Action Policy

- `UCursorPartFocusActionComponent` lifecycle:
  - `CanBeginPartFocusAction`
  - `BeginPartFocusAction`
  - `CancelPartFocusAction`
  - `AbortPartFocusAction`
  - `IsPartActionEngaged`
- 상태 전환은 C++ wrapper가 관리하고, 실제 파츠 동작은 BP 이벤트로 구현한다:
  - `Receive Part Focus Begin`
  - `Receive Part Focus Cancel`
  - `Receive Part Focus Abort`
- owner actor BP 바인딩 경로를 위해 `UCursorPartFocusActionComponent`는 `BlueprintAssignable` delegate를 제공한다:
  - `OnPartFocusBegin/Cancel/Abort`
  - `OnPartFocusPreviewKeyAction`
  - `OnPartFocusPreviewR/F/C`
- `ReceivePartFocus...`는 component subclass 구현 경로, `OnPartFocus...`는 owner actor BP 이벤트 바인딩 경로로 공존한다.
- action 정책 데이터:
  - `ProvidedStateTags`
  - `RequiredStateTags`
  - `ExclusiveGroup`
- cancel cascade:
  - cancel 대상의 `ProvidedStateTags`를 요구하는 dependent action을 최신 engaged 순서 역순으로 먼저 cancel
  - 이후 원래 action cancel

## Input Notes

- Host FocusEngaged 진입은 기존 FocusConfirm 경로를 유지한다.
- Host FocusEngaged 이후 PartFocus 조작:
  - `LMB`: PartFocus begin/cancel 토글
  - `LMB Completed`: engaged action release hook으로 전달
  - `R/F/C`: 현재 hover preview 대상의 preview key action dispatch
- `F` 키는 PartFocus engage/cancel 또는 FocusCancel 입력으로 사용하지 않는다.

## Crosshair Policy

- 크로스헤어 가시성의 단일 기준점은 `UBeekeeperFocusComponent`다.
- 구체 action은 `WantsCrosshairHiddenWhileEngaged()`와 `ShouldRestoreCrosshairOnCancelStart()`로 정책만 제공한다.
- UI/HUD/Blueprint는 action component를 직접 찾지 말고 `ShouldHideCrosshair()` 또는 `OnCrosshairVisibilityChanged`를 사용한다.
- cancel 시작 시 즉시 복구가 필요한 action은 `ShouldRestoreCrosshairOnCancelStart()`를 true로 반환한다.

## Dependencies

- Character
- Camera
- Inventory

## Completed Refactoring Notes

- `UFocusTargetComponent::bClearFocusOnConfirm` 제거
- `UFocusTargetComponent::ShouldClearFocusOnConfirm()` 제거
- 제거 근거: C++/Content post-migration 검사에서 참조 없음

## Design Notes

- Focus system은 widget 인스턴스를 직접 참조하지 않는다.
- Action component는 UI를 직접 제어하지 않고 정책을 반환하거나, 필요한 경우 PlayerController input mode만 적용한다.
- `UAnchoredFocusCursorActionComponent`는 cursor/input mode를 담당하지만 crosshair 최종 브로드캐스트는 Focus component가 담당한다.
- Focus target의 item rule은 Inventory/Hotbar가 구독하는 공통 정책 데이터다.

## Manual Review Points

## PartFocus Delegate Contract

- `UCursorPartFocusActionComponent`의 `BlueprintAssignable` delegate는 action component 자기 자신을 첫 인자로 전달한다.
  - `OnPartFocusBegin(ActionComponent, ScopeComponent, InteractingCharacter)`
  - `OnPartFocusCancel(ActionComponent, ScopeComponent, InteractingCharacter)`
  - `OnPartFocusAbort(ActionComponent, ScopeComponent, InteractingCharacter)`
  - `OnPartFocusPreviewKeyAction(ActionComponent, ScopeComponent, InteractingCharacter, Key)`
  - `OnPartFocusPreviewR/F/C(ActionComponent, ScopeComponent, InteractingCharacter)`
- `ReceivePartFocus...` 이벤트 경로(component subclass 구현)와 owner actor delegate 바인딩 경로는 동시에 유지한다.

- confirm 실패 시 preview target 복원 여부
- cancel/abort 시 crosshair, cursor, input mode, hotbar rule 복구 순서
- FocusTargetComponent가 배치된 Blueprint/level 로드 시 missing property 경고 재발 여부
