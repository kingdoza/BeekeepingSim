# 왕롱 여왕벌 포획과 분봉 본진 최종 완료 조건 구현 프롬프트

## 목표

현재 구현된 분봉/BeeCarrier 포획 구조 위에 `왕롱` 아이템으로 모든 `AQueenBeeActor`를 포획하는 기능을 추가한다.

왕롱은 무조건 여왕벌 1마리만 담을 수 있다. 포획 결과는 world actor reference가 아니라 `UItemInstance` optional runtime state로 저장한다. 벌통 여왕벌과 분봉 본진 여왕벌은 모두 같은 `AQueenBeeActor` 경로로 포획 대상이 되어야 한다.

추가로 분봉 본진의 최종 완료 조건을 수정한다. 기존에는 벌 수가 모두 포획되면 `ABeeSwarmClusterActor`가 captured로 전환했지만, 이제 분봉 본진은 BeeCarrier로 벌을 모두 포획하고 왕롱으로 여왕벌까지 포획해야 최종 완료된다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`

## 현재 코드 전제

- `AQueenBeeActor`는 `QueenBeeMesh`, `BaseEggLayingPower`, `DiseaseValue`를 가진다.
- `ABeehive`는 `QueenBeeChildActor` child actor component를 소유하고 `GetQueenBeeActor()`, `IsQueenBeeAttachedToComb()`, `TryBrushQueenBeeFromCombVisibleFace()`를 제공한다.
- `ABeeSwarmClusterActor`는 `QueenBeeChildActor` child actor component를 소유하고, 벌 포획 source of truth로 `CapturedBeeAmount`와 `SpawnAmount`를 사용한다.
- `ABeeSwarmClusterActor::IsCaptured()`와 `bCaptured`는 현재 벌만 모두 포획되어도 true가 된다.
- `UItemInstance`에는 `FBeehiveCombItemState`, `FHoneyContainerItemState`, `FBeeCarrierItemState` optional runtime state 패턴이 있다.
- `ItemStackMoveUtils`는 runtime state compatibility와 `MaxStack=1` 특수 definition 처리를 담당한다.
- `UCursorItemUseAreaScopeComponent`는 `Context.FocusEngagedHostActor`, `Context.ItemUseEffectTargetObject`, item-use-area hit context를 item action에 전달한다.
- `UItemUseAreaMeshProviderComponent`는 host actor의 direct child actor 안에 있는 `UItemUseAreaMeshComponent`도 descriptor로 수집한다. 따라서 `ABeehive`의 `QueenBeeChildActor`와 `ABeeSwarmClusterActor`의 `QueenBeeChildActor` 안에 있는 여왕벌 use-area를 같은 방식으로 노출할 수 있다.

## 핵심 확정 사항

- 왕롱 포획 결과는 `UItemInstance` optional `FQueenCageItemState`에 저장한다.
- 왕롱 item은 `UQueenCageItemDefinition` 기반이며 `MaxStack=1` invariant를 가진다.
- 왕롱은 `bHasQueen=false/true`만으로 비어 있음/가득 참을 표현하고, 2마리 이상 포획할 수 없다.
- 왕롱에 world actor reference는 저장하지 않는다.
- 왕롱에 저장할 최소 state:
  - `bHasState`
  - `bHasQueen`
  - `CapturedQueenBeeClass`
  - `BaseEggLayingPower`
  - `DiseaseValue`
- 모든 여왕벌 포획용 item-use-area는 `AQueenBeeActor`가 소유한다.
- `QueenCageUseAreaMesh`는 `AQueenBeeActor::QueenBeeMesh` 하위에 attach한다.
- area tag는 `Item.UseArea.QueenBee.QueenCage`다.
- effect target은 `ComponentOwner`로 두어 `Context.ItemUseEffectTargetObject`가 `AQueenBeeActor`가 되게 한다.
- 실제 포획 가능 여부, 여왕벌 제거/비활성화, host 상태 변경은 `IQueenBeeCaptureSource` 구현체가 담당한다.
- `ABeehive`와 `ABeeSwarmClusterActor`가 `IQueenBeeCaptureSource`를 구현한다.
- 왕롱 사용은 drag/rate/progress 없이 유효 여왕벌 영역 위에서 즉시 1회 포획한다.
- 이미 여왕벌이 든 왕롱은 추가 포획을 시작/적용할 수 없다.
- 벌통 여왕벌 포획 시 기존 `ColonyBeeCount`, active comb bee count/target count는 즉시 변경하지 않는다.
- 분봉 본진 최종 완료 조건은 `AllBeesCaptured && bQueenCaptured`다.
- 벌만 전부 포획되면 BeeCarrier use-area와 `AliveRadius`만 완료 처리한다. `bCaptured`/`ReceiveSwarmCaptured` 같은 최종 완료는 여왕벌까지 포획된 뒤 발생한다.
- `Content/` asset은 수정하지 않는다.

## 구현 대상

### 새 파일

- `Source/BeekeepingSim/Public/Inventory/QueenCageItemDefinition.h`
- `Source/BeekeepingSim/Public/Inventory/QueenCageUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/QueenCageUseAction.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeCaptureSource.h`

### 수정 파일

- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`

### 구현 후 문서 갱신

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/Architecture/FocusSystem.md`

## `UQueenCageItemDefinition`

`UItemDefinition` subclass를 추가한다.

권장:

```cpp
UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UQueenCageItemDefinition : public UItemDefinition
{
    GENERATED_BODY()

public:
    UQueenCageItemDefinition()
    {
        MaxStack = 1;
    }
};
```

추가 정적 용량 값은 필요 없다. 왕롱 capacity는 항상 여왕벌 1마리다.

## `FQueenCageItemState`

`UItemInstance`에 optional runtime state를 추가한다.

권장 구조:

```cpp
USTRUCT(BlueprintType)
struct FQueenCageItemState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Queen Cage")
    bool bHasState = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Queen Cage")
    bool bHasQueen = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Queen Cage")
    TSubclassOf<AQueenBeeActor> CapturedQueenBeeClass = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Queen Cage", meta = (ClampMin = "0.0"))
    float BaseEggLayingPower = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Queen Cage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float DiseaseValue = 0.0f;
};
```

필요 include/forward declaration:

- `class AQueenBeeActor;` forward declaration을 우선 사용한다.
- UHT/compile 문제 발생 시 `ItemInstance.h`에서 `WorldActors/QueenBeeActor.h` include로 전환한다.

`UItemInstance` 권장 API:

- `SetQueenCageEmptyState()`
- `SetQueenCageState(const FQueenCageItemState& NewState)`
- `SetCapturedQueenBeeState(TSubclassOf<AQueenBeeActor> QueenClass, float BaseEggLayingPower, float DiseaseValue)`
- `ClearQueenCageState()`
- `HasQueenCageState() const`
- `GetQueenCageState() const`
- `HasCapturedQueen() const`
- `CanAcceptQueenBee() const`

구현 규칙:

- definition이 `UQueenCageItemDefinition`이 아니면 queen cage state를 clear한다.
- `InitializeFromDefinition()`에서 definition이 `UQueenCageItemDefinition`이면 `SetQueenCageEmptyState()`를 호출한다.
- `SetQueenCageEmptyState()`는 `bHasState=true`, `bHasQueen=false`, class null, 수치 0으로 설정한다.
- `SetQueenCageState()`는 `bHasQueen=false`이면 class null, 수치 0으로 sanitize한다.
- `bHasQueen=true`이면 `CapturedQueenBeeClass`는 null이 아니어야 한다. null이면 empty state로 sanitize한다.
- `BaseEggLayingPower`는 `>=0`, `DiseaseValue`는 `0..1`로 clamp한다.
- `CanAcceptQueenBee()`는 definition이 `UQueenCageItemDefinition`이고 state가 있으며 `bHasQueen=false`일 때 true다.
- `CopyRuntimeStateFrom()`은 queen cage state도 복사한다.
- source가 queen cage state를 가지지 않으면 target queen cage state를 clear한다.
- `SetStackCount()`에서 `UQueenCageItemDefinition`도 꿀 용기/BeeCarrier처럼 `MaxStack=1`로 강제한다.

## `ItemStackMoveUtils`

수정 규칙:

- `QueenCageItemDefinition.h` include 추가
- `ResolveMaxStack()`에서 `UQueenCageItemDefinition`은 `1` 반환
- runtime state comparison에 `FQueenCageItemState` 포함
- `HasRuntimeState()`에 `HasQueenCageState()` 포함
- `HasEquivalentRuntimeState()`에 queen cage state equality 포함

권장 equality:

```cpp
return A.bHasState == B.bHasState
    && A.bHasQueen == B.bHasQueen
    && A.CapturedQueenBeeClass == B.CapturedQueenBeeClass
    && FMath::IsNearlyEqual(A.BaseEggLayingPower, B.BaseEggLayingPower, DurabilityStackTolerance)
    && FMath::IsNearlyEqual(A.DiseaseValue, B.DiseaseValue, DurabilityStackTolerance);
```

## `AQueenBeeActor` 변경

`AQueenBeeActor`에 포획용 use-area mesh를 추가한다.

상속:

```cpp
class BEEKEEPINGSIM_API AQueenBeeActor : public AActor, public IItemUseAreaActivationProvider
```

컴포넌트:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
TObjectPtr<UItemUseAreaMeshComponent> QueenCageUseAreaMesh;
```

constructor 규칙:

- `QueenCageUseAreaMesh = CreateDefaultSubobject<UItemUseAreaMeshComponent>(TEXT("QueenCageUseAreaMesh"))`
- `QueenCageUseAreaMesh->SetupAttachment(QueenBeeMesh)`
- `QueenCageUseAreaMesh->SetAreaId(TEXT("QueenBee.QueenCage"))`
- tag `Item.UseArea.QueenBee.QueenCage`를 resolve해서 `SetAreaTags`
- `QueenCageUseAreaMesh->SetEffectTargetPolicy(EItemUseAreaEffectTargetPolicy::ComponentOwner)`
- collision은 기존 item-use-area mesh 패턴과 맞춘다.
  - 기본값은 `NoCollision`이어도 된다. `UCursorItemUseAreaScopeComponent`가 active hit component를 `QueryOnly`와 trace block으로 전환한다.
  - mesh asset, material, relative transform은 BP child/details에서 authoring한다.

activation:

- `AQueenBeeActor`에 `bCaptured` 또는 `bQueenCaptured`를 추가한다.
- `IsCaptured() const`, `SetCaptured(bool bNewCaptured)` 같은 Blueprint API를 추가한다.
- `IsItemUseAreaMeshActive_Implementation()`은 `QueenCageUseAreaMesh`에 대해 `!IsCaptured()`일 때만 true를 반환한다.

state export:

- `FQueenCageItemState MakeQueenCageItemState() const` 또는 유사 API를 추가한다.
- export 값:
  - `bHasState=true`
  - `bHasQueen=true`
  - `CapturedQueenBeeClass=GetClass()`
  - `BaseEggLayingPower=GetBaseEggLayingPower()`
  - `DiseaseValue=GetDiseaseValue()`

주의:

- `AQueenBeeActor`는 자신의 owner를 cast해서 벌통/분봉 본진 상태를 직접 바꾸지 않는다.
- 포획 가능성의 최종 판단과 host state mutation은 `IQueenBeeCaptureSource`가 한다.

## `IQueenBeeCaptureSource`

새 interface 파일 `WorldActors/QueenBeeCaptureSource.h`를 추가한다.

권장:

```cpp
UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UQueenBeeCaptureSource : public UInterface
{
    GENERATED_BODY()
};

class BEEKEEPINGSIM_API IQueenBeeCaptureSource
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Queen Bee|Capture")
    bool CanCaptureQueenBee(AQueenBeeActor* QueenBee) const;

    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Queen Bee|Capture")
    bool CaptureQueenBee(AQueenBeeActor* QueenBee, FQueenCageItemState& OutCapturedState);
};
```

include 의존성:

- interface가 `FQueenCageItemState`를 파라미터로 쓰므로 `Inventory/ItemInstance.h` include가 필요할 수 있다.
- 순환 include가 생기면 `FQueenCageItemState`만 별도 header로 분리하는 대안을 사용한다. 단, 1차 구현은 기존 `ItemInstance.h` optional state 패턴을 유지한다.

## `UQueenCageUseAction`

`UHoldItemUseAction` subclass를 추가한다.

constructor:

- tag query는 `Item.UseArea.QueenBee.QueenCage` all-tags-match

조건:

- `Super::CanBeginUse(Context)` true
- source item instance가 `UQueenCageItemDefinition` 기반
- source item instance `CanAcceptQueenBee() == true`
- target queen은 `Cast<AQueenBeeActor>(Context.ItemUseEffectTargetObject)`
- target queen이 유효하고 captured 아님
- `Context.FocusEngagedHostActor`가 `IQueenBeeCaptureSource` 구현체
- source host의 `CanCaptureQueenBee(QueenBee)` true
- `Context.bHasItemUseAreaHit && Context.ItemUseAreaHitComponent`

`ApplyUseEffect`:

1. source 왕롱 item instance resolve
2. target `AQueenBeeActor` resolve
3. capture source host resolve
4. `CaptureSource->CaptureQueenBee(QueenBee, CapturedState)` 호출
5. 성공하면 `SourceItemInstance->SetQueenCageState(CapturedState)`
6. `Result.bSucceeded=true`
7. item stack/durability는 변경하지 않는다

주의:

- 이 action은 즉시 1회 성공 action이다. `DeltaTime`, drag speed, progress를 사용하지 않는다.
- 첫 tick 성공 후 왕롱이 가득 차므로 같은 hold session에서 재적용되어도 `CanApplyUseEffect()`가 false가 되어야 한다.
- `Result.bConsumedItem=false`, `StackDelta=0`, `DurabilityDelta=0` 유지.

## `ABeehive` 변경

상속:

```cpp
class ABeehive : public AActor, ..., public IQueenBeeCaptureSource
```

state:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Bee")
bool bHasQueenBee = true;
```

권장 API:

- `HasQueenBee() const`
- `SetHasQueenBee(bool bNewHasQueenBee)`
- `CanCaptureQueenBee_Implementation(AQueenBeeActor* QueenBee) const`
- `CaptureQueenBee_Implementation(AQueenBeeActor* QueenBee, FQueenCageItemState& OutCapturedState)`

구현 규칙:

- `EnsureQueenBeeChildActorClass()`는 `bHasQueenBee=false`이면 child class를 null로 만들거나 child actor를 destroy하고 재생성하지 않는다.
- `GetQueenBeeActor()`는 `bHasQueenBee=false`이면 null을 반환한다.
- `UpdateQueenBeeLocation()`은 `bHasQueenBee=false`이면 no-op.
- `CalculateBeeIncreaseAmount()`는 `bHasQueenBee=false`이면 0.
- `IsQueenBeeAttachedToComb()`은 `bHasQueenBee=false`이면 false.
- `TryBrushQueenBeeFromCombVisibleFace()`는 `bHasQueenBee=false`이면 false.
- `CanCaptureQueenBee()`는 전달된 queen이 현재 `QueenBeeChildActor->GetChildActor()`와 같고 `bHasQueenBee=true`일 때만 true.
- `CaptureQueenBee()`는 queen state를 export하고 `bHasQueenBee=false`로 전환한다.
- 포획 성공 후 `QueenBeeChildActor` child actor는 제거/숨김 처리한다.
- 포획 성공 후 `RebuildItemUseAreaDescriptorsIfAvailable()`를 호출해 여왕벌 use-area가 즉시 사라지게 한다.
- 포획 성공 후 `ColonyBeeCount`, active comb bee count/target count, honey state는 변경하지 않는다.

주의:

- `SetHasQueenBee(true)`는 후속 여왕벌 방출/삽입 기능 범위가 아니다. 이번 구현에서 public setter를 만들더라도 true 전환은 최소 동작만 하거나 BP/후속 기능 영역으로 남긴다.

## `ABeeSwarmClusterActor` 변경

현재 `bCaptured` 의미를 바꿔야 한다.

새 state:

```cpp
UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Capture")
bool bBeesCaptured = false;

UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Bee Swarm Cluster|Queen Bee")
bool bQueenCaptured = false;
```

의미:

- `bBeesCaptured`: BeeCarrier로 벌이 모두 포획되었거나 `SpawnAmount <= 0`인 상태
- `bQueenCaptured`: 왕롱으로 분봉 본진 여왕벌이 포획된 상태
- `bCaptured`: 분봉 본진 최종 완료. 반드시 `bBeesCaptured && bQueenCaptured`

권장 API:

- `IsBeesCaptured() const`
- `IsQueenCaptured() const`
- `CanCaptureQueenBee_Implementation(AQueenBeeActor* QueenBee) const`
- `CaptureQueenBee_Implementation(AQueenBeeActor* QueenBee, FQueenCageItemState& OutCapturedState)`
- `HandleBeesCapturedIfNeeded()`
- `HandleSwarmCapturedIfNeeded()`

기존 `HandleCapturedIfNeeded()`는 rename하지 않아도 된다. 단, 내부 의미를 최종 완료 전용으로 명확히 분리한다.

벌 포획 경로 변경:

- `CaptureBees()`는 `bCaptured`가 아니라 `bBeesCaptured`를 기준으로 벌 포획 가능 여부를 판단한다.
- `SetCapturedBeeAmount()` 후 벌이 모두 포획되면:
  - `bBeesCaptured=true`
  - `CapturedBeeAmount=TotalBeeAmount`
  - `AliveRadius=0`
  - cluster Niagara parameter 적용
  - BeeCarrier `CaptureUseAreaMesh` 비활성화
  - item-use-area descriptor rebuild
  - `ReceiveSwarmCaptured()`는 아직 호출하지 않는다
- 이후 `HandleSwarmCapturedIfNeeded()`에서 `bBeesCaptured && bQueenCaptured && !bCaptured`일 때만:
  - `bCaptured=true`
  - `ReceiveSwarmCaptured()` 호출

여왕벌 포획 경로:

- `CanCaptureQueenBee()`는 전달된 queen이 현재 `GetQueenBeeActor()`와 같고 `bQueenCaptured=false`일 때 true.
- `CaptureQueenBee()`는 queen state export 후 `bQueenCaptured=true`로 설정한다.
- 포획 성공 후 queen child actor 제거/숨김 처리.
- 포획 성공 후 item-use-area descriptor rebuild.
- 포획 성공 후 `HandleSwarmCapturedIfNeeded()` 호출.

초기화:

- `InitializeSwarmCluster()`:
  - `bCaptured=false`
  - `bBeesCaptured=false`
  - `bQueenCaptured=false`
  - queen child actor 생성/transform 적용
- `OnConstruction()`/`BeginPlay()`도 sanitize:
  - `bCaptured = bBeesCaptured && bQueenCaptured`처럼 강제하지 말고, 입력값을 해치지 않되 invalid state를 최소 보정한다.
  - `bQueenCaptured=true`이면 queen child를 재생성하지 않는다.
  - `bBeesCaptured=true` 또는 `bCaptured=true`이면 BeeCarrier use-area 비활성.

`IsItemUseAreaMeshActive_Implementation()`:

- BeeCarrier `CaptureUseAreaMesh`는 `!bBeesCaptured`일 때만 active.
- 기존처럼 `!IsCaptured()`를 쓰면 벌만 남았는지/여왕벌만 남았는지 상태를 구분하지 못한다.

주의:

- `ReceiveSwarmCaptured`는 최종 완료 이벤트로 유지한다.
- 벌만 완료된 이벤트가 필요하면 `ReceiveSwarmBeesCaptured` BlueprintImplementableEvent를 새로 추가해도 된다. 필수는 아니다.
- `DecreaseAliveRadius` legacy/manual visual API는 삭제/rename하지 않는다. 단, 이 API가 최종 완료를 강제로 발생시키면 안 된다. 필요한 경우 벌 완료 처리까지만 연결하거나 legacy visual adjustment로 유지한다.

## `QueenBeeChildActor` 제거/숨김 방식

권장 우선순위:

1. `UChildActorComponent::SetChildActorClass(nullptr)` 또는 child actor destroy 방식으로 실제 child actor를 제거한다.
2. `bHasQueenBee=false` 또는 `bQueenCaptured=true` 상태에서 `EnsureQueenBeeChildActorClass()`가 다시 class를 세팅하지 않게 한다.
3. 단순 `SetHiddenInGame(true)`만 사용하지 않는다. use-area collision/descriptor가 남을 수 있기 때문이다.

구현 편의상 child actor 제거 API가 까다로우면:

- child actor를 `SetActorHiddenInGame(true)`, `SetActorEnableCollision(false)`, `SetActorTickEnabled(false)` 처리하고,
- `AQueenBeeActor::SetCaptured(true)`로 `QueenCageUseAreaMesh` inactive 처리,
- host descriptor rebuild를 반드시 호출한다.

하지만 최종적으로는 재생성 차단 state가 source of truth여야 한다.

## 문서 반영

구현 후 다음 내용을 문서에 반영한다.

### `.md/0_ARCHITECTURE.md`

- Inventory가 `FQueenCageItemState`를 optional runtime state로 보존한다.
- `UQueenCageItemDefinition`, `UQueenCageUseAction` 추가.
- `AQueenBeeActor`가 `QueenBeeMesh` 하위 `QueenCageUseAreaMesh`로 왕롱 use-area를 제공한다.
- `ABeehive`와 `ABeeSwarmClusterActor`가 `IQueenBeeCaptureSource`로 여왕벌 포획 host mutation을 처리한다.
- 분봉 본진 최종 완료 조건이 벌 전량 포획 + 여왕벌 포획이다.

### `.md/Architecture/InventorySystem.md`

- `FQueenCageItemState`
- `UQueenCageItemDefinition`
- `UQueenCageUseAction`
- `UItemInstance` queen cage state API
- runtime state copy/move compatibility
- 왕롱 `MaxStack=1`

### `.md/Architecture/WorldActorsSystem.md`

- `AQueenBeeActor::QueenCageUseAreaMesh`
- `IQueenBeeCaptureSource`
- `ABeehive`의 `bHasQueenBee`와 여왕벌 포획 후 산란/위치 갱신 비활성
- `ABeeSwarmClusterActor`의 `bBeesCaptured`, `bQueenCaptured`, 최종 `bCaptured`
- `ReceiveSwarmCaptured`가 최종 완료 이벤트라는 점

### `.md/Architecture/FocusSystem.md`

- 새 Focus 시스템은 추가하지 않는다.
- 기존 item-use-area provider/scope 경로로 queen cage use-area가 수집된다.

## 수정하면 안 되는 것

- 기존 UCLASS/USTRUCT/UENUM rename 금지
- 기존 BlueprintCallable API 삭제/rename 금지
- `ABeeSwarmClusterActor::DecreaseAliveRadius` 삭제/rename 금지
- 새 Focus 시스템/입력 경로 추가 금지
- `Content/` asset 저장 금지
- 왕롱 포획 시 `ColonyBeeCount`, active comb bee count/target count 즉시 변경 금지
- BeeCarrier 벌 포획량/부피 기반 `AliveRadius` 공식 변경 금지
- 자동 분봉 발생 조건 추가 금지
- 여왕벌 방출/재배치/왕롱 교체 기능 추가 금지

## 검증

공백/패치 검증:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

UBT 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

검색 검증:

```powershell
rg -n "FQueenCageItemState|UQueenCageItemDefinition|UQueenCageUseAction|IQueenBeeCaptureSource|QueenCageUseAreaMesh|Item.UseArea.QueenBee.QueenCage|bHasQueenBee|bBeesCaptured|bQueenCaptured" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg -n "ReceiveSwarmCaptured|HandleCapturedIfNeeded|IsCaptured\\(|SetCaptureUseAreaActive|CaptureBees" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

두 번째 검색은 분봉 본진의 벌 포획 완료와 최종 완료 조건이 분리되었는지 확인하기 위한 것이다.

## 수동 PIE 확인

1. 왕롱 DataAsset parent를 `UQueenCageItemDefinition`으로 만들고 `UQueenCageUseAction`을 action spec에 추가한다.
2. 왕롱 DataAsset에 gameplay tag가 필요하면 `Item.QueenCage`를 부여한다.
3. `AQueenBeeActor` BP child에서 `QueenCageUseAreaMesh` mesh/material/relative transform이 `QueenBeeMesh` 하위에서 여왕벌 주변을 덮는지 확인한다.
4. 벌통 FocusEngaged 상태에서 왕롱으로 벌통 여왕벌을 포획하면 왕롱 `bHasQueen=true`가 되고 벌통 `bHasQueenBee=false`가 되는지 확인한다.
5. 벌통 여왕벌 포획 후 colony population 증가량이 0이 되는지 확인한다.
6. 벌통 여왕벌 포획 후 queen location bucket/update가 여왕벌을 재생성하지 않는지 확인한다.
7. 이미 여왕벌이 든 왕롱으로 다른 여왕벌을 포획할 수 없는지 확인한다.
8. 분봉 본진에서 BeeCarrier로 벌을 모두 포획해도 `ReceiveSwarmCaptured`가 아직 호출되지 않는지 확인한다.
9. 분봉 본진에서 벌이 모두 포획된 뒤 왕롱으로 여왕벌을 포획하면 그때 `ReceiveSwarmCaptured`가 1회만 호출되는지 확인한다.
10. 분봉 본진에서 여왕벌을 먼저 포획하고 나중에 벌을 모두 포획해도 최종 완료가 1회만 발생하는지 확인한다.
11. hotbar/storage 이동 후 왕롱의 `FQueenCageItemState`가 유지되는지 확인한다.
12. `Content/` asset은 필요한 BP/DataAsset 수동 authoring 외에는 C++ 구현 중 저장하지 않는다.

## 최종 보고 요구사항

- 변경 파일
- 새 USTRUCT/UCLASS/interface 목록
- 추가/변경 Blueprint API
- 왕롱 item state 저장/복사 경로
- `AQueenBeeActor` item-use-area 구성
- `IQueenBeeCaptureSource` 구현 방식
- 벌통 여왕벌 포획 후 `bHasQueenBee` 영향 범위
- 분봉 본진 `bBeesCaptured`, `bQueenCaptured`, `bCaptured` 상태 전이
- `ReceiveSwarmCaptured` 발생 조건
- 아키텍처 문서 반영 내용
- 빌드 결과 또는 미수행 사유
- 필요한 수동 BP/Content 작업 목록
