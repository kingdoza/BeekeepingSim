# 구현 수정 프롬프트: ItemUseAreaMeshComponent 통합 리뷰 Findings

## 우선순위

1. Medium: inactive descriptor의 empty `AreaTags`가 empty query hold-use action에서 active로 매칭되는 문제 수정
2. Low: `WorldActorsSystem.md`의 구 actor-level provider 문구 정리

## 발견 문제

### 1. inactive descriptor가 empty query item action에 다시 active로 잡힐 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
  - `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
  - `Source/BeekeepingSim/Private/Focus/ItemUseAreaMeshProviderComponent.cpp`
- 원인:
  - `UItemUseAreaMeshProviderComponent`는 inactive component도 descriptor로 등록하되 `AreaTags`를 빈 컨테이너로 만든다.
  - 하지만 `UCursorItemUseAreaScopeComponent::DoesDescriptorMatchActionQuery()`가 `Query.IsEmpty()`이면 true를 반환한다.
  - 따라서 empty query를 가진 `UHoldItemUseAction`/Blueprint action은 inactive descriptor도 active descriptor로 다시 추가할 수 있다.
- 영향:
  - occupied placement slot 또는 non-lifted comb BeeBrush area가 특정 hold-use action에서 active/hover/effect 대상으로 살아날 수 있다.
  - inactive descriptor를 유지해 visual/collision을 관리한다는 설계 의도와, `AreaTags` empty로 query matching을 막는 정책이 충돌한다.
- 수정 방향:
  - 단기 수정: `DoesDescriptorMatchActionQuery()`에서 `Descriptor.AreaTags.IsEmpty()`이면 false를 반환한다.
  - 단기 수정 예:

```cpp
bool UCursorItemUseAreaScopeComponent::DoesDescriptorMatchActionQuery(const FItemUseAreaDescriptor& Descriptor, const UHoldItemUseAction* HoldAction) const
{
	if (!HoldAction || Descriptor.AreaTags.IsEmpty())
	{
		return false;
	}

	const FGameplayTagQuery Query = HoldAction->GetUseAreaTagQuery();
	return Query.IsEmpty() || Query.Matches(Descriptor.AreaTags);
}
```

  - 장기적으로 empty `AreaTags`를 active sentinel로 쓰지 않으려면 `FItemUseAreaDescriptor`에 explicit active flag를 추가하는 방식을 검토한다. 현재 QnA는 empty tag 정책으로 확정되어 있으므로 단기 수정이 우선이다.

### 2. WorldActors 문서에 구 provider override 표현이 남아 있음

- 대상 파일:
  - `.md/Architecture/WorldActorsSystem.md`
- 원인:
  - 최신 update에는 component 기반 전환이 기록되어 있지만, Design Notes에 "host actor는 자신의 provider 구현 또는 host에 붙은 provider component"라는 구 표현이 남아 있다.
- 영향:
  - 새 구현 기준인 `UItemUseAreaMeshProviderComponent` 단일 runtime source와 문서 문구가 충돌한다.
- 수정 방향:
  - 해당 문구를 `host actor에 붙은 UItemUseAreaMeshProviderComponent가 UItemUseAreaMeshComponent를 수집해 descriptor를 구성한다`는 의미로 정리한다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg -n "DoesDescriptorMatchActionQuery|AreaTags.IsEmpty|BuildItemUseAreaDescriptors" Source/BeekeepingSim`
  - `Select-String -Path .md/Architecture/WorldActorsSystem.md -Pattern "provider 구현|ItemUseAreaMeshProvider"`
- PIE:
  - BeeBrush 선택 전/comb 미-lift 상태에서 BeeBrush area가 hover/effect 대상이 아닌지 확인
  - comb lift 후 lifted comb만 BeeBrush area active인지 확인
  - placement slot occupied 상태에서 slot use-area가 hover/effect 대상으로 잡히지 않는지 확인
  - empty query를 가진 임시 hold-use action 또는 BP action으로 inactive descriptor가 active화되지 않는지 확인

## 문서 반영 필요 여부

- 선택.
- 단기 수정은 QnA 확정 정책을 구현에 맞추는 작업이라 아키텍처 문서 갱신은 필수는 아니다.
