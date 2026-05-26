# Child ItemUseArea Provider Component 구현 프롬프트

## 목표

여러 FocusEngaged host actor가 owner의 child actor slot을 `ItemUseArea`로 노출할 수 있는 generic provider component를 구현한다.

확정 방향:

- 신규 컴포넌트 이름은 `UChildItemUseAreaProviderComponent`
- 실제 ItemUseArea 등록 주체는 계속 `UCursorItemUseAreaScopeComponent`
- `UChildItemUseAreaProviderComponent`는 descriptor를 제공하는 중간 provider
- owner actor의 직접 `UChildActorComponent` 중 `Component Tags` 조건을 만족하는 child actor만 scan
- child actor가 `IItemUseAreaProvider`를 구현하면 `GetItemUseAreaDescriptors` 결과를 그대로 append
- `ABeehive`에는 이 컴포넌트를 native default subobject로 추가
- 기존 `ABeehive` 전용 `PlacementSlotComponents` 방식은 전부 삭제

`BP_ItemPlacementSlotActor`가 ItemUseArea로 들어가는 경로는 `GetItemUseAreaDescriptors`뿐이다. 생성자/OnConstruction/BeginPlay는 mesh/material/transform 준비만 수행하고 등록을 하지 않는다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

## 런타임 흐름

```text
FocusEngaged 진입
→ UCursorItemUseAreaScopeComponent::RebuildItemUseAreaDescriptors()
→ host actor provider 호출
→ host actor의 provider component 호출
→ UChildItemUseAreaProviderComponent::GetItemUseAreaDescriptors()
→ owner의 직접 UChildActorComponent scan
→ Component Tags에 RequiredChildActorComponentTag가 있는 것만 통과
→ child actor가 IItemUseAreaProvider면 descriptor 요청
→ UCursorItemUseAreaScopeComponent::RegisterItemUseAreaDescriptor()
→ 선택 아이템 action UseAreaTagQuery와 AreaTags 매칭
→ active descriptor만 표시/hover/use 가능
```

## 폐기할 로직

아래는 반드시 제거한다.

- `ABeehive`의 `PlacementSlotComponents` UPROPERTY
- `ABeehive::GetItemUseAreaDescriptors_Implementation()` 안에서 `PlacementSlotComponents`를 순회해 child slot descriptor를 append하는 코드
- `UChildActorItemUseAreaProviderComponent`
- `UItemUseAreaProviderChildActorComponent`
- `FComponentReference` 기반 `ProviderChildActorComponents` 설계
- scope 내부 `GetAttachedActors(...)` 자동 순회
- scope 내부 모든 `UChildActorComponent` 자동 순회

유지해야 하는 것:

- `ABeehive`의 lid/comb item-use-area descriptor
- 소독약용 `Item.UseArea.Beehive.Disinfectant` tag
- sanitation state/API
- `AItemPlacementSlotActor`, `IItemPlacementSlot`, `UItemPlacementUseAction`

## 신규 타입

### `UChildItemUseAreaProviderComponent`

권장 위치:

- `Source/BeekeepingSim/Public/Focus/ChildItemUseAreaProviderComponent.h`
- `Source/BeekeepingSim/Private/Focus/ChildItemUseAreaProviderComponent.cpp`

상속/구현:

- `UActorComponent`
- `IItemUseAreaProvider`

UCLASS:

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UChildItemUseAreaProviderComponent
	: public UActorComponent
	, public IItemUseAreaProvider
{
	GENERATED_BODY()
};
```

UPROPERTY:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
FName RequiredChildActorComponentTag = TEXT("ItemUseAreaChild");

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
TSubclassOf<AActor> RequiredChildActorClass = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Use Area")
bool bLogSkippedChildren = false;
```

결정사항:

- `RequiredChildActorComponentTag` 기본값은 `ItemUseAreaChild`
- `RequiredChildActorComponentTag`가 `None`이면 tag 필터 없이 모든 provider child actor 허용
- `RequiredChildActorClass` 기본값은 `nullptr`
- `RequiredChildActorClass`가 null이면 class 필터 없음
- owner actor에 직접 붙은 `UChildActorComponent`만 scan
- attached actor 재귀 scan 없음
- child actor 내부 child actor 재귀 scan 없음
- child provider가 반환한 descriptor는 변조하지 않고 그대로 append
- skip 로그는 기본 off, 켜면 Verbose 수준으로 남김

구현 요구:

```cpp
virtual void GetItemUseAreaDescriptors_Implementation(TArray<FItemUseAreaDescriptor>& OutDescriptors) const override;
```

구현 절차:

1. `AActor* Owner = GetOwner()`
2. owner가 없으면 return
3. `Owner->GetComponents<UChildActorComponent>(ChildActorComponents)`로 직접 component 조회
4. 각 child actor component에 대해:
   - component null이면 skip
   - `RequiredChildActorComponentTag`가 None이 아니고 `ComponentHasTag(RequiredChildActorComponentTag)`가 false면 skip
   - `AActor* ChildActor = ChildActorComponent->GetChildActor()`
   - child actor null이면 skip
   - `RequiredChildActorClass`가 있고 `!ChildActor->IsA(RequiredChildActorClass)`이면 skip
   - child actor가 `UItemUseAreaProvider` interface를 구현하지 않으면 skip
   - `IItemUseAreaProvider::Execute_GetItemUseAreaDescriptors(ChildActor, ChildDescriptors)` 호출
   - `OutDescriptors.Append(MoveTemp(ChildDescriptors))`

주의:

- `Component Tags`는 child actor component의 tag다. child actor의 actor tag가 아니다.
- `BP_ItemPlacementSlotActor`는 `AItemPlacementSlotActor::GetItemUseAreaDescriptors`에서 자기 descriptor를 반환한다.
- 이 컴포넌트는 `RegisterItemUseAreaDescriptor`를 호출하지 않는다.

## Scope 수정

대상:

- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`

`UCursorItemUseAreaScopeComponent::RebuildItemUseAreaDescriptors()` 흐름:

```cpp
RebuildDescriptorsFromProviderActor(HostActor);
RebuildDescriptorsFromProviderComponents(HostActor);
RebuildDescriptorsFromDirectComponentTags(HostActor);
RefreshActiveUseAreas();
```

신규 helper:

```cpp
void RebuildDescriptorsFromProviderComponents(AActor* HostActor);
```

구현 요구:

- `HostActor->GetComponents(...)`로 host actor에 직접 붙은 `UActorComponent`만 확인
- component가 `this`이면 skip
- component class가 `UItemUseAreaProvider` interface를 구현하면 `IItemUseAreaProvider::Execute_GetItemUseAreaDescriptors(Component, ProviderDescriptors)` 호출
- 반환 descriptor는 `RegisterItemUseAreaDescriptor(...)`를 통해 등록
- `GetAttachedActors(...)` 순회 금지
- 모든 `UChildActorComponent` 자동 순회 금지

## Beehive 수정

대상:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

추가:

```cpp
class UChildItemUseAreaProviderComponent;
```

UPROPERTY:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Item Use Area")
TObjectPtr<UChildItemUseAreaProviderComponent> ChildItemUseAreaProvider;
```

생성자:

```cpp
ChildItemUseAreaProvider = CreateDefaultSubobject<UChildItemUseAreaProviderComponent>(TEXT("ChildItemUseAreaProvider"));
```

제거:

- `PlacementSlotComponents` UPROPERTY
- `GetItemUseAreaDescriptors_Implementation()` 안의 placement slot child actor provider 순회

유지:

- lid descriptor
- comb descriptor
- disinfectant tag 부여
- sanitation API/state

## 기존 slot actor/action 유지 기준

`AItemPlacementSlotActor` 유지 요구:

- `AActor`, `IItemUseAreaProvider`, `IItemPlacementSlot`
- `SlotMeshComponent` 하나가 descriptor의 `HitComponent`와 `VisualComponents[0]`를 겸함
- `SlotMeshAsset`, `SlotMeshMaterial`, `SlotMeshRelativeTransform`, `AttachRelativeTransform`, `AttachSocketName`
- occupied 상태에서는 descriptor를 반환하지 않음
- descriptor `EffectTargetObject`는 slot actor 자신

`UItemPlacementUseAction` 유지 요구:

- `Context.ItemUseEffectTargetObject`가 `IItemPlacementSlot`인지 확인
- `TryPlaceItem(...)` 성공 시 `bSucceeded=true`, `bConsumedItem=true`, `StackDelta=-1`
- stack delta 적용 실패 시 scope가 `ClearPlacedItem()` rollback

## BP/Editor 작업 문서화

`.md/USER_UNREAL.md`에 다음 절차를 반영한다.

상위 actor 공통:

- host actor에 `UChildItemUseAreaProviderComponent`가 필요하다.
- slot용 일반 `ChildActorComponent`를 추가한다.
- Child Actor Class를 `BP_ItemPlacementSlotActor`로 지정한다.
- 그 `ChildActorComponent`의 `Component Tags`에 `ItemUseAreaChild`를 추가한다.
- child actor template에서 `AreaId`, `AreaTags`, `SlotMeshAsset`, `SlotMeshMaterial`, `SlotMeshRelativeTransform`, `AttachRelativeTransform`을 설정한다.
- `Actor Tags`가 아니라 `Component Tags`를 사용한다.

벌통 예시:

- `ABeehive`에는 native `ChildItemUseAreaProvider` component가 기본으로 존재한다.
- `BP_Beehive`에서 화분떡 위치마다 일반 `ChildActorComponent`를 추가한다.
- 각 component의 Child Actor Class는 `BP_ItemPlacementSlotActor`
- 각 component의 Component Tags에 `ItemUseAreaChild` 추가
- child actor template의 `AreaTags`는 `Item.UseArea.Beehive.PollenPatty`
- 별도 등록 배열은 없다.

## 문서 갱신

구현 후 갱신 대상:

- `.md/Architecture/FocusSystem.md`
  - scope가 host provider component도 수집한다는 내용
  - `UChildItemUseAreaProviderComponent` 역할
  - child actor 자동 순회는 scope가 하지 않는다는 내용
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehive` 전용 `PlacementSlotComponents` 설명 제거
  - `ABeehive`가 native `ChildItemUseAreaProvider` component를 가진다는 내용
  - `AItemPlacementSlotActor`는 reusable slot actor로 유지
- `.md/QNA_ARCHITECTURE.md`
  - `PlacementSlotComponents`/`FComponentReference`/provider child actor component 답변을 폐기하고 `UChildItemUseAreaProviderComponent` 방식으로 정리
- `.md/USER_UNREAL.md`
  - 에디터 설정 절차를 `Component Tags = ItemUseAreaChild` 기준으로 갱신

## 검증 기준

### 코드 검색

아래가 없어야 한다.

- `ABeehive`의 `PlacementSlotComponents`
- `ABeehive::GetItemUseAreaDescriptors_Implementation()` 내부 placement slot child actor 순회
- `UChildActorItemUseAreaProviderComponent`
- `UItemUseAreaProviderChildActorComponent`
- `ProviderChildActorComponents`
- `FComponentReference` 기반 child slot 등록 설계
- `UCursorItemUseAreaScopeComponent` 내부 `GetAttachedActors(...)` 순회
- `UCursorItemUseAreaScopeComponent` 내부 모든 `UChildActorComponent` 자동 순회

아래가 있어야 한다.

- `UChildItemUseAreaProviderComponent`
- `RequiredChildActorComponentTag`
- 기본 tag 값 `ItemUseAreaChild`
- `RequiredChildActorClass`
- `ABeehive::ChildItemUseAreaProvider`
- `UCursorItemUseAreaScopeComponent::RebuildDescriptorsFromProviderComponents(...)`
- scope rebuild 순서:
  - host actor provider
  - host provider components
  - direct component tag fallback
- `AItemPlacementSlotActor` descriptor의 `HitComponent = SlotMeshComponent`
- `AItemPlacementSlotActor` descriptor의 `VisualComponents.Add(SlotMeshComponent)`

### 빌드

가능하면 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

### PIE 수동 검증

Content 직접 수정은 하지 말고 `.md/USER_UNREAL.md`에 절차를 남긴 뒤, 사용자가 설정 후 확인한다.

확인 항목:

1. `BP_Beehive`의 slot용 `ChildActorComponent`에 `Component Tags = ItemUseAreaChild`를 추가하면 화분떡 선택 시 slot use area가 표시된다.
2. 같은 child actor class라도 component tag가 없는 slot은 표시되지 않는다.
3. 다른 host actor BP에 `UChildItemUseAreaProviderComponent`를 붙이고 같은 tag를 쓰면 동일 방식으로 slot use area를 등록할 수 있다.
4. 화분떡 배치 성공 시 hotbar stack이 1 감소한다.
5. 배치된 slot은 occupied 상태가 되어 descriptor를 반환하지 않고 즉시 표시 대상에서 사라진다.
6. 소독약 lid/comb 사용영역은 기존처럼 유지된다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `IItemUseAreaProvider`가 `UActorComponent`에서 BlueprintNativeEvent 실행 경로로 호출되지 않는 경우
- `ABeehive` Blueprint가 `PlacementSlotComponents`를 이미 저장해 Blueprint compile/save 또는 migration 없이는 제거가 위험한 경우
- `UChildItemUseAreaProviderComponent` 추가가 모듈 의존 방향상 Focus에 둘 수 없는 경우
- stack delta rollback 경로가 현재 scope/context와 맞지 않는 경우
- Content asset 직접 수정 없이는 검증 가능한 BP 설정이 불가능한 경우
