# 코드 개선 요청 프롬프트

아래 리뷰 결과를 기준으로 Unreal C++ 코드와 설계 문서를 수정하라.

## 대상 파일

- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- `.md/0_ARCHITECTURE.md`

## 치명적 문제

없음.

## 중요 문제 1: `AllowedItemTags`가 비어 있을 때 아이템 슬롯을 허용해 설계와 충돌함

### 문제
`UBeekeeperHotbarComponent::IsSlotAllowedByActiveRule()`에서 engaged 상태이고 슬롯에 아이템이 있는 경우에도 `AllowedItemTags.IsEmpty()`이면 `true`를 반환한다.

`.md/0_ARCHITECTURE.md`의 Hotbar 필터링 흐름은 "허용 태그가 비어 있으면 아이템 슬롯은 비허용"이라고 정의한다. 현재 구현은 해당 정책과 반대다.

### 영향
FocusTarget의 item rule이 비어 있는 engaged action에서 모든 hotbar 아이템 슬롯이 enabled로 남는다. 이는 focus 중 허용 아이템만 활성화한다는 기존 설계를 깨고, UI가 사용 불가능해야 할 아이템을 사용 가능 상태로 표시할 수 있다.

### 수정 방향
빈 슬롯은 계속 허용하되, 아이템이 있는 슬롯에서 `AllowedItemTags`가 비어 있으면 `false`를 반환한다. StorageBox의 선택 유지 정책은 이미 `ShouldClearSelectionByActiveFocusPolicy()`로 분리되어 있으므로, 이 수정이 선택 유지 정책과 충돌하지 않아야 한다.

### 수정 코드
파일 경로: `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`

```cpp
bool UBeekeeperHotbarComponent::IsSlotAllowedByActiveRule(int32 Index) const
{
	if (!IsIndexValid(Index))
	{
		return false;
	}

	if (!bIsEngagedFocusActive)
	{
		return true;
	}

	if (!Slots[Index].ItemInstance)
	{
		return true;
	}

	const FGameplayTagContainer& AllowedItemTags = ActiveFocusRule.AllowedItemTags;
	if (AllowedItemTags.IsEmpty())
	{
		return false;
	}

	if (AllItemsRootTag.IsValid() && AllowedItemTags.HasTagExact(AllItemsRootTag))
	{
		return true;
	}

	const FGameplayTagContainer ItemTags = GetItemTagsForSlot(Index);
	return ItemTags.HasAny(AllowedItemTags);
}
```

## 개선 제안 1: `0_ARCHITECTURE.md`의 Storage UI 흐름이 새 중립 라우터 구조와 일부 불일치함

### 문제
최신 설계 결정에는 `UStorageBoxWidget`에서 drop 라우터 역할을 제거했다고 적혀 있지만, `Storage Box Focus UI 흐름` 섹션은 여전히 engaged 동안 UI가 `UStorageBoxWidget` API를 통해 storage/hotbar 이동 및 hotbar 교환을 요청한다고 설명한다.

### 영향
구현 에이전트나 Blueprint 작업자가 새 `UItemSlotDragDropLibrary::HandleItemSlotDrop()` 대신 `UStorageBoxWidget` 중심 라우팅을 계속 사용해야 한다고 오해할 수 있다.

### 수정 방향
Storage UI drag/drop 라우팅 설명을 `UItemSlotDragDropLibrary` 중심으로 바꾸고, `UStorageBoxWidget`은 생성/초기화 및 root 참조 제공 역할로 한정한다고 명시한다.

### 수정 코드
파일 경로: `.md/0_ARCHITECTURE.md`

```md
3. engaged 동안 slot widget 은 `UStorageSlotDragDropOperation` payload 와 target component/index 를 구성해 `UItemSlotDragDropLibrary::HandleItemSlotDrop()` 으로 drop 을 라우팅한다.
   - hotbar -> hotbar: 같은 hotbar component 내부 swap
   - hotbar -> storage: hotbar item 을 target storage 로 이동/교환
   - storage -> hotbar: storage item 을 target hotbar 로 이동/교환
   - storage -> storage: 같은 storage component 내부 swap
   - 서로 다른 hotbar 간 이동, 서로 다른 storage 간 이동은 현재 범위 밖으로 false
4. `UStorageBoxWidget` 은 storage/hotbar component 참조 제공과 UI root 역할만 담당하며, drop 조합 라우팅을 소유하지 않는다.
```

## 검증 항목

- UBT/UHT 빌드가 성공하는지 확인한다.
- `AllowedItemTags`가 비어 있는 engaged focus에서 아이템 슬롯은 disabled, 빈 슬롯은 enabled로 남는지 확인한다.
- StorageBox action의 `ShouldClearHotbarSelectionOnFocusEngaged() == false` 정책이 위 필터 변경 후에도 선택 인덱스를 지우지 않는지 확인한다.
- Hotbar -> Hotbar drop은 같은 hotbar component에서만 `SwapSlots()`로 처리되는지 확인한다.
- Storage -> Storage drop은 같은 storage component에서만 `SwapStorageSlots()`로 처리되는지 확인한다.
- Hotbar -> Storage, Storage -> Hotbar drop에서 `OnHotbarChanged`와 `OnStorageChanged`가 기존 component API 경로로 정상 broadcast되는지 확인한다.
- Blueprint slot widget 이 `UStorageBoxWidget` 참조 없이 operation payload와 target component를 넘겨 `UItemSlotDragDropLibrary::HandleItemSlotDrop()`을 호출할 수 있는지 확인한다.

## 0_ARCHITECTURE.md 반영 필요 여부

필요. 코드 구조 설명 일부는 새 라우터를 반영했지만, 실행 흐름 섹션에 `UStorageBoxWidget` 중심 라우팅 설명이 남아 있어 최신 설계와 일치하도록 수정해야 한다.
