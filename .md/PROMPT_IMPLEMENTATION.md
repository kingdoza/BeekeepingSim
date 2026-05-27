# ItemUseAreaMeshComponent 통합 등록 구조 구현 프롬프트

## 전제

이번 작업은 기존 ItemUseArea 등록 방식을 `UItemUseAreaMeshComponent` 기반으로 통합한다.

확정 방향:

- 실질 ItemUseArea의 `HitComponent`, `VisualComponents`, material 표시 설정은 `UItemUseAreaMeshComponent`가 담당한다.
- host actor는 `GetItemUseAreaDescriptors`를 직접 override해서 descriptor를 수동 생성하지 않는다.
- host actor에 부착된 담당 component가 host 자신 및 직접 child actor 내부의 `UItemUseAreaMeshComponent`를 수집해 descriptor를 만든다.
- child actor가 `UItemUseAreaMeshComponent`를 가지고 있으면, 상위 host의 담당 component가 child actor를 순회해서 수집한다.
- 소비장 BeeBrush use area와 placement slot use area는 이 generic 구조의 사례다.
- 기존 actor-level `IItemUseAreaProvider`/Blueprint override/`ComponentTags=ItemUseArea` 등록 방식은 새 구조로 대체한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

## 설계 결정 반영

`.md/QNA_ARCHITECTURE.md`의 `ItemUseAreaMeshComponent 통합 설계 QnA` 답변을 따른다.

- `IItemUseAreaProvider` 등록 경로는 `UItemUseAreaMeshProviderComponent` 경로로 대체한다.
- host 담당 component 이름은 `UItemUseAreaMeshProviderComponent`다.
- child actor 수집은 host의 직접 `UChildActorComponent`까지만 한다.
- child actor 필터는 `RequiredChildActorComponentTag`만 제공한다. 태그가 비어 있으면 모든 child actor를 허용한다.
- inactive use area도 descriptor로 등록하되 effective `AreaTags`를 비운다.
- active 조건 확장은 child actor가 `IItemUseAreaActivationProvider`를 구현하는 방식으로 처리한다.
- `EffectTargetObject`는 `UItemUseAreaMeshComponent`의 `EffectTargetPolicy`로 결정한다.
- `bItemUseAreaEnabled` 기본값은 true다.
- `AItemPlacementSlotActor`도 새 mesh component 방식으로 전환하고, occupied 조건은 slot actor가 active provider로 판단한다.
- Blueprint/API migration 리스크가 있으면 기존 class/API는 deprecated 단계로 두되, runtime 수집 경로에서는 사용하지 않는다.

## 목표

1. `UItemUseAreaMeshComponent`를 추가한다.
2. `IItemUseAreaActivationProvider`를 추가한다.
3. `UItemUseAreaMeshProviderComponent`를 추가한다.
4. `UCursorItemUseAreaScopeComponent`의 descriptor 수집 경로를 새 provider component 중심으로 바꾼다.
5. `ABeehive`의 actor-level ItemUseArea provider override 경로를 제거하거나 runtime 미사용 상태로 만든다.
6. `ABeehiveCombActor`의 BeeBrush use area를 `UItemUseAreaMeshComponent`로 전환한다.
7. `AItemPlacementSlotActor`의 slot use area를 `UItemUseAreaMeshComponent`로 전환하고 occupied active 조건을 구현한다.
8. 기존 `UChildItemUseAreaProviderComponent`/`IItemUseAreaProvider`/direct component tag fallback 사용을 제거 또는 deprecated 처리한다.
9. 문서와 Unreal Editor 수동 설정 문서를 갱신한다.

## 수정 대상

새 파일:

- `Source/BeekeepingSim/Public/Focus/ItemUseAreaMeshComponent.h`
- `Source/BeekeepingSim/Private/Focus/ItemUseAreaMeshComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaMeshProviderComponent.h`
- `Source/BeekeepingSim/Private/Focus/ItemUseAreaMeshProviderComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaActivationProvider.h`

주요 수정:

- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`

정리/검토 대상:

- `Source/BeekeepingSim/Public/Focus/ItemUseAreaProvider.h`
- `Source/BeekeepingSim/Public/Focus/ChildItemUseAreaProviderComponent.h`
- `Source/BeekeepingSim/Private/Focus/ChildItemUseAreaProviderComponent.cpp`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/USER_UNREAL.md`
- `.md/0_ARCHITECTURE.md`

## 신규 타입 1: UItemUseAreaMeshComponent

위치:

- `Public/Focus/ItemUseAreaMeshComponent.h`
- `Private/Focus/ItemUseAreaMeshComponent.cpp`

class:

```cpp
UENUM(BlueprintType)
enum class EItemUseAreaEffectTargetPolicy : uint8
{
	ComponentOwner,
	HostActor,
	ExplicitObject
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UItemUseAreaMeshComponent : public UStaticMeshComponent
```

필수 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
bool bItemUseAreaEnabled = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
FName AreaId;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
FGameplayTagContainer AreaTags;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
FItemUseAreaVisualSettings VisualSettings;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
EItemUseAreaEffectTargetPolicy EffectTargetPolicy = EItemUseAreaEffectTargetPolicy::ComponentOwner;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area", meta = (EditCondition = "EffectTargetPolicy == EItemUseAreaEffectTargetPolicy::ExplicitObject"))
TObjectPtr<UObject> ExplicitEffectTargetObject = nullptr;
```

필수 API:

```cpp
UFUNCTION(BlueprintPure, Category = "Item Use Area")
bool IsItemUseAreaEnabled() const;

UFUNCTION(BlueprintPure, Category = "Item Use Area")
FName GetResolvedAreaId() const;

UFUNCTION(BlueprintPure, Category = "Item Use Area")
const FGameplayTagContainer& GetAreaTags() const;

UFUNCTION(BlueprintPure, Category = "Item Use Area")
const FItemUseAreaVisualSettings& GetVisualSettings() const;

UFUNCTION(BlueprintPure, Category = "Item Use Area")
UObject* ResolveEffectTargetObject(AActor* HostActor) const;
```

정책:

- `GetResolvedAreaId()`는 `AreaId`가 비어 있으면 component `GetFName()`을 반환한다.
- `ResolveEffectTargetObject`:
  - `ComponentOwner`: `GetOwner()`
  - `HostActor`: 전달받은 host actor
  - `ExplicitObject`: `ExplicitEffectTargetObject`
- explicit target이 null이면 fallback은 `GetOwner()`로 둔다.
- constructor에서 item-use-area 기본 collision은 기존 scope가 active 상태에서 제어하므로 과도하게 hard-code하지 않는다. 단, 새 component 기본 collision 정책이 필요하면 `NoCollision` 또는 `QueryOnly + Visibility Ignore` 중 기존 authoring과 충돌하지 않는 쪽을 선택한다.

## 신규 타입 2: IItemUseAreaActivationProvider

위치:

- `Public/Focus/ItemUseAreaActivationProvider.h`

interface:

```cpp
UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UItemUseAreaActivationProvider : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API IItemUseAreaActivationProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Use Area")
	bool IsItemUseAreaMeshActive(UItemUseAreaMeshComponent* Component, AActor* HostActor) const;
};
```

정책:

- 구현하지 않은 actor는 active true로 간주한다.
- 구현 actor가 false를 반환하면 descriptor는 등록하되 `AreaTags`를 비운다.
- 이 interface는 item action 실행을 하지 않는다. active 조건 판단만 담당한다.

## 신규 타입 3: UItemUseAreaMeshProviderComponent

위치:

- `Public/Focus/ItemUseAreaMeshProviderComponent.h`
- `Private/Focus/ItemUseAreaMeshProviderComponent.cpp`

class:

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UItemUseAreaMeshProviderComponent : public UActorComponent
```

필수 property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
FName RequiredChildActorComponentTag = NAME_None;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
bool bIncludeOwnerComponents = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
bool bIncludeDirectChildActors = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
bool bLogCollectionDebug = false;
```

필수 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Item Use Area")
void BuildItemUseAreaDescriptors(TArray<FItemUseAreaDescriptor>& OutDescriptors) const;
```

수집 규칙:

1. owner actor 자신의 `UItemUseAreaMeshComponent`들을 수집한다.
2. owner actor의 직접 `UChildActorComponent`들을 순회한다.
3. `RequiredChildActorComponentTag`가 비어 있지 않으면 해당 component tag가 있는 child actor component만 허용한다.
4. child actor가 null이면 skip한다.
5. child actor 내부의 `UItemUseAreaMeshComponent`들을 수집한다.
6. recursive child actor 순회는 하지 않는다.

descriptor 생성 규칙:

```cpp
Descriptor.AreaId = Component->GetResolvedAreaId();
Descriptor.OwnerActor = Component->GetOwner();
Descriptor.HitComponent = Component;
Descriptor.VisualComponents.Add(Component);
Descriptor.VisualSettings = Component->GetVisualSettings();
Descriptor.EffectTargetObject = Component->ResolveEffectTargetObject(HostActor);
Descriptor.AreaTags = bActive ? Component->GetAreaTags() : FGameplayTagContainer();
```

active 판단:

```cpp
bool bActive = true;
if (ComponentOwner implements IItemUseAreaActivationProvider)
{
	bActive = IItemUseAreaActivationProvider::Execute_IsItemUseAreaMeshActive(ComponentOwner, Component, HostActor);
}
```

등록 skip 조건:

- component null
- `bItemUseAreaEnabled == false`
- `AreaId` resolve 실패는 component name fallback으로 처리하므로 skip하지 않는다.
- static mesh가 없어도 등록할지 여부는 결정 필요하면 `.md/QNA_IMPLEMENTATION.md`에 질문한다. 권장은 static mesh가 없어도 descriptor 생성은 허용하되 visual/hit이 빈 mesh라 실제 렌더만 없게 둔다.

inactive 정책:

- active false인 component도 descriptor를 등록한다.
- 단 `Descriptor.AreaTags`는 비운다.
- 이로써 scope가 material/collision을 꺼줄 수 있다.

## UCursorItemUseAreaScopeComponent 변경 요구

현재 제거/대체 대상:

- `RebuildDescriptorsFromProviderActor`
- `RebuildDescriptorsFromProviderComponents`
- `RebuildDescriptorsFromDirectComponentTags`
- `IItemUseAreaProvider::Execute_GetItemUseAreaDescriptors` 호출
- `ComponentTags=ItemUseArea` fallback
- 임시 디버그 로그(`ItemUseArea ProviderActor=...`)가 있으면 제거한다.

새 흐름:

```cpp
void UCursorItemUseAreaScopeComponent::RebuildItemUseAreaDescriptors()
{
	ClearAllVisualState();
	RestoreOriginalCollisionStates();
	DynamicMaterials.Reset();
	RegisteredDescriptors.Reset();
	ActiveDescriptorIndices.Reset();
	HoveredDescriptorIndex = INDEX_NONE;

	AActor* HostActor = ResolveActiveHostActor();
	ActiveHostActor = HostActor;
	if (!HostActor)
	{
		return;
	}

	RebuildDescriptorsFromItemUseAreaMeshProviders(HostActor);
	RefreshActiveUseAreas();
}
```

새 private helper:

```cpp
void RebuildDescriptorsFromItemUseAreaMeshProviders(AActor* HostActor);
```

구현:

- `HostActor->GetComponents<UItemUseAreaMeshProviderComponent>(Providers)` 사용
- provider가 없으면 descriptor 없음
- 각 provider의 `BuildItemUseAreaDescriptors` 결과를 `RegisterItemUseAreaDescriptor`로 등록

주의:

- `UCursorItemUseAreaScopeComponent`는 descriptor 생성자가 아니다.
- scope는 계속 선택 아이템 query 매칭, hover trace, visual material, collision, hold-use lifecycle만 담당한다.
- active descriptor 판정은 기존 `DoesDescriptorMatchActionQuery` 구조를 유지한다.

## ABeehive 변경 요구

`ABeehive`는 더 이상 `IItemUseAreaProvider`를 구현하지 않는다.

제거/정리:

- `#include "Focus/ItemUseAreaProvider.h"` 제거
- class inheritance에서 `public IItemUseAreaProvider` 제거
- `GetItemUseAreaDescriptors_Implementation(...)` 제거
- 기존 lid/comb/BeeBrush descriptor 수동 생성 로직 제거
- `UChildItemUseAreaProviderComponent* ChildItemUseAreaProvider` 제거 또는 deprecated unused 처리

추가:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UItemUseAreaMeshProviderComponent> ItemUseAreaMeshProvider;
```

constructor:

```cpp
ItemUseAreaMeshProvider = CreateDefaultSubobject<UItemUseAreaMeshProviderComponent>(TEXT("ItemUseAreaMeshProvider"));
```

설정:

- `bIncludeOwnerComponents = true`
- `bIncludeDirectChildActors = true`
- `RequiredChildActorComponentTag`는 기본 `NAME_None`
- 벌통 child actor가 많아 필터가 필요하면 BP에서 `RequiredChildActorComponentTag`를 지정하도록 `.md/USER_UNREAL.md`에 문서화한다.

`RebuildItemUseAreaDescriptorsIfAvailable()`는 유지한다.

- 소비장 lift/return/abort 후 rebuild 호출은 계속 필요하다.
- placement slot place/clear 후 host scope rebuild 경로도 유지한다.

## ABeehiveCombActor 변경 요구

`BeeBrushUseAreaMesh`를 `UItemUseAreaMeshComponent`로 전환한다.

변경:

```cpp
TObjectPtr<UItemUseAreaMeshComponent> BeeBrushUseAreaMesh;
```

getter:

```cpp
UItemUseAreaMeshComponent* GetBeeBrushUseAreaMesh() const;
```

기존 `BeeBrushUseAreaId`, `BeeBrushUseAreaTags`, `BeeBrushUseAreaVisualSettings` property/getter는 제거한다.

- 해당 값은 이제 `BeeBrushUseAreaMesh` component의 Details에서 설정한다.

`ABeehiveCombActor`는 `IItemUseAreaActivationProvider`를 구현한다.

```cpp
virtual bool IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const override;
```

정책:

- `Component != BeeBrushUseAreaMesh`이면 true 반환한다.
- `HostActor`가 `ABeehive`이고, `HostActor`의 lifted comb가 `this`이면 true.
- 그 외 false.

필요한 Beehive API:

```cpp
UFUNCTION(BlueprintPure, Category = "Beehive|Comb")
ABeehiveCombActor* GetLiftedCombActor() const;
```

구현:

- `CombLiftComponent->GetLiftedCombSlotIndex()`
- `GetCombSlotComponentByIndex(Index)->GetChildActor()`
- `ABeehiveCombActor` cast

BeeBrush component 기본 설정:

- `EffectTargetPolicy = ComponentOwner`
- `AreaTags` 기본값은 C++에서 gameplay tag를 강제 주입하지 않는다.
- `BP_BeehiveComb`에서 `AreaTags = Item.UseArea.Beehive.BeeBrush`를 설정한다.
- `VisualSettings`는 component Details에서 조정한다.

## AItemPlacementSlotActor 변경 요구

slot actor도 `UItemUseAreaMeshComponent` 기반으로 전환한다.

변경:

- `SlotMeshComponent` 타입을 `UStaticMeshComponent`에서 `UItemUseAreaMeshComponent`로 변경한다.
- `AItemPlacementSlotActor`의 `IItemUseAreaProvider` inheritance 제거
- `GetItemUseAreaDescriptors_Implementation` 제거
- `AreaId`, `AreaTags` property는 slot actor에서 제거하거나 deprecated 처리하고, `SlotMeshComponent`의 component Details로 이동한다.
- `SlotMeshAsset`, `SlotMeshMaterial`, `SlotMeshRelativeTransform` authoring helper는 유지 가능하다. 단 적용 대상은 `UItemUseAreaMeshComponent`다.

`AItemPlacementSlotActor`는 `IItemUseAreaActivationProvider`를 구현한다.

```cpp
virtual bool IsItemUseAreaMeshActive_Implementation(UItemUseAreaMeshComponent* Component, AActor* HostActor) const override;
```

정책:

- `Component != SlotMeshComponent`이면 true
- `SlotMeshComponent`는 `!IsPlacementOccupied()`일 때 active true
- occupied면 active false

결과:

- empty slot: descriptor 등록 + AreaTags 유지 + placement action과 매칭
- occupied slot: descriptor 등록 + AreaTags 비움 + material/collision은 scope가 비활성 처리

place/clear 후:

- `RequestHostItemUseAreaRebuild()` 유지
- host scope rebuild로 active state 갱신

`EffectTargetPolicy`:

- placement slot의 `SlotMeshComponent`는 `ComponentOwner`로 설정한다.
- placement action은 `Context.ItemUseEffectTargetObject`를 `IItemPlacementSlot`로 해석한다.

## 기존 provider/API 제거 정책

완전 전환 대상:

- `IItemUseAreaProvider`
- `UItemUseAreaProvider`
- `UChildItemUseAreaProviderComponent`
- actor-level `GetItemUseAreaDescriptors_Implementation`
- BP `Get Item Use Area Descriptors` override workflow
- direct `Component Tags = ItemUseArea` fallback
- child actor `ItemUseAreaChild` interface-provider workflow

단, 구현 중 Blueprint 참조/compile 위험이 확인되면:

- 파일/class는 즉시 삭제하지 말고 deprecated 주석/문서만 남긴다.
- `UCursorItemUseAreaScopeComponent` runtime 경로에서는 더 이상 호출하지 않는다.
- `.md/QNA_IMPLEMENTATION.md`에 삭제 시점 질문을 남기고 중단하지 말고 가능한 범위에서 새 경로 전환을 완료한다.

## EffectTargetObject 구현 설명

`EffectTargetObject`는 item action이 실제 domain mutation을 수행할 대상이다.

경로:

```text
UItemUseAreaMeshComponent
-> UItemUseAreaMeshProviderComponent가 descriptor 생성
-> Descriptor.EffectTargetObject 설정
-> UCursorItemUseAreaScopeComponent::BuildItemActionContext
-> Context.ItemUseEffectTargetObject
-> UHoldItemUseAction::ApplyUseEffect
```

사례:

- BeeBrush 소비장: `EffectTargetPolicy = ComponentOwner` -> `ABeehiveCombActor`
- 소독약 벌통 영역: `EffectTargetPolicy = HostActor` -> `ABeehive`
- placement slot: `EffectTargetPolicy = ComponentOwner` -> `AItemPlacementSlotActor`
- 특수 대상: `ExplicitObject`

## slot actor active 담당자 설명

배치 상태는 `AItemPlacementSlotActor`가 소유한다.

따라서 slot use area active 가능 여부도 slot actor가 `IItemUseAreaActivationProvider`로 판단한다.

```text
empty slot
-> active true
-> AreaTags 유지

occupied slot
-> active false
-> AreaTags 비움
-> descriptor는 유지되어 scope가 material/collision을 끈다
```

provider component는 slot 상태를 직접 해석하지 않는다.

## Editor 작업 문서화

`.md/USER_UNREAL.md`에 추가/수정:

1. `BP_Beehive`:
   - `ItemUseAreaMeshProvider`가 있는지 확인
   - 필요 시 `RequiredChildActorComponentTag` 설정
2. `BP_BeehiveComb`:
   - `BeeBrushUseAreaMesh`가 `UItemUseAreaMeshComponent` 타입인지 확인
   - mesh/material/transform 설정
   - `AreaTags = Item.UseArea.Beehive.BeeBrush`
   - `EffectTargetPolicy = ComponentOwner`
3. placement slot BP:
   - `SlotMeshComponent`가 `UItemUseAreaMeshComponent` 타입인지 확인
   - 기존 slot actor의 `AreaId/AreaTags`가 component Details로 이동했음을 반영
   - `EffectTargetPolicy = ComponentOwner`
4. 더 이상 `Get Item Use Area Descriptors` BP override를 사용하지 않는다.
5. 더 이상 `Component Tags = ItemUseArea` fallback을 사용하지 않는다.

## 문서 갱신

구현 후 갱신:

- `.md/Architecture/FocusSystem.md`
  - `UItemUseAreaMeshComponent`
  - `UItemUseAreaMeshProviderComponent`
  - `IItemUseAreaActivationProvider`
  - `UCursorItemUseAreaScopeComponent`의 새 descriptor source
  - 기존 provider/interface/tag fallback 제거 또는 deprecated
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehive`의 `ItemUseAreaMeshProvider`
  - `ABeehiveCombActor`의 BeeBrush use area component
  - `AItemPlacementSlotActor`의 mesh component 방식 전환
- `.md/Architecture/InventorySystem.md`
  - `EffectTargetObject`가 component policy로 결정되고 item action이 context target을 cast해 효과를 적용하는 정책
- `.md/0_ARCHITECTURE.md`
  - FocusEngaged ItemUseArea 등록 흐름 요약 업데이트
- `.md/USER_UNREAL.md`
  - BP migration 절차

## 검증 기준

### 코드 검색

있어야 함:

- `UItemUseAreaMeshComponent`
- `EItemUseAreaEffectTargetPolicy`
- `UItemUseAreaMeshProviderComponent`
- `IItemUseAreaActivationProvider`
- `BuildItemUseAreaDescriptors`
- `IsItemUseAreaMeshActive`
- `RebuildDescriptorsFromItemUseAreaMeshProviders`
- `GetLiftedCombActor`

runtime 경로에서 없어야 함:

- `IItemUseAreaProvider::Execute_GetItemUseAreaDescriptors`
- `RebuildDescriptorsFromProviderActor`
- `RebuildDescriptorsFromProviderComponents`
- `RebuildDescriptorsFromDirectComponentTags`
- `ComponentHasTag(TEXT("ItemUseArea"))`

source 전체 삭제가 가능하면 없어야 함:

- `UChildItemUseAreaProviderComponent`
- `IItemUseAreaProvider`

단, BP migration risk로 deprecated 유지한 경우 최종 보고에 남긴다.

### 동작 확인

1. 벌통 FocusEngaged 진입
   - `UCursorItemUseAreaScopeComponent`가 `UItemUseAreaMeshProviderComponent`에서 descriptor를 수집한다.
2. BeeBrush 미선택
   - 소비장 BeeBrush use area mesh는 scope 관리 대상이지만 보이지 않는다.
3. BeeBrush 선택 + 소비장 미-lift
   - BeeBrush area는 active 조건 false라 AreaTags empty
   - 표시/hover/use가 안 된다.
4. BeeBrush 선택 + 소비장 lift
   - lifted 소비장만 BeeBrush area active
   - hover highlighting 표시
   - LMB hold 시 `ABeehiveCombActor::TargetBeeCount` 감소
5. 소비장 return/abort
   - 모든 BeeBrush area inactive
   - material opacity 0
6. placement slot empty
   - slot use area active
   - placement item과 tag query 매칭
7. placement slot occupied
   - descriptor는 유지되지만 AreaTags empty
   - slot use area 표시/hover/use 비활성
8. 뚜껑 PartFocus
   - ItemUseArea collision 관리가 뚜껑 PartFocus hit component를 임의로 ignore 처리하지 않아야 한다.
   - 뚜껑을 ItemUseArea로 쓰려면 별도 `UItemUseAreaMeshComponent`를 사용한다.

### 빌드

가능하면 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 기존 Blueprint serialized component type 변경 때문에 `BeeBrushUseAreaMesh` 또는 `SlotMeshComponent` migration이 자동으로 불가능한 경우
- `IItemUseAreaProvider` 삭제가 Blueprint compile/save 이전에 대량 asset break를 유발하는 경우
- `UItemUseAreaMeshComponent`를 `UStaticMeshComponent` subclass로 만들 수 없는 엔진/모듈 제약이 있는 경우
- `EffectTargetPolicy=ExplicitObject`가 UObject property로 안전하게 저장되지 않는 경우
- inactive descriptor를 등록하되 AreaTags를 비우는 방식이 기존 item-use action query 정책과 충돌하는 경우
