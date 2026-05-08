# 구현 리뷰 프롬프트: FocusEngaged Item Use Area (A~E)

## 우선순위

1. High: 입력 라우팅 정책 정확성 (selected item + host 지원 여부)
2. High: item-use session / effect 호출 경계 정확성
3. High: API/Blueprint 계약 회귀 여부 (rename/delete 금지 항목)
4. Medium: `ABeehive` first host provider/scope 통합 정확성
5. Medium: 문서 정합성 및 수동 검증 항목 충족

---

## 리뷰 대상 파일

### Focus
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaProvider.h`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/BeekeeperFocusComponent.h`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`

### Inventory
- `Source/BeekeepingSim/Public/Inventory/HoldItemUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/HoldItemUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemActionContext.h`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`

### Character
- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`

### WorldActors
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

### 문서
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

---

## 핵심 검증 항목

### High 1: 입력 라우팅 정책

- `PartFocusClickAction`이 `Started`와 `Completed` 모두 바인딩되는지
- `UBeekeeperFocusComponent`에 released 경로(`HandlePartFocusClickReleasedInput`)가 추가되었는지
- `UFocusActionComponent`에 release hook이 추가되었는지 (기본 `false`)
- `UAnchoredFocusCursorActionComponent`에서:
  - host에 `UCursorItemUseAreaScopeComponent`가 있고 selected item이 있으면 press에서 item-use 우선 처리
  - release에서 item-use session 종료 처리
  - item-use가 처리되지 않을 때 기존 `UCursorPartFocusScopeComponent` fallback 유지

### High 2: session/effect 호출 경계

- `UCursorItemUseAreaScopeComponent`가 다음을 충족하는지:
  - scope active 상태에서만 동작
  - selected item 변경 시 descriptor rebuild가 아니라 active filter만 갱신
  - LMB press/hotbar state로 `BeginUse` 세션 시작
  - Tick에서 `TickUse` 호출
  - hovered active area 위에서만 `CanApplyUseEffect` + `ApplyUseEffect`
  - release/cancel/abort/deactivate/endplay에서 `EndUse(..., bWasCanceled)` 정리

### High 3: 계약 회귀

- 아래 public 계약이 rename/delete 없이 유지되는지:
  - `PartFocusClickAction` property
  - 기존 `UCursorPartFocusScopeComponent`, `UCursorPartFocusActionComponent`, `UItemAction` 공개 API
- `#include "Public/..."` 패턴이 없는지
- Blueprint native parent rename이 없는지

### Medium 1: PartFocus suppression

- selected item 존재 + item-use-area scope active 시
  - PartFocus hover outline/prompt만 suppression 되는지
  - hover resolve/active action stack/cancel cascade는 유지되는지

### Medium 2: Beehive first host 통합

- `ABeehive`에 `ItemUseAreaScope` 컴포넌트가 추가되었는지
- `ABeehive`가 `IItemUseAreaProvider`를 구현하는지
- provider가 lid/comb descriptor를 제공하는지:
  - `AreaId`, `AreaTags`, `HitComponent`, `VisualComponents`, `EffectTargetObject`
- 가상 mesh가 없어도 기존 mesh 기반 no-op 안전 동작이 가능한지

### Medium 3: 문서 정합성

- 아키텍처 문서가 A~E 구현 상태를 반영하는지
- `USER_UNREAL.md`에 editor 수동 작업(태그/머티리얼 파라미터/PIE 체크)이 있는지

---

## 빌드/검색 검증

- UBT:
  - `BeekeepingSimEditor Win64 Development`

- 검색:
  - `rg "HandlePartFocusClickReleasedInput|HandlePartFocusClickReleasedInputWhileEngaged|ETriggerEvent::Completed" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
  - `rg "CursorItemUseAreaScopeComponent|ItemUseAreaProvider|FItemUseAreaDescriptor|FItemUseAreaVisualSettings" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
  - `rg "FindHoldItemUseAction|HoldItemUseAction|ItemUseAreaId|ItemUseEffectTargetObject" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
  - `rg "SetHoverOutlineSuppressed|ItemUseAreaScope|GetItemUseAreaDescriptors_Implementation" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/Focus Source/BeekeepingSim/Private/Focus -n`

---

## 리뷰 결과 출력 형식

1. Findings (High → Medium → Low)
2. Open Questions / Assumptions
3. Regression Risks
4. 최종 판단: Pass / Conditional Pass / Fail
