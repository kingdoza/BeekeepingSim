# 구현 리뷰 프롬프트: Queen Bee Actor / Queen Location Update

## 우선순위

1. High: 여왕벌 위치 업데이트 규칙(60분 bucket, lifted comb 제외, 중앙 가중치, Front/Back 50:50, `0~360` yaw) 정확성
2. High: `AQueenBeeActor` Tick yaw jitter 누적 구현 정확성
3. High: 기존 Beehive/CombLift 동작 회귀 여부
4. Medium: 문서 정합성 및 Blueprint 수동 검증 필요 지점 확인

---

## 리뷰 대상

- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

---

## 핵심 검증 항목

### High 1: Queen bucket 구독/처리 검증

- `ABeehive::GetGameTimeBucketSubscriptions_Implementation`에 `QueenBeeLocation` subscription이 추가되었는지
- `BucketMinutes`가 `QueenBeeLocationBucketMinutes`를 `1..1440` clamp해서 사용하는지
- `bApplyImmediatelyOnBeginPlay`가 `bUpdateQueenBeeLocationOnBeginPlayBucket`와 연결되는지
- `CatchUpPolicy`가 `LatestOnly`인지
- `OnGameTimeBucketEvent_Implementation`에서 `BeeSwarm` 기존 처리와 충돌 없이 `QueenBeeLocation -> UpdateQueenBeeLocation()` 분기가 동작하는지

### High 2: 여왕벌 위치 업데이트 규칙 검증

- 후보가 `0 <= Index < CurrentCombCount` + 유효 slot + `ABeehiveCombActor` child를 만족하는지
- 현재 lifted comb slot(`CombLiftComponent->GetLiftedCombSlotIndex()`)이 후보에서 제외되는지
- 후보 comb의 front/back attach point 유효성 체크가 있는지
- 후보 없을 때 no-op(기존 위치 유지, destroy/hide 없음, 로그 스팸 없음)인지

### High 3: 중앙 가중 랜덤 정확성 검증

- 중심 계산이 `Center=(CurrentCombCount-1)*0.5` 형태인지
- 거리 정규화 후 center factor를 통해 중앙일수록 높은 weight를 부여하는지
- `QueenBeeCenterWeightMultiplier`가 `>=1`로 안전 처리되는지
- weighted random pick이 total weight 기반으로 동작하고 fallback이 있는지
- 짝수 comb 개수에서 중앙 두 슬롯이 동등 최고 가중치를 갖는지

### High 4: Front/Back 부착 + 랜덤 yaw 적용 검증

- `FMath::RandBool()`로 Front/Back을 50:50 선택하는지
- 선택 attach point에 `SnapToTargetNotIncludingScale`로 부착하는지
- 상대 위치를 0으로 맞추는지
- 상대 회전에 `0~360` 랜덤 yaw를 적용하는지
- 랜덤 yaw가 attach point 회전 기준으로 추가되는 형태인지

### High 5: Tick yaw jitter 구현 검증

- `AQueenBeeActor`가 Tick 활성화(`PrimaryActorTick.bCanEverTick=true`)인지
- `YawJitterDegreesPerTick`가 Details 노출(`EditAnywhere`) 및 clamp(min 0)인지
- 매 Tick `FRandRange(-n1, n1)` + `AddActorLocalRotation` 누적 방식인지
- DeltaTime 보정이 없는지(요구사항 부합)

### High 6: CombLift 연동 정책 회귀 검증

- queen 위치 갱신 외 경로에서 comb lift 시 queen 강제 detach/reposition 코드가 추가되지 않았는지
- queen이 comb attach 상태일 때 comb lift를 따라 이동하는 기존 attach 기반 동작을 깨지 않았는지

### Medium 1: Blueprint/API 계약 영향 검증

- 기존 UCLASS/USTRUCT/UENUM rename/delete가 없는지
- 기존 Blueprint callable/pure API rename/delete가 없는지
- `ABeehiveCombActor` 신규 attach point 컴포넌트가 BP child에서 조정 가능 상태인지

### Medium 2: 문서 정합성 검증

- `.md/0_ARCHITECTURE.md`에 queen actor + 60분 bucket 갱신 + Environment direct reference 금지 경로가 반영됐는지
- `.md/Architecture/WorldActorsSystem.md`에 scope/key classes/composition/design notes가 반영됐는지

---

## 빌드/검색 검증

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg "QueenBeeLocation|UpdateQueenBeeLocation|QueenBeeChildActor|QueenBeeActorClass" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`
  - `rg "QueenFrontAttachPoint|QueenBackAttachPoint|GetQueenAttachPoint" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`
  - `rg "YawJitterDegreesPerTick|AddActorLocalRotation|FRandRange" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors -n`

---

## 리뷰 결과 출력 형식

1. Findings (High → Medium → Low)
2. Open Questions / Assumptions
3. Regression Risks
4. 최종 판단: Pass / Conditional Pass / Fail
