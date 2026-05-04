# 코드 리뷰 요청 프롬프트 (Beehive Comb Actor 구현)

이번 변경의 리뷰 대상은 `ABeehiveCombActor` 신규 추가와 `ABeehive`의 comb slot 관리/배치/파라미터 전달 확장, 그리고 Niagara details customization 조건 확장이다.

## 리뷰 목표

1. `ABeehive`가 `MaxCombCount` 슬롯을 유지하고 `CurrentCombCount`와 활성 comb actor 수를 일치시키는지
2. `CombRackRoot` local space 기준 배치(`slot i -> FVector(0, -i * CombSlotSpacing, 0)`)가 올바른지
3. SpawnAmount/TargetBeeCount 계산 및 clamp 규칙이 QnA 정본과 일치하는지
4. SpawnAmount 변경 경로에서 active comb의 TargetBeeCount 리셋이 누락 없이 적용되는지
5. `FrontFaceBeeNiagara`/`BackFaceBeeNiagara`의 `OverrideParameters` 숨김 정책이 정확히 적용되는지
6. 기존 outgoing/ingoing spline swarm 및 attraction swarm 동작 회귀가 없는지

## 필수 검증 포인트

### A. 신규 클래스/구성
- `ABeehiveCombActor` 파일:
  - `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
  - `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- 컴포넌트:
  - `Root`
  - `CombMesh`
  - `FrontFaceBeeNiagara`
  - `BackFaceBeeNiagara`
- 제공 API:
  - `ApplyCombBeeParameters`
  - `SetSpawnAmountAndResetTargetBeeCount`
  - `SetTargetBeeCount`
  - `ResetTargetBeeCountToSpawnAmount`
  - `ReduceTargetBeeCountByRatio`
  - `ReduceTargetBeeCountByAmount`
  - `GetSpawnAmount`, `GetTargetBeeCount`

### B. 상태/계산 규칙
- `SpawnAmount >= 0` clamp
- `TargetBeeCount`는 항상 `0..SpawnAmount` clamp
- ratio 감소는 `0..1` clamp + `RoundToInt(CurrentTargetBeeCount * Ratio)`
- `ABeehive::CalculateCombSpawnAmount()`:
  - `CurrentCombCount <= 0`이면 0
  - 아니면 `RoundToInt(ColonyBeeCount * Clamp01(CombSpawnAmountRatio) / CurrentCombCount)`

### C. ABeehive slot 관리
- `MaxCombCount`만큼 `UChildActorComponent` 슬롯 유지
- `i < CurrentCombCount` 슬롯만 `CombActorClass` child actor 활성
- `i >= CurrentCombCount` 슬롯은 child actor class 비움(활성 actor 없음)
- `MaxCombCount` 감소 시 초과 슬롯/child actor 정리 시 stale actor 미잔존
- `MaxCombCount` 변경 시 `CurrentCombCount = Clamp(CurrentCombCount, 0, MaxCombCount)` 정책 준수

### D. 갱신 경로
- `OnConstruction`, `BeginPlay`, `PostEditChangeProperty`에서 comb layout/parameter 갱신
- `SetColonyBeeCount`에서 comb spawn amount 갱신 + target reset 적용
- `CombSlotSpacing`, `CombActorClass`, `MaxCombCount`, `CombPlaneSize`, `CombSpawnAmountRatio` 변경 시 즉시 반영
- 테스트 API:
  - `IncreaseCurrentCombCountForTest`
  - `DecreaseCurrentCombCountForTest`
  - `SetCurrentCombCountForTest`

### E. Niagara customization
- `BeehiveDualSwarmActorCustomization`에서 아래 조건에 `OverrideParameters` 숨김 적용:
  - owner: `ABeehiveCombActor`
  - component: `FrontFaceBeeNiagara` 또는 `BackFaceBeeNiagara`
- 기존 숨김 조건(`ABeehiveDualSwarmActor`, `ABeehive::AttractionSwarmNiagara`) 유지

### F. 금지사항 회귀 체크
- C++에서 Niagara system asset path 직접 로드 없음
- Environment 시스템/`GameTimeBucketSubsystem`과 comb 로직 신규 연결 없음
- 기존 `ABeehiveDualSwarmActor`, `ABeeSplineSwarmActor`, `AttractionSwarmNiagara` 비활성화/대체 없음

## 권장 검증 명령

- `rg "ABeehiveCombActor|CombRackRoot|CombSlotSpacing|CombPlaneSize|CurrentCombCount|TargetBeeCount" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md`
- `rg "OverrideParameters|FrontFaceBeeNiagara|BackFaceBeeNiagara|AttractionSwarmNiagara" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/BeekeepingSim.cpp`
- `rg "EnvironmentTimeOfDayActor|GameTimeBucketSubsystem" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors`

## 리뷰 결과 출력 형식

1. Findings (High -> Medium -> Low)
2. Open Questions / Assumptions
3. Regression Risk Checklist
4. 최종 판정: Pass / Conditional Pass / Fail
