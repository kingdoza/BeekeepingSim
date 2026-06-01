# 리뷰 프롬프트: 훈연기와 벌통 공격성 구현

## 리뷰 목적

이번 리뷰는 FocusEngaged item-use-area hold-use 경로에서 훈연기(`USmokerUseAction`)가 벌통(`ABeehive`) 공격성 수치를 감소시키는 구현이 설계 합의와 일치하는지 검증한다.

범위는 **공격성 상태 + 훈연기 동작**만 포함한다.

제외:
- 공격/피해/공격력 시스템
- 공격성 자동 회복
- 훈연기 자원 소모
- Content `.uasset` 수정 자체

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md` (`[훈연기와 벌통 공격성]`)
- `.md/QNA_IMPLEMENTATION.md`

---

## 리뷰 범위 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/Inventory/SmokerUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/SmokerUseAction.cpp`
- `Config/DefaultGameplayTags.ini`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

---

## 핵심 검증 질문

1. `ABeehive`가 aggression 상태 owner로 구현되었는가?
   - `MaxAggressionValue`, `AggressionValue`
   - `DecreaseAggression`, `SetAggressionValue`, `GetAggressionRatio`
2. 초기값/클램프 정책이 맞는가?
   - 기본 `100/100`
   - `SetAggressionValue`는 `0..MaxAggressionValue` clamp
   - BeginPlay에서 authored 값을 무조건 max로 리셋하지 않는가?
3. `USmokerUseAction`이 `UHoldItemUseAction` subclass로 추가되었는가?
4. 훈연기 tag query가 `Item.UseArea.Beehive.Smoker`로 구성되는가?
5. `BeginUse`/`EndUse`가 소독약과 동일한 held item active lifecycle을 호출하는가?
6. `ApplyUseEffect`가 `ABeehive::DecreaseAggression`만 호출하는가?
   - `bSucceeded=true`
   - item 소비/stack delta 변경 없음
7. 공격력 관련 API/필드가 추가되지 않았는가?
8. `DefaultGameplayTags.ini`에 `Item.UseArea.Beehive.Smoker`가 등록되었는가?

---

## 검색 검증

```powershell
rg "SmokerUseAction|AggressionValue|MaxAggressionValue|DecreaseAggression|GetAggressionRatio" Source/BeekeepingSim .md Config
rg "Item.UseArea.Beehive.Smoker" Source/BeekeepingSim .md Config
rg "EffectiveAttackPower|BaseAttackPower|AttackPower|MinAttackMultiplier" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

---

## 빌드 검증

- 가능하면 UBT:
  - `BeekeepingSimEditor Win64 Development`
- 실패 시:
  - 이번 변경 기인 오류와 기존 워크트리/unity 이슈 분리 보고

---

## 수동 검증 포인트 (PIE)

1. 벌통 FocusEngaged 상태 진입
2. 훈연기 아이템 선택 후 smoker use-area에서 hold-use
3. hold 중 `AggressionValue`가 감소하는지 확인
4. 아이템 stack/durability가 감소하지 않는지 확인
5. use 종료 후 값이 유지되는지 확인(자동 회복 없음)

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
  - 남은 검증 공백(PIE/Content 수동 설정 등)만 간단히 기재
