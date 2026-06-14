# BeeCarrier 포획 상태와 분봉 본진 부피 기준 포획 구현 프롬프트

## 목표

이미 구현된 분봉 테스트 기능을 확장해 `벌 운반통`이 실제 포획 벌 수를 item instance runtime state로 저장하게 한다.

현재 `UBeeCarrierUseAction`은 drag speed 기반으로 `ABeeSwarmClusterActor::AliveRadius`를 직접 감소시킨다. 이 구현을 부피 기준 포획 모델로 바꾼다. 새 모델에서는 분봉 본진의 포획 진행 source of truth가 `AliveRadius`가 아니라 벌 수(`CapturedBeeAmount` 또는 `RemainingBeeAmount`)이며, `AliveRadius`는 남은 벌 수 비율에서 구 부피 공식으로 파생해 Niagara `User.AliveRadius`에 주입한다.

새 actor나 새 Focus 경로를 만들지 않는다. 기존 `ABeeSwarmClusterActor`, `UBeeCarrierUseAction`, `ABeehive::BeginSwarmingAtTransform/Actor`를 확장한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`

## 핵심 확정 사항

- BeeCarrier 포획 결과는 `UItemInstance` optional `FBeeCarrierItemState`로 저장한다.
- BeeCarrier item은 `MaxStack=1` invariant를 가진다.
- BeeCarrier 최대 수용량은 `UBeeCarrierItemDefinition::MaxCapturedBeeAmount`가 source of truth다.
- 분봉 본진은 `InitialAliveRadius`와 포획/잔여 벌 수를 source of truth로 삼는다.
- `AliveRadius`는 `InitialAliveRadius * Cbrt(RemainingBeeAmount / TotalBeeAmount)`로 계산한다.
- `UBeeCarrierUseAction`은 `bees/sec` 단위 capture rate를 계산한다.
- 실제 포획량은 분봉 본진 잔여 벌 수와 BeeCarrier 남은 수용량으로 clamp한다.
- 내부 포획량 state는 `float`으로 저장한다. 정수 마릿수는 표시/Blueprint 편의 query에서만 변환한다.
- 기존 `ABeeSwarmClusterActor::DecreaseAliveRadius(float)`는 삭제/rename하지 않는다. BeeCarrier 포획 경로는 새 `CaptureBees(float RequestedBeeAmount)` API를 사용한다.
- 기존 벌통 `ColonyBeeCount`, 기존 벌통 여왕벌, 소비장 벌 수/target count는 변경하지 않는다.
- `Content/` asset은 수정하지 않는다.

## 구현 대상

### Inventory

새 파일:

- `Source/BeekeepingSim/Public/Inventory/BeeCarrierItemDefinition.h`

수정:

- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/Inventory/BeeCarrierUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/BeeCarrierUseAction.cpp`
- 필요 시 stack max 계산 helper에서 honey container와 같은 방식으로 BeeCarrier `MaxStack=1` 보장

### WorldActors

수정:

- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`

### 문서

구현 후 갱신:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/Architecture/FocusSystem.md`

## `FBeeCarrierItemState`

`UItemInstance`에 optional runtime state를 추가한다.

권장 구조:

```cpp
USTRUCT(BlueprintType)
struct FBeeCarrierItemState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Bee Carrier")
    bool bHasState = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Bee Carrier", meta = (ClampMin = "0.0"))
    float CapturedBeeAmount = 0.0f;
};
```

`UItemInstance` 권장 API:

- `SetBeeCarrierState(float CapturedBeeAmount)`
- `AddCapturedBees(float BeeAmount)`
- `ClearBeeCarrierState()`
- `HasBeeCarrierState() const`
- `GetBeeCarrierState() const`
- `GetCapturedBeeAmount() const`
- `GetCapturedBeeCountRounded() const`
- `GetBeeCarrierFreeCapacity() const`

구현 규칙:

- `SetBeeCarrierState`는 definition이 `UBeeCarrierItemDefinition`이 아니면 state를 clear한다.
- `CapturedBeeAmount`는 `0..MaxCapturedBeeAmount`로 clamp한다.
- `InitializeFromDefinition`에서 definition이 `UBeeCarrierItemDefinition`이면 `SetBeeCarrierState(DefaultCapturedBeeAmount)`를 호출한다.
- `CopyRuntimeStateFrom`은 BeeCarrier state도 복사한다.
- `SetStackCount` 또는 max stack 계산에서 BeeCarrier definition은 꿀 용기와 같이 `MaxStack=1`로 강제한다.
- 비-BeeCarrier item은 `bHasState=false` BeeCarrier state를 무시한다.

## `UBeeCarrierItemDefinition`

`UItemDefinition` subclass를 추가한다.

권장:

```cpp
UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UBeeCarrierItemDefinition : public UItemDefinition
{
    GENERATED_BODY()

public:
    UBeeCarrierItemDefinition()
    {
        MaxStack = 1;
    }

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Bee Carrier", meta = (ClampMin = "0.0"))
    float MaxCapturedBeeAmount = 1000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Bee Carrier", meta = (ClampMin = "0.0"))
    float DefaultCapturedBeeAmount = 0.0f;
};
```

`MaxCapturedBeeAmount`는 정적 용량이다. instance state에는 저장하지 않는다.

## `ABeeSwarmClusterActor` 변경

현재 `AliveRadius`, `SpawnAmount`, `SphereRadius`와 Niagara parameter 적용 경로는 유지한다.

추가 state:

```cpp
float InitialAliveRadius;
float CapturedBeeAmount;
```

`TotalBeeAmount`는 기존 `SpawnAmount`를 사용한다. `SpawnAmount <= 0`이면 포획 불가/완료 상태로 처리한다.

권장 API:

- `CaptureBees(float RequestedBeeAmount)`
- `SetCapturedBeeAmount(float NewCapturedBeeAmount)`
- `GetCapturedBeeAmount() const`
- `GetCapturedBeeCountRounded() const`
- `GetRemainingBeeAmount() const`
- `GetRemainingBeeCountRounded() const`
- `GetTotalBeeAmount() const`
- `GetInitialAliveRadius() const`
- `CalculateAliveRadiusFromRemainingBees() const`
- `RefreshAliveRadiusFromBeeAmounts()`

초기화:

```cpp
bCaptured = false;
InitialAliveRadius = Max(0, InAliveRadius);
AliveRadius = InitialAliveRadius;
SpawnAmount = Max(0, InSpawnAmount);
CapturedBeeAmount = 0.0f;
SphereRadius = Max(0, InSphereRadius);
```

부피 공식:

```cpp
const float TotalBeeAmount = FMath::Max(0.0f, static_cast<float>(SpawnAmount));
const float RemainingBeeAmount = FMath::Clamp(TotalBeeAmount - CapturedBeeAmount, 0.0f, TotalBeeAmount);
const float RemainingRatio = TotalBeeAmount > 0.0f ? RemainingBeeAmount / TotalBeeAmount : 0.0f;
AliveRadius = InitialAliveRadius * FMath::Pow(RemainingRatio, 1.0f / 3.0f);
```

`CaptureBees`:

```cpp
ActualCaptured = Clamp(RequestedBeeAmount, 0, TotalBeeAmount - CapturedBeeAmount);
CapturedBeeAmount += ActualCaptured;
RefreshAliveRadiusFromBeeAmounts();
ApplyClusterNiagaraParameters();
ReceiveAliveRadiusChanged(AliveRadius);
HandleCapturedIfNeeded();
return ActualCaptured;
```

captured 판정:

- `CapturedBeeAmount >= TotalBeeAmount` 또는 `RemainingBeeAmount <= small threshold`이면 captured.
- captured 전환은 기존처럼 1회만 발생한다.
- captured 시 `AliveRadius=0`을 Niagara에 적용하고 capture use-area를 비활성화한다.

기존 `DecreaseAliveRadius(float)`:

- 삭제/rename하지 않는다.
- BeeCarrier 포획 경로에서 사용하지 않는다.
- legacy/manual visual wrapper로 유지한다.
- 이 함수가 `AliveRadius`를 직접 조정해도 `CapturedBeeAmount` source of truth와 혼동되지 않게 주석을 남긴다.
- 구현자가 더 일관성을 원하면 `DecreaseAliveRadius` 내부에서 radius를 줄인 뒤 그 radius에 대응하는 captured amount를 역산하지 말고, 기존 수동 visual 조정 API로만 취급한다. 포획 gameplay는 반드시 `CaptureBees`를 사용한다.

## `UBeeCarrierUseAction` 변경

현재 `BaseAliveRadiusDecreasePerSecond`, `DragSpeedToAliveRadiusDecreaseScale`, `MaxAliveRadiusDecreasePerSecond`는 radius 단위 설정이다. 삭제/rename은 Blueprint serialized 값에 영향을 줄 수 있으므로 신중히 처리한다.

권장:

- 기존 radius 설정 UPROPERTY는 deprecated metadata로 남기거나 사용하지 않되 삭제하지 않는다.
- 새 bees/sec 설정을 추가한다.

새 설정:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
float BaseBeeCapturePerSecond = 10.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
float DragSpeedToBeeCaptureScale = 0.02f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Bee Carrier", meta = (ClampMin = "0.0"))
float MaxBeeCapturePerSecond = 80.0f;
```

기존 `MinDragSpeedForBonus`, `bHasLastImpactPoint`, `LastImpactPoint`는 유지한다.

`CanBeginUse` / `CanApplyUseEffect` 추가 조건:

- target cluster 유효
- cluster captured 아님
- selected/source item instance가 `UBeeCarrierItemDefinition` 기반
- BeeCarrier free capacity > 0
- cluster remaining bee amount > 0
- item-use-area hit context 유효

`ApplyUseEffect`:

1. 기존 방식대로 impact point와 `DeltaTime`으로 drag speed 계산
2. `bees/sec` capture rate 계산
3. requested amount = rate * DeltaTime
4. carrier free capacity로 requested amount clamp
5. `Cluster->CaptureBees(RequestedAmount)` 호출
6. 실제 capture amount를 BeeCarrier item state에 더함
7. 실제 capture amount가 0보다 크면 `Result.bSucceeded=true`

권장 pseudo:

```cpp
const float BonusSpeed = FMath::Max(0.0f, DragSpeedCmPerSecond - FMath::Max(0.0f, MinDragSpeedForBonus));
float CaptureRate = FMath::Max(0.0f, BaseBeeCapturePerSecond)
    + BonusSpeed * FMath::Max(0.0f, DragSpeedToBeeCaptureScale);
CaptureRate = FMath::Clamp(CaptureRate, 0.0f, FMath::Max(0.0f, MaxBeeCapturePerSecond));

float RequestedBeeAmount = CaptureRate * SafeDeltaTime;
RequestedBeeAmount = FMath::Min(RequestedBeeAmount, CarrierFreeCapacity);

const float ActualCaptured = ClusterActor->CaptureBees(RequestedBeeAmount);
if (ActualCaptured > KINDA_SMALL_NUMBER)
{
    SourceItemInstance->AddCapturedBees(ActualCaptured);
    Result.bSucceeded = true;
}
```

주의:

- item stack/durability는 변경하지 않는다.
- 포획량은 `float`으로 누적한다.
- 표시용 마릿수는 rounded query로 제공한다.
- 운반통이 가득 차면 use session이 더 이상 시작/적용되지 않아야 한다.

## Runtime State Copy/Move

현재 hotbar/storage partial move는 `UItemInstance::CopyRuntimeStateFrom`을 사용한다.

구현 규칙:

- `CopyRuntimeStateFrom`이 BeeCarrier state도 복사해야 한다.
- source가 BeeCarrier state를 가지지 않으면 target BeeCarrier state를 clear한다.
- BeeCarrier item은 `MaxStack=1`이라 일반 stack merge 충돌 가능성은 낮지만, state-aware move helper가 runtime state compatibility를 확인하는 경우 BeeCarrier state도 비교 대상에 포함해야 한다.

## 문서 반영

구현 후 다음 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - BeeCarrier item state 저장
  - 분봉 본진 포획 source of truth가 벌 수이고 `AliveRadius`는 부피 공식 파생값이라는 점
- `.md/Architecture/InventorySystem.md`
  - `FBeeCarrierItemState`
  - `UBeeCarrierItemDefinition`
  - `UBeeCarrierUseAction` bees/sec 포획 rate와 capacity clamp
  - runtime state copy
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeeSwarmClusterActor::CaptureBees`
  - `InitialAliveRadius`, `CapturedBeeAmount`, `RemainingBeeAmount`
  - `AliveRadius = InitialAliveRadius * cbrt(Remaining/Total)` Niagara 반영
- `.md/Architecture/FocusSystem.md`
  - Focus path는 변경 없음이라고 명시하면 충분하다.

Core Redirect 문서는 신규 class/state 추가만이면 갱신하지 않는다.

## 수정하면 안 되는 것

- 기존 UCLASS/USTRUCT/UENUM rename 금지
- 기존 BlueprintCallable API 삭제/rename 금지
- `ABeeSwarmClusterActor::DecreaseAliveRadius` 삭제/rename 금지
- 새 Focus/item-use-area 경로 추가 금지
- `Content/` asset 저장 금지
- 기존 벌통 `ColonyBeeCount`, 기존 여왕벌, 소비장 벌 수/target count 변경 금지
- 자동 분봉 발생 조건 추가 금지

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
rg -n "FBeeCarrierItemState|UBeeCarrierItemDefinition|CaptureBees|BaseBeeCapturePerSecond|CapturedBeeAmount|InitialAliveRadius" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg -n "DecreaseAliveRadius\\(" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
```

두 번째 검색은 BeeCarrier 포획 경로가 더 이상 `DecreaseAliveRadius`를 사용하지 않는지 확인하기 위한 것이다. legacy/manual API 선언과 구현은 남아 있어야 한다.

## 수동 PIE 확인

1. BeeCarrier item definition을 `UBeeCarrierItemDefinition` 기반으로 만들고 `MaxCapturedBeeAmount`를 설정한다.
2. BeeCarrier item을 hotbar/storage 이동해도 `CapturedBeeAmount`가 유지되는지 확인한다.
3. 분봉 본진에서 BeeCarrier로 포획하면 `CapturedBeeAmount`가 증가하는지 확인한다.
4. 같은 drag speed에서 포획 벌 수가 시간에 대해 거의 선형 증가하는지 확인한다.
5. `AliveRadius`는 같은 조건에서 선형이 아니라 `InitialAliveRadius * cbrt(Remaining/Total)`로 감소하는지 확인한다.
6. 본진 `SpawnAmount=500`, `InitialAliveRadius=20`, `CapturedBeeAmount=437.5` 근처에서 `AliveRadius`가 약 10인지 확인한다.
7. BeeCarrier가 가득 차면 더 이상 포획되지 않는지 확인한다.
8. 분봉 본진 잔여 벌 수가 0이면 captured event가 1회만 발생하고 use-area가 비활성화되는지 확인한다.
9. 기존 `DecreaseAliveRadius`를 BP에서 수동 호출해도 compile/runtime 문제가 없는지 확인한다.

## 최종 보고 요구사항

- 변경 파일
- 새 USTRUCT/UCLASS 목록
- 추가/변경 Blueprint API
- BeeCarrier state 저장/복사 경로
- BeeCarrier capacity clamp 방식
- 분봉 본진 부피 기준 `AliveRadius` 계산식
- `UBeeCarrierUseAction` bees/sec capture rate 계산식
- `DecreaseAliveRadius` 유지 방식
- 아키텍처 문서 반영 내용
- 빌드 결과 또는 미수행 사유
- 필요한 수동 BP/Content 작업 목록
