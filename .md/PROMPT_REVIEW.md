# 리뷰 프롬프트: 꿀 숙성도 시스템 구현

## 리뷰 목적

이번 리뷰는 `HoneyProduction` 시간 bucket에서 꿀 생산 전에 가득 찬 소비장의 꿀 숙성도를 증가시키는 구현이 설계 기준과 일치하는지 검증한다.

핵심:
- 꿀 숙성도 상태 owner는 `ABeehiveCombActor`
- 시간 bucket 처리 owner는 기존처럼 `ABeehive`
- 숙성도는 절대값(`CurrentHoneyRipeness`)으로 저장
- material에는 `0..1` 정규화값(`HoneyRipeness`) 전달
- `ApplyHoneyProductionUpdate()` 직접 호출은 꿀 생산만 수행
- 숙성 + 생산 순서는 `HoneyProduction` bucket event branch가 책임

제외:
- Content `.uasset` 직접 수정
- `HoneyRipeness` 전용 bucket subscription 추가
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

---

## 리뷰 범위 파일

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/USER_UNREAL.md`

주의:
- `Content/*.uasset` 변경은 이번 코드 리뷰의 주 검토 대상이 아니다.
- 기존 워크트리에 있는 unrelated 변경은 revert하지 않는다.

---

## 핵심 검증 질문

1. `ABeehiveCombActor`가 꿀 숙성도 상태 owner인가?
   - `MaxHoneyRipeness`
   - `CurrentHoneyRipeness`
   - `HoneyRipenessMaterialParameterName`
2. 숙성도 public API가 기존 honey API와 일관되게 추가되었는가?
   - `AddHoneyRipeness`
   - `SetCurrentHoneyRipeness`
   - `GetCurrentHoneyRipeness`
   - `GetHoneyRipenessRatio`
   - `IsHoneyFull`
3. sanitize 정책이 맞는가?
   - `MaxHoneyRipeness >= KINDA_SMALL_NUMBER`
   - `CurrentHoneyRipeness`는 `0..MaxHoneyRipeness` clamp
   - 음수 `AddHoneyRipeness`는 증가하지 않음
4. `ApplyHoneyVisualState()`가 기존 `HoneyAmount`를 유지하면서 `HoneyRipeness` normalized ratio도 front/back material instance 양쪽에 적용하는가?
5. `IsHoneyFull()` 기준이 `GetHoneyFillRatio() >= 1.0f - KINDA_SMALL_NUMBER`인가?
6. `ABeehive::ApplyHoneyRipenessUpdate()`가 active comb 전체를 순회하는가?
   - empty slot 제외
   - lifted comb 포함
   - full comb만 숙성
   - `HoneyRipenessIncreasePerBucket <= 0` no-op
7. `HoneyProduction` bucket branch 순서가 정확한가?
   - `ApplyHoneyRipenessUpdate()`
   - `ApplyHoneyProductionUpdate()`
8. `ApplyHoneyProductionUpdate()` 내부 또는 직접 호출 경로에 숙성 처리가 섞이지 않았는가?
9. `GetGameTimeBucketSubscriptions_Implementation()`에 별도 `HoneyRipeness` subscription이 추가되지 않았는가?
10. 이번 bucket에서 처음 full이 된 comb가 같은 bucket에서 숙성되지 않는 구조인가?
    - 숙성 update가 production update보다 먼저 실행되어야 한다.
11. `FBeehiveCombItemState`가 숙성도를 절대값으로 보존하는가?
    - `HoneyRipeness`
    - `SetBeehiveCombStateWithRipeness`
    - 기존 `SetBeehiveCombState(float, bool)` 시그니처 유지
12. `ABeehiveCombActor::ApplyStateFromItemInstance()`와 `WriteStateToItemInstance()`가 `HoneyAmount`, `HoneyRipeness`, visible face를 모두 보존하는가?
13. UCLASS/USTRUCT/UENUM rename이 없고 Core Redirect가 불필요한가?

---

## 검색 검증

```powershell
rg "HoneyRipeness|CurrentHoneyRipeness|MaxHoneyRipeness|ApplyHoneyRipenessUpdate|SetBeehiveCombStateWithRipeness" Source/BeekeepingSim .md
rg "HoneyProduction|ApplyHoneyProductionUpdate|OnGameTimeBucketEvent" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors .md
rg "FBeehiveCombItemState|SetBeehiveCombState|ApplyStateFromItemInstance|WriteStateToItemInstance" Source/BeekeepingSim .md
```

확인할 것:
- `ApplyHoneyProductionUpdate()` 직접 호출 경로는 숙성을 수행하지 않는다.
- `HoneyProduction` bucket branch에서만 숙성 후 생산 순서가 실행된다.
- item state에는 normalized ratio가 아니라 절대값 `HoneyRipeness`가 저장된다.
- material에는 `0..1` 정규화값이 전달된다.
- 기존 `SetBeehiveCombState(float, bool)` 시그니처가 유지된다.

---

## 빌드 검증

- 권장:
  - `BeekeepingSimEditor Win64 Development`
- 실패 시:
  - 변경 코드 오류인지, UBT 로그 권한/Editor DLL lock 같은 환경 문제인지 분리 보고

---

## 수동 검증 포인트 (PIE)

1. full이 아닌 소비장이 이번 `HoneyProduction` bucket에서 full이 되어도 같은 bucket에서는 숙성도가 증가하지 않는지 확인
2. 이미 full 상태였던 소비장이 다음 `HoneyProduction` bucket에서 `HoneyRipenessIncreasePerBucket`만큼 숙성되는지 확인
3. `ApplyHoneyProductionUpdate()`를 수동 호출하면 꿀 생산만 수행되고 숙성도는 증가하지 않는지 확인
4. 소비장 회수 후 재배치 시 `HoneyAmount`, `HoneyRipeness`, visible face가 모두 복원되는지 확인
5. honey plane material에 `HoneyRipeness` scalar parameter가 있고 정규화값에 따라 시각 변화가 발생하는지 확인

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순서로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- 이슈가 없으면:
  - `No blocking issues found.` 명시
  - 남은 검증 공백(Content 수동 설정/PIE 등)만 간단히 기재
