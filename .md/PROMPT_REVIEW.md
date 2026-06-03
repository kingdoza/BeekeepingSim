# 리뷰 프롬프트: 벌통 벌 수 절대 자연감소치 구현

## 리뷰 목적

이번 리뷰는 `ABeehive`의 colony population 갱신에서 기존 비례 자연감소량에 bucket당 절대 자연감소량이 설계대로 추가되었는지 검증한다.

핵심:
- 절대감소치는 `ABeehive`의 `Beehive|Colony Population` authoring 값이다.
- `BeeDecreaseAbsoluteAmountPerBucket` 기본값은 `0.0f`이며 기존 gameplay 결과를 유지해야 한다.
- 감소량은 비례 감소량과 절대 감소량을 더한 뒤 `ItemLifespanBonus`와 `TemperatureScore` 영향을 받는다.
- 감소량은 bucket 시작 시점의 기존 `ColonyBeeCount`를 초과하면 안 된다.
- 최종 벌 수 rounding은 기존처럼 `ApplyColonyPopulationUpdate()` 마지막 단계에서만 수행되어야 한다.

제외:
- `Content/` asset 직접 수정/저장 검토
- `UGameTimeBucketSubsystem` 또는 Environment bucket 구조 변경
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect 추가
- colony population 전용 신규 subsystem/component 추가

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/AGENT_IMPLEMENTATION.md`
- `.md/QNA_REVIEW.md`
- `.md/QNA_IMPLEMENTATION.md`

---

## 리뷰 범위 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

주의:
- 현재 워크트리에는 이번 변경 전부터 `.md/PROMPT_IMPLEMENTATION.md`와 `Content/` 에셋 변경이 있을 수 있다. 이번 리뷰의 주 검토 대상에서 제외하고 revert하지 않는다.
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`, `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`, 위 두 아키텍처 문서 변경만 이번 구현과 직접 연결해 본다.

---

## 구현 요약

`ABeehive`에 다음 property가 추가되었다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Colony Population", meta = (ClampMin = "0.0"))
float BeeDecreaseAbsoluteAmountPerBucket = 0.0f;
```

`CalculateBeeDecreaseAmount()`는 다음 정책을 따라야 한다.

```text
ProportionalDecrease = ColonyBeeCount * BeeDecreaseCoefficient
AbsoluteDecrease = BeeDecreaseAbsoluteAmountPerBucket
RawDecrease = (ProportionalDecrease + AbsoluteDecrease) / ItemLifespanBonus / TemperatureScore
Decrease = Min(ColonyBeeCount, Max(0, RawDecrease))
```

`ApplyColonyPopulationUpdate()`는 기존 최종 적용 정책을 유지해야 한다.

```text
NewBeeCount = Max(0, RoundToInt(ColonyBeeCount + Increase - Decrease))
```

---

## 핵심 검증 질문

1. `BeeDecreaseAbsoluteAmountPerBucket`가 `ABeehive`의 `Beehive|Colony Population` category에 `EditAnywhere`, `BlueprintReadOnly`, `ClampMin=0.0`, 기본값 `0.0f`로 추가되었는가?
2. 새 property 추가 외에 기존 BlueprintCallable/BlueprintPure API 삭제 또는 rename이 없는가?
3. `BeeDecreaseCoefficient`가 유지되고, 의미가 현재 벌 수에 비례하는 감소율로 남아 있는가?
4. `CalculateBeeDecreaseAmount()`가 `ColonyBeeCount`, `BeeDecreaseCoefficient`, `BeeDecreaseAbsoluteAmountPerBucket`를 각각 음수 방어 처리하는가?
5. `ItemLifespanBonus`와 `TemperatureScore` divisor가 `KINDA_SMALL_NUMBER` 이상으로 보호되는가?
6. 절대감소치도 `ItemLifespanBonus`와 `TemperatureScore` 영향을 받는가?
7. 최종 감소량이 기존 `ColonyBeeCount`를 초과하지 않도록 clamp되는가?
8. `BeeDecreaseAbsoluteAmountPerBucket = 0.0f`일 때 기존 감소식과 동일한 결과인가?
9. `ItemEggLayingBonus`가 증가 항 전용으로 유지되고 감소 항에 적용되지 않는가?
10. `CalculateBeeIncreaseAmount()`, `GetItemEggLayingBonus()`, `GetItemLifespanBonus()`, `GetTemperatureScore()` 정책이 불필요하게 변경되지 않았는가?
11. `ApplyColonyPopulationUpdate()`에서 `RoundToInt`가 마지막 단계에만 남아 있는가?
12. `ApplyColonyPopulationUpdate()`가 기존 갱신 경로를 유지하는가?
    - `ApplyBeeSwarmSettings()`
    - `ApplyAttractionSwarmSettings()`
    - `RefreshCombSpawnAmounts(true, true)`
13. `ColonyPopulation` bucket subscription과 `LatestOnly` catch-up policy가 변경되지 않았는가?
14. Core Redirect가 필요한 rename/delete가 없는가?
15. 아키텍처 문서가 실제 코드 이름과 public authoring 의미를 정확히 반영하는가?

---

## 검색 검증

```powershell
rg "BeeDecreaseAbsoluteAmountPerBucket|CalculateBeeDecreaseAmount|BeeDecreaseCoefficient|ColonyPopulation" Source/BeekeepingSim .md
rg "Decrease =|colony population 계산식|ItemLifespanBonus|TemperatureScore" .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md
rg "FMath::RoundToInt\\(RawNewBeeCount\\)|ApplyBeeSwarmSettings\\(\\)|ApplyAttractionSwarmSettings\\(\\)|RefreshCombSpawnAmounts\\(true, true\\)" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp
```

확인할 것:
- 새 property가 `ABeehive`에만 추가되었다.
- 기존 `CalculateBeeDecreaseAmount()` public BlueprintPure API 이름은 유지된다.
- `ApplyColonyPopulationUpdate()`는 마지막 단계에서만 `RoundToInt`한다.
- 절대감소치도 `ItemLifespanBonus`와 `TemperatureScore`의 영향을 받는다.
- 감소량은 기존 `ColonyBeeCount`를 초과하지 않는다.
- 문서의 계산식과 코드가 일치한다.

---

## 빌드 검증

권장:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

구현 시점 빌드 결과:
- `BeekeepingSimEditor Win64 Development`
- `Result: Succeeded`

리뷰에서는 필요하면 재빌드하거나 기존 빌드 결과의 신뢰 범위를 보고한다.

---

## 수동 검증 포인트 (Editor/PIE)

1. `BeeDecreaseAbsoluteAmountPerBucket = 0`이면 기존 population 변화와 동일한지 확인
2. `BeeDecreaseAbsoluteAmountPerBucket > 0`이면 population bucket마다 추가 감소가 반영되는지 확인
3. 벌 수가 매우 적고 절대감소량이 커도 최종 벌 수가 음수가 되지 않는지 확인
4. 벌 수가 매우 적고 증가량이 있는 경우, 감소량이 기존 벌 수까지만 적용되어 신규 증가분을 바로 깎지 않는지 확인
5. 벌 수 변경 후 외부 벌떼, attraction swarm, 소비장 벌 표시량이 기존 갱신 경로로 갱신되는지 확인

---

## 리뷰 결과 출력 형식

`.md/AGENT_REVIEW.md`의 출력 형식을 따른다.

특히:
- Findings를 우선 제시하고 `High -> Medium -> Low` 순서로 정렬
- 각 Finding에 파일/라인, 원인, 영향, 수정 제안 포함
- 이슈가 없으면 `No blocking issues found.`를 명시
- Blueprint/API 영향과 Core Redirect 불필요 여부를 별도 확인
- 남은 검증 공백이 있으면 Editor/PIE 수동 검증 항목으로만 분리
