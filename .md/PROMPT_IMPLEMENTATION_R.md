# 구현 수정 프롬프트: FocusEngaged Item Use Area Review Findings

## 우선순위

1. High: `UAnchoredFocusCursorActionComponent`가 FocusEngaged 진입 시 selected hotbar item을 유지하도록 정책 수정
2. High: item-use area visual 적용이 실제 gameplay mesh visibility/collision을 파괴하지 않도록 수정
3. High: cancel 입력이 active item-use session을 즉시 취소하도록 lifecycle 경계 수정
4. Medium: item-use area 활성화 직후 PartFocus hover outline/prompt suppression이 즉시 적용되도록 수정

## 발견 문제

### 1. FocusEngaged 진입 시 selected item clear 가능

- 대상 파일:
  - `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
  - `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- 원인:
  - `UFocusActionComponent::ShouldClearHotbarSelectionOnFocusEngaged()` 기본값이 `true`인데, `UAnchoredFocusCursorActionComponent`가 override하지 않는다.
  - `UBeekeeperHotbarComponent::ApplyFocusRule()`는 FocusEngaged 진입 시 이 정책에 따라 `SelectedIndex`를 clear한다.
- 영향:
  - 벌통 FocusEngaged 진입 직후 selected item이 사라져 `UCursorItemUseAreaScopeComponent`가 hold-use action을 찾지 못한다.
  - A~E의 핵심 요구사항인 "selected item + host 지원 시 item-use 우선 처리"가 동작하지 않는다.
- 수정 방향:
  - `UAnchoredFocusCursorActionComponent::ShouldClearHotbarSelectionOnFocusEngaged()`를 override해 `false`를 반환한다.
  - 필요 시 slot/wheel 입력 차단 정책은 기존 설계에 맞춰 별도 판단하되, selected item 유지가 우선이다.

### 2. item-use area visual 적용이 실제 mesh를 숨기고 collision을 끈다

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
  - 필요 시 `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
  - 필요 시 `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 원인:
  - `ApplyVisualStateForDescriptor()`가 `Descriptor.HitComponent`와 `VisualComponents`에 `SetVisibility()` / `SetCollisionEnabled()`를 직접 호출한다.
  - `ABeehive::GetItemUseAreaDescriptors_Implementation()`는 fallback으로 `LidComponent`/`CombMesh` 같은 실제 gameplay mesh를 hit+visual로 등록한다.
- 영향:
  - active descriptor가 아니거나 scope deactivate 시 벌통/뚜껑/소비장 실제 mesh가 숨겨지고 collision이 꺼진다.
  - visual component collision을 켜면 투명 visual mesh가 trace를 가로막아 `HitComponent` exact-match hover가 실패할 수 있다.
- 수정 방향:
  - 실제 gameplay mesh의 visibility/collision 원상태를 item-use scope가 임의로 변경하지 않는다.
  - visual 표현은 MID parameter만 갱신하거나, 별도 visual-only component가 있을 때만 표시 상태를 제어한다.
  - 최소 수정으로는 `HitComponent` collision/visibility 변경을 제거하고, `VisualComponents`도 collision 변경을 하지 않는다.
  - 실제 mesh fallback에서는 "no-op safe"가 보장되어야 한다.

### 3. cancel 입력이 active item-use session을 종료하지 않을 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
  - `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
  - `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- 원인:
  - `UAnchoredFocusCursorActionComponent::HandleCancelInputWhileEngaged()`는 `UCursorPartFocusScopeComponent::HandleCancelInput()`만 우선 호출한다.
  - active item-use session을 취소하는 public 경로가 release/deactivate 외에는 없다.
- 영향:
  - LMB hold-use 중 `Esc`가 PartFocus action cancel cascade에 소비되면 item-use session이 계속 Tick될 수 있다.
  - 요구사항의 `release/cancel/abort/deactivate/endplay` cleanup 경계를 만족하지 못한다.
- 수정 방향:
  - cancel 입력 처리 시작 시 active item-use session이 있으면 `EndUse(..., true)`가 호출되도록 public cancel API를 추가한다.
  - item-use cancel 처리 후 PartFocus cancel cascade와 host focus cancel fallback은 기존 우선순위를 유지한다.

### 4. 활성화 직후 PartFocus suppression 미적용

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- 원인:
  - `ActivateItemUseAreaScope()`에서 `UpdatePartFocusOutlineSuppression()`을 `bIsScopeActive = true` 이전에 호출한다.
  - `RefreshSelectedItemAndAction()`는 selected item/action이 동일하면 early return하므로 이후 suppression이 보정되지 않는다.
- 영향:
  - selected item이 이미 있는 상태로 FocusEngaged에 진입하면 PartFocus outline/prompt가 숨겨지지 않을 수 있다.
- 수정 방향:
  - `bIsScopeActive`와 tick 활성화 이후 `UpdatePartFocusOutlineSuppression()`을 호출한다.
  - 또는 activation 마지막에 명시적으로 suppression을 재적용한다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg "ShouldClearHotbarSelectionOnFocusEngaged|SetVisibility\\(|SetCollisionEnabled\\(|SetHoverOutlineSuppressed" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
- PIE:
  - 벌통 FocusEngaged 진입 후 선택 hotbar item이 유지되는지 확인
  - hold-use action 아이템 선택 상태에서 LMB press/hold/release가 item-use session으로 동작하는지 확인
  - item-use area 활성/비활성 전환 후 벌통/뚜껑/소비장 mesh visibility/collision이 깨지지 않는지 확인
  - selected item이 있을 때 PartFocus hover outline/prompt만 suppression 되고 cancel cascade는 유지되는지 확인

## 문서 반영 필요 여부

- 설계 변경 없이 구현 버그 수정이면 문서 반영은 불필요하다.
- visual component 제어 정책을 새 UPROPERTY로 분리하면 `.md/Architecture/FocusSystem.md`, `.md/Architecture/WorldActorsSystem.md`, `.md/USER_UNREAL.md`에 authoring 기준을 추가한다.
