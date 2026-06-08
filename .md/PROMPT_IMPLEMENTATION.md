# 밀도 작업대 + 밀도질 구현 프롬프트

## 목표

밀도 작업대와 소비장 밀도질을 구현한다.

- 밀도 작업대는 C++ native WorldActor다.
- 밀도 작업대는 FocusConfirm으로 FocusEngaged 작업 상태에 진입한다.
- 밀도 작업대는 소비장 슬롯 1개를 가진다.
- 소비장 슬롯에는 기존 소비장 actor(`ABeehiveCombActor`)만 배치할 수 있다.
- 밀도 도구는 LMB hold-use로 현재 visible face의 밀랍/capping mask를 원형 브러시로 부분 제거한다.
- 제거된 밀랍 영역 아래의 기존 honey plane이 보인다.
- 이번 범위에서는 채밀/수확, 꿀 아이템 생산, 밀도 도구 내구도 감소를 구현하지 않는다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

참고:

- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

현재 `.md/QNA_IMPLEMENTATION.md`의 `FocusPrompt Asset 반영` 항목은 이번 작업 범위와 직접 관련 없다. 이번 작업 중 새 애매함이 생기면 기존 항목을 지우지 말고 새 질문을 추가한다.

## 확정된 QnA 반영

- 작업대 슬롯 class는 작업대 전용 subclass로 만든다.
- empty slot 배치는 기존 `UItemPlacementUseAction` + item-use-area LMB 경로를 쓴다.
- occupied comb 회수는 PartFocus secondary retrieve 경로를 쓴다.
- FocusEngaged 진입 시 기존 anchored cursor 정책처럼 hotbar 선택을 비운다.
- 밀도질은 `IsHoneyFull()`이고 해당 visible face에 남은 capping mask가 있을 때만 active다.
- 현재 visible face만 밀도질 가능하다.
- 작업대 소비장 뒤집기는 벌통 소비장과 같은 horizontal drag flip UX를 사용한다.
- 기존 `UBeehiveCombPartFocusActionComponent`는 직접 재사용하지 않는다.
- 작업대 전용 action 이름은 `UCombUncappingPartFocusActionComponent`로 한다.
- capping mask는 `FBeehiveCombItemState`에 저장해 회수/재배치 후에도 보존한다.
- mask 해상도는 `CombPlaneSize` 비율에 맞추고, brush 판정은 plane local 거리 기준이다.
- mask source of truth는 face별 `TArray<uint8>` byte buffer다.
- visual 반영은 transient `UTexture2D`를 만들어 material parameter `WaxCappingMask`에 주입한다.
- item-use-area hit 정보는 `FItemActionContext`에 추가하고 Focus scope가 채운다.
- 밀도질 action 이름은 `UCombUncappingUseAction`이다.
- 밀도질 use-area tag는 `Item.UseArea.UncappingTable.Comb`이다.
- `bSucceeded=true`는 실제 mask pixel이 하나 이상 제거된 Tick에만 반환한다.
- 이미 다 지워진 부분을 문질러 mask 변화가 없으면 `bSucceeded=false`다.
- brush stamp는 `MinStampInterval` + `MinStampDistanceCm` 기반 rate limit을 둔다.
- face별 remaining mask ratio가 `UncappedThreshold` 이하이면 해당 face 완료다. 양면 모두 완료되어야 소비장 전체 완료다.
- 이번 범위는 밀도 작업대와 밀도질 mask 제거까지만 구현한다.
- `BP_BeehiveComb` capping material graph의 `WaxCappingMask` texture parameter 연결은 수동 Content 작업으로 둔다.

## 권장 새 클래스/파일

WorldActors:

- `Source/BeekeepingSim/Public/WorldActors/UncappingTable.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTable.cpp`
- `Source/BeekeepingSim/Public/WorldActors/UncappingTableCombSlot.h`
- `Source/BeekeepingSim/Private/WorldActors/UncappingTableCombSlot.cpp`
- `Source/BeekeepingSim/Public/WorldActors/CombUncappingPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/CombUncappingPartFocusActionComponent.cpp`

Inventory:

- `Source/BeekeepingSim/Public/Inventory/CombUncappingUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/CombUncappingUseAction.cpp`

기존 파일 확장:

- `Source/BeekeepingSim/Public/Inventory/ItemActionContext.h`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Config/DefaultGameplayTags.ini`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/Architecture/CoreSystem.md`
- `.md/USER_UNREAL.md`
- 필요 시 `.md/PROMPT_REVIEW.md`

## 수정 금지

- `Content/` asset 직접 수정/저장
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- `UBeehiveCombPartFocusActionComponent`를 작업대에서 직접 재사용
- 벌통 소비장 lift/shake 정책 변경
- `ABeehive` honey production/ripeness bucket 변경
- 채밀/수확/꿀 아이템 생산 구현
- 밀도 도구 active-use durability drain 구현
- Core Redirect 추가

`Config/DefaultGameplayTags.ini`에 GameplayTag를 추가하는 것은 허용한다. 이것은 Core Redirect가 아니다.

## 구현 지시

### 1. GameplayTag 추가

`Config/DefaultGameplayTags.ini`에 아래 tag를 추가한다.

```ini
+GameplayTagList=(Tag="Item.UseArea.UncappingTable",DevComment="")
+GameplayTagList=(Tag="Item.UseArea.UncappingTable.Comb",DevComment="")
```

이미 같은 tag가 있으면 중복 추가하지 않는다.

### 2. item-use-area hit context 확장

`FItemActionContext`에 아래 필드를 추가한다.

권장:

```cpp
UPROPERTY(BlueprintReadWrite, Category = "Item Action")
bool bHasItemUseAreaHit = false;

UPROPERTY(BlueprintReadWrite, Category = "Item Action")
FVector ItemUseAreaImpactPoint = FVector::ZeroVector;

UPROPERTY(BlueprintReadWrite, Category = "Item Action")
FVector ItemUseAreaImpactNormal = FVector::UpVector;
```

`UCursorItemUseAreaScopeComponent` 변경:

- cursor trace hit 결과를 `ResolveHoveredActiveDescriptor()`에서 버리지 말고 보존한다.
- action이 직접 mouse deproject/line trace를 다시 수행하지 않게 한다.
- 권장 구현:
  - private cached fields:
    - `bool bHasHoveredItemUseAreaHit`
    - `FHitResult HoveredItemUseAreaHit`
  - `UpdateHoveredDescriptorFromCursor()`에서 hover index와 hit result를 함께 갱신한다.
  - hover가 사라지면 cached hit도 clear한다.
  - `BuildItemActionContext(DescriptorIndex)`에서 `DescriptorIndex == HoveredDescriptorIndex`이고 cached hit가 유효하면 context hit fields를 채운다.

주의:

- `TickUse`, `BeginUse`, `EndUse` context에도 가능한 한 현재 hover hit가 들어가게 유지한다.
- durability-only context(`INDEX_NONE`)에는 hit fields를 채우지 않아도 된다.
- 기존 `ItemUseAreaHitComponent`와 `ItemUseEffectTargetObject` 의미는 변경하지 않는다.

### 3. 소비장 capping mask 상태 추가

`ABeehiveCombActor`가 face별 capping mask의 runtime source of truth를 가진다.

권장 runtime fields:

```cpp
UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping", meta = (ClampMin = "1"))
int32 CappingMaskLongSideResolution = 512;

UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping", meta = (ClampMin = "0.0", ClampMax = "1.0"))
float UncappedThreshold = 0.01f;

UPROPERTY(EditAnywhere, Category = "Beehive|Wax Capping")
FName WaxCappingMaskMaterialParameterName = TEXT("WaxCappingMask");

UPROPERTY(VisibleAnywhere, Category = "Beehive|Wax Capping")
int32 CappingMaskWidth = 0;

UPROPERTY(VisibleAnywhere, Category = "Beehive|Wax Capping")
int32 CappingMaskHeight = 0;

UPROPERTY(VisibleAnywhere, Category = "Beehive|Wax Capping")
TArray<uint8> FrontWaxCappingMask;

UPROPERTY(VisibleAnywhere, Category = "Beehive|Wax Capping")
TArray<uint8> BackWaxCappingMask;

UPROPERTY(Transient)
TObjectPtr<UTexture2D> FrontWaxCappingMaskTexture;

UPROPERTY(Transient)
TObjectPtr<UTexture2D> BackWaxCappingMaskTexture;
```

값 의미:

- `255`: 밀랍 완전히 남아 있음
- `0`: 밀랍 제거됨

mask dimension 산출:

- `CombPlaneSize`의 X/Y 비율을 사용한다.
- 더 긴 축을 `CappingMaskLongSideResolution`으로 둔다.
- 짧은 축은 비율대로 `RoundToInt`하고 최소 1로 clamp한다.
- `CombPlaneSize`가 invalid이면 square fallback을 사용한다.

초기화 정책:

- 새 소비장 또는 저장 mask가 없는 소비장은 front/back mask를 전부 `255`로 채운다.
- `CurrentHoney`가 full이 아니어도 mask는 유지한다.
- `CurrentHoney`가 full이 아니면 capping plane은 숨기되 mask를 reset하지 않는다.
- `ApplyStateFromItemInstance`에서 저장 mask가 있고 dimension이 현재 actor dimension과 맞으면 복원한다.
- 저장 mask가 없거나 invalid면 full mask로 초기화한다.

### 4. `FBeehiveCombItemState`에 mask 보존 추가

`FBeehiveCombItemState`에 capping mask fields를 추가한다.

권장:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0"))
int32 CappingMaskWidth = 0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb", meta = (ClampMin = "0"))
int32 CappingMaskHeight = 0;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
TArray<uint8> FrontWaxCappingMask;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Beehive Comb")
TArray<uint8> BackWaxCappingMask;
```

기존 setter 호환:

- `SetBeehiveCombState(...)`
- `SetBeehiveCombStateWithRipeness(...)`

위 기존 API는 삭제/rename하지 않는다.

권장 추가 API:

- `SetBeehiveCombStateWithCapping(...)`
- 또는 기존 내부 helper에서 mask까지 복사하는 private path

주의:

- 기존 setter를 호출하면 mask는 비어 있는 state로 저장되어도 된다.
- `ABeehiveCombActor::ApplyStateFromItemInstance`는 비어 있는 mask를 full mask fallback으로 처리해야 한다.
- `WriteStateToItemInstance`는 honey amount, ripeness, visible face와 함께 current capping masks를 저장한다.

### 5. 소비장 capping use-area mesh 추가

`ABeehiveCombActor`에 front/back capping use-area mesh를 추가한다.

권장 component:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UItemUseAreaMeshComponent> FrontWaxCappingUseAreaMesh;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UItemUseAreaMeshComponent> BackWaxCappingUseAreaMesh;
```

생성 정책:

- `CombMesh` 하위에 attach한다.
- `EffectTargetPolicy`는 `ComponentOwner`.
- 기본 collision은 `NoCollision`/ignore로 두고, `UCursorItemUseAreaScopeComponent`가 active descriptor에 대해 query collision을 켜는 기존 패턴을 따른다.
- `AreaTags`에는 `Item.UseArea.UncappingTable.Comb` tag를 넣는다.
  - tag가 invalid이면 silent no-op이 아니라 log warning 또는 QnA 중단을 검토한다.
- material/mesh/relative transform은 C++ 기본값 + BP authoring으로 조정 가능하게 둔다.

active 조건:

- `HostActor`가 `AUncappingTable`일 때만 active다.
- `IsHoneyFull()`이어야 한다.
- 현재 visible face와 component face가 일치해야 한다.
- 해당 face remaining mask ratio가 `UncappedThreshold`보다 커야 한다.
- `BeeBrushUseAreaMesh`의 기존 active 조건은 변경하지 않는다.

필요 getter:

- `GetFrontWaxCappingUseAreaMesh()`
- `GetBackWaxCappingUseAreaMesh()`

### 6. capping mask visual 적용

기존 `FrontWaxCappingPlane` / `BackWaxCappingPlane` material dynamic instance 경로를 유지한다.

추가 구현:

- mask byte buffer에서 transient `UTexture2D`를 생성/갱신한다.
- texture는 `WaxCappingMaskMaterialParameterName`으로 capping material instance에 주입한다.
- mask texture는 runtime transient이므로 `UPROPERTY(Transient)`로 GC 보호한다.
- texture update 후 `UpdateResource()` 호출을 빠뜨리지 않는다.

권장 helper:

- `EnsureCappingMaskState()`
- `EnsureCappingMaskTextures()`
- `UpdateCappingMaskTexture(EBeehiveCombVisibleFace Face)`
- `ApplyWaxCappingMaskMaterialParameters()`
- `GetWaxCappingRemainingRatio(EBeehiveCombVisibleFace Face) const`
- `IsWaxCappingFaceComplete(EBeehiveCombVisibleFace Face) const`
- `IsWaxCappingComplete() const`

`ApplyHoneyCappingVisualState()` 표시 조건 변경:

- 기존: `IsHoneyFull()`
- 변경: face별로 `IsHoneyFull() && !IsWaxCappingFaceComplete(Face)`

주의:

- capping plane 전체 visibility만 끄는 방식으로 부분 밀도질을 대체하지 않는다.
- `HoneyRipeness` scalar 주입은 유지한다.
- `HoneyAmount`를 capping material에 새로 주입하지 않는다.
- material graph의 `WaxCappingMask` 연결은 Content 수동 작업으로 남긴다.

### 7. 밀도질 brush API 추가

`ABeehiveCombActor`에 밀도질 action이 호출할 public API를 추가한다.

권장:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Wax Capping")
bool ApplyWaxCappingBrush(UPrimitiveComponent* HitComponent, const FVector& WorldImpactPoint, float BrushRadiusCm);
```

동작:

1. `HitComponent`가 front/back capping use-area mesh 중 하나인지 확인한다.
2. component face가 현재 visible face인지 확인한다.
3. `IsHoneyFull()`인지 확인한다.
4. hit component local X/Y plane을 mask 좌표계로 본다.
5. local X/Y 기준 실제 cm 거리로 원형 brush 판정을 한다.
6. brush 반경 안의 mask pixel을 `0`으로 만든다.
7. 하나 이상의 pixel이 `>0`에서 `0`으로 바뀌면 true 반환.
8. 변경이 있으면 해당 face texture/material/visibility를 갱신한다.

좌표 contract:

- capping use-area mesh local X/Y가 `CombPlaneSize.X/Y` 작업면에 대응한다고 정의한다.
- local origin은 작업면 중심이다.
- C++은 `WorldImpactPoint`를 hit component local position으로 변환해 사용한다.
- BP에서는 front/back use-area mesh transform을 이 계약에 맞춰 수동 조정한다.

### 8. `UCombUncappingUseAction` 추가

`UHoldItemUseAction` subclass로 만든다.

권장 properties:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping", meta = (ClampMin = "0.0"))
float BrushRadiusCm = 8.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping", meta = (ClampMin = "0.0"))
float MinStampInterval = 0.03f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Comb Uncapping", meta = (ClampMin = "0.0"))
float MinStampDistanceCm = 4.0f;
```

state:

- `bool bHasLastStamp`
- `FVector LastStampWorldPoint`
- `float TimeSinceLastStamp`

constructor:

- `UseAreaTagQuery`를 `Item.UseArea.UncappingTable.Comb` all-tags-match로 구성한다.

`EndUse`:

- stamp state reset 후 super 호출.

`ApplyUseEffect`:

1. `Context.ItemUseEffectTargetObject`를 `ABeehiveCombActor`로 cast한다.
2. `Context.bHasItemUseAreaHit`와 `Context.ItemUseAreaHitComponent`를 검증한다.
3. 첫 stamp는 즉시 허용한다.
4. 이후 stamp는 `TimeSinceLastStamp >= MinStampInterval`이고 `Distance >= MinStampDistanceCm`일 때만 허용한다.
5. 허용되면 `CombActor->ApplyWaxCappingBrush(...)` 호출.
6. 실제 mask pixel 변경이 있으면 `Result.bSucceeded = true`.
7. mask 변화가 없으면 `Result.bSucceeded = false`.
8. 이번 범위에서는 `Result.DurabilityDelta`를 설정하지 않는다.

주의:

- action이 직접 hotbar durability를 변경하지 않는다.
- action이 직접 line trace를 다시 하지 않는다.
- 이미 지워진 부분에서 no-op이 나오는 것은 정상이다.

### 9. 작업대 전용 PartFocus action 추가

`UCombUncappingPartFocusActionComponent`를 추가한다.

역할:

- 작업대 slot에 놓인 소비장의 horizontal drag flip UX만 담당한다.
- 기존 `UBeehiveCombPartFocusActionComponent`를 상속하거나 직접 재사용하지 않는다.
- base는 `UCursorPartFocusActionComponent`.

권장 정책:

- `EngageMode = PersistentAction`
- Required tags 없음
- Exclusive group은 비워도 된다.
- prompt text는 작업대 맥락에 맞게 짧게 둔다. 예: `잡기` / `놓기` 또는 기존 패턴과 맞는 값.
- vertical shake, bee reduction, lid required tag, comb lift group은 구현하지 않는다.

drag 동작:

- `CanBeginPartFocusDrag`는 action engaged 상태이고 slot에 placed comb가 있을 때 true.
- `BeginPartFocusDrag`에서 drag state reset.
- `UpdatePartFocusDrag`에서 `GetPartFocusDragDeltaFromPress()`의 X/Y를 본다.
- 기존 벌통과 같은 기준값을 사용하되 property로 둔다.
  - `CombFlipDragThresholdPixels = 120.0f`
  - `HorizontalDominanceRatio = 1.5f`
- `AbsX >= Threshold`이고 `AbsX > AbsY * HorizontalDominanceRatio`이면 한 번만 flip 실행.
- delta X가 양수면 `EBeehiveCombFlipDirection::Right`, 음수면 `Left`.
- 대상 comb는 owner slot의 `GetPlacedCombActor()`로 찾는다.

secondary retrieve:

- 기존 `UBeehiveCombPartFocusActionComponent`의 retrieve bridge 패턴을 참고한다.
- slot에 놓인 comb의 `PlacementRetrieveAction`을 사용해 `TryRetrievePlacementOccupant`를 호출한다.
- 성공 시 `CombActor->WriteStateToItemInstance(AcquiredItemInstance)` 호출 후 owning slot `ClearPlacedItem`.
- prompt availability와 실제 실행은 같은 retrieve helper를 사용한다.

### 10. 작업대 comb slot 추가

`AUncappingTableCombSlot : AItemPlacementSlotActor`를 추가한다.

역할:

- `ABeehiveCombActor`만 accept한다.
- place 성공 후 `CombActor->ApplyStateFromItemInstance(SourceItemInstance)` 호출.
- clear/place 후 owning `AUncappingTable`의 part focus와 item-use-area descriptor rebuild를 요청한다.
- `UCombUncappingPartFocusActionComponent`를 기본 subobject로 가진다.
- occupied comb part descriptor를 직접 제공한다.

PartFocus descriptor:

- `GetCursorPartFocusDescriptors_Implementation` override.
- empty면 descriptor 없음.
- occupied comb가 있으면:
  - `PartId = "UncappingTable.Comb"`
  - `OwnerActor = CombActor`
  - `HitComponent = CombActor->GetCombMeshComponent()`
  - `OutlineComponents`에 comb mesh 추가
  - `ActionHandler = CombPartFocusAction`
  - `EngageMode = CombPartFocusAction->GetEngageMode()`
  - prompt display name은 `소비장` fallback

주의:

- base `AItemPlacementSlotActor` generic occupied descriptor가 기존 `ABeehiveCombActor::PartFocusAction`을 노출하지 않도록 override에서 직접 descriptor를 구성한다.
- `ABeehiveCombSlotActor`를 상속하지 않는다.
- 벌통 refresh는 호출하지 않는다.

### 11. 작업대 actor 추가

`AUncappingTable`을 추가한다.

권장 components:

- `Root`
- `TableMesh` (`UStaticMeshComponent`)
- `FocusTarget` (`UFocusTargetComponent`)
- `FocusAction` (`UAnchoredFocusCursorActionComponent`)
- `CursorPartFocusScope` (`UCursorPartFocusScopeComponent`)
- `CursorPartFocusRegistration` (`UCursorPartFocusRegistrationComponent`)
- `ChildCursorPartFocusProvider` (`UChildCursorPartFocusProviderComponent`)
- `ItemUseAreaScope` (`UCursorItemUseAreaScopeComponent`)
- `ItemUseAreaMeshProvider` (`UItemUseAreaMeshProviderComponent`)
- `CombSlotRoot` (`USceneComponent`)
- `CombSlotChildActor` (`UChildActorComponent`)

properties:

- `TSubclassOf<AUncappingTableCombSlot> CombSlotActorClass`
- prompt display names/text as needed

setup:

- `CombSlotActorClass` default는 `AUncappingTableCombSlot::StaticClass()`.
- `CombSlotChildActor` class를 `CombSlotActorClass`로 설정한다.
- `CombSlotChildActor`에 component tag `PartFocusChild`를 추가한다.
- `ChildCursorPartFocusProvider` 기본 tag 정책과 맞춘다.

public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Uncapping Table|Part Focus")
void RebuildCursorPartFocusDescriptors();

UFUNCTION(BlueprintCallable, Category = "Uncapping Table|Item Use Area")
void RebuildItemUseAreaDescriptors();

UFUNCTION(BlueprintPure, Category = "Uncapping Table|Comb Slot")
AUncappingTableCombSlot* GetCombSlotActor() const;
```

`RebuildCursorPartFocusDescriptors`:

- `CursorPartFocusScope->ClearRegisteredParts()`
- `CursorPartFocusRegistration->AppendCursorPartFocusDescriptorsToScope()`

`RebuildItemUseAreaDescriptors`:

- `ItemUseAreaScope->RebuildItemUseAreaDescriptors()`

FocusEngaged 진입:

- 기존 `ABeehive`의 `UFocusTargetComponent` + `UAnchoredFocusCursorActionComponent` + item-use-area scope 패턴을 따른다.
- 별도 input binding을 만들지 않는다.

### 12. 문서 업데이트

구현 후 아래 문서를 갱신한다.

`.md/0_ARCHITECTURE.md`

- Source 파일 수/분포 변경
- WorldActors/Inventory/Focus 흐름에 밀도 작업대/밀도질 요약 추가
- `FBeehiveCombItemState`가 capping mask를 보존한다는 내용 추가
- `Item.UseArea.UncappingTable.Comb` tag 추가 사항 기록

`.md/Architecture/WorldActorsSystem.md`

- `AUncappingTable`
- `AUncappingTableCombSlot`
- `UCombUncappingPartFocusActionComponent`
- `ABeehiveCombActor` capping mask/use-area/visual state
- 작업대 배치/회수/flip flow

`.md/Architecture/FocusSystem.md`

- `FItemActionContext` item-use-area hit fields
- `UCursorItemUseAreaScopeComponent`가 hovered hit result를 context에 전달하는 계약

`.md/Architecture/InventorySystem.md`

- `UCombUncappingUseAction`
- `FBeehiveCombItemState` capping mask 보존
- 내구도 미구현 범위

`.md/USER_UNREAL.md`

- `BP_BeehiveComb`에서 front/back capping use-area mesh transform/mesh/material 조정 필요
- `FrontWaxCappingPlane`/`BackWaxCappingPlane` material graph에 texture parameter `WaxCappingMask`를 alpha/opacity mask로 연결해야 함
- 밀도 도구 DataAsset에 `UCombUncappingUseAction` action spec 추가 필요
- 밀도 작업대 BP/native child에서 mesh/material/slot transform authoring 필요

`.md/PROMPT_REVIEW.md`

- 구현 완료 후 리뷰 프롬프트를 새 범위에 맞게 작성하거나 갱신한다.

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
- `FItemActionContext`에 hit fields가 있다.
- `Config/DefaultGameplayTags.ini`에 새 tag가 있다.
- `FBeehiveCombItemState`에 capping mask가 저장된다.
- `WriteStateToItemInstance`가 capping mask를 저장한다.
- `ApplyStateFromItemInstance`가 capping mask를 복원하거나 full mask fallback을 수행한다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 수동 검증 항목

Editor/PIE에서 확인한다.

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
14. 양면 완료 여부는 `IsWaxCappingComplete()` 또는 동등 API로 확인된다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문을 추가한다.

- `UTexture2D` transient mask 갱신이 현재 UE 5.7 API에서 불가능하거나 별도 render target pipeline이 필요하다고 판단되는 경우
- capping mask를 `TArray<uint8>`로 `FBeehiveCombItemState`에 저장할 수 없는 경우
- `UCursorItemUseAreaScopeComponent` hit context 확장이 기존 hold-use action을 깨뜨릴 가능성이 있는 경우
- 작업대 actor를 기존 FocusEngaged 경로에 붙이려면 input binding 변경이 필요하다고 판단되는 경우
- `UBeehiveCombPartFocusActionComponent`를 직접 재사용해야만 한다고 판단되는 경우
- Content asset 수정/저장이 C++ 구현 완료 조건이 되는 경우
- UCLASS/USTRUCT/UENUM rename 또는 Core Redirect 필요성이 생기는 경우
- `ABeehive` honey production/ripeness bucket 변경이 필요하다고 판단되는 경우
- 채밀/수확/꿀 아이템 생산이 이번 구현에 필요하다고 판단되는 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 Config 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Core Redirect 불필요 여부
- Blueprint/API 영향
- Content 수동 작업 목록
- 밀도 작업대 FocusEngaged 진입 확인
- 소비장 배치/회수 경로 확인
- visible face만 밀도질 가능함을 확인
- capping mask 저장/복원 확인
- `WaxCappingMask` material parameter 주입 확인
- 밀도 도구 내구도 미구현 확인
- 채밀/수확 미구현 확인
