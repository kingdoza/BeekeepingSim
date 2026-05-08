# FocusEngaged Item Use Area 구현 프롬프트

## 목표

FocusEngaged 상태의 host actor가 선택적으로 제공하는 **item-use-area** 시스템을 구현한다.

이 기능은 벌통 전용이 아니다. `ABeehive`는 첫 구현 host일 뿐이며, 이후 다른 FocusEngaged actor도 같은 구조를 재사용할 수 있어야 한다.

핵심 요구사항:

- FocusEngaged host가 item-use-area scope/provider를 지원하고 대상 아이템이 선택되어 있으면, 해당 아이템에 대응되는 사용영역은 LMB 조작 여부와 무관하게 항상 표시/점멸한다.
- FocusEngaged host가 item-use-area scope/provider를 지원하지 않으면, 선택 아이템이 있더라도 기존 FocusAction/PartFocus 입력 정책을 따른다.
- 선택 아이템이 있고 host가 item-use-area를 지원하면 LMB는 item-use action으로 처리한다.
- 선택 아이템이 없으면 기존 FocusAction/PartFocus 입력 정책을 따른다.
- LMB Press/Hold/Release는 "아이템 사용중" 세션과 "실질 아이템사용효과" 적용 여부만 제어한다.
- 실질 아이템사용효과는 LMB hold 중 커서가 현재 item에 대응되는 active use area 위에 있을 때만 적용한다.
- item-use area 활성 중에는 PartFocus outline보다 item-use area 표시를 우선한다. 결정된 정책 기준으로 선택 아이템이 있을 때 PartFocus outline은 숨긴다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

## 구현 범위

주요 변경 시스템:

- Focus: item-use-area scope, descriptor, provider, FocusAction input routing
- Inventory: hold item-use action lifecycle와 item action query
- Character: LMB Started/Completed 입력 라우팅
- WorldActors: `ABeehive` first host integration
- 문서: 구현 후 architecture 문서 최신화

예상 신규 파일:

- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaProvider.h`
- `Source/BeekeepingSim/Public/Inventory/HoldItemUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/HoldItemUseAction.cpp`

예상 수정 파일:

- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `Source/BeekeepingSim/Public/Focus/BeekeeperFocusComponent.h`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemActionContext.h`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

문서 반영 대상:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/USER_UNREAL.md`

## 설계 기준

### Generic naming

벌통 전용 이름을 만들지 않는다.

사용할 naming:

- `UCursorItemUseAreaScopeComponent`
- `FItemUseAreaDescriptor`
- `IItemUseAreaProvider`
- `UHoldItemUseAction`

`BeehiveItemUseArea`, `BeehiveUseAreaScope` 같은 이름은 사용하지 않는다.

### 시스템 책임

- Focus
  - FocusEngaged host 내부 item-use-area scope 활성/비활성
  - 선택 item 기반 active area filter
  - 커서 hit/hover 판정
  - LMB Press/Hold/Release routing
  - PartFocus outline과 item-use area 표시 우선순위 제어
- Inventory
  - 선택 item의 hold-use action 제공
  - 사용 가능한 area tag query 제공
  - Begin/Hold/End lifecycle와 실질 효과 실행 owner
- WorldActors
  - host actor와 child actor가 use area descriptor 제공
  - `ABeehive`는 첫 host로 provider/scope 연결
- Character
  - Enhanced Input Started/Completed를 Focus component로 전달

## 데이터 구조

### `FItemUseAreaVisualSettings`

`CursorItemUseAreaTypes.h`에 추가한다.

필드 후보:

```cpp
USTRUCT(BlueprintType)
struct FItemUseAreaVisualSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    FLinearColor UseAreaColor = FLinearColor(0.2f, 0.8f, 1.0f, 0.35f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area", meta = (ClampMin = "0.0"))
    float UseAreaOpacity = 0.35f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area", meta = (ClampMin = "0.0"))
    float PulseSpeed = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HoverStrength = 1.0f;
};
```

### `FItemUseAreaDescriptor`

`CursorItemUseAreaTypes.h`에 추가한다.

```cpp
USTRUCT(BlueprintType)
struct FItemUseAreaDescriptor
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    FName AreaId;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    FGameplayTagContainer AreaTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    TObjectPtr<AActor> OwnerActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    TObjectPtr<UPrimitiveComponent> HitComponent = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    TArray<TObjectPtr<UPrimitiveComponent>> VisualComponents;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    TObjectPtr<UObject> EffectTargetObject = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Use Area")
    FItemUseAreaVisualSettings VisualSettings;
};
```

`EffectTargetObject`는 item action이 실질 효과를 적용할 대상이다. 예:

- host actor 자체
- host child actor
- 전용 component
- `ABeehiveCombActor`

## Provider

### `IItemUseAreaProvider`

`Focus/ItemUseAreaProvider.h`에 `UINTERFACE(BlueprintType)`로 추가한다.

권장 API:

```cpp
UINTERFACE(BlueprintType)
class BEEKEEPINGSIM_API UItemUseAreaProvider : public UInterface
{
    GENERATED_BODY()
};

class BEEKEEPINGSIM_API IItemUseAreaProvider
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Item Use Area")
    void GetItemUseAreaDescriptors(TArray<FItemUseAreaDescriptor>& OutDescriptors) const;
};
```

주의:

- `BlueprintNativeEvent`를 사용해 C++ host와 Blueprint child actor 모두 구현 가능하게 한다.
- `FItemUseAreaDescriptor`는 Focus system type이므로 `ItemUseAreaProvider.h`는 Focus 폴더에 둔다.
- UObject/UCLASS rename이 아니므로 Core Redirect는 필요 없다.

## Item action 확장

### `UHoldItemUseAction`

`Inventory/HoldItemUseAction.h/cpp`를 추가하고 `UItemAction`을 상속한다.

역할:

- 특정 item이 어떤 area tag에서 사용 가능한지 제공
- LMB Press/Hold/Release lifecycle 처리
- hover area 위에서만 실질 효과 실행

권장 API:

```cpp
UCLASS(Abstract, Blueprintable, EditInlineNew, DefaultToInstanced)
class BEEKEEPINGSIM_API UHoldItemUseAction : public UItemAction
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category = "Item Action|Use Area")
    virtual FGameplayTagQuery GetUseAreaTagQuery() const;

    UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
    virtual bool CanBeginUse(const FItemActionContext& Context) const;

    UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
    virtual bool BeginUse(const FItemActionContext& Context);

    UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
    virtual void TickUse(const FItemActionContext& Context, float DeltaTime);

    UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
    virtual void EndUse(const FItemActionContext& Context, bool bWasCanceled);

    UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
    virtual bool CanApplyUseEffect(const FItemActionContext& Context) const;

    UFUNCTION(BlueprintCallable, Category = "Item Action|Use Area")
    virtual FItemActionExecutionResult ApplyUseEffect(const FItemActionContext& Context, float DeltaTime);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action|Use Area")
    FGameplayTagQuery UseAreaTagQuery;
};
```

Blueprint hook가 필요하면 `BlueprintImplementableEvent` 또는 `BlueprintNativeEvent` wrapper를 추가해도 된다. 단, C++ lifecycle 이름은 위 기준을 유지한다.

### `UItemInstance` helper

선택 item에서 hold action을 찾기 위한 helper를 추가한다.

권장:

```cpp
UFUNCTION(BlueprintPure, Category = "Item")
UHoldItemUseAction* FindHoldItemUseAction() const;
```

여러 hold action이 있을 수 있는 구조가 필요하면 `GetHoldItemUseActions()`도 가능하다. 1차 구현은 선택 item당 대표 hold action 1개를 우선한다. 여러 개이거나 우선순위가 애매하면 `.md/QNA_IMPLEMENTATION.md`에 질문하고 중단한다.

## Item action context 확장

`FItemActionContext`에 item-use-area용 context를 추가한다.

후보 필드:

```cpp
UPROPERTY(BlueprintReadWrite, Category = "Item Action")
TObjectPtr<AActor> FocusEngagedHostActor = nullptr;

UPROPERTY(BlueprintReadWrite, Category = "Item Action")
FName ItemUseAreaId;

UPROPERTY(BlueprintReadWrite, Category = "Item Action")
FGameplayTagContainer ItemUseAreaTags;

UPROPERTY(BlueprintReadWrite, Category = "Item Action")
TObjectPtr<UPrimitiveComponent> ItemUseAreaHitComponent = nullptr;

UPROPERTY(BlueprintReadWrite, Category = "Item Action")
TObjectPtr<UObject> ItemUseEffectTargetObject = nullptr;
```

기존 `FocusTarget`, `Character`, `PlayerController`, `World` 필드는 유지한다.

## Scope component

### `UCursorItemUseAreaScopeComponent`

Focus system에 추가한다.

역할:

- FocusEngaged host에서만 활성화
- host/provider에서 `FItemUseAreaDescriptor` 수집
- 선택 item의 `UHoldItemUseAction::GetUseAreaTagQuery()`로 active area filter 갱신
- active area visual 표시/점멸
- cursor trace로 hover area 갱신
- LMB Press/Hold/Release item-use session 관리
- 실질 effect는 item action에 위임

권장 public API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Item Use Area")
void ActivateItemUseAreaScope(ABeekeeperCharacter* InteractingCharacter);

UFUNCTION(BlueprintCallable, Category = "Item Use Area")
void DeactivateItemUseAreaScope(bool bCancelActiveUse);

UFUNCTION(BlueprintCallable, Category = "Item Use Area")
void RebuildItemUseAreaDescriptors();

UFUNCTION(BlueprintCallable, Category = "Item Use Area")
void RegisterItemUseAreaDescriptor(const FItemUseAreaDescriptor& Descriptor);

UFUNCTION(BlueprintCallable, Category = "Item Use Area")
bool HandleItemUsePressed();

UFUNCTION(BlueprintCallable, Category = "Item Use Area")
bool HandleItemUseReleased();

UFUNCTION(BlueprintPure, Category = "Item Use Area")
bool IsItemUseAreaScopeActive() const;

UFUNCTION(BlueprintPure, Category = "Item Use Area")
bool IsItemUseInProgress() const;

UFUNCTION(BlueprintPure, Category = "Item Use Area")
bool HasActiveUseAreas() const;
```

권장 private state:

- owner character
- hotbar component
- focus component
- active host actor
- registered descriptors
- active descriptor indices
- hovered descriptor index
- selected item instance cache
- active hold action
- `bIsUseInProgress`

### Activation

`ActivateItemUseAreaScope`:

1. `InteractingCharacter` 저장
2. hotbar/focus component 캐시
3. current engaged host actor resolve
4. `RebuildItemUseAreaDescriptors()`
5. hotbar changed delegate 구독
6. selected item 기준 active area filter 갱신
7. tick 활성화

`DeactivateItemUseAreaScope`:

1. active use session이 있으면 `EndUse(..., bWasCanceled=true)` 호출
2. 모든 active/hover visual 끄기
3. delegates 해제
4. caches clear
5. tick 비활성화

### Descriptor rebuild

`RebuildItemUseAreaDescriptors`는 다음을 수행한다.

1. 기존 descriptor clear
2. host actor가 `IItemUseAreaProvider` 구현 시 descriptor 수집
3. host actor의 child actor들을 순회해 `IItemUseAreaProvider` 구현 actor에서 descriptor 수집
4. host actor의 direct primitive components를 component tag 기반으로 descriptor로 수집

Direct component tag scan은 1차 구현에서 최소 규칙만 둔다.

권장:

- component tag가 `ItemUseArea`이면 사용영역 후보
- area tag는 별도 component tag naming에서 추출하기 어렵기 때문에, C++ host provider 경로를 우선한다.
- direct scan만으로 area tags를 안정적으로 구성하기 어렵다면 `ABeehive`에서 provider 구현으로 직접 descriptor를 만든다.

중요:

- 사용영역 mesh는 기존 gameplay mesh, 반투명 가상 mesh, child actor 내부 mesh 모두 허용한다.
- 최종 처리 기준은 descriptor의 `HitComponent`, `VisualComponents`, `EffectTargetObject`다.
- descriptor가 유효하지 않으면 skip한다.

### Active area filter

선택 item 변경 시 descriptor rebuild를 하지 않는다. 기존 descriptor를 유지하고 active area filter만 갱신한다.

조건:

- host scope active
- selected item exists
- selected item has `UHoldItemUseAction`
- hold action의 `UseAreaTagQuery`가 descriptor `AreaTags`와 match

선택 item이 없거나 hold action이 없으면 active area는 모두 비활성화한다.

### Visual control

Active area는 LMB와 무관하게 표시/점멸한다.

공통 material parameter:

- `UseAreaColor`
- `UseAreaOpacity`
- `PulseSpeed`
- `HoverStrength`

구현 기준:

- active descriptor의 `VisualComponents`에 visibility true
- active descriptor의 `HitComponent`는 query collision enabled
- inactive descriptor는 visibility false, query collision disabled
- hover descriptor는 `HoverStrength`를 1.0, non-hover active descriptor는 0.0
- material index 0에 dynamic material instance를 만들어 parameter 적용
- component가 여러 material slot을 쓰는 경우도 1차 구현은 index 0만 처리한다.

주의:

- 기존 gameplay mesh를 VisualComponent로 쓰는 경우 원래 material을 바꿀 수 있다. 가능하면 가상 반투명 mesh를 visual로 쓰는 경로를 권장한다.
- 실제 gameplay mesh를 hit-only로 쓰고 visual은 별도 mesh로 두는 descriptor 구성이 가능해야 한다.

### Cursor hit/hover

커서 trace:

- 기존 visibility trace 사용
- hit component가 active descriptor의 `HitComponent`인지 검사
- 여러 후보가 있으면 trace hit result에서 가장 가까운 active area component 1개 사용

유효 hover:

- scope active
- selected item has hold action
- hovered descriptor active
- descriptor area tag가 action query와 match

### LMB lifecycle

`HandleItemUsePressed`:

- scope inactive면 false
- host가 item-use-area를 지원하지 않으면 false
- selected item 없음이면 false
- selected item에 hold action 없음이면 false
- action `CanBeginUse(Context)` false면 true를 반환해 입력은 소비하되 session은 시작하지 않는다.
- BeginUse 성공 시 `bIsUseInProgress=true`
- 매칭 사용영역이 없어도 session은 시작 가능하다. 단, 실질 effect는 없음.

`TickComponent`:

- selected item 변경 감지 후 active area filter 갱신
- cursor hover 갱신
- active use session이면 action `TickUse(Context, DeltaTime)` 호출
- active use session이고 valid hovered area가 있으면 `CanApplyUseEffect` 확인 후 `ApplyUseEffect(Context, DeltaTime)` 호출

`HandleItemUseReleased`:

- session이 있으면 `EndUse(Context, false)` 호출
- session clear
- true 반환

Deactivate/Abort:

- session이 있으면 `EndUse(Context, true)` 호출

## Focus input routing

### Character

현재 `PartFocusClickAction`은 `Started`만 바인딩되어 있다.

변경:

- `Started` -> `ABeekeeperCharacter::PartFocusClickInput` 또는 새 `ItemUsePressedInput`
- `Completed` -> 새 `ABeekeeperCharacter::PartFocusClickReleaseInput` 또는 `ItemUseReleasedInput`

권장:

- 기존 `PartFocusClickInput()`은 유지한다.
- 새 함수 `PartFocusClickReleaseInput()`을 추가한다.
- 내부에서 `BeekeeperFocus->HandlePartFocusClickReleasedInput()` 같은 새 API를 호출한다.

기존 Blueprint-exposed property 이름은 변경하지 않는다. `PartFocusClickAction` 자체를 계속 LMB action으로 사용한다.

### `UBeekeeperFocusComponent`

추가 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Focus")
bool HandlePartFocusClickReleasedInput();
```

`HandlePartFocusClickInput()` 동작 변경:

1. engaged focus action이 있으면 action의 `HandlePartFocusClickInputWhileEngaged` 호출
2. action이 item-use-area supported host를 찾아 item-use pressed를 우선 처리
3. 처리되지 않으면 기존 PartFocus click fallback

release도 engaged action에 위임한다.

### `UFocusActionComponent`

release hook 추가:

```cpp
UFUNCTION(BlueprintCallable, Category = "Focus Action")
virtual bool HandlePartFocusClickReleasedInputWhileEngaged(ABeekeeperCharacter* InteractingCharacter);
```

기본 구현은 `false`.

기존 API 삭제/rename 금지.

### `UAnchoredFocusCursorActionComponent`

현재 anchored cursor action은 owner actor에서 `UCursorPartFocusScopeComponent`를 찾아 PartFocus 입력을 처리한다.

변경:

- BeginFocusAction 시 owner actor의 `UCursorItemUseAreaScopeComponent`가 있으면 activate
- ReturnCompleted/Abort 시 deactivate
- click pressed:
  1. owner actor의 `UCursorItemUseAreaScopeComponent`가 active이고 selected item이 있으면 `HandleItemUsePressed()`를 우선 호출
  2. 그 결과 true면 PartFocus click은 실행하지 않음
  3. false면 기존 `CursorPartFocusScope->HandlePartFocusClickInput()` 실행
- click released:
  1. item-use-area scope가 active이면 `HandleItemUseReleased()` 호출
  2. true면 consume
  3. 아니면 false

host가 item-use-area scope를 갖지 않으면 기존 PartFocus behavior가 유지되어야 한다.

### PartFocus outline 숨김

결정 정책:

- item-use-area를 지원하는 host에서 선택 item이 있으면 PartFocus outline은 숨긴다.

구현 후보:

- `UCursorPartFocusScopeComponent`에 `SetPreviewSuppressed(bool)` 또는 `SetHoverOutlineSuppressed(bool)` 추가
- `UCursorItemUseAreaScopeComponent`가 selected item/active state 변경 시 sibling `UCursorPartFocusScopeComponent`에 suppression 전달

주의:

- PartFocus action stack cancel/abort 기능은 유지한다.
- outline/prompt만 숨기는지, hover resolve 자체를 중단하는지 구현 전에 판단한다.
- 추천: 1차 구현은 hover outline/prompt를 숨기고, 기존 cancel cascade 등 상태 처리는 유지한다.

애매하면 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

## `ABeehive` first host integration

`ABeehive`에 `UCursorItemUseAreaScopeComponent`를 추가한다.

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Item Use Area")
TObjectPtr<UCursorItemUseAreaScopeComponent> ItemUseAreaScope;
```

`ABeehive`가 `IItemUseAreaProvider`를 구현할지, 별도 provider component를 붙일지 선택한다.

권장:

- 1차 구현은 `ABeehive`가 `IItemUseAreaProvider`를 직접 구현한다.
- 이유: 이미 `ABeehive`가 active comb membership과 child actor 구성 정보를 알고 있다.

Provider에서 등록할 descriptor 예:

- lid area
  - `AreaId = "Lid"`
  - `AreaTags = Beehive.UseArea.Lid`
  - `EffectTargetObject = this`
  - `HitComponent`: 기존 lid mesh 또는 반투명 가상 mesh
  - `VisualComponents`: 반투명 가상 mesh 권장
- comb area
  - `AreaId = "Comb_{Index}"`
  - `AreaTags = Beehive.UseArea.Comb`
  - `EffectTargetObject = ABeehiveCombActor`
  - `HitComponent`: comb actor 또는 child actor provider가 제공한 component
  - `VisualComponents`: comb actor 내부 가상 mesh 권장

기존 벌통에 아직 가상 use area mesh가 없다면:

- C++에서 필수 mesh component를 무리하게 추가하지 않는다.
- provider는 component tag 기반으로 찾을 수 있는 component가 있을 때만 descriptor를 만든다.
- 필요한 Blueprint/Editor 작업은 `.md/USER_UNREAL.md` 또는 최종 보고에 명시한다.

## GameplayTag 기준

1차 구현에서 native tag 등록까지 진행할지 확인한다.

권장 tag:

- `Beehive.UseArea.Lid`
- `Beehive.UseArea.Comb`
- `Beehive.UseArea.Inner`

이미 프로젝트에 GameplayTags 설정/매니저 패턴이 있으면 기존 방식 사용.

태그 등록 위치가 불명확하면 `.md/QNA_IMPLEMENTATION.md`에 질문하고 중단한다.

## Blueprint / Editor 작업 문서화

Content asset을 직접 수정하지 않는다.

구현 후 사용자가 Editor에서 해야 할 작업을 `.md/USER_UNREAL.md` 또는 최종 보고에 작성한다.

예상 작업:

1. `BP_Beehive` 또는 child Blueprint에 item-use area용 반투명 mesh component 추가
2. component tag 또는 provider 설정으로 `AreaId`, `AreaTags` 연결
3. item-use area material에 parameter 추가
   - `UseAreaColor`
   - `UseAreaOpacity`
   - `PulseSpeed`
   - `HoverStrength`
4. item definition/action에 `UHoldItemUseAction` subclass 추가
5. 해당 action의 `UseAreaTagQuery` 설정
6. Blueprint compile/save

## 구현 단계

### 1단계: 타입과 action 기반

- `CursorItemUseAreaTypes.h`
- `ItemUseAreaProvider.h`
- `HoldItemUseAction.h/cpp`
- `ItemActionContext.h` 확장
- `ItemInstance` hold action lookup 추가

이 단계는 빌드 가능해야 한다.

### 2단계: scope component

- `UCursorItemUseAreaScopeComponent` 구현
- descriptor 등록/수집
- selected item filter
- visual on/off
- cursor hover
- LMB lifecycle

이 단계에서 host integration 없이도 component 단위로 빌드 가능해야 한다.

### 3단계: Focus routing

- Character release input 추가
- `UBeekeeperFocusComponent` release API 추가
- `UFocusActionComponent` release hook 추가
- `UAnchoredFocusCursorActionComponent`에서 item-use scope 우선 처리
- PartFocus outline suppression 구현

기존 PartFocus R/F/C 입력은 변경하지 않는다.

### 4단계: Beehive integration

- `ABeehive`에 scope component 추가
- `IItemUseAreaProvider` 구현 또는 provider component 연결
- 기존 direct component/child actor provider descriptor 수집 경로 연결
- 가상 mesh가 없는 경우 no-op으로 동작하게 한다.

### 5단계: 문서/검증

- architecture 문서 업데이트
- `.md/USER_UNREAL.md`에 필요한 Editor 작업 작성
- UBT 빌드

## 검증 기준

### 빌드

가능하면 아래 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

### 코드 검증

- `#include "Public/..."` 형태가 없어야 한다.
- 기존 Blueprint native parent class rename 없음
- 기존 `UCursorPartFocusScopeComponent`, `UCursorPartFocusActionComponent`, `UItemAction` public API 삭제 없음
- `PartFocusClickAction` property rename 없음
- host가 item-use-area scope를 갖지 않는 경우 기존 PartFocus click 동작 유지
- selected item이 없으면 기존 FocusAction/PartFocus 입력 정책 유지
- selected item이 있고 host가 scope를 지원하면 LMB press는 item-use action 우선
- LMB release 시 active session 정리
- Focus cancel/abort/end play 시 active session abort
- active area visual은 LMB와 무관하게 selected item 기준으로 표시
- hovered active area 위에서만 `ApplyUseEffect` 호출
- 여러 active area가 겹치면 가장 가까운 hit만 effect 대상

### 수동 검증

Editor 작업 후 확인할 항목:

1. item-use-area 미지원 FocusEngaged actor에서 기존 조작이 유지된다.
2. 벌통 FocusEngaged + 빈손 상태에서 기존 PartFocus click이 동작한다.
3. 벌통 FocusEngaged + item-use action이 있는 아이템 선택 시 대응 area가 LMB와 무관하게 점멸한다.
4. 선택 item 변경 시 active area가 갱신된다.
5. LMB hold 중 area 밖에서는 사용중 연출만 유지되고 실질 효과는 발생하지 않는다.
6. LMB hold 중 area 위에서는 실질 효과가 Tick 기반으로 호출된다.
7. Focus cancel/abort 시 area visual과 item-use session이 정리된다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- GameplayTag 등록 위치/방식이 불명확하다.
- item 하나에 hold action이 여러 개일 때 우선순위가 필요하다.
- PartFocus outline suppression을 hover resolve까지 막아야 하는지 불명확하다.
- Blueprint asset migration 없이는 C++ 빌드/동작 보장이 어려운 경우.
- UCLASS/USTRUCT/UENUM rename 또는 file rename이 필요해 Core Redirect 검토가 필요한 경우.
- 기존 public Blueprint API 삭제/변경이 필요해 보이는 경우.

## 주의사항

- Content asset은 직접 수정하지 않는다.
- Config 변경은 GameplayTag 또는 trace channel 때문에 꼭 필요할 때만 수행하고, 필요하면 먼저 QnA에 질문한다.
- 전용 collision channel은 이번 결정에서 선택하지 않았다. 기존 visibility trace + active descriptor component filter를 사용한다.
- item-use-area는 generic Focus 기능이다. 벌통 전용 조건을 Focus/Inventory 타입 이름에 넣지 않는다.
- 실제 게임 상태 변경은 item action/effect 호출 경계 안에 둔다. 이후 authority/RPC로 감쌀 수 있게 scope가 직접 도메인 상태를 바꾸지 않는다.
