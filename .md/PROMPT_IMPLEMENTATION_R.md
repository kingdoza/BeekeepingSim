# 구현 프롬프트: Beehive Comb Actor 리뷰 수정

## 우선순위

1. High: `ABeehive` comb slot component 생성/정리 방식 안정화
2. Medium: `CombActorClass` 미설정 시 `CurrentCombCount`와 활성 comb actor 수 불일치 방지

## 발견 문제

### 1. 동적 comb slot component가 `AddInstanceComponent` + `Transient` 배열로만 추적됨

- 대상 파일: `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- 대상 파일: `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 현재 구현은 `CombSlotComponents`를 `Transient` 배열로 보관하면서 `OnConstruction`/`PostEditChangeProperty` 경로에서 `NewObject<UChildActorComponent>` 후 `AddInstanceComponent`/`RegisterComponent`로 슬롯을 만든다.
- `AddInstanceComponent` 기반 component는 actor instance component로 남을 수 있는데, 추적 배열은 transient라 editor reload, construction rerun, hot reload 후 기존 슬롯을 재발견하지 못할 수 있다.
- 결과적으로 stale `CombSlot_*` component/child actor가 남거나 동일 이름 component 재생성 문제가 생겨 `MaxCombCount`와 실제 슬롯 수, `CurrentCombCount`와 활성 comb actor 수가 어긋날 수 있다.

수정 방향:

- construction-created slot component의 lifecycle을 명확히 소유한다.
- 권장안 A: 슬롯 생성 시 construction-script component로 취급되도록 `CreationMethod`를 지정하고, rebuild 전에 기존 `CombSlot_*` child actor component를 owner/attachment/name 기준으로 찾아 정리하거나 재구성한다.
- 권장안 B: `CombSlotComponents`를 transient-only 추적에 의존하지 말고 owner의 components에서 `CombRackRoot` 하위 `CombSlot_*`를 재발견해 배열을 복원한 뒤 reconcile한다.
- 어떤 방식이든 `MaxCombCount` 감소 시 초과 slot child actor와 component가 실제로 제거되어 stale actor가 남지 않아야 한다.

검증 방법:

- Editor에서 `MaxCombCount`를 6 -> 2 -> 5로 반복 변경한다.
- Actor 선택 해제/재선택, map save/reopen 또는 editor restart 후 `CombSlot_*` component 수가 정확히 `MaxCombCount`인지 확인한다.
- `CurrentCombCount`를 테스트 API로 줄인 뒤 inactive slot에 child actor가 남지 않는지 확인한다.
- UBT: `BeekeepingSimEditor Win64 Development` 빌드 성공.

### 2. `CombActorClass` 기본값 없음

- 대상 파일: `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- 대상 파일: `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 현재 `CurrentCombCount`는 초기 `MaxCombCount`로 설정되지만 `CombActorClass`가 비어 있으면 active slot의 `SetChildActorClass(nullptr)` 상태가 되어 활성 comb actor 수가 `CurrentCombCount`와 일치하지 않는다.

수정 방향:

- `ABeehive` constructor에서 `CombActorClass = ABeehiveCombActor::StaticClass();`를 기본값으로 지정하거나, class 미설정 상태를 명시적으로 비활성 comb 정책으로 문서화/검증한다.
- 현재 요구사항은 활성 actor 수 일치를 우선하므로 native class default 지정이 권장된다.

검증 방법:

- BP 설정 없이 native `ABeehive`만 배치해도 `CurrentCombCount`만큼 `ABeehiveCombActor` child actor가 생기는지 확인한다.
- BP에서 `CombActorClass`를 `BP_BeehiveCombActor`로 덮어쓸 때 동일 invariant가 유지되는지 확인한다.

## 문서 반영 필요 여부

- lifecycle 구현 방식이 바뀌어도 외부 아키텍처 계약은 동일하므로 `.md/0_ARCHITECTURE.md`와 `.md/Architecture/WorldActorsSystem.md` 추가 변경은 원칙적으로 불필요하다.
- 단, `CombActorClass` 미설정을 허용하는 정책으로 결정하면 WorldActors 문서와 USER_UNREAL 수동 검증 항목에 그 정책을 명시해야 한다.
