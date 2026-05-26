# Generic PartFocus Provider 기반 Placed Item 회수 구현 프롬프트

## 전제

이번 작업은 배치된 아이템을 전체 Focus 대상이 아니라 **FocusEngaged host 내부 PartFocus 대상**으로 다루도록 구조를 정리한다.

확정 요구사항:

- `APlacedItemActor`는 전체 `FocusTarget` 대상이 아니다.
- `APlacedItemActor`는 벌통 같은 host의 `CursorPartFocusScope`에 등록되는 runtime part다.
- `AItemPlacementSlotActor`가 PartFocus provider를 구현한다.
- slot이 비어 있으면 item-use-area descriptor를 제공하고, PartFocus descriptor는 제공하지 않는다.
- slot에 아이템이 배치되어 있으면 item-use-area descriptor는 제공하지 않고, placed item PartFocus descriptor를 제공한다.
- 파츠 Focus 대상이라고 모두 RMB 회수 가능한 것은 아니다.
- RMB 회수는 `APlacedItemActor`에 붙은 전용 PartFocus action component가 선택적으로 처리한다.
- 회수는 item 1개를 hotbar로 되돌리는 작업이다.
- hotbar에 1개를 추가할 수 있으면 회수 성공이다.
- 회수 실패 시 placed actor와 slot 점유 상태는 유지된다.
- `APollenPattyActor` native class rename은 하지 않는다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/CharacterSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 목표

1. PartFocus descriptor provider 인터페이스를 추가한다.
2. PartFocus descriptor 등록을 host actor 직접 구현이 아니라 generic component 경로로 확장한다.
3. `AItemPlacementSlotActor`가 점유 상태에 따라 placed item PartFocus descriptor를 제공하게 한다.
4. `APlacedItemActor`를 global Focus 대상에서 제외하고 PartFocus 대상 actor로 정리한다.
5. PartFocus hover 상태의 RMB 입력을 action handler로 전달한다.
6. placed item 전용 PartFocus action에서 hotbar 회수를 처리한다.

## 새 타입/파일

### Focus

- `Source/BeekeepingSim/Public/Focus/CursorPartFocusProvider.h`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusRegistrationComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusRegistrationComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/ChildCursorPartFocusProviderComponent.h`
- `Source/BeekeepingSim/Private/Focus/ChildCursorPartFocusProviderComponent.cpp`

### WorldActors

- `Source/BeekeepingSim/Public/WorldActors/PlacedItemRetrievePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemRetrievePartFocusActionComponent.cpp`

기존 수정 후보:

- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/PlacedItemActor.h`
- `Source/BeekeepingSim/Private/WorldActors/PlacedItemActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

## 설계 원칙

- `UCursorPartFocusScopeComponent`는 part 목록 저장, hover trace, prompt/outline, 입력 라우팅을 담당한다.
- part descriptor 생성은 provider가 담당한다.
- part descriptor 최종 등록은 `UCursorPartFocusRegistrationComponent`가 담당한다.
- `ABeehive` 같은 host actor는 가능한 한 provider/registration component를 조립하는 역할로 축소한다.
- `AItemPlacementSlotActor`는 slot 상태 owner다.
- `AItemPlacementSlotActor`는 비어 있을 때 item-use-area를 제공하고, 점유 중일 때 placed item PartFocus를 제공한다.
- RMB 회수는 모든 PartFocus action의 공통 동작이 아니라 optional secondary part action이다.
- Inventory mutation은 `UBeekeeperHotbarComponent::TryAcquireItem(ItemDefinition, 1)` 경로를 사용한다.
- `APollenPattyActor` rename, UCLASS rename, Blueprint API 삭제는 하지 않는다.

## 구현 상세

### 1. PartFocus provider 인터페이스

새 인터페이스:

```cpp
UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UCursorPartFocusProvider : public UInterface
{
	GENERATED_BODY()
};

class BEEKEEPINGSIM_API ICursorPartFocusProvider
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Cursor Part Focus")
	void GetCursorPartFocusDescriptors(TArray<FCursorPartFocusPartDescriptor>& OutDescriptors) const;
};
```

파일 위치:

- `Focus/CursorPartFocusProvider.h`

주의:

- `FCursorPartFocusPartDescriptor`를 사용하므로 `CursorPartFocusScopeComponent.h` 또는 descriptor 타입 분리를 검토한다.
- 순환 include가 생기면 descriptor struct를 별도 `CursorPartFocusDescriptorTypes.h`로 분리할지 판단한다.
- 분리가 필요하면 QnA 없이 진행 가능하나 문서에 기록한다.

### 2. `UCursorPartFocusRegistrationComponent`

host actor에 붙는 generic 등록 관리자.

역할:

- owner actor에서 `UCursorPartFocusScopeComponent`를 찾는다.
- `ClearRegisteredParts()`를 호출한다.
- owner actor/provider component/child provider component에서 descriptor를 수집한다.
- 수집한 descriptor를 `RegisterPartDescriptor()`로 scope에 등록한다.

권장 API:

```cpp
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class BEEKEEPINGSIM_API UCursorPartFocusRegistrationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus")
	void RebuildCursorPartFocusDescriptors();
};
```

수집 순서:

1. owner actor가 `ICursorPartFocusProvider`이면 수집
2. owner actor의 component 중 `ICursorPartFocusProvider` 구현 component 수집
3. 수집한 descriptor를 scope에 등록

주의:

- 자기 자신이 provider가 아니면 skip한다.
- 기존 `ABeehive` lid/comb 직접 등록을 즉시 모두 제거하기 어렵다면, 1차 구현은 placed item provider 수집만 component화하고 기존 beehive 직접 등록과 공존시킬 수 있다.
- 공존 경로를 택하면 clear 타이밍 충돌을 피해야 한다. registration component가 clear를 담당하면 `ABeehive::RebuildCursorPartFocusDescriptors()`의 clear와 중복되지 않게 구조를 정리한다.

### 3. `UChildCursorPartFocusProviderComponent`

`UChildItemUseAreaProviderComponent`의 PartFocus 버전.

역할:

- owner actor의 `UChildActorComponent`들을 순회한다.
- `RequiredChildActorComponentTag` 조건을 검사한다.
- `RequiredChildActorClass` 조건을 검사한다.
- child actor가 `ICursorPartFocusProvider`를 구현하면 descriptor를 받아 `OutDescriptors`에 append한다.

권장 properties:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
FName RequiredChildActorComponentTag = TEXT("PartFocusChild");

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
TSubclassOf<AActor> RequiredChildActorClass = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cursor Part Focus")
bool bLogSkippedChildren = false;
```

이 component 자체가 `ICursorPartFocusProvider`를 구현한다.

### 4. `AItemPlacementSlotActor`가 PartFocus provider 구현

`AItemPlacementSlotActor`에 `ICursorPartFocusProvider`를 추가한다.

동작:

- `PlacedActor`가 없거나 invalid이면 descriptor 반환 안 함
- `PlacedActor`가 `APlacedItemActor`이면 descriptor 1개 반환
- `PlacedActor`가 provider interface를 직접 구현하면 그 provider 결과를 위임하는 확장도 가능하지만, 1차 구현은 `APlacedItemActor` cast 기준으로 충분하다.

필요 API:

```cpp
UFUNCTION(BlueprintPure, Category = "Item Placement Slot")
AActor* GetPlacedActor() const;
```

descriptor 구성:

```cpp
FCursorPartFocusPartDescriptor Descriptor;
Descriptor.PartId = FName(*FString::Printf(TEXT("PlacedItem.%s"), *GetName()));
Descriptor.OwnerActor = PlacedItemActor;
Descriptor.HitComponent = PlacedItemActor->GetPartFocusHitComponent();
Descriptor.OutlineComponents.Add(PlacedItemActor->GetPartFocusHitComponent());
Descriptor.ActionHandler = PlacedItemActor->GetPartFocusActionComponent();
Descriptor.EngageMode = Descriptor.ActionHandler
	? Descriptor.ActionHandler->GetEngageMode()
	: ECursorPartFocusEngageMode::PreviewOnly;
Descriptor.PromptData.bIsValid = true;
Descriptor.PromptData.DisplayName = PlacedItemActor->GetPlacedItemDisplayName();
Descriptor.PromptData.InteractionKeyText = FText::FromString(TEXT("RMB"));
OutDescriptors.Add(Descriptor);
```

주의:

- PartFocus descriptor는 occupied slot에서만 반환한다.
- item-use-area descriptor는 empty slot에서만 반환하는 기존 정책을 유지한다.
- descriptor 반환 전 `SanitizeAndCheckOccupied()`로 invalid actor를 정리한다.

### 5. `APlacedItemActor` 정리

`APlacedItemActor`는 PartFocus 대상 actor다.

권장 구성:

- `Root`
- `ItemMesh`
- `UPlacedItemRetrievePartFocusActionComponent`
- `ItemDefinition`
- `OwningPlacementSlotActor`

제거/비사용:

- `UFocusTargetComponent`
- `UFocusSecondaryActionComponent`
- global Focus hover 회수용 `UPlacedItemRetrieveFocusActionComponent`

필요 API:

```cpp
UPrimitiveComponent* GetPartFocusHitComponent() const;
UCursorPartFocusActionComponent* GetPartFocusActionComponent() const;
FText GetPlacedItemDisplayName() const;
UItemDefinition* GetItemDefinition() const;
AActor* GetOwningPlacementSlotActor() const;
```

`InitializePlacedItem(...)` mesh 정책:

```cpp
if (ItemMesh && ItemDefinition && ItemDefinition->WorldMesh)
{
	ItemMesh->SetStaticMesh(ItemDefinition->WorldMesh);
}
```

즉 `ItemDefinition->WorldMesh`가 있을 때만 BP-authored mesh를 override한다.

### 6. Optional secondary PartFocus action API

`UCursorPartFocusActionComponent`에 optional secondary API 추가.

기본값은 false/no-op이다.

```cpp
UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Secondary")
virtual bool CanHandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter) const;

UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Secondary")
virtual bool HandleSecondaryPartFocusAction(UCursorPartFocusScopeComponent* ScopeComponent, ABeekeeperCharacter* InteractingCharacter);
```

기본 구현:

- `CanHandle...` false
- `Handle...` false

Blueprint hook이 필요하면 `BlueprintNativeEvent` 추가를 검토한다. 단, 현재 범위에서는 C++ action component로 충분하다.

### 7. `UCursorPartFocusScopeComponent::HandleSecondaryInput`

scope가 현재 hovered descriptor의 action handler에 secondary input을 전달한다.

권장 동작:

- scope inactive면 false
- hover 갱신
- hovered descriptor 없으면 false
- descriptor preview가 allowed가 아니면 true 또는 false를 신중히 선택
  - 권장: required state 불만족이면 true로 consume하지 말고 false
- action 없으면 false
- action이 `CanHandleSecondaryPartFocusAction(...)` false면 false
- action `HandleSecondaryPartFocusAction(...)` 호출

### 8. Focus engaged secondary 입력 라우팅

현재 global `UBeekeeperFocusComponent::HandleSecondaryInput()` 경로가 있다면, preview target 전용이 아니라 FocusEngaged action에도 전달하도록 정리한다.

권장:

`UFocusActionComponent`에 추가:

```cpp
virtual bool HandleSecondaryInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);
```

기본 false.

`UBeekeeperFocusComponent::HandleSecondaryInput()`:

- `bIsFocusEngaged`이면 `EngagedFocusAction->HandleSecondaryInputWhileEngaged(OwnerCharacter)` 호출
- non-engaged preview secondary가 더 이상 필요 없으면 제거 또는 false 유지

`UAnchoredFocusCursorActionComponent::HandleSecondaryInputWhileEngaged(...)`:

- owner의 `UCursorPartFocusScopeComponent`를 찾아 `HandleSecondaryInput()` 호출

`ABeekeeperCharacter::FocusSecondaryInput()`:

- 기존 RMB input action을 유지해도 된다.
- 단, 의미는 preview global secondary가 아니라 engaged part secondary로 문서화한다.

### 9. `UPlacedItemRetrievePartFocusActionComponent`

`UCursorPartFocusActionComponent` subclass.

역할:

- owner `APlacedItemActor`를 회수한다.
- secondary PartFocus action만 처리한다.
- LMB begin/cancel lifecycle은 기본 동작을 쓰지 않거나 no-op으로 둔다.

실행 조건:

- owner가 `APlacedItemActor`
- item definition 유효
- interacting character와 hotbar 유효

실행:

```cpp
const FHotbarItemAcquireResult Result = Hotbar->TryAcquireItem(ItemDefinition, 1);
if (!Result.bSuccess || Result.AddedQuantity != 1)
{
	return false;
}

if (AActor* SlotActor = PlacedItem->GetOwningPlacementSlotActor())
{
	if (SlotActor->GetClass()->ImplementsInterface(UItemPlacementSlot::StaticClass()))
	{
		IItemPlacementSlot::Execute_ClearPlacedItem(SlotActor);
		return true;
	}
}

PlacedItem->Destroy();
return true;
```

주의:

- 실패 시 actor를 destroy하지 않는다.
- 성공 후 slot이 clear되면 item-use-area descriptor가 다시 제공되어야 한다.
- 회수 후 PartFocus descriptor rebuild가 필요하다. 최소 경로:
  - slot clear 후 host registration component rebuild 호출
  - 또는 next explicit rebuild 시 반영
- 즉시 반영이 필요하면 `AItemPlacementSlotActor::ClearPlacedItem()` 또는 retrieve action에서 host registration component를 찾아 rebuild한다.

### 10. `ABeehive` 적용

1차 적용 전략:

- `ABeehive`에 `UCursorPartFocusRegistrationComponent` 추가
- `ABeehive`에 `UChildCursorPartFocusProviderComponent` 추가
- pollen/placement slot child actor component에 `RequiredChildActorComponentTag`와 일치하는 tag를 부여하는 BP 절차 문서화
- 기존 lid/comb 등록은 당장 전부 provider로 옮기지 않아도 된다.

중요한 선택:

- `RebuildCursorPartFocusDescriptors()`의 clear/register 흐름과 registration component clear/register 흐름이 충돌하면 안 된다.

권장 1차 구현:

- 기존 `ABeehive::RebuildCursorPartFocusDescriptors()`가 계속 clear와 lid/comb 등록을 담당한다.
- 그 함수 마지막에 registration component의 "append only" 수집 함수를 호출한다.
- 이 경우 `UCursorPartFocusRegistrationComponent`에는 clear 포함 rebuild와 clear 없는 append API를 둘 수 있다.

예:

```cpp
void RebuildCursorPartFocusDescriptors(); // clear + append
void AppendCursorPartFocusDescriptorsToScope(); // clear 없이 append
```

`ABeehive`에서는:

```cpp
CursorPartFocusScope->ClearRegisteredParts();
Register lid;
Register comb;
Register preview-only;
PartFocusRegistrationComponent->AppendCursorPartFocusDescriptorsToScope();
```

장기적으로는 lid/comb도 provider화해서 `ABeehive` 직접 등록 코드를 줄인다.

## 문서 갱신

구현 후 갱신:

- `.md/0_ARCHITECTURE.md`
  - PartFocus descriptor provider/registration 흐름 추가
  - placed item이 FocusTarget이 아니라 host 내부 PartFocus part임을 기록
- `.md/Architecture/FocusSystem.md`
  - `ICursorPartFocusProvider`
  - `UCursorPartFocusRegistrationComponent`
  - `UChildCursorPartFocusProviderComponent`
  - secondary PartFocus input 정책
- `.md/Architecture/WorldActorsSystem.md`
  - `AItemPlacementSlotActor` empty/occupied 상태별 provider 정책
  - `APlacedItemActor` 역할
  - placed item 회수 PartFocus action
- `.md/Architecture/CharacterSystem.md`
  - RMB/secondary input은 FocusEngaged PartFocus secondary로 라우팅됨을 기록
- `.md/Architecture/InventorySystem.md`
  - placed item 회수는 `TryAcquireItem(ItemDefinition, 1)` 성공 시에만 world actor 제거
- `.md/USER_UNREAL.md`
  - 벌통 child slot actor component에 PartFocus provider 수집 tag를 붙이는 절차
  - `APlacedItemActor` 기반 BP mesh authoring 절차
  - RMB input action mapping 절차

## 검증 기준

### 코드 검색

있어야 함:

- `ICursorPartFocusProvider`
- `UCursorPartFocusRegistrationComponent`
- `UChildCursorPartFocusProviderComponent`
- `GetCursorPartFocusDescriptors`
- `HandleSecondaryPartFocusAction`
- `HandleSecondaryInputWhileEngaged`
- `UPlacedItemRetrievePartFocusActionComponent`
- `AItemPlacementSlotActor`의 PartFocus provider 구현
- `APlacedItemActor::GetPartFocusHitComponent`
- `TryAcquireItem(ItemDefinition, 1)`
- `AddedQuantity != 1`

없어야 함:

- `APlacedItemActor`의 `UFocusTargetComponent` 기본 subobject
- placed item 회수용 global `UFocusSecondaryActionComponent` 의존
- `APollenPattyActor` UCLASS rename
- `Config/DefaultEngine.ini` CoreRedirect 추가
- UI widget에서 회수 mutation 직접 처리

### 빌드

가능하면 수행:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

### 수동 검증

1. `APlacedItemActor` 기반 BP를 만든다.
2. BP의 mesh를 설정한다.
3. placement action의 `PlacedActorClass`를 해당 BP로 설정한다.
4. 벌통의 placement slot child actor component가 item-use-area provider와 part-focus provider 수집 조건을 모두 만족하는지 확인한다.
5. 빈 slot에서는 item-use-area 보라색 표시가 보인다.
6. 아이템을 설치하면 해당 slot의 item-use-area는 사라진다.
7. 설치된 item mesh는 host FocusEngaged 상태에서 PartFocus hover/outline 대상이 된다.
8. 설치된 item hover 중 RMB를 누르면 hotbar에 item 1개가 추가되고 slot이 빈 상태로 돌아간다.
9. hotbar에 공간이 없으면 RMB 회수는 실패하고 actor/slot 상태는 유지된다.
10. 다른 PartFocus 대상, 예: lid/comb는 RMB 회수되지 않는다.
11. 설치/회수 후 남은 빈 slot들의 item-use-area visual이 계속 정상 표시된다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `FCursorPartFocusPartDescriptor` include 구조 때문에 큰 파일 분리가 필요하고 영향 범위가 예상보다 커지는 경우
- `ABeehive` 기존 lid/comb 직접 등록과 generic registration component clear 타이밍이 충돌하는 경우
- Focus secondary input을 기존 global preview action과 공존시킬지 제거할지 결정이 필요한 경우
- `APlacedItemActor`의 기존 `FocusTarget` 제거가 Blueprint native parent/serialized component에 영향을 줄 가능성이 확인되는 경우
- `APollenPattyActor` parent 변경 또는 component rename이 필요해지는 경우
- 회수 후 즉시 PartFocus descriptor rebuild를 어느 actor/component가 책임질지 불명확한 경우
- `TryAcquireItem(ItemDefinition, 1)` 결과 semantics가 회수 성공 판정에 부적절한 경우
- Content asset compile/save 없이는 migration 확인이 불가능한 경우
