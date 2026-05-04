# 구현 프롬프트: Beehive Attraction Swarm Niagara

## 목표

`ABeehive`에 구심점을 기준으로 비행하는 신규 NiagaraComponent를 추가한다.

신규 Niagara User Parameter:

- `User.AttractionPower` : Float
- `User.NoisePower` : Float
- `User.SpawnSphereRadius` : Float
- `User.SpawnAmount` : Int32

요구:

- `AttractionPower`, `NoisePower`, `SpawnSphereRadius`는 `ABeehive` 액터 단계에서 노출한다.
- 위 값들은 변경 시 즉시 Niagara User Parameter에 반영한다.
- Niagara `User.SpawnAmount`는 `ColonyBeeCount * SpawnAmountScale`로 계산한다.
- 계산 결과는 `RoundToInt` 후 `MaxSpawnAmount`로 clamp한다.
- `User.SpawnAmount`는 특정 시간/10분 경계마다 자동 갱신하지 않는다.
- `ColonyBeeCount`, 설정값 변경, Construction/BeginPlay 등 벌통 값 적용 시점에만 재계산/적용한다.
- Niagara User Parameter 직접 수정 UI는 숨긴다.

## 확정 QnA

`.md/QNA_ARCHITECTURE.md`의 “Beehive 구심점 비행 Niagara 설계 QnA” 답변을 따른다.

확정 사항:

- `ABeehive`가 `UNiagaraComponent* AttractionSwarmNiagara`를 직접 소유한다.
- 별도 child actor를 만들지 않는다.
- 구심점은 `AttractionSwarmNiagara` 컴포넌트 위치 자체를 사용한다.
- Niagara System asset은 `BP_Beehive`에서 `AttractionSwarmNiagara` 컴포넌트 details에 직접 지정한다.
- 설정은 `FBeehiveAttractionSwarmSettings` 구조체로 묶는다.
- `SpawnAmount = RoundToInt(ColonyBeeCount * SpawnAmountScale)`.
- 최종 `SpawnAmount`는 `0~MaxSpawnAmount`로 clamp한다.
- `AttractionPower`, `NoisePower`, `SpawnSphereRadius`는 `OnConstruction`, `PostEditChangeProperty`, `BeginPlay`, 명시적 apply 함수에서 적용한다.
- 시간 경계 자동 업데이트는 사용하지 않는다.
- Niagara User Parameter UI는 custom details customization으로 숨기고, Beehive 적용 경로에서도 항상 덮어쓴다.
- `User.SpawnAmount`는 `SetVariableInt`로 적용한다.
- `ColonyBeeCount` 변경 시 `SpawnAmount`를 즉시 반영한다.
- 기존 Outgoing/Ingoing spline swarm과 신규 Attraction swarm은 동시에 동작한다.

## 구현 전 참고 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- 현재 `ABeehive`, `ABeehiveDualSwarmActor`, `BeeSwarmTypes` 구현

## 작업 범위

수정 후보:

- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActorCustomization.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActorCustomization.cpp`
- `Source/BeekeepingSim/BeekeepingSim.Build.cs`

문서 갱신:

- `.md/Architecture/WorldActorsSystem.md`
- `.md/0_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

주의:

- `Content/` asset 자동 생성/수정 금지
- C++에서 Niagara System asset path 직접 로드 금지
- `AEnvironmentTimeOfDayActor`, `UGameTimeBucketSubsystem`에 신규 Attraction swarm 업데이트를 연결하지 말 것

## Type 설계

### `FBeehiveAttractionSwarmSettings`

`BeeSwarmTypes.h` 또는 Beehive 전용 적절한 타입 파일에 추가한다.

권장:

```cpp
USTRUCT(BlueprintType)
struct BEEKEEPINGSIM_API FBeehiveAttractionSwarmSettings
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
    float AttractionPower = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
    float NoisePower = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
    float SpawnSphereRadius = 200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0.0"))
    float SpawnAmountScale = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beehive|Attraction Swarm", meta = (ClampMin = "0"))
    int32 MaxSpawnAmount = 1000;
};
```

의미:

- `AttractionPower`, `NoisePower`, `SpawnSphereRadius`는 Niagara에 float로 직접 적용한다.
- `SpawnAmountScale`은 최종 `User.SpawnAmount` 계산에만 사용한다.
- `MaxSpawnAmount`는 성능 안전장치다.

## `ABeehive` 변경 요구사항

### Component 추가

`ABeehive`에 NiagaraComponent를 직접 추가한다.

권장:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Attraction Swarm")
TObjectPtr<UNiagaraComponent> AttractionSwarmNiagara;
```

생성자:

```cpp
AttractionSwarmNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("AttractionSwarmNiagara"));
AttractionSwarmNiagara->SetupAttachment(Root);
```

주의:

- 별도 `AttractionSwarmCenter` SceneComponent를 만들지 않는다.
- `AttractionSwarmNiagara`의 위치 자체가 구심점이다.
- `BP_Beehive`에서 이 컴포넌트를 이동해 구심점 위치를 조정할 수 있어야 한다.
- Niagara System asset은 `BP_Beehive`에서 컴포넌트 details에 지정한다.

### Settings 추가

`ABeehive`에 구조체 노출:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Attraction Swarm")
FBeehiveAttractionSwarmSettings AttractionSwarmSettings;
```

### Apply 함수

권장 public 또는 protected 함수:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Attraction Swarm")
void ApplyAttractionSwarmSettings();

int32 CalculateAttractionSwarmSpawnAmount() const;
```

`CalculateAttractionSwarmSpawnAmount()`:

```text
BeeCount = Max(0, ColonyBeeCount)
Scale = Max(0.0, AttractionSwarmSettings.SpawnAmountScale)
MaxSpawn = Max(0, AttractionSwarmSettings.MaxSpawnAmount)
Raw = BeeCount * Scale
Rounded = RoundToInt(Raw)
Result = Clamp(Rounded, 0, MaxSpawn)
```

`ApplyAttractionSwarmSettings()`:

1. `AttractionSwarmNiagara` 유효성 확인
2. float user parameters 적용
   - `User.AttractionPower`
   - `User.NoisePower`
   - `User.SpawnSphereRadius`
3. int user parameter 적용
   - `User.SpawnAmount`
4. 모든 값은 음수 방지 clamp 적용

Niagara setter:

```cpp
AttractionSwarmNiagara->SetVariableFloat(TEXT("User.AttractionPower"), FMath::Max(0.0f, AttractionSwarmSettings.AttractionPower));
AttractionSwarmNiagara->SetVariableFloat(TEXT("User.NoisePower"), FMath::Max(0.0f, AttractionSwarmSettings.NoisePower));
AttractionSwarmNiagara->SetVariableFloat(TEXT("User.SpawnSphereRadius"), FMath::Max(0.0f, AttractionSwarmSettings.SpawnSphereRadius));
AttractionSwarmNiagara->SetVariableInt(TEXT("User.SpawnAmount"), CalculateAttractionSwarmSpawnAmount());
```

### 호출 시점

필수:

- `ABeehive::OnConstruction`
- `ABeehive::BeginPlay`
- `ABeehive::PostEditChangeProperty`
- `ApplyAttractionSwarmSettings()` 명시 호출 시

`ColonyBeeCount` 변경 시 즉시 반영:

- 현재 `ColonyBeeCount`가 단순 UPROPERTY라면 `PostEditChangeProperty`에서 변경 감지 후 `ApplyAttractionSwarmSettings()` 호출
- gameplay에서 벌 수를 변경하는 setter/API가 있다면 setter에서 `ApplyAttractionSwarmSettings()` 호출
- setter가 없다면 이번 구현에서 `SetColonyBeeCount(int32 NewBeeCount)` 추가를 고려한다.

권장 setter:

```cpp
UFUNCTION(BlueprintCallable, Category = "Beehive|Bee Swarm")
void SetColonyBeeCount(int32 NewBeeCount);
```

정책:

- `ColonyBeeCount = Max(0, NewBeeCount)`
- 기존 dual swarm settings도 벌 수에 의존하면 기존 적용 함수도 같이 호출
- 신규 attraction swarm settings도 같이 호출

### 시간 기반 자동 업데이트 금지

다음 작업을 하지 않는다.

- `UGameTimeBucketSubsystem` subscription 추가
- `AttractionSwarm`용 `SubscriptionTag` 추가
- `OnGameTimeBucketEvent`에서 `ApplyAttractionSwarmSettings()` 호출
- `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`에 Attraction swarm 갱신 연결
- 10분 경계마다 `SpawnAmount` 업데이트

`ApplyBeeSwarmHour24()`가 기존 spline swarm을 갱신하더라도, 신규 Attraction swarm의 `SpawnAmount` 시간 갱신은 넣지 않는다.

단, `ApplyAttractionSwarmSettings()`가 다른 일반 설정 적용 경로에서 호출되는 것은 허용된다.

## Niagara User Parameter 숨김

요구사항은 `AttractionSwarmNiagara`의 User Parameter를 모두 숨기는 것이다.

기존 `BeehiveDualSwarmActorCustomization`이 있다면 확장하거나, Beehive용 customization을 추가한다.

권장:

- `ABeehive` details customization 또는 NiagaraComponent details customization에서 owner가 `ABeehive`이고 component name이 `AttractionSwarmNiagara`인 경우 User Parameter 관련 category/property를 숨긴다.
- 숨김이 완전하지 않더라도 `ApplyAttractionSwarmSettings()`가 Beehive settings로 항상 덮어쓰도록 유지한다.

주의:

- Editor-only customization은 `#if WITH_EDITOR` / editor module boundary를 안전하게 처리한다.
- 런타임 패키징에 `UnrealEd`, `PropertyEditor` 의존이 섞이지 않도록 주의한다.
- 현재 프로젝트가 runtime module에서 editor customization을 처리 중이면 기존 패턴을 따른다.

## 기존 Dual Spline Swarm과 관계

- 기존 Outgoing/Ingoing spline swarm은 유지한다.
- 신규 Attraction swarm은 동시에 동작하는 부가 Niagara다.
- 기존 dual swarm child actor 구조를 제거하거나 비활성화하지 않는다.
- `ApplyBeeSwarmHour24()`의 기존 역할을 불필요하게 바꾸지 않는다.

## 금지 작업

- 별도 `ABeehiveAttractionSwarmActor` 생성
- `UChildActorComponent`로 Attraction swarm 소유
- `AttractionSwarmCenter` SceneComponent 추가
- C++에서 Niagara System asset path 직접 로드
- `Content/` asset 자동 생성/수정
- 시간 bucket/10분 경계/시간 delegate에 Attraction swarm `SpawnAmount` 자동 갱신 연결
- `User.SpawnAmount`에 `SetVariableFloat` 사용
- `AttractionPower`, `NoisePower`, `SpawnSphereRadius`, `SpawnAmount`를 NiagaraComponent details에서 직접 수정하도록 방치
- 기존 Outgoing/Ingoing spline swarm 비활성화

## 문서 반영

### `.md/Architecture/WorldActorsSystem.md`

추가:

- `ABeehive`가 `AttractionSwarmNiagara`를 직접 소유
- `AttractionSwarmNiagara` 위치가 구심점
- `FBeehiveAttractionSwarmSettings`
- Niagara User Parameter 목록과 적용 책임
- `SpawnAmount = RoundToInt(ColonyBeeCount * SpawnAmountScale)` 후 `MaxSpawnAmount` clamp
- 시간 기반 자동 업데이트는 사용하지 않음
- 기존 dual spline swarm과 동시에 동작

### `.md/0_ARCHITECTURE.md`

- Beehive 구성 요약에 Attraction swarm 추가

### `.md/USER_UNREAL.md`

에디터 작업:

- `BP_Beehive` 열기
- `AttractionSwarmNiagara` 컴포넌트에 Niagara System asset 지정
- `AttractionSwarmNiagara` 컴포넌트 위치를 구심점 위치로 조정
- `AttractionSwarmSettings` 값 설정
  - `AttractionPower`
  - `NoisePower`
  - `SpawnSphereRadius`
  - `SpawnAmountScale`
  - `MaxSpawnAmount`
- Niagara asset에 User Parameter 타입 확인
  - `User.AttractionPower` Float
  - `User.NoisePower` Float
  - `User.SpawnSphereRadius` Float
  - `User.SpawnAmount` Int32

### `.md/PROMPT_REVIEW.md`

리뷰 체크:

- `ABeehive`가 직접 NiagaraComponent를 소유하는지
- 별도 child actor가 생기지 않았는지
- Attraction swarm에 시간 bucket subscription이 추가되지 않았는지
- `SetVariableInt("User.SpawnAmount")`를 쓰는지
- `ColonyBeeCount` 변경 시 즉시 SpawnAmount가 반영되는지
- User Parameter 숨김 구현의 editor boundary가 안전한지
- 기존 dual spline swarm 동작이 유지되는지

## 검증 명령

```powershell
rg "AttractionSwarm|AttractionPower|NoisePower|SpawnSphereRadius" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg "SetVariableInt\\(.*SpawnAmount|SetVariableFloat\\(.*SpawnAmount" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
rg "AttractionSwarm.*Bucket|AttractionSwarm.*Hour|AttractionSwarm.*Time|SubscriptionTag.*Attraction" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
```

빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 검증

1. `BP_Beehive`에 `AttractionSwarmNiagara` 컴포넌트가 보이는지 확인한다.
2. `AttractionSwarmNiagara`에 Niagara System asset을 지정한다.
3. `AttractionSwarmNiagara` 컴포넌트를 이동해 구심점 위치가 바뀌는지 확인한다.
4. `AttractionSwarmSettings.AttractionPower` 변경 시 즉시 Niagara에 반영되는지 확인한다.
5. `NoisePower`, `SpawnSphereRadius`도 동일하게 확인한다.
6. `ColonyBeeCount=100`, `SpawnAmountScale=0.5`이면 `User.SpawnAmount=50`이 적용되는지 확인한다.
7. `MaxSpawnAmount`보다 큰 계산값은 clamp되는지 확인한다.
8. `ColonyBeeCount=0`이면 `User.SpawnAmount=0`인지 확인한다.
9. 시간 경계/10분 bucket 변화만으로 Attraction swarm `SpawnAmount`가 재적용되지 않는지 확인한다.
10. 기존 Outgoing/Ingoing spline swarm이 그대로 동작하는지 확인한다.
11. `AttractionSwarmNiagara`의 User Parameter 직접 수정 UI가 숨겨졌는지 확인한다.

## 완료 기준

- `ABeehive`가 `AttractionSwarmNiagara`를 직접 소유한다.
- `AttractionSwarmNiagara` 위치가 구심점이다.
- `FBeehiveAttractionSwarmSettings`가 노출된다.
- `AttractionPower`, `NoisePower`, `SpawnSphereRadius` 변경이 즉시 반영된다.
- `SpawnAmount`는 `ColonyBeeCount * SpawnAmountScale` 기반으로 계산된다.
- `SpawnAmount`는 `RoundToInt`와 `MaxSpawnAmount` clamp를 거친다.
- `SpawnAmount`는 `SetVariableInt`로 적용된다.
- 특정 시간/10분 경계 자동 업데이트는 없다.
- `ColonyBeeCount` 변경 시 즉시 `SpawnAmount`가 반영된다.
- Niagara User Parameter 직접 수정 UI가 숨겨진다.
- 기존 dual spline swarm은 유지된다.
- 관련 문서가 실제 구현과 일치한다.
