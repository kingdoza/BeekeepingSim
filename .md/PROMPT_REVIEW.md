# 구현 리뷰 프롬프트: Beehive Comb Lift FacingAxis

## 우선순위

1. High: `CombLiftRotationOffset` 제거 및 `CombLiftFacingAxis` 기반 회전 계산 전환 검증
2. High: 기존 Lift 정책(슬롯 이동, 보간, refresh 재적용) 회귀 여부 검증
3. Medium: FacingAxis 변경 시 실제 카메라를 향하는 면이 바뀌는지 검증

---

## 리뷰 대상

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombLiftComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombLiftComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

---

## 핵심 검증 항목

### High 1: RotationOffset 제거 검증

- `CombLiftRotationOffset` UPROPERTY가 완전히 제거되었는지 확인
- C++/Config/Content 기준으로 `CombLiftRotationOffset` 참조가 남아있지 않은지 확인
- 문서는 제거 지시/리뷰 문맥에서의 참조만 허용하고, 구현 기준과 충돌하는 잔존 문구는 없는지 확인

### High 2: FacingAxis enum/설정 노출 검증

- `ECombLiftFacingAxis`가 `+X/-X/+Y/-Y/+Z/-Z`를 제공하는지 확인
- `CombLiftFacingAxis`가 `EditAnywhere`로 Details에 노출되는지 확인
- 기본값이 명시되어 있는지 확인

### High 3: 목표 회전 계산 정확성 검증

- Lift Begin 시점에만 목표 회전을 계산하고 고정하는지 확인
- 선택된 local facing axis가 `DirectionToCamera`를 향하도록 계산되는지 확인
- 최종 회전이 slot parent(`CombRackRoot`) 기준 relative rotation으로 변환되는지 확인
- 보간 중 카메라 이동이 목표 회전에 영향을 주지 않는지 확인

### High 4: 기존 Lift 정책 회귀 검증

- 소비장 actor detach 없이 slot `UChildActorComponent`만 이동하는지
- `CombLiftTargetRoot` 위치를 목표 위치로 계속 사용하는지
- `CombLiftMoveDuration` 기반 보간/0초 즉시이동 동작 유지 여부
- `RefreshCombSlotTransforms()` 이후 lifted transform 즉시 재적용 유지 여부
- Abort/EndPlay 즉시 원복 정책 유지 여부

### Medium 1: FacingAxis 동작 수동 검증

- `CombLiftFacingAxis=+Y`: local +Y 면이 카메라를 향하는지
- `CombLiftFacingAxis=-Y`: local -Y 면이 카메라를 향하는지
- `+X/-X/+Z/-Z` 변경 시 기대한 면 전환이 일어나는지

### Medium 2: 문서 정합성 검증

- `.md/USER_UNREAL.md`에서 RotationOffset 안내가 제거되고 FacingAxis 선택 안내로 교체됐는지
- `.md/Architecture/WorldActorsSystem.md`가 FacingAxis 기반 회전 정책으로 업데이트됐는지

---

## 빌드/검색 검증

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg -a "CombLiftRotationOffset" Source Config Content -n`
  - `rg "CombLiftRotationOffset|CombLiftFacingAxis|ECombLiftFacingAxis" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md -n`

---

## 리뷰 결과 출력 형식

1. Findings (High → Medium → Low)
2. Open Questions / Assumptions
3. Regression Risks
4. 최종 판단: Pass / Conditional Pass / Fail
