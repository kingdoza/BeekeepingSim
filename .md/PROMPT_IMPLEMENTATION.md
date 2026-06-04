# 벌통 질병 VFX 단일화 구현 프롬프트

## 목표

현재 위생 기반 질병 기능은 벌통 위생도(`SanitationValue`)에서 `DiseaseRatio(0..1)`를 계산하고, 이 값을 벌떼 Niagara/소비장 Niagara/여왕벌 material에 직접 전파한다.

이번 변경은 **질병 시각 표현 경로만** 바꾼다.

- 유지: 위생 임계값 기반 `DiseaseRatio` 계산
- 유지: `SanitationDecreaseMultiplier`를 colony population 감소량에 반영
- 변경: 벌들의 Niagara와 여왕벌 material에 직접 `Disease`를 넣는 코드는 주석처리
- 추가: `ABeehive`가 직접 소유하는 질병 VFX Niagara 컴포넌트에만 `User.Disease`를 세팅

질병 VFX Niagara는 C++ 생성자 단계에서 `CreateDefaultSubobject`로 생성하고, `ABeehive` 멤버 변수로 참조한다. Niagara System asset과 transform은 Blueprint/Details에서 authoring한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

필요 시 함께 확인한다.

- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

## 현재 Source 전제

현재 Source에는 이전 질병 시각 전파 구현이 이미 들어와 있다.

- `ABeehive`
  - `SanitationDiseaseThreshold`
  - `MaxSanitationBeeDecreaseMultiplier`
  - `GetSanitationDiseaseRatio()`
  - `GetSanitationBeeDecreaseMultiplier()`
  - `RefreshHiveDiseaseVisuals()`
  - `ApplyDiseaseToCombActors(...)`
  - `ApplyDiseaseToQueenBee(...)`
- `ABeehiveDualSwarmActor`
  - outgoing/ingoing Niagara에 `User.Disease` 세팅
- `ABeehiveCombActor`
  - front/back bee Niagara에 `User.Disease` 세팅
  - `SetBeeDiseaseValue(...)`
- `AQueenBeeActor`
  - material scalar `Disease` 세팅

이번 작업은 이 구조를 전부 삭제하는 리팩토링이 아니다. 기존 public API와 property는 가능한 유지하고, 기존 시각 적용 코드만 주석처리해서 Blueprint/API 파손을 피한다.

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`
- 필요 시 `.md/PROMPT_REVIEW.md`

수정 금지:

- `Content/` asset 직접 수정/저장
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect가 필요한 rename
- `UGameTimeBucketSubsystem` 또는 Environment 시간 bucket 구조 변경
- 별도 disease tick actor/subsystem/component 추가
- 별도 disease bucket subscription 추가

## 유지할 gameplay 계산

아래 계산은 유지한다.

```text
EffectiveThreshold = Clamp(SanitationDiseaseThreshold, 0, MaxSanitationValue)

if EffectiveThreshold <= 0:
  DiseaseRatio = 0
else if SanitationValue >= EffectiveThreshold:
  DiseaseRatio = 0
else:
  DiseaseRatio = Clamp((EffectiveThreshold - SanitationValue) / EffectiveThreshold, 0, 1)
```

감소 배율도 유지한다.

```text
SanitationDecreaseMultiplier =
  Lerp(1.0, MaxSanitationBeeDecreaseMultiplier, DiseaseRatio)
```

`CalculateBeeDecreaseAmount()`는 `SanitationDecreaseMultiplier`를 `BaseDecrease` 계산 단계에서 반영하는 현재 정책을 유지한다.

```text
BaseDecrease =
  ((ProportionalDecrease + AbsoluteDecrease)
    / ItemLifespanBonus
    / TemperatureScore)
  * SanitationDecreaseMultiplier

Decrease = Min(ColonyBeeCount, BaseDecrease)
```

## `ABeehive` 변경

### 질병 VFX 컴포넌트 추가

`ABeehive`에 새 Niagara component를 추가한다.

Header 권장 선언:

```cpp
UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Beehive|Sanitation Disease")
TObjectPtr<UNiagaraComponent> DiseaseVfxNiagara;
```

Constructor 권장 구현:

```cpp
DiseaseVfxNiagara = CreateDefaultSubobject<UNiagaraComponent>(TEXT("DiseaseVfxNiagara"));
DiseaseVfxNiagara->SetupAttachment(Root);
```

주의:

- C++ 생성자 단계에서 생성한다.
- 별도 actor로 spawn하지 않는다.
- Niagara System asset은 C++에서 hard reference로 지정하지 않는다.
- BP/Details에서 `DiseaseVfxNiagara`의 Niagara System과 transform을 authoring한다.

### Disease parameter 적용

새 parameter name은 별도 namespace에 둔다.

```cpp
namespace BeehiveDiseaseVfxNames
{
	static const FName Disease(TEXT("User.Disease"));
}
```

`RefreshHiveDiseaseVisuals()`는 이제 벌/여왕벌이 아니라 벌통 질병 VFX에만 `DiseaseRatio`를 주입한다.

권장 형태:

```cpp
void ABeehive::RefreshHiveDiseaseVisuals()
{
	const float DiseaseRatio = GetSanitationDiseaseRatio();
	if (DiseaseVfxNiagara)
	{
		DiseaseVfxNiagara->SetVariableFloat(BeehiveDiseaseVfxNames::Disease, DiseaseRatio);
	}

	// 기존 벌떼/소비장/여왕벌 Disease 직접 표현은 비활성화한다.
	// ApplySettingsToDualSwarmChildActor();
	// ApplyDiseaseToCombActors(DiseaseRatio);
	// ApplyDiseaseToQueenBee(DiseaseRatio);
}
```

유지할 호출 시점:

- `SetSanitationValue()`에서 clamp 후 `RefreshHiveDiseaseVisuals()`
- `OnConstruction()` / `BeginPlay()` / `PostEditChangeProperty()`에서 기존처럼 `SetSanitationValue(SanitationValue)` 또는 refresh 경로 유지
- comb 배치/회수 후 refresh 호출이 남아 있어도 무방하지만, 새 정책에서는 comb에 직접 적용하지 않아야 한다.

### 기존 attraction swarm Disease 적용 주석처리

`ApplyAttractionSwarmSettings()` 안의 attraction swarm disease 직접 세팅은 주석처리한다.

```cpp
// AttractionSwarmNiagara->SetVariableFloat(BeehiveAttractionSwarmNames::Disease, GetSanitationDiseaseRatio());
```

`BeehiveAttractionSwarmNames::Disease` 상수도 해당 용도로만 쓰이면 주석처리한다.

```cpp
// static const FName Disease(TEXT("User.Disease"));
```

주의:

- `AttractionSwarmNiagara` 자체는 유지한다.
- attraction swarm의 spawn/attraction/noise 설정 로직은 변경하지 않는다.

## `ABeehiveDualSwarmActor` 변경

outgoing/ingoing swarm Niagara의 `User.Disease` 직접 적용 코드를 주석처리한다.

주석처리 대상:

```cpp
// static const FName Disease(TEXT("User.Disease"));
```

```cpp
// OutgoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::Disease, FMath::Clamp(Parameters.Disease, 0.0f, 1.0f));
// IngoingNiagara->SetVariableFloat(BeehiveDualSwarmActorNames::Disease, FMath::Clamp(Parameters.Disease, 0.0f, 1.0f));
```

`FBeehiveDualSwarmNiagaraParameters::Disease`와 `ABeehive::BuildDualSwarmParameters()`의 `Parameters.Disease = ...`는 삭제하지 말고, 필요하면 주석처리한다.

권장:

```cpp
// Parameters.Disease = GetSanitationDiseaseRatio();
```

이유:

- struct field 삭제는 Blueprint/API 영향이 커질 수 있다.
- 이번 작업은 시각 전파 경로 변경이지 DTO 정리 리팩토링이 아니다.

## `ABeehiveCombActor` 변경

소비장 front/back bee Niagara의 `User.Disease` 직접 적용 코드를 주석처리한다.

주석처리 대상:

```cpp
// static const FName Disease(TEXT("User.Disease"));
```

```cpp
// FrontFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, FMath::Clamp(BeeDiseaseValue, 0.0f, 1.0f));
// BackFaceBeeNiagara->SetVariableFloat(BeehiveCombActorNames::Disease, FMath::Clamp(BeeDiseaseValue, 0.0f, 1.0f));
```

`SetBeeDiseaseValue(...)`, `GetBeeDiseaseValue()`, `BeeDiseaseValue`는 삭제하지 않는다. 단, 함수 내부의 실제 Niagara parameter 적용은 더 이상 일어나지 않아야 한다.

권장 형태:

```cpp
void ABeehiveCombActor::SetBeeDiseaseValue(float NewDiseaseValue)
{
	BeeDiseaseValue = FMath::Clamp(NewDiseaseValue, 0.0f, 1.0f);
	// Disease is now represented by ABeehive::DiseaseVfxNiagara.
	// ApplyNiagaraUserParameters();
}
```

`ABeehive::RefreshCombSpawnAmounts(...)` 안에 있는 `CombActor->SetBeeDiseaseValue(GetSanitationDiseaseRatio())`도 주석처리한다.

```cpp
// CombActor->SetBeeDiseaseValue(GetSanitationDiseaseRatio());
```

## `AQueenBeeActor` 변경

여왕벌 material scalar `Disease` 직접 적용 코드를 주석처리한다.

삭제하지 말고 유지할 것:

- `SetDiseaseValue(...)`
- `GetDiseaseValue()`
- `DiseaseMaterialParameterName`
- `DiseaseValue`
- material instance helper 함수 선언/정의

주석처리 대상:

```cpp
// ApplyDiseaseMaterialParameter();
```

적용 위치:

- `OnConstruction(...)`
- `BeginPlay()`
- `SetDiseaseValue(...)`

또는 `ApplyDiseaseMaterialParameter()` 내부의 실제 scalar 세팅 라인만 주석처리한다.

```cpp
// MaterialInstance->SetScalarParameterValue(DiseaseMaterialParameterName, DiseaseValue);
```

권장:

- `SetDiseaseValue(...)`는 값 clamp까지만 유지한다.
- 실제 material parameter 적용은 주석처리한다.

`ABeehive::ApplyDiseaseToQueenBee(...)`가 호출되는 경로도 주석처리한다.

```cpp
// ApplyDiseaseToQueenBee(DiseaseRatio);
```

## `ABeehive` helper 정리 정책

다음 helper는 삭제하지 않는다.

- `ApplyDiseaseToCombActors(float DiseaseRatio)`
- `ApplyDiseaseToQueenBee(float DiseaseRatio)`

대신 `RefreshHiveDiseaseVisuals()`에서 호출을 주석처리한다. 필요하면 helper 내부 실제 적용 라인도 주석처리한다.

이유:

- 이번 변경은 전파 경로 비활성화이며 API/구조 정리 리팩토링이 아니다.
- 주석처리 요구사항을 만족하면서 이후 되돌리기 쉽다.

## 문서 갱신

구현 후 실제 코드와 일치하도록 아래 문서를 갱신한다.

`.md/0_ARCHITECTURE.md`

- `ABeehive`의 sanitation disease source of truth는 유지
- disease visual 전파 대상은 더 이상 attraction/outgoing/ingoing/comb/queen이 아님을 반영
- 새 대상은 `ABeehive::DiseaseVfxNiagara` 단일 Niagara component
- `DiseaseVfxNiagara.User.Disease`에 `GetSanitationDiseaseRatio()` 값을 세팅한다고 명시
- colony population 감소식은 유지

`.md/Architecture/WorldActorsSystem.md`

- `ABeehive` composition에 `UNiagaraComponent DiseaseVfxNiagara` 추가
- `RefreshHiveDiseaseVisuals()`는 `DiseaseVfxNiagara.User.Disease`만 갱신한다고 변경
- 기존 벌떼/소비장/여왕벌 Disease 직접 표현 코드는 주석처리된 legacy path로 기록
- `ABeehiveDualSwarmActor`, `ABeehiveCombActor`, `AQueenBeeActor`의 직접 Disease 시각 적용 설명 제거 또는 비활성화 상태로 수정

`.md/USER_UNREAL.md`

- `BP_Beehive`의 `DiseaseVfxNiagara`에 질병 VFX Niagara System을 지정해야 함
- 해당 Niagara System에는 float user parameter `User.Disease`가 필요
- 기존 벌떼/소비장 Niagara 및 QueenBee material의 `Disease` 파라미터는 더 이상 필수 수동 작업이 아님

`.md/PROMPT_REVIEW.md` 필요 시

- 리뷰 기준을 새 단일 VFX 경로 기준으로 갱신

## 검색 검증

```powershell
rg "DiseaseVfxNiagara|BeehiveDiseaseVfxNames|RefreshHiveDiseaseVisuals|User.Disease" Source/BeekeepingSim .md
rg "SetBeeDiseaseValue|ApplyDiseaseToCombActors|ApplyDiseaseToQueenBee|SetDiseaseValue|DiseaseMaterialParameterName" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "SetVariableFloat\\(.*Disease|SetScalarParameterValue\\(.*Disease|Parameters\\.Disease" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors
```

확인할 것:

- `ABeehive` 생성자에서 `DiseaseVfxNiagara`가 `CreateDefaultSubobject`로 생성된다.
- `RefreshHiveDiseaseVisuals()`가 `DiseaseVfxNiagara->SetVariableFloat("User.Disease", DiseaseRatio)`를 수행한다.
- 위생값 변경 시 `RefreshHiveDiseaseVisuals()` 호출 경로가 유지된다.
- 벌 수 감소 계산의 `SanitationDecreaseMultiplier`는 유지된다.
- attraction/outgoing/ingoing/comb front/back/queen material의 직접 Disease 적용 라인은 주석처리되어 있다.
- 기존 public API 삭제/rename이 없다.
- Core Redirect가 필요한 rename/delete가 없다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 수동 검증 항목

Editor/PIE에서 확인할 항목:

- `BP_Beehive`의 `DiseaseVfxNiagara`에 `NS_Disease` 같은 질병 VFX Niagara System을 지정한다.
- 질병 VFX Niagara System에 float user parameter `User.Disease`가 있다.
- `SanitationValue >= SanitationDiseaseThreshold`이면 `DiseaseVfxNiagara.User.Disease = 0`
- `SanitationValue`가 threshold 아래로 내려가면 `DiseaseVfxNiagara.User.Disease`가 `0..1`로 증가
- 소독약 hold-use로 `SanitationValue`가 증가하면 `DiseaseVfxNiagara.User.Disease`가 즉시 낮아짐
- 벌떼 Niagara/소비장 Niagara/QueenBee material은 더 이상 Disease 직접 표현 경로에 의존하지 않음
- `MaxSanitationBeeDecreaseMultiplier > 1`일 때 colony population bucket 감소량 증가는 기존처럼 유지됨

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 기존 public API 삭제/rename 없이는 구현이 불가능한 경우
- `DiseaseVfxNiagara`를 `ABeehive` 생성자 subobject가 아니라 runtime spawn actor로 만들어야 한다고 판단되는 경우
- 벌 수 감소 계산까지 바꿔야 한다고 판단되는 경우
- `UGameTimeBucketSubsystem` 또는 별도 disease tick/bucket이 필요하다고 판단되는 경우
- Content asset 수정/저장이 C++ 구현 완료 조건이 되는 경우
- UCLASS/USTRUCT/UENUM rename 또는 Core Redirect 필요성이 생기는 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Core Redirect 불필요 여부
- Blueprint/API 영향
- Content 수동 작업 목록
- 벌떼/소비장/여왕벌 직접 Disease 적용 코드가 주석처리되었는지 확인
- `DiseaseVfxNiagara.User.Disease`가 위생값 변경 시 즉시 갱신되는지 확인
- `SanitationDecreaseMultiplier` 기반 벌 수 감소 계산이 유지되는지 확인
