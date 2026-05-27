# 구현 수정 프롬프트: Generic Placement Occupant + Beehive Comb Slot 리뷰 Findings

## 우선순위

1. High: queen 부착 소비장 회수 차단 판정 수정
2. Medium: `APlacedItemActor` Blueprint 계약 migration 보강
3. Medium: 상태 보존 comb item의 stack merge 정책 명시/강제
4. Medium: occupant component 없는 placement spawn 성공 차단
5. Low: `git diff --check` blank line at EOF 정리

## 발견 문제

### 1. queen 부착 소비장 회수 차단 판정이 실패할 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`
- 원인:
  - `ABeehive::UpdateQueenBeeLocation()`은 `QueenBeeChildActor` 컴포넌트를 소비장 attach point에 붙인다.
  - `ABeehive::IsQueenBeeAttachedToComb()`은 child actor의 `GetAttachParentActor()`가 comb actor인지 비교한다.
  - child actor의 root parent는 보통 `QueenBeeChildActor` 컴포넌트이고, 해당 컴포넌트 owner는 beehive라서 comb actor 비교가 false가 될 수 있다.
- 영향:
  - `UBeehiveCombPlacementOccupantComponent`의 queen 미부착 조건이 실제 회수 경로에서 우회된다.
  - queen이 붙은 소비장을 회수할 수 있어 queen attach/lifecycle과 벌통 상태가 깨질 수 있다.
- 수정 방향:
  - `QueenBeeChildActor->GetAttachParent()` 또는 attach parent component의 owner를 기준으로 판정한다.
  - 가능하면 front/back attach point 중 하나와 직접 비교한다.
- 수정 예:

```cpp
bool ABeehive::IsQueenBeeAttachedToComb(const ABeehiveCombActor* CombActor) const
{
	if (!CombActor || !QueenBeeChildActor)
	{
		return false;
	}

	const USceneComponent* AttachParent = QueenBeeChildActor->GetAttachParent();
	return AttachParent && AttachParent->GetOwner() == CombActor;
}
```

### 2. `APlacedItemActor` Blueprint 계약 migration 보강 필요

- 대상 파일:
  - `Source/BeekeepingSim/Public/WorldActors/PlacedItemActor.h`
  - `Source/BeekeepingSim/Private/WorldActors/PlacedItemActor.cpp`
  - `Content/Items/BP/BP_PlacedItem.uasset` 수동 검증
- 원인:
  - 기존 `BlueprintReadOnly` UPROPERTY였던 `ItemDefinition`, `OwningPlacementSlotActor`가 제거되고 getter wrapper만 남았다.
  - native `RetrieveAction` subobject 타입도 `UPlacedItemRetrievePartFocusActionComponent`에서 `UPlacementSlotRetrievePartFocusActionComponent`로 바뀌었다.
  - 현재 Content 검색상 `BP_PlacedItem.uasset`에는 기존 `PlacedItemRetrievePartFocusActionComponent` 문자열이 남아 있다.
- 영향:
  - child Blueprint 그래프가 기존 protected 변수를 직접 읽고 있으면 compile break가 날 수 있다.
  - inherited native component template class 변경은 Editor load/compile/save 검증이 필요하다.
- 수정 방향:
  - 제거된 변수는 한 단계 deprecated UPROPERTY로 유지하거나, Blueprint compile/save 및 참조 scan으로 미사용을 확정한다.
  - `RetrieveAction`은 wrapper class를 기본 subobject로 유지하는 방안도 검토한다. wrapper는 이미 generic base subclass라 기능상 중복이 작다.
  - 최소 검증: `BP_PlacedItem` 및 파생/참조 BP compile/save 후 missing property/component 경고 확인.

### 3. comb 상태 보존이 stack merge에 의존해 잘못 섞일 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
  - `Content/Items/Definitions/DA_HoneyComb.uasset` 수동 검증
- 원인:
  - `TryAcquireItem(ItemDefinition, 1)`은 기존 동일 definition stack에 먼저 merge한다.
  - comb 회수는 반환된 `LastModifiedItemInstance`에 honey/visible face를 기록한다.
  - `UItemInstance`의 comb state는 stack 단위라, `MaxStack > 1`이면 한 stack 안의 여러 comb 상태를 구분할 수 없다.
- 영향:
  - 상태가 있는 comb를 기존 stack에 회수하면 stack 전체가 마지막 회수 comb 상태처럼 취급될 수 있다.
  - 이후 재배치 시 honey/visible face가 다른 comb에 잘못 복원될 수 있다.
- 수정 방향:
  - comb item definition의 `MaxStack=1`을 강제 검증한다.
  - 더 안전한 구조가 필요하면 stateful item acquire는 merge를 금지하고 새 `UItemInstance`로만 추가하는 API를 둔다.
  - PIE 전에 `DA_HoneyComb.MaxStack`이 1인지 확인한다.

### 4. occupant component 없는 placement spawn이 성공할 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
  - `Source/BeekeepingSim/Private/Inventory/ItemPlacementUseAction.cpp`
- 원인:
  - `TryPlaceItem_Implementation()`은 spawn actor에 `UPlacementOccupantComponent`가 없어도 placement 성공을 반환한다.
  - `UItemPlacementUseAction`은 placement 성공 시 item stack을 소비한다.
- 영향:
  - 잘못 설정된 `PlacedActorClass`가 배치되면 item은 소비되지만 return definition/owning slot/retrieve 계약이 없는 점유자가 생긴다.
  - generic occupant/retrieve 구조가 asset misconfiguration에 취약해진다.
- 수정 방향:
  - `CanAcceptOccupantActor()` 또는 spawn 직후 검증에서 `UPlacementOccupantComponent`를 필수로 요구한다.
  - 실패 시 actor destroy + warning log + placement false를 반환한다.
  - 의도적으로 회수 불가능한 occupant를 허용해야 한다면 별도 opt-in flag를 두고 문서화한다.

### 5. `git diff --check` 포맷 이슈

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/PlacedItemRetrievePartFocusActionComponent.cpp`
- 원인:
  - 파일 끝에 불필요한 blank line이 있다.
- 영향:
  - 빌드는 통과하지만 diff check가 실패한다.
- 수정 방향:
  - EOF blank line 제거.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg -n "IsQueenBeeAttachedToComb|QueenBeeChildActor->GetAttachParent|GetAttachParentActor" Source/BeekeepingSim`
  - `rg -a -n "PlacedItemRetrievePartFocusActionComponent|ItemDefinition|OwningPlacementSlotActor" Content/Items/BP/BP_PlacedItem.uasset`
  - `rg -n "TryAcquireItem|LastModifiedItemInstance|SetBeehiveCombState|MaxStack" Source/BeekeepingSim`
- PIE:
  - `TargetBeeCount > 0` 소비장 회수 실패
  - queen 부착 소비장 회수 실패
  - `TargetBeeCount == 0` + queen 미부착 소비장 회수 성공
  - 회수 후 재배치 시 honey/visible face 복원
  - hotbar 공간 부족 시 회수 실패 및 actor/slot 유지
  - `BP_PlacedItem`, `BP_BeehiveComb` compile/save 후 missing property/component 경고 없음

## 문서 반영 필요 여부

- 코드 수정 후 문서 반영은 선택.
- 단, `MaxStack=1`을 comb state 보존의 필수 invariant로 정하면 `.md/Architecture/InventorySystem.md`와 `.md/USER_UNREAL.md`에 "권장"이 아니라 "필수"로 갱신한다.
