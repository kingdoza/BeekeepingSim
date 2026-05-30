# Pollen Patty Fixed Consumption 구현 프롬프트

## 목표

벌통(`ABeehive`)에 배치된 화분떡을 시간 bucket마다 고정량 소모한다.

이번 작업은 이미 구현된 placed item durability remaining 시스템을 전제로 한다. 즉, 화분떡의 잔량은 `UPlacedItemRemainingComponent`가 소유하고, 소모는 `ConsumeAmount(...)` 호출로 처리한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

특히 `.md/QNA_ARCHITECTURE.md`의 `화분떡 고정 소모 로직 설계 QnA` 22-26번 답변을 따른다.

## 전제

- `APlacedItemActor`는 `UPlacedItemRemainingComponent`를 기본 subobject로 가진다.
- 화분떡은 전용 actor가 아니라 generic `APlacedItemActor`로 배치된다.
- 화분떡 item definition은 durability 기반 placed remaining을 사용한다.
- 화분떡 slot은 `AItemPlacementSlotActor` 계열 child actor로 벌통에 배치된다.
- `Content/` asset은 직접 수정하거나 resave하지 않는다.

위 전제가 현재 Source와 맞지 않으면 구현하지 말고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

## 명시적 제외 범위

- colony population, 산란력, 수명, 온도, 벌 수에 따른 소모량 보정 추가 금지
- 화분떡이 colony/honey 생산에 주는 효과 추가 금지
- 여러 화분떡에 소모량 분배 금지
- 선택된 화분떡이 부족할 때 남은 소모량을 다음 화분떡으로 넘기는 처리 금지
- `Item.UseArea.Beehive.PollenPatty` 태그 문자열을 `ABeehive` 소모 대상 탐색 로직에 하드코딩 금지
- `Content/` 수정 금지

## 확정 정책

1. 소모량은 벌통별 고정값 `PollenPattyConsumptionAmountPerBucket`만 사용한다.
2. 기본 소모량은 `1.0f`다.
3. 기본 bucket은 `PollenPattyConsumptionBucketMinutes=60`이다.
4. BeginPlay 즉시 소모는 기본 비활성화한다.
5. bucket 길이는 소모 주기만 바꾸며, 소모량을 시간 비율로 스케일하지 않는다.
6. 여러 화분떡이 있으면 한 bucket에서 하나만 소모한다.
7. 기본 선택 방향은 `Leftmost`다.
8. `Leftmost`/`Rightmost` 판정은 벌통 local Y 기준이다.
9. 소모 대상 위치 비교는 occupied actor가 아니라 slot actor 위치를 벌통 local space로 변환해 사용한다.
10. 같은 local Y 값 tie는 먼저 수집된 slot을 유지한다.
11. 소모 대상은 direct child `AItemPlacementSlotActor` 계열 slot 중에서 찾는다.
12. slot `AreaTags`가 `ABeehive::PollenPattyConsumptionAreaTags`를 모두 포함해야 소모 후보가 된다.
13. `PollenPattyConsumptionAreaTags`가 비어 있으면 소모 대상이 없는 것으로 처리한다.
14. occupied actor에 active `UPlacedItemRemainingComponent`가 없으면 후보에서 제외한다.
15. remaining current amount가 0 이하인 actor는 후보에서 제외한다.
16. 선택된 target에만 `ConsumeAmount(PollenPattyConsumptionAmountPerBucket)`를 호출한다.
17. 선택된 target이 소진되어 제거되더라도 같은 bucket에서 다른 화분떡으로 남은 소모량을 넘기지 않는다.

## 주요 구현 대상

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

## `AItemPlacementSlotActor` 변경

`ABeehive`가 slot의 configured area tags를 descriptor 경유 없이 읽을 수 있어야 한다.

`AItemPlacementSlotActor`에 public getter를 추가한다.

```cpp
UFUNCTION(BlueprintPure, Category = "Item Placement Slot")
FGameplayTagContainer GetSlotAreaTags() const;
```

구현 정책:

- source of truth는 `SlotMeshComponent->GetAreaTags()`다.
- `SlotMeshComponent`가 없으면 deprecated `AreaTags`를 fallback으로 반환해도 된다.
- active descriptor의 `AreaTags`를 사용하지 않는다.

이 getter는 Public API 추가이며 기존 API 삭제/rename이 아니므로 Core Redirect가 필요하지 않다.

## `ABeehive` public API 추가

`Beehive.h`에 enum을 추가한다.

```cpp
UENUM(BlueprintType)
enum class EPollenPattyConsumptionSide : uint8
{
    Leftmost,
    Rightmost
};
```

`ABeehive`에 callable update API를 추가한다.

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Pollen Patty")
void ApplyPollenPattyConsumptionUpdate();
```

필요하면 private helper를 추가한다.

```cpp
AItemPlacementSlotActor* FindPollenPattyConsumptionTargetSlot(UPlacedItemRemainingComponent*& OutRemainingComponent) const;
bool DoesSlotMatchPollenPattyConsumptionTags(const AItemPlacementSlotActor* SlotActor) const;
```

helper 이름은 기존 style에 맞게 조정해도 된다.

## `ABeehive` 설정값 추가

`Beehive.h`에 아래 UPROPERTY를 추가한다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty Time", meta = (ClampMin = "1", ClampMax = "1440"))
int32 PollenPattyConsumptionBucketMinutes = 60;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty Time")
bool bApplyPollenPattyConsumptionOnBeginPlayBucket = false;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty", meta = (ClampMin = "0.0"))
float PollenPattyConsumptionAmountPerBucket = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty")
EPollenPattyConsumptionSide PollenPattyConsumptionSide = EPollenPattyConsumptionSide::Leftmost;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Pollen Patty")
FGameplayTagContainer PollenPattyConsumptionAreaTags;
```

주의:

- `FGameplayTagContainer`를 header에서 쓰므로 필요한 include를 추가한다.
- `PollenPattyConsumptionAreaTags`는 디테일창/Blueprint asset에서 설정한다.
- `ABeehive` constructor 또는 탐색 함수에서 `Item.UseArea.Beehive.PollenPatty`를 `RequestGameplayTag(...)`로 직접 넣지 않는다.
- 권장 editor 설정은 `.md/USER_UNREAL.md`에 남긴다.

## Bucket subscription

`ABeehive::GetGameTimeBucketSubscriptions_Implementation(...)`에 새 subscription을 추가한다.

```cpp
FGameTimeBucketSubscription PollenPattySubscription;
PollenPattySubscription.BucketMinutes = FMath::Clamp(PollenPattyConsumptionBucketMinutes, 1, 1440);
PollenPattySubscription.bApplyImmediatelyOnBeginPlay = bApplyPollenPattyConsumptionOnBeginPlayBucket;
PollenPattySubscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
PollenPattySubscription.SubscriptionTag = FName(TEXT("PollenPattyConsumption"));
OutSubscriptions.Add(PollenPattySubscription);
```

`ABeehive::OnGameTimeBucketEvent_Implementation(...)`에 새 branch를 추가한다.

```cpp
else if (Event.SubscriptionTag == FName(TEXT("PollenPattyConsumption")))
{
    ApplyPollenPattyConsumptionUpdate();
}
```

소모량은 `Event`의 시간 길이나 catch-up count로 스케일하지 않는다.

## 후보 수집 및 선택

`ABeehive`의 direct child actor component를 순회한다.

```cpp
TInlineComponentArray<UChildActorComponent*> ChildActorComponents(this);
```

각 child actor에 대해:

1. `AItemPlacementSlotActor`로 cast한다.
2. slot tags가 `PollenPattyConsumptionAreaTags`를 모두 포함하는지 확인한다.
3. `SlotActor->GetOccupiedActor()`로 occupied actor를 얻는다.
4. occupied actor에서 `UPlacedItemRemainingComponent`를 찾는다.
5. `HasRemaining()`이 false면 제외한다.
6. `GetCurrentAmount() <= 0.0f`이면 제외한다.
7. slot actor world location을 벌통 local space로 변환하고 local Y를 후보 값으로 사용한다.

tag match는 아래 의미로 구현한다.

```cpp
if (PollenPattyConsumptionAreaTags.IsEmpty())
{
    return false;
}

return SlotTags.HasAll(PollenPattyConsumptionAreaTags);
```

선택:

- `PollenPattyConsumptionSide == Leftmost`: local Y가 가장 작은 후보
- `PollenPattyConsumptionSide == Rightmost`: local Y가 가장 큰 후보
- tie는 `<` 또는 `>` 비교만 사용해 먼저 찾은 후보 유지

provider descriptor를 쓰지 않는다. 현재 구조에서 occupied slot은 item-use descriptor의 `AreaTags`가 비워지므로 소모 대상 식별에 부적합하다.

## 소모 실행

`ApplyPollenPattyConsumptionUpdate()` 흐름:

```cpp
void ABeehive::ApplyPollenPattyConsumptionUpdate()
{
    const float Amount = FMath::Max(0.0f, PollenPattyConsumptionAmountPerBucket);
    if (Amount <= 0.0f)
    {
        return;
    }

    UPlacedItemRemainingComponent* RemainingComponent = nullptr;
    AItemPlacementSlotActor* TargetSlot = FindPollenPattyConsumptionTargetSlot(RemainingComponent);
    if (!TargetSlot || !RemainingComponent)
    {
        return;
    }

    RemainingComponent->ConsumeAmount(Amount);
}
```

`ConsumeAmount(...)`가 소진 처리와 owning slot clear를 담당한다. 별도 destroy/clear를 `ABeehive`에서 직접 수행하지 않는다.

## 문서 갱신

구현 후 아래 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - `ABeehive`가 `PollenPattyConsumption` bucket으로 화분떡 고정 소모를 처리한다고 기록
  - 소모량이 벌 수/온도/bucket 길이와 무관한 고정값임을 기록
  - 소모 대상은 `PollenPattyConsumptionAreaTags`와 slot `AreaTags` 매칭으로 식별한다고 기록
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehive` composition/flow에 pollen patty consumption 설정값과 동작 기록
  - left/right local Y 선택 규칙 기록
  - provider descriptor를 쓰지 않고 direct child `AItemPlacementSlotActor`를 수집한다는 구현 기준 기록
- `.md/USER_UNREAL.md`
  - 벌통 Blueprint/레벨 인스턴스에서 `PollenPattyConsumptionAreaTags`를 설정해야 한다고 기록
  - 권장값: `{Item.UseArea.Beehive.PollenPatty}`
  - pollen slot `SlotMeshComponent.AreaTags`도 같은 태그를 포함해야 한다고 기록
  - 화분떡 item definition은 durability placed remaining 설정이 필요하다고 기록

새 QnA는 필요하지 않다. 구현 중 문서 확정 정책과 충돌하는 구조를 발견하면 `.md/QNA_IMPLEMENTATION.md`에 질문하고 중단한다.

## 검색 검증

```powershell
rg "PollenPattyConsumption" Source/BeekeepingSim .md
rg "Item.UseArea.Beehive.PollenPatty" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "GetSlotAreaTags|GetOccupiedActor|ConsumeAmount" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
```

확인할 것:

- `ABeehive` 소모 대상 탐색 경로에 `Item.UseArea.Beehive.PollenPatty` 문자열이 없다.
- `PollenPattyConsumption` subscription과 event branch가 모두 존재한다.
- 소모는 `UPlacedItemRemainingComponent::ConsumeAmount(...)`로만 수행한다.
- source에서 `Content/` asset 수정이 없다.

## 시나리오 검증

가능하면 PIE 또는 단위 테스트성 디버그 호출로 아래를 확인한다.

1. `PollenPattyConsumptionAreaTags`가 비어 있으면 아무 것도 소모되지 않는다.
2. 태그가 매칭되는 화분떡 slot 1개가 있으면 bucket마다 `1.0f`만 감소한다.
3. 여러 화분떡이 있으면 기본값 기준 local Y가 가장 작은 slot 하나만 감소한다.
4. `PollenPattyConsumptionSide=Rightmost`로 바꾸면 local Y가 가장 큰 slot 하나만 감소한다.
5. 선택된 화분떡 잔량이 소모량보다 작아도 같은 bucket에서 다음 화분떡으로 spillover되지 않는다.
6. remaining이 없는 occupied actor는 후보에서 제외된다.
7. hotbar 회수 시 남은 durability가 기존 remaining 시스템을 통해 보존된다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `UPlacedItemRemainingComponent` 또는 `ConsumeAmount(...)`가 현재 Source에 없거나 의미가 문서와 다른 경우
- 화분떡 slot이 direct child `AItemPlacementSlotActor`가 아니라는 근거가 발견되어 수집 범위를 바꿔야 하는 경우
- `AItemPlacementSlotActor`에서 configured `AreaTags`를 읽는 public getter 추가만으로 해결되지 않는 경우
- `ABeehive`에 `FGameplayTagContainer` UPROPERTY를 추가하는 것이 기존 Blueprint/API 계약과 충돌하는 경우
- Content asset 수정 없이는 기능을 C++에서 검증할 수 없는 경우
