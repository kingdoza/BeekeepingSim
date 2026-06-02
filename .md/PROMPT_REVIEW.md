# 리뷰 프롬프트: Active-Use 내구도 Tick 감소 구현

## 리뷰 목적

이번 리뷰는 FocusEngaged item-use-area hold-use 경로에서 active-use durability drain이 설계 합의(QnA)와 일치하게 구현되었는지 검증한다.

핵심:
- `UItemDefinition` base class 비확장
- 전용 subclass(`UActiveUseDurabilityItemDefinition`) 기반 opt-in
- selected item durability mutation authority는 `UBeekeeperHotbarComponent`
- Focus scope는 입력/영역/결과 라우팅 담당
- 훈연기/소독약 대상, 벌솔 제외 정책 유지

제외:
- Content `.uasset` 직접 수정
- repair/broken item 신규 시스템
- 기존 UCLASS/USTRUCT/UENUM rename

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md` (`[사용영역 active 중 아이템 내구도 Tick 감소]`)
- `.md/QNA_IMPLEMENTATION.md`

---

## 리뷰 범위 파일

- `Source/BeekeepingSim/Public/Inventory/ActiveUseDurabilityItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/ItemActionTypes.h`
- `Source/BeekeepingSim/Public/Inventory/HoldItemUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/HoldItemUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/USER_UNREAL.md`

주의:
- `Content/*.uasset` 변경은 이번 코드 리뷰의 주 검토 대상이 아니다. Content dirty 상태는 별도 Editor 수동 작업/검증 항목으로만 다룬다.
- 기존 워크트리에 있는 unrelated 변경은 revert하지 않는다.

---

## 핵심 검증 질문

1. `UActiveUseDurabilityItemDefinition`이 신규 추가되었고 `UItemDefinition` base에는 active-use drain property가 추가되지 않았는가?
2. `FItemActionExecutionResult::DurabilityDelta`가 추가되었는가?
   - `bConsumedItem`/`StackDelta`와 독립 해석되는가?
3. `UHoldItemUseAction::ResolveActiveUseDurabilityDelta(...)`가 QnA 조건을 충족하는가?
   - source item/definition 유효성
   - `bUsesDurability`, `MaxDurability>0`, `MaxStack==1`
   - `DurabilityDrainPerSecond>0`
   - durability 0 이하 차단
   - `DrainPolicy`별 조건 분기가 맞는가?
     - `WhenUseEffectSucceeded`: valid use-area + `EffectResult.bSucceeded`
     - `WhileOverValidUseArea`: valid use-area
     - `WhileUseSessionActive`: active use session only
4. `UHoldItemUseAction` 기본 can-path(`ReceiveCanBeginUse`, `ReceiveCanApplyUseEffect`)에 durability 0/invalid config 차단이 반영되었는가?
5. `UBeekeeperHotbarComponent::ApplySelectedItemDurabilityDelta(...)`가 authority mutation API로 동작하는가?
   - nearly-zero/no selected/no item/no durability 아이템 방어
   - clamp, applied/no-op 판정
   - depleted + remove 정책
   - mutation 시 `ReevaluateSlotsInternal()` + `BroadcastHotbarChanged()`
6. `UCursorItemUseAreaScopeComponent`가 stack/durability 결과를 독립 처리하는가?
   - 기존 `!bConsumedItem` 조기 return이 제거되었는가?
   - `Result.DurabilityDelta != 0`일 때만 hotbar durability API 호출하는가?
   - `bRemoveWhenDepleted`를 selected item definition(`UActiveUseDurabilityItemDefinition`)에서 읽는가?
   - `WhileUseSessionActive` 정책에서 hovered active descriptor가 없어도 durability-only result를 처리하는가?
   - `WhenUseEffectSucceeded`/`WhileOverValidUseArea` 정책에서 invalid area일 때 durability-only drain이 발생하지 않는가?
7. durability 0 도달 시 use session이 `EndUseSession(false)`로 종료되는가?
   - item 제거 여부와 무관하게 active end 이벤트 누락 없이 종료되는가?
8. placement rollback(`bConsumedItem` + stack delta 실패 + placement 성공 경로) 정책이 유지되는가?
9. 벌솔(`UBeeBrushUseAction`)에 active-use durability drain 전용 코드가 추가되지 않았는가?
10. rename이 없으므로 Core Redirect 추가가 불필요한 상태를 유지하는가?

---

## 검색 검증

```powershell
rg "ActiveUseDurabilityItemDefinition|EActiveUseDurabilityDrainPolicy|DurabilityDrainPerSecond|DrainPolicy|bRemoveItemWhenDepleted" Source/BeekeepingSim .md
rg "DurabilityDelta|ApplySelectedItemDurabilityDelta|FHotbarItemDurabilityMutationResult|ResolveActiveUseDurabilityDelta" Source/BeekeepingSim .md
rg "ApplyUseEffectResultToSelectedItem|bConsumedItem|StackDelta" Source/BeekeepingSim/Private/Focus Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory
rg "BeeBrushUseAction" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory .md
```

확인할 것:
- `UItemDefinition` base class에 active-use durability property가 없어야 한다.
- `UPollenPattyItemDefinition` 기존 계약과 충돌 없어야 한다.
- selected durability mutation은 `ApplySelectedItemDurabilityDelta` 경로만 사용해야 한다.

---

## 빌드 검증

- 권장:
  - `BeekeepingSimEditor Win64 Development`
- 에디터 DLL 잠금으로 Editor 타깃 링크 실패 시:
  - `BeekeepingSim Win64 Development` 성공 여부를 대체 컴파일 근거로 보고
  - 실패 원인이 변경 코드인지 환경 잠금인지 분리 보고

---

## 수동 검증 포인트 (PIE)

1. 훈연기/소독약 DataAsset을 `UActiveUseDurabilityItemDefinition` 기반으로 전환
2. `DrainPolicy=WhenUseEffectSucceeded`: FocusEngaged + 유효 use-area hold-use 중 effect success Tick에서만 durability 감소 확인
3. `DrainPolicy=WhileOverValidUseArea`: 유효 use-area 위에서는 effect result와 무관하게 감소하고, use-area 밖에서는 감소하지 않는지 확인
4. `DrainPolicy=WhileUseSessionActive`: LMB active use session 동안 use-area 밖/target 없음 상태에서도 durability가 감소하는지 확인
5. `bRemoveItemWhenDepleted=true` 시 0 도달 후 아이템 제거 + session 종료 확인
6. `bRemoveItemWhenDepleted=false` 시 0 유지 + 이후 begin/effect 차단 확인
7. 벌솔은 동일 조건에서도 durability drain이 발생하지 않는지 확인

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
  - 남은 검증 공백(Content 수동 전환/PIE, Editor DLL lock 등)만 간단히 기재
