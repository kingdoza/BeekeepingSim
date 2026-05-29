# 리뷰 프롬프트: Beehive Comb Face Bee Count 분리

## 리뷰 목적

이번 리뷰는 `ABeehiveCombActor`의 벌 수 상태를 단일값에서 **소비장 전체 + face별 상태**로 전환한 변경의 정확성과 회귀 위험을 검증한다.

핵심 목표:
- `TotalSpawnAmount` / `TotalTargetBeeCount` 의미가 코드/문서/호출부에서 일관되는지
- Niagara 주입이 face 분배값으로 바뀌었는지
- BeeBrush(visible face만 감소), Shake(양면 감소), 회수 조건(total target 0) 정책이 정확히 반영되었는지
- spawn 갱신 시 ratio 보존 경로와 초기 reset 경로가 의도대로 분리되었는지

중요: 워크트리에 다른 변경이 있을 수 있으므로 **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md` (`소비장 면별 BeeBrush TargetBeeCount 설계 QnA`)
- `.md/QNA_IMPLEMENTATION.md`

---

## 핵심 검증 질문

1. `SpawnAmount/TargetBeeCount` 단일 상태가 제거되고 `TotalSpawnAmount + Front/BackFaceTargetBeeCount`로 완전히 전환되었는가?
2. 분배 규칙 `Front=(Total+1)/2`, `Back=Total/2`가 helper 한 곳에서 재사용되고, `Spawn/Target` 모두 동일 규칙을 쓰는가?
3. `ApplyNiagaraUserParameters()`가 양면에 동일 total 값을 넣지 않고 face별 `User.SpawnAmount`/`User.TargetBeeCount`를 넣는가?
4. `SanitizeState()`가 다음 invariant를 보장하는가?
   - `TotalSpawnAmount >= 0`
   - `0 <= FrontTarget <= FrontSpawn`
   - `0 <= BackTarget <= BackSpawn`
   - `TotalTarget <= TotalSpawn`
5. `ABeehive::RefreshCombSpawnAmounts`가 일반 갱신에서는 ratio 보존 API를, 초기 채움 경로에서는 reset API를 호출하는가?
6. BeeBrush가 `ReduceVisibleFaceTargetBeeCountByAmount`를 호출해 visible face만 감소시키는가?
7. 회수 조건이 `GetTotalTargetBeeCount()==0 && queen 미부착`으로 바뀌었는가?
8. 기존 모호 API(`GetTargetBeeCount`, `GetSpawnAmount`, `ReduceTargetBeeCountBy...`, `SetSpawnAmountAndResetTargetBeeCount`)가 호출부에서 제거되었는가?

---

## 리뷰 범위 (우선 파일)

### 구현 코드
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/Inventory/BeeBrushUseAction.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`

### 문서
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`

---

## 상세 체크리스트

### 1) ABeehiveCombActor 상태 모델/API
- Public API 이름이 total/face 의미를 명확히 드러내는지
- `SetTotalTargetBeeCount`가 total 입력을 분배 후 적용하는지
- `ResetTargetBeeCountsToSpawnAmount`가 face spawn 기준으로 채우는지
- `ReduceAllTargetBeeCountsByRatio`가 face별 current target 기준 라운딩 감소인지
- `ReduceVisibleFaceTargetBeeCountByAmount`가 `VisibleCombFace` 기준으로만 감소하는지

### 2) Spawn 갱신 정책
- `SetTotalSpawnAmountPreservingTargetRatios` 구현이 QnA 규칙과 일치하는지
  - old face spawn == 0 -> new face spawn으로 reset
  - else -> `RoundToInt(NewFaceSpawn * OldFaceTarget / OldFaceSpawn)`
- spawn 변경 시 Niagara reinitialize 조건이 기존처럼 spawn 변화 기준인지

### 3) BeeBrush / Shake 정책 분리
- BeeBrush는 visible face 전용 감소
- Shake/legacy 전체 감소는 양면 감소 유지
- colony bee count를 BeeBrush가 건드리지 않는지

### 4) 회수/배치 계약
- 회수 가능 조건이 total target 기반으로 변경되었는지
- queen attach 차단 조건 유지되는지

### 5) 문서 동기화
- 0_ARCHITECTURE: “양면 동일값 주입” 제거 및 face 분배 주입으로 갱신
- WorldActorsSystem: 상태 모델/분배 규칙/API/갱신 정책/BeeBrush·Shake·회수 조건 반영
- InventorySystem: BeeBrush visible-face 정책, 회수 조건 total target 반영

---

## 코드 검색 기준

### 없어야 함(소스 호출부)
- `GetSpawnAmount(`
- `GetTargetBeeCount(`
- `SetSpawnAmountAndResetTargetBeeCount`
- `ReduceTargetBeeCountBy`
- `SetTargetBeeCount(`
- `ResetTargetBeeCountToSpawnAmount`

### 있어야 함
- `GetTotalSpawnAmount(`
- `GetTotalTargetBeeCount(`
- `SetTotalSpawnAmountAndResetTargetBeeCounts(`
- `SetTotalSpawnAmountPreservingTargetRatios(`
- `ReduceAllTargetBeeCountsByRatio(`
- `ReduceVisibleFaceTargetBeeCountByAmount(`

주의: Niagara 파라미터 문자열 `User.TargetBeeCount`는 의도적으로 유지된다.

---

## 검증 방법

1. 코드 리뷰 + 검색 근거 제시
2. UBT 빌드 확인
   - `BeekeepingSimEditor Win64 Development`
3. 가능 시 수동 시나리오 검증
   - total 500/501 분배값
   - BeeBrush front/back 개별 감소
   - shake 양면 감소
   - total target 0 + queen 미부착일 때만 회수 가능

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순서로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- Findings 이후:
  - 가정/불확실성
  - 테스트 공백
  - 문서 동기화 누락 여부
