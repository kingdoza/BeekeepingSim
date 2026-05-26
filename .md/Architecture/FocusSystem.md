# Focus System

## Scope

- `Source/BeekeepingSim/Public/Focus/BeekeeperFocusComponent.h`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusTargetComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusInteractable.h`
- `Source/BeekeepingSim/Public/Focus/BeekeepingSimFocusSettings.h`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusTypes.h`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaProvider.h`
- `Source/BeekeepingSim/Public/Focus/ChildItemUseAreaProviderComponent.h`
- `Source/BeekeepingSim/Private/Focus/ChildItemUseAreaProviderComponent.cpp`
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
- cursor part focus screen edge cancel 두께의 project settings 제공
- FocusEngaged host 내부 item-use-area scope/provider 계약 제공

## Key Classes

- `UBeekeeperFocusComponent`: focus 상태의 단일 오너
- `UBeekeepingSimFocusSettings`: cursor part focus 등 Focus subsystem tuning 값을 보관하는 `UDeveloperSettings`
- `UFocusTargetComponent`: prompt, item rule, outline, focus event dispatch 오너
- `UFocusActionComponent`: confirm/cancel/abort 공통 액션 베이스
- `UAnchoredFocusActionComponent`: 캐릭터 앵커 이동과 카메라 블렌드 액션
- `UAnchoredFocusCursorActionComponent`: anchored action에 cursor/input mode 정책 추가
- `UCursorPartFocusScopeComponent`: FocusEngaged Host 내부 파츠 hover/confirm/cancel/outline/prompt 스코프
- `UCursorPartFocusActionComponent`: 파츠별 begin/cancel/abort lifecycle + tag/group 정책
- `ECursorPartFocusPreviewInputKey`: PartFocus hover preview key 입력(`R`, `F`, `C`) 구분 enum
- `UCursorItemUseAreaScopeComponent`: FocusEngaged host 내부 item-use-area 수집/표시/hover/LMB hold-use scope
- `IItemUseAreaProvider`: FocusEngaged host actor 또는 host가 명시적으로 위임한 actor가 `FItemUseAreaDescriptor`를 제공하는 인터페이스
- `UChildItemUseAreaProviderComponent`: owner의 직접 child actor component를 tag/class 조건으로 수집해 descriptor를 위임하는 component
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
  - 화면 외곽 취소 영역 처리
- 화면 외곽 취소 영역 두께는 `UBeekeepingSimFocusSettings::ScreenEdgeCancelRegionThickness`가 source of truth다. 기본값은 64px이며 `DefaultGame.ini` config로 조정된다.
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
- item-use-area descriptor는 provider actor가 명시 슬롯 기반으로 제공할 수 있다(예: `AItemPlacementSlotActor`).
- scope는 engaged host에서 아래만 수집한다.
  - host actor provider
  - host actor에 직접 붙은 provider component
  - host actor primitive tag fallback
- attached actor/child actor provider는 scope가 자동 순회하지 않는다.
- child slot 수집은 `UChildItemUseAreaProviderComponent`가 owner의 직접 `UChildActorComponent` 중 `RequiredChildActorComponentTag`/`RequiredChildActorClass` 조건을 통과한 항목만 위임 수집한다.
- placement action이 성공한 뒤 stack delta 반영이 실패하면, target이 `IItemPlacementSlot`일 때 `ClearPlacedItem` rollback을 수행한다.
- item-use session 중 처리:
  - `LMB Press`: `BeginUse(Context)`
  - `Hold Tick`: `TickUse(Context, DeltaTime)`
  - `active + valid hovered area`: `CanApplyUseEffect` 확인 후 `ApplyUseEffect(Context, DeltaTime)`
  - `Release/Cancel/Deactivate`: `EndUse(Context, bWasCanceled)`
- `ApplyUseEffect` 결과의 stack 변화(`bConsumedItem`, `StackDelta`)는 scope가 해석해 hotbar authority API로 반영한다.
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
  - `LMB Started`: pointer press 후보 저장
  - `LMB Completed`: click 확정 시 begin/cancel 실행 또는 drag end 실행
  - `R/F/C`: 현재 hover preview 대상의 preview key action dispatch
- Host FocusEngaged 상태에서 `FocusConfirmAction Started`는 즉시 confirm hook을 실행하지 않고 consume-only로 유지한다.
- `F` 키는 PartFocus engage/cancel 또는 FocusCancel 입력으로 사용하지 않는다.

## LMB Gesture Model (2026-05-25)

- 전역 Focus confirm과 PartFocus click은 `LMB down`에서 즉시 실행하지 않는다.
- click 확정 조건:
  - press 대상과 release 대상이 동일
  - press~release 최대 이동거리가 `ClickCancelThresholdPixels` 이하
  - drag가 시작되지 않음
- threshold 초과 시 click은 즉시 취소된다.
  - drag 불가 대상: click만 취소, 추가 동작 없음
  - drag 가능 대상: click 취소 후 drag begin 시도
- drag 시작 후 release: drag end만 실행, click 미실행
- mode 미확정/조건 미충족 drag session은 release 시 no-op 허용 (fallback 실행 없음)
- Focus pending click state owner: `UBeekeeperFocusComponent`
- PartFocus pending click/drag state owner: `UCursorPartFocusScopeComponent`
- drag delta contract (`UCursorPartFocusScopeComponent`):
  - `GetPartFocusDragDeltaFromPress()`
  - `GetPartFocusDragDeltaSinceLastUpdate()`
  - drag update 전에 scope가 screen delta를 갱신한 뒤 action lifecycle에 전달
- `UAnchoredFocusCursorActionComponent`는 engaged 입력 라우터로 유지된다.
- item-use-area hold-use는 기존 press begin/release end를 유지하며, 입력 소비 시 PartFocus gesture를 시작하지 않는다.
- edge cancel click은 release 확정 시점에서만 판정한다(press/release 모두 edge cancel 영역 + threshold 이하).

## Settings Contract

- `UBeekeepingSimFocusSettings`는 `Config=Game, DefaultConfig`인 project setting class다.
- 현재 설정값:
  - `ClickCancelThresholdPixels` (기본 12)
  - `ScreenEdgeCancelRegionThickness` (기본 64)
- `UBeekeeperFocusComponent`와 `UCursorPartFocusScopeComponent`가 `GetDefault<UBeekeepingSimFocusSettings>()`로 읽는다.
- Focus tuning 값을 추가할 때는 component hard-code보다 settings class 확장을 우선 검토한다.

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

## PartFocus Delegate Contract

- `UCursorPartFocusActionComponent`의 `BlueprintAssignable` delegate는 action component 자기 자신을 첫 인자로 전달한다.
  - `OnPartFocusBegin(ActionComponent, ScopeComponent, InteractingCharacter)`
  - `OnPartFocusCancel(ActionComponent, ScopeComponent, InteractingCharacter)`
  - `OnPartFocusAbort(ActionComponent, ScopeComponent, InteractingCharacter)`
  - `OnPartFocusPreviewKeyAction(ActionComponent, ScopeComponent, InteractingCharacter, Key)`
  - `OnPartFocusPreviewR/F/C(ActionComponent, ScopeComponent, InteractingCharacter)`
- `ReceivePartFocus...` 이벤트 경로(component subclass 구현)와 owner actor delegate 바인딩 경로는 동시에 유지한다.

## Manual Review Points

- confirm 실패 시 preview target 복원 여부
- cancel/abort 시 crosshair, cursor, input mode, hotbar rule 복구 순서
- `ScreenEdgeCancelRegionThickness`가 project settings와 runtime scope 판정에서 같은 값으로 적용되는지 확인
- FocusTargetComponent가 배치된 Blueprint/level 로드 시 missing property 경고 재발 여부

## Update 2026-05-27

- PreviewFocus 상태에서 secondary input(RMB 등)을 별도 라우팅하는 `UFocusSecondaryActionComponent` 기반 경로를 추가했다.
- `UBeekeeperFocusComponent::HandleSecondaryInput()`은 다음 정책을 따른다.
  - FocusEngaged 상태에서는 실행하지 않는다.
  - 현재 preview target owner에서 `UFocusSecondaryActionComponent`를 찾는다.
  - `CanExecuteSecondaryFocusAction` 통과 시 `ExecuteSecondaryFocusAction`을 호출한다.
- Focus는 secondary action의 실제 inventory/world mutation을 직접 수행하지 않고 실행 위임만 담당한다.

## Update 2026-05-27 (PartFocus Provider)

- FocusEngaged secondary 입력 경로를 PartFocus scope 기반으로 전환했다.
  - `UBeekeeperFocusComponent::HandleSecondaryInput()`은 engaged action의 `HandleSecondaryInputWhileEngaged()`를 호출한다.
  - `UAnchoredFocusCursorActionComponent::HandleSecondaryInputWhileEngaged()`는 owner의 `UCursorPartFocusScopeComponent::HandleSecondaryInput()`으로 위임한다.
- `UCursorPartFocusActionComponent`에 optional secondary API를 추가했다.
  - `CanHandleSecondaryPartFocusAction(...)`
  - `HandleSecondaryPartFocusAction(...)`
  - 기본 구현은 false/no-op
- `ICursorPartFocusProvider`를 추가했다.
  - provider가 `FCursorPartFocusPartDescriptor` 생성을 담당한다.
- `UCursorPartFocusRegistrationComponent`를 추가했다.
  - host의 scope 등록을 관리하며 provider(actor/component) descriptor를 수집해 scope에 등록한다.
  - `RebuildCursorPartFocusDescriptors()`와 `AppendCursorPartFocusDescriptorsToScope()`를 제공한다.
- `UChildCursorPartFocusProviderComponent`를 추가했다.
  - child actor component tag/class 조건을 통해 child provider descriptor를 수집한다.
