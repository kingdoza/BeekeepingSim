# 리뷰 프롬프트: 밀도 작업대 + 소비장 capping mask 밀도질 구현

## 리뷰 목적

이번 리뷰는 밀도 작업대와 소비장 밀도질 구현이 기존 Focus/Inventory/WorldActors 경계를 지키면서 동작하는지 검증한다.

핵심:
- `AUncappingTable`은 C++ native WorldActor이며 기존 anchored cursor FocusEngaged 경로를 사용한다.
- 작업대 comb slot은 `AUncappingTableCombSlot` 전용 subclass다.
- 작업대 PartFocus action은 `UCombUncappingPartFocusActionComponent`이며 `UBeehiveCombPartFocusActionComponent`를 직접 재사용하지 않는다.
- 밀도질 action은 `UCombUncappingUseAction`이며 직접 cursor trace를 하지 않는다.
- `ABeehiveCombActor`가 face별 capping mask byte buffer와 transient texture를 소유한다.
- `FBeehiveCombItemState`가 capping mask를 저장/복원한다.
- 이번 범위에서 채밀/수확, 꿀 아이템 생산, 밀도 도구 내구도 감소는 구현하지 않는다.

제외:
- `Content/` asset 직접 수정/저장
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect 추가
- 벌통 소비장 lift/shake 정책 변경
- `ABeehive` honey production/ripeness bucket 변경

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

참고:
- `.md/QNA_IMPLEMENTATION.md`의 `FocusPrompt Asset 반영` 항목은 이번 구현 범위와 직접 관련 없다.

## 리뷰 범위 파일

Source:
- `Source/BeekeepingSim/Public/WorldActors/UncappingTable.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTable.cpp`
- `Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp`
- `Source/BeekeepingSim/Public/WorldActors/CombUncappingPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/CombUncappingPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Inventory/CombUncappingUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/CombUncappingUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemActionContext.h`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

Config:
- `Config/DefaultGameplayTags.ini`

문서:
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

## 핵심 검증 질문

1. `AUncappingTable`이 `UFocusTargetComponent` + `UAnchoredFocusCursorActionComponent`를 사용하고 별도 input binding을 추가하지 않았는가?
2. FocusEngaged 진입 시 hotbar 선택 clear는 기존 `UAnchoredFocusCursorActionComponent` 정책을 재사용하는가?
3. `AUncappingTableCombSlot`은 `ABeehiveCombActor`만 accept하고 `ABeehiveCombSlotActor`를 상속하지 않는가?
4. empty slot 배치는 `UItemPlacementUseAction` + item-use-area LMB 경로를 재사용하는가?
   - 소비장 placement action query가 비어 있지 않다면 `Item.UseArea.UncappingTable` empty slot tag를 허용해야 한다.
5. occupied comb 회수는 PartFocus secondary에서 comb의 `PlacementRetrieveAction`을 bridge하고, acquire 성공 후 `WriteStateToItemInstance`를 호출하는가?
6. `UCombUncappingPartFocusActionComponent`가 `UBeehiveCombPartFocusActionComponent`를 상속하거나 직접 재사용하지 않는가?
7. 작업대 flip은 horizontal drag threshold/dominance 기반이며 vertical shake/bee reduction/lid required tag를 포함하지 않는가?
8. `FItemActionContext`의 hit fields가 `BeginUse`, `TickUse`, `ApplyUseEffect`, `EndUse` context에 가능한 한 현재 hover hit로 채워지는가?
9. `UCursorItemUseAreaScopeComponent`가 hover trace hit를 cache하고, action이 직접 mouse deproject/line trace를 하지 않는가?
10. `FBeehiveCombItemState`가 `HoneyAmount`, `HoneyRipeness`, visible face와 capping mask를 함께 보존하는가?
11. 기존 `SetBeehiveCombState(...)`와 `SetBeehiveCombStateWithRipeness(...)`가 삭제/rename 없이 유지되는가?
12. 저장 mask가 없거나 dimension mismatch이면 `ABeehiveCombActor::ApplyStateFromItemInstance`가 full mask fallback을 수행하는가?
13. `ABeehiveCombActor::WriteStateToItemInstance`가 capping mask를 저장하는가?
14. capping mask source of truth가 face별 `TArray<uint8>`이고 transient `UTexture2D`는 runtime visual 전용인가?
15. `WaxCappingMask` texture parameter가 capping material dynamic instance에 주입되는가?
16. capping plane 표시 조건이 face별 `IsHoneyFull() && !IsWaxCappingFaceComplete(Face)`인가?
17. full honey가 아니어도 mask를 reset하지 않는가?
18. capping use-area active 조건이 host `AUncappingTable`, `IsHoneyFull()`, 현재 visible face, remaining mask ratio threshold를 모두 확인하는가?
19. `UCombUncappingUseAction`의 query tag가 `Item.UseArea.UncappingTable.Comb`인가?
20. `UCombUncappingUseAction`이 `MinStampInterval`과 `MinStampDistanceCm`를 둘 다 만족할 때만 stamp를 찍는가?
21. 실제 mask pixel 변경이 있을 때만 `bSucceeded=true`이고 no-op stamp는 false인가?
22. `UCombUncappingUseAction`이 `DurabilityDelta`를 만들지 않는가?
23. 채밀/수확/꿀 아이템 생산 코드가 추가되지 않았는가?
24. `Config/DefaultGameplayTags.ini`에 `Item.UseArea.UncappingTable`과 `Item.UseArea.UncappingTable.Comb`가 중복 없이 추가되었는가?
25. Core Redirect가 추가되지 않았고 rename이 없는가?

## 검색 검증

```powershell
rg "AUncappingTable|AUncappingTableCombSlot|UCombUncappingPartFocusActionComponent" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "UCombUncappingUseAction|Item.UseArea.UncappingTable.Comb" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory Config/DefaultGameplayTags.ini
rg "bHasItemUseAreaHit|ItemUseAreaImpactPoint|ItemUseAreaImpactNormal|HoveredItemUseAreaHit" Source/BeekeepingSim/Public/Inventory/ItemActionContext.h Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp
rg "WaxCappingMask|FrontWaxCappingMask|BackWaxCappingMask|ApplyWaxCappingBrush|UncappedThreshold" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp Source/BeekeepingSim/Public/Inventory/ItemInstance.h Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp
rg "UBeehiveCombPartFocusActionComponent" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

확인할 것:
- 작업대 전용 PartFocus action이 기존 벌통 action을 직접 재사용하지 않는다.
- `UCombUncappingUseAction`은 직접 line trace를 하지 않는다.
- `UCombUncappingUseAction`은 durability delta를 만들지 않는다.
- `FBeehiveCombItemState`에 capping mask가 저장된다.
- `ApplyStateFromItemInstance`가 mask 복원 또는 full fallback을 수행한다.
- `WriteStateToItemInstance`가 mask를 저장한다.

## 빌드 검증

권장:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

구현 시점 빌드 결과:
- `BeekeepingSimEditor Win64 Development`: 성공 (`Result: Succeeded`)

## 수동 검증 포인트

Editor/PIE에서 확인:

1. 밀도 작업대에 FocusConfirm으로 진입된다.
2. FocusEngaged 진입 시 hotbar 선택이 비워진다.
3. 소비장을 선택한 뒤 empty 작업대 slot에 LMB로 배치된다.
4. 배치된 소비장은 PartFocus secondary로 회수된다.
5. 회수 후 hotbar item instance에 honey amount/ripeness/visible face/capping mask가 보존된다.
6. 재배치하면 capping mask가 복원된다.
7. 밀도 도구를 선택하면 현재 visible face capping use-area만 표시된다.
8. LMB hold로 커서 중심 원형 영역의 밀랍만 제거된다.
9. 지워진 부분 아래의 honey plane이 보인다.
10. 이미 지워진 영역을 문질러도 추가 변화가 없다.
11. horizontal drag flip으로 visible face가 바뀐다.
12. full honey가 아닌 소비장은 밀도질 use-area가 active가 아니다.
13. 한 face가 threshold 이하로 제거되면 해당 face capping plane이 숨겨지고 use-area가 inactive된다.
14. 양면 완료 여부는 `IsWaxCappingComplete()`로 확인된다.

## 리뷰 결과 출력 형식

`.md/AGENT_REVIEW.md`의 출력 형식을 따른다.

특히:
- Findings를 우선 제시하고 `High -> Medium -> Low` 순서로 정렬
- 각 Finding에 파일/라인, 원인, 영향, 수정 제안 포함
- 이슈가 없으면 `No blocking issues found.`를 명시
- Blueprint/API 영향과 Core Redirect 불필요 여부를 별도 확인
- Content 수동 작업(`WaxCappingMask` material 연결, 밀도 도구 DataAsset, 작업대 BP authoring)을 명시
