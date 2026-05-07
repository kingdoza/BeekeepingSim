# 구현 리뷰 프롬프트: Beehive Honey System (60분 생산)

## 우선순위

1. High: HoneyProduction 60분 bucket 구독/처리 순서 정확성
2. High: 꿀 생산량 계산/분배/클램프 정책 정확성
3. High: `ABeehiveCombActor` 꿀 상태(절대값)와 시각값(정규화 ratio) 분리 정확성
4. Medium: 기존 BeeSwarm/Queen/ColonyPopulation 회귀 여부

---

## 리뷰 대상

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

---

## 핵심 검증 항목

### High 1: bucket 구독 및 처리 순서

- `GetGameTimeBucketSubscriptions_Implementation()`에서 순서가 아래와 같은지:
  1. `BeeSwarm`
  2. `QueenBeeLocation`
  3. `HoneyProduction`
  4. `ColonyPopulation`
- `OnGameTimeBucketEvent_Implementation()`에 `HoneyProduction -> ApplyHoneyProductionUpdate()` 분기가 있는지
- 같은 60분 경계에서 꿀 생산이 colony population보다 먼저 처리되는지

### High 2: 꿀 생산량 계산 및 분배

- `CalculateTotalHoneyIncreaseAmount()` 공식이 `ColonyBeeCount * HoneyProductionCoefficient`인지
- 음수 방어 (`ColonyBeeCount>=0`, `HoneyProductionCoefficient>=0`)가 있는지
- active comb 수 0 또는 `TotalHoneyIncrease<=0`이면 no-op인지
- 분배 방식이 랜덤 가중치 정규화인지:
  - `Weight = Rand[1-d, 1+d]`, `d=Clamp(HoneyDistributionDeviationRatio,0,1)`
  - `Increase_i = Total * Weight_i / WeightSum`
- 소비장이 포화되지 않은 경우 분배 총합이 총생산량과 일치하는지
- 포화로 인한 초과분 재분배/저장고 누적 없이 폐기되는지
- 꿀 업데이트가 lifted comb 포함 모든 active comb에 적용되는지

### High 3: Comb honey 상태/시각 분리

- `ABeehiveCombActor`에 다음이 존재하는지:
  - `FrontHoneyPlane`, `BackHoneyPlane`
  - `MaxHoneyPerComb`, `CurrentHoney`
  - empty/full relative location들
  - `HoneyMaterialParameterName`
  - MID 캐시(`FrontHoneyMaterialInstance`, `BackHoneyMaterialInstance`)
- API:
  - `AddHoneyAmount`, `SetCurrentHoney`, `GetCurrentHoney`, `GetHoneyFillRatio`
- Honey clamp:
  - `MaxHoneyPerComb >= KINDA_SMALL_NUMBER`
  - `CurrentHoney`는 `0..MaxHoneyPerComb`
- 시각 업데이트:
  - ratio=`Clamp(CurrentHoney/MaxHoneyPerComb,0,1)`
  - plane 위치를 empty/full 사이 보간
  - material index 0 scalar(`HoneyAmount`)에 ratio 적용
  - Front/Back 중 하나가 null이어도 나머지 적용 가능

### Medium 1: 기존 기능 회귀

- `SetColonyBeeCount` / `ApplyColonyPopulationUpdate` / `UpdateQueenBeeLocation` 동작 회귀 없는지
- 기존 comb bee Niagara 적용/재초기화 경로가 유지되는지
- lifted comb skip 정책(`RefreshCombSpawnAmounts(true)`)이 꿀 업데이트와 섞이지 않았는지

### Medium 2: 문서 정합성

- `.md/0_ARCHITECTURE.md`에 HoneyProduction, 순서, 공식, 절대값/ratio 분리가 반영됐는지
- `.md/Architecture/WorldActorsSystem.md`에 Honey 설정/버킷/분배/시각 정책이 반영됐는지

---

## 빌드/검색 검증

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg "HoneyProduction|ApplyHoneyProductionUpdate|CalculateTotalHoneyIncreaseAmount" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`
  - `rg "FrontHoneyPlane|BackHoneyPlane|CurrentHoney|MaxHoneyPerComb|HoneyAmount|GetHoneyFillRatio" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`
  - `rg "ColonyPopulation|QueenBeeLocation|BeeSwarm" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp -n`

---

## 리뷰 결과 출력 형식

1. Findings (High → Medium → Low)
2. Open Questions / Assumptions
3. Regression Risks
4. 최종 판단: Pass / Conditional Pass / Fail
