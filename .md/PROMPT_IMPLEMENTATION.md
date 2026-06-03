# 벌통 벌 수 절대 자연감소치 구현 프롬프트

## 목표

벌통의 colony population 갱신에서 기존 비례 자연감소량에 bucket당 절대 자연감소량을 추가한다.

현재 감소식은 다음과 같다.

```text
Decrease = ColonyBeeCount * BeeDecreaseCoefficient / ItemLifespanBonus / TemperatureScore
```

변경 후 감소식은 다음을 기준으로 한다.

```text
ProportionalDecrease = ColonyBeeCount * BeeDecreaseCoefficient
AbsoluteDecrease = BeeDecreaseAbsoluteAmountPerBucket
RawDecrease = (ProportionalDecrease + AbsoluteDecrease) / ItemLifespanBonus / TemperatureScore
Decrease = Min(ColonyBeeCount, RawDecrease)
```

최종 벌 수 적용은 기존처럼 `ApplyColonyPopulationUpdate()`에서 마지막 단계에만 반올림한다.

```text
NewBeeCount = Max(0, RoundToInt(ColonyBeeCount + Increase - Decrease))
```

## 확정 설계

- 절대감소치는 `ABeehive`의 colony population 설정이다.
- 새 값은 population bucket 1회당 고정 감소량으로 해석한다.
- 기본값은 `0.0f`로 두어 기존 동작을 유지한다.
- 절대감소치도 `ItemLifespanBonus`와 `TemperatureScore` 영향을 받는다.
- 감소량은 현재 bucket 시작 시점의 기존 `ColonyBeeCount`를 초과하지 않는다.
  - 이유: 같은 bucket에서 `Increase`로 새로 늘어난 벌까지 자연감소가 먹지 않게 하기 위해서다.
- `ItemLifespanBonus()`와 `GetTemperatureScore()`의 현재 `1.0f` placeholder 정책은 유지한다.
- `ItemEggLayingBonus`는 기존처럼 증가 항에만 적용하고, 감소 항에는 적용하지 않는다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

필요 시 함께 확인한다.

- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

수정 금지:

- `Content/` asset 직접 수정/저장
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect가 필요한 rename
- `UGameTimeBucketSubsystem` 또는 Environment 시간 bucket 구조 변경
- colony population 전용 신규 subsystem/component 추가

## `ABeehive` Header 변경

`Beehive|Colony Population` 카테고리에 새 property를 추가한다.

권장 위치:

- `BeeIncreaseCoefficient`
- `BeeDecreaseCoefficient`
- 신규 `BeeDecreaseAbsoluteAmountPerBucket`

권장 선언:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population", meta = (ClampMin = "0.0"))
float BeeDecreaseAbsoluteAmountPerBucket = 0.0f;
```

의미:

- `BeeDecreaseCoefficient`: 현재 벌 수에 비례하는 감소율
- `BeeDecreaseAbsoluteAmountPerBucket`: population bucket 1회당 고정 감소량

주의:

- Blueprint에 노출되는 새 `UPROPERTY` 추가이므로 rename/delete가 아니다.
- 기본값이 `0.0f`이면 기존 gameplay 결과가 바뀌지 않아야 한다.
- Core Redirect는 필요하지 않다.

## `ABeehive` 구현 변경

`CalculateBeeDecreaseAmount()`를 수정한다.

권장 구현:

```cpp
float ABeehive::CalculateBeeDecreaseAmount() const
{
	const float CurrentBeeCount = static_cast<float>(FMath::Max(0, ColonyBeeCount));
	const float DecreaseCoefficient = FMath::Max(0.0f, BeeDecreaseCoefficient);
	const float AbsoluteDecreaseAmount = FMath::Max(0.0f, BeeDecreaseAbsoluteAmountPerBucket);
	const float ItemLifespanBonus = FMath::Max(KINDA_SMALL_NUMBER, GetItemLifespanBonus());
	const float TemperatureScore = FMath::Max(KINDA_SMALL_NUMBER, GetTemperatureScore());
	const float RawDecrease = ((CurrentBeeCount * DecreaseCoefficient) + AbsoluteDecreaseAmount)
		/ ItemLifespanBonus
		/ TemperatureScore;

	return FMath::Min(CurrentBeeCount, FMath::Max(0.0f, RawDecrease));
}
```

유지할 것:

- `ApplyColonyPopulationUpdate()`의 최종 `RoundToInt` 정책
- `NewBeeCount` 최소 0 clamp
- `SetColonyBeeCount()` 경로
- `ApplyBeeSwarmSettings()`
- `ApplyAttractionSwarmSettings()`
- `RefreshCombSpawnAmounts(true, true)`
- `ColonyPopulation` bucket subscription과 `LatestOnly` catch-up policy

변경하지 말 것:

- `CalculateBeeIncreaseAmount()` 계산식
- `GetItemEggLayingBonus()` 정책
- `GetItemLifespanBonus()`의 현재 `1.0f` 반환
- `GetTemperatureScore()`의 현재 `1.0f` 반환
- `OnGameTimeBucketEvent_Implementation()`의 subscription branch 구조

## 계산 예시

예시 1: 기존 호환

```text
ColonyBeeCount = 100
BeeDecreaseCoefficient = 0.02
BeeDecreaseAbsoluteAmountPerBucket = 0
ItemLifespanBonus = 1
TemperatureScore = 1

Decrease = 100 * 0.02 = 2
```

예시 2: 절대감소 포함

```text
ColonyBeeCount = 100
BeeDecreaseCoefficient = 0.02
BeeDecreaseAbsoluteAmountPerBucket = 3
ItemLifespanBonus = 1
TemperatureScore = 1

Decrease = (100 * 0.02 + 3) / 1 / 1 = 5
```

예시 3: 감소량이 기존 벌 수보다 큰 경우

```text
ColonyBeeCount = 3
Increase = 8
RawDecrease = 10
Decrease = Min(3, 10) = 3
NewBeeCount = RoundToInt(3 + 8 - 3) = 8
```

이 예시는 같은 bucket에서 새로 증가한 벌 8마리까지 자연감소가 적용되지 않음을 보장한다.

## 문서 갱신

구현 후 실제 코드 이름과 일치하도록 아래 문서를 갱신한다.

`.md/0_ARCHITECTURE.md`

- colony population 계산식 요약에 절대감소 항 추가
- `BeeDecreaseAbsoluteAmountPerBucket` 의미 추가
- `ItemLifespanBonus`/`TemperatureScore`가 전체 감소량에 적용된다는 점 반영
- 감소량이 기존 `ColonyBeeCount`를 초과하지 않는다는 clamp 정책 반영

`.md/Architecture/WorldActorsSystem.md`

- `ABeehive` colony population 설정 목록에 `BeeDecreaseAbsoluteAmountPerBucket` 추가
- colony population 계산식에 절대감소 항 추가
- `ItemEggLayingBonus`는 증가 항 전용, `ItemLifespanBonus`/`TemperatureScore`는 전체 감소 항에 적용된다는 정책 반영
- `ApplyColonyPopulationUpdate()`의 최종 rounding 정책은 그대로 유지된다고 명시

문서에는 과도한 코드 설명을 반복하지 말고, 설계 정책과 public authoring 의미를 중심으로 적는다.

## 검색 검증

```powershell
rg "BeeDecreaseAbsoluteAmountPerBucket|CalculateBeeDecreaseAmount|BeeDecreaseCoefficient|ColonyPopulation" Source/BeekeepingSim .md
rg "Decrease =|colony population 계산식|ItemLifespanBonus|TemperatureScore" .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md
```

확인할 것:

- 새 property가 `ABeehive`에만 추가되었다.
- 기존 `BeeDecreaseCoefficient`는 유지된다.
- 기존 `CalculateBeeDecreaseAmount()` public BlueprintPure API 이름은 유지된다.
- `ApplyColonyPopulationUpdate()`는 마지막 단계에서만 `RoundToInt`한다.
- 기본값 `BeeDecreaseAbsoluteAmountPerBucket = 0.0f`에서 기존 계산과 동일하다.
- 절대감소치도 `ItemLifespanBonus`와 `TemperatureScore`의 영향을 받는다.
- 감소량은 기존 `ColonyBeeCount`를 초과하지 않는다.
- Core Redirect가 필요한 rename/delete가 없다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 수동 검증 항목

Editor/PIE에서 확인할 항목:

- `BeeDecreaseAbsoluteAmountPerBucket = 0`이면 기존 population 변화와 동일하다.
- `BeeDecreaseAbsoluteAmountPerBucket > 0`이면 population bucket마다 추가 감소가 반영된다.
- 벌 수가 매우 적을 때 절대감소량이 커도 최종 벌 수가 음수가 되지 않는다.
- 벌 수가 매우 적고 증가량이 있는 경우, 감소량이 기존 벌 수까지만 적용되어 신규 증가분을 바로 깎지 않는다.
- 벌 수 변경 후 외부 벌떼, attraction swarm, 소비장 벌 표시량이 기존 갱신 경로로 갱신된다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 기존 `CalculateBeeDecreaseAmount()` API 삭제/rename이 필요해지는 경우
- `BeeDecreaseAbsoluteAmountPerBucket`을 `ABeehive`가 아닌 별도 settings/data asset에 둬야 한다고 판단되는 경우
- 절대감소치가 bucket 길이에 따라 자동 스케일되어야 한다고 판단되는 경우
- `ItemLifespanBonus` 또는 `TemperatureScore` 의미 변경이 필요해지는 경우
- Environment bucket 시스템 변경이 필요해지는 경우
- Content asset 수정/저장이 구현 완료 조건이 되는 경우
- UCLASS/USTRUCT/UENUM rename 또는 Core Redirect 필요성이 생기는 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Core Redirect 불필요 여부
- Blueprint/API 영향
- 기본값 `0.0f`에서 기존 동작 유지 여부
- 절대감소치가 `ItemLifespanBonus`/`TemperatureScore` 영향을 받는다는 확인
