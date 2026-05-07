# Queen Bee Actor / Queen Location Update 구현 프롬프트

## 목표

벌통 내부 여왕벌 actor와 여왕벌 위치 자동 업데이트 기능을 구현한다.

핵심 요구사항:

```text
여왕벌 actor는 Tick마다 Details 노출값 n1 기준 `-n1~n1` 범위의 랜덤 yaw를 `AddActorLocalRotation`으로 더해 떨림을 만든다.
여왕벌 위치는 게임시간 기준 60분마다 자동 업데이트된다.
위치 업데이트 후보에서는 현재 들어올려진 소비장을 제외한다.
선택 가능한 소비장 중 중앙 쪽 소비장이 더 높은 확률로 선택된다.
소비장이 결정되면 Front/Back 면 중 하나를 50:50으로 선택한다.
여왕벌은 선택된 면의 중앙에 부착된다.
위치 업데이트 시 여왕벌 actor yaw는 `0~360도` 완전 랜덤으로 추가 적용한다.
```

## 확정 QnA

`.md/QNA_ARCHITECTURE.md`의 `Queen Bee Actor QnA` 답변을 기준으로 구현한다.

- 위치 업데이트 yaw 랜덤 범위: 옵션 A, `0~360도` 완전 랜덤 yaw
- 위치 업데이트 yaw 기준: 옵션 A, 선택된 Front/Back attach point 회전 기준
- Tick yaw 떨림: 옵션 A, 매 Tick `AddActorLocalRotation`으로 랜덤 yaw 누적
- 여왕벌이 붙은 소비장이 들어올려질 때: 옵션 A, 소비장에 attach된 상태로 같이 이동

추가 QnA는 필요 없다.

## 수정 대상

우선 다음 파일을 확인하고 수정한다.

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/Environment/GameTimeBucketTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombLiftComponent.h`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

새 파일 추가:

- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`

필요 시 `BeekeepingSim.Build.cs`를 확인하되, 일반 Actor/StaticMeshComponent만 사용하면 새 모듈 의존성은 필요 없어야 한다.

## 현재 구조

`ABeehive`:

- `IGameTimeBucketListener`를 이미 구현한다.
- `BeginPlay()`에서 `UGameTimeBucketSubsystem`에 자신을 등록한다.
- `EndPlay()`에서 등록 해제한다.
- `GetGameTimeBucketSubscriptions_Implementation()`에서 `BeeSwarm` bucket subscription을 반환한다.
- `OnGameTimeBucketEvent_Implementation()`에서 `BeeSwarm` tag를 처리한다.
- `CombRackRoot`와 `CombSlotComponents`로 소비장 슬롯을 관리한다.
- `CurrentCombCount` 이하 slot만 active comb actor를 가진다.
- `CombLiftComponent->GetLiftedCombSlotIndex()`로 현재 들어올려진 소비장 슬롯을 알 수 있다.

`ABeehiveCombActor`:

- 소비장 actor다.
- `CombMesh`, `FrontFaceBeeNiagara`, `BackFaceBeeNiagara`, `PartFocusAction`을 가진다.
- 현재 여왕벌 부착 전용 attach point는 없다.

## 설계 원칙

- 여왕벌 기능은 `WorldActors` 시스템에 둔다.
- `Environment` 시스템은 수정하지 않는다. 시간 갱신은 기존 `UGameTimeBucketSubsystem` + `IGameTimeBucketListener` 경로만 사용한다.
- `AEnvironmentTimeOfDayActor`를 `ABeehive`나 여왕벌 actor에서 직접 찾거나 참조하지 않는다.
- `AQueenBeeActor`는 자기 시각/떨림만 담당한다.
- 소비장 후보 선택, Front/Back 선택, 60분 위치 업데이트는 `ABeehive`가 담당한다.
- 기존 UCLASS/USTRUCT/UENUM rename은 하지 않는다. Core Redirect는 필요 없다.

## AQueenBeeActor 추가

`AQueenBeeActor`를 `WorldActors` 폴더에 추가한다.

권장 구성:

```cpp
UCLASS(Blueprintable)
class BEEKEEPINGSIM_API AQueenBeeActor : public AActor
```

컴포넌트:

- `USceneComponent* Root`
- `UStaticMeshComponent* QueenBeeMesh`

Tick:

- `PrimaryActorTick.bCanEverTick = true`
- `Tick(float DeltaTime)`에서 다음 로직을 수행한다.

```cpp
const float YawDelta = FMath::FRandRange(-YawJitterDegreesPerTick, YawJitterDegreesPerTick);
AddActorLocalRotation(FRotator(0.0f, YawDelta, 0.0f));
```

Details 노출값:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Queen Bee|Motion", meta=(ClampMin="0.0"))
float YawJitterDegreesPerTick = 1.0f;
```

요구사항상 DeltaTime 보정은 하지 않는다. Tick당 랜덤 누적 yaw가 목표 동작이다.

## 소비장 attach point 추가

`ABeehiveCombActor`에 여왕벌 부착 기준점을 추가한다.

권장 컴포넌트:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
TObjectPtr<USceneComponent> QueenFrontAttachPoint;

UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
TObjectPtr<USceneComponent> QueenBackAttachPoint;
```

권장 API:

```cpp
UFUNCTION(BlueprintPure, Category="Beehive|Queen Bee")
USceneComponent* GetQueenFrontAttachPoint() const;

UFUNCTION(BlueprintPure, Category="Beehive|Queen Bee")
USceneComponent* GetQueenBackAttachPoint() const;

UFUNCTION(BlueprintPure, Category="Beehive|Queen Bee")
USceneComponent* GetQueenAttachPoint(bool bFrontFace) const;
```

구성:

- 두 attach point는 `CombMesh` 또는 `Root` 아래에 붙인다.
- 기본 relative location은 소비장 면 중앙을 의미하도록 둔다.
- 정확한 offset은 Blueprint child 또는 Details에서 조정할 수 있게 `VisibleAnywhere` component transform으로 제공한다.
- Niagara component를 attach 기준으로 쓰지 않는다. VFX와 gameplay placement 기준을 분리한다.

## ABeehive 여왕벌 소유

`ABeehive`에 여왕벌 child actor를 추가한다.

권장 필드:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Beehive|Queen Bee")
TObjectPtr<UChildActorComponent> QueenBeeChildActor;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Beehive|Queen Bee")
TSubclassOf<AQueenBeeActor> QueenBeeActorClass;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Beehive|Queen Bee Time", meta=(ClampMin="1", ClampMax="1440"))
int32 QueenBeeLocationBucketMinutes = 60;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Beehive|Queen Bee Time")
bool bUpdateQueenBeeLocationOnBeginPlayBucket = true;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Beehive|Queen Bee", meta=(ClampMin="1.0"))
float QueenBeeCenterWeightMultiplier = 4.0f;
```

권장 API:

```cpp
UFUNCTION(BlueprintCallable, Category="Beehive|Queen Bee")
void UpdateQueenBeeLocation();

UFUNCTION(BlueprintPure, Category="Beehive|Queen Bee")
AQueenBeeActor* GetQueenBeeActor() const;
```

private helper 예시:

```cpp
void EnsureQueenBeeChildActorClass();
bool ChooseQueenBeeCombSlotIndex(int32& OutSlotIndex) const;
float CalculateQueenBeeCombSlotWeight(int32 SlotIndex) const;
USceneComponent* ResolveQueenBeeAttachPoint(int32 SlotIndex, bool bFrontFace) const;
```

## 여왕벌 child actor 초기화

`ABeehive` 생성자:

- `QueenBeeChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("QueenBeeChildActor"));`
- 기본 attach parent는 `Root` 또는 `CombRackRoot`로 둔다.
- `QueenBeeActorClass = AQueenBeeActor::StaticClass();`

`OnConstruction()` / `BeginPlay()`:

- `EnsureQueenBeeChildActorClass()` 호출
- `bUpdateQueenBeeLocationOnBeginPlayBucket`이 true인 경우 BeginPlay 또는 bucket initial apply에서 위치 배치가 수행되도록 한다.
- child actor class 변경은 기존 `BeehiveSwarmChildActor` 패턴처럼 class만 보정하고 불필요한 재생성을 피한다.

## 위치 업데이트 후보 선택

`UpdateQueenBeeLocation()`은 다음 순서로 동작한다.

1. 여왕벌 child actor/class가 유효한지 보정한다.
2. active comb 후보를 만든다.
3. 현재 들어올려진 소비장 index는 후보에서 제외한다.
4. 중앙 가중 랜덤으로 소비장 슬롯을 선택한다.
5. `FMath::RandBool()`로 Front/Back을 선택한다.
6. 선택된 attach point에 여왕벌 child actor component를 attach한다.
7. attach point 기준 회전에 `0~360도` 랜덤 yaw를 relative rotation으로 적용한다.

후보 조건:

```text
0 <= Index < CurrentCombCount
CombSlotComponents.IsValidIndex(Index)
CombSlotComponents[Index] 유효
ChildActor가 ABeehiveCombActor
Index != CombLiftComponent->GetLiftedCombSlotIndex()
선택된 face attach point 유효
```

후보가 없으면 no-op:

```text
여왕벌 actor를 destroy/hide하지 않는다.
기존 위치를 유지한다.
로그 스팸은 남기지 않는다.
```

중앙 가중치 계산 권장:

```cpp
const float Center = static_cast<float>(CurrentCombCount - 1) * 0.5f;
const float MaxDistance = FMath::Max(Center, static_cast<float>(CurrentCombCount - 1) - Center);
const float Distance = FMath::Abs(static_cast<float>(SlotIndex) - Center);
const float Distance01 = MaxDistance > KINDA_SMALL_NUMBER ? Distance / MaxDistance : 0.0f;
const float CenterFactor = 1.0f - FMath::Clamp(Distance01, 0.0f, 1.0f);
const float Weight = FMath::Lerp(1.0f, FMath::Max(1.0f, QueenBeeCenterWeightMultiplier), CenterFactor);
```

선택 알고리즘:

- 후보별 weight 합산
- `FRandRange(0, TotalWeight)`로 weighted random pick
- fallback으로 마지막 후보 선택

짝수 소비장 수:

- 가운데 두 슬롯이 같은 최고 가중치를 가져야 한다.

## Front/Back 부착

Front/Back 선택:

```cpp
const bool bFrontFace = FMath::RandBool();
```

부착:

```cpp
QueenBeeChildActor->AttachToComponent(AttachPoint, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
QueenBeeChildActor->SetRelativeLocation(FVector::ZeroVector);
QueenBeeChildActor->SetRelativeRotation(FRotator(0.0f, RandomYaw, 0.0f));
```

주의:

- random yaw는 `0.0f~360.0f` 범위다.
- attach point 회전을 기준으로 yaw가 더해져야 한다.
- scale은 attach point scale을 강제로 상속하지 않도록 `SnapToTargetNotIncludingScale`을 사용한다.
- Tick yaw 떨림은 actor 내부에서 계속 `AddActorLocalRotation`으로 누적된다.

## 60분 게임시간 업데이트

기존 `ABeehive::GetGameTimeBucketSubscriptions_Implementation()`에 subscription을 추가한다.

```cpp
FGameTimeBucketSubscription QueenSubscription;
QueenSubscription.BucketMinutes = FMath::Clamp(QueenBeeLocationBucketMinutes, 1, 1440);
QueenSubscription.bApplyImmediatelyOnBeginPlay = bUpdateQueenBeeLocationOnBeginPlayBucket;
QueenSubscription.CatchUpPolicy = EGameTimeBucketCatchUpPolicy::LatestOnly;
QueenSubscription.SubscriptionTag = FName(TEXT("QueenBeeLocation"));
OutSubscriptions.Add(QueenSubscription);
```

`OnGameTimeBucketEvent_Implementation()`에 tag 분기 추가:

```cpp
if (Event.SubscriptionTag == FName(TEXT("QueenBeeLocation")))
{
    UpdateQueenBeeLocation();
}
```

기존 `BeeSwarm` 처리와 충돌하지 않게 한다.

## 소비장 들어올림과 여왕벌

확정 정책:

```text
이미 여왕벌이 붙어 있는 소비장이 들어올려지면 여왕벌은 소비장에 attach된 상태로 같이 이동한다.
```

따라서 `UBeehiveCombLiftComponent`의 lift 동작에서 여왕벌을 강제로 detach/reposition하지 않는다.

단, 다음 60분 위치 업데이트 후보에서는 현재 들어올려진 소비장을 제외한다.

## 문서 갱신

구현 후 다음 문서를 갱신한다.

`.md/0_ARCHITECTURE.md`:

- WorldActors 요약에 여왕벌 actor와 60분 bucket 위치 업데이트 흐름 추가.
- Environment는 직접 참조하지 않고 bucket listener 경로만 사용한다고 기록.

`.md/Architecture/WorldActorsSystem.md`:

- Scope에 `QueenBeeActor.h/.cpp` 추가.
- Key Classes에 `AQueenBeeActor` 추가.
- `ABeehive` composition에 `QueenBeeChildActor`, `QueenBeeActorClass`, queen bucket 설정 추가.
- `ABeehiveCombActor` composition에 `QueenFrontAttachPoint`, `QueenBackAttachPoint` 추가.
- Design Notes에 중앙 가중 랜덤, lifted comb 제외, Front/Back 50:50, 위치 업데이트 yaw `0~360`, Tick yaw 누적 정책 기록.

## Blueprint/API/Core Redirect 영향

- 새 UCLASS `AQueenBeeActor` 추가는 Core Redirect가 필요 없다.
- 기존 UCLASS/USTRUCT/UENUM rename 금지.
- 기존 Blueprint API 삭제/rename 금지.
- `ABeehiveCombActor`에 component를 추가하므로 기존 BP child에서 component transform이 의도대로 보이는지 수동 확인 대상이다.
- `AQueenBeeActor`가 Blueprint native parent로 사용되면 이후 rename 시 Core Redirect 대상이 된다.

## 검증 기준

- C++ 빌드 성공.
- `AQueenBeeActor`가 Details에서 `YawJitterDegreesPerTick` 값을 노출한다.
- 여왕벌 actor가 Tick마다 `-n1~n1` 범위 yaw를 `AddActorLocalRotation`으로 누적한다.
- `ABeehive`가 기존 `BeeSwarm` bucket과 별개로 `QueenBeeLocation` 60분 bucket을 구독한다.
- 게임시간 기준 60분마다 `UpdateQueenBeeLocation()`이 호출된다.
- 현재 들어올려진 소비장은 위치 업데이트 후보에서 제외된다.
- 중앙 쪽 소비장의 선택 확률이 가장자리보다 높다.
- Front/Back face 선택은 50:50이다.
- 여왕벌은 선택된 face attach point 중앙에 부착된다.
- 위치 업데이트 시 attach point 기준으로 `0~360도` 랜덤 yaw가 적용된다.
- 여왕벌이 붙은 소비장을 들어올리면 여왕벌도 같이 이동한다.
- 후보 소비장이 없으면 기존 여왕벌 위치를 유지하고 crash가 없어야 한다.

## QnA 필요 여부

추가 QnA는 필요 없다.

확정 기준:

```text
QNA_ARCHITECTURE.md의 Queen Bee Actor QnA 4개 답변을 모두 반영한다.
```
