# 구현 수정 프롬프트: 작업대 소비장 BP 이동 Hook 리뷰 Finding

## 우선순위

1. Medium: 리뷰 제외 범위인 `Content/` uasset 변경이 작업 트리에 포함됨

## 발견 문제

### 1. `Content/` 에셋 변경이 포함되어 리뷰 제외 조건과 충돌함

- 대상 파일:
  - `Content/UncappingTable/BP_UncappingTable.uasset`
  - `Content/__ExternalActors__/Beekeeper/Lvl_BeekeeperTest/B/TY/CBSCZKPDWZ0EC8SZ1B49HB.uasset`
- 확인 결과:
  - `git status --short`에서 위 두 uasset이 modified로 표시된다.
  - 이번 리뷰 프롬프트는 `Content/` 에셋을 수정 대상에서 제외했다.
  - C++ hook 자체는 `AUncappingTableCombSlot`에 좁게 추가되어 있지만, 현재 작업 트리 상태로는 "Content 변경 없음" 검증을 통과할 수 없다.
- 영향:
  - C++ Blueprint hook 변경과 Editor/레벨 asset 변경이 같은 변경 세트에 섞일 수 있다.
  - BP/레벨 저장 여부가 의도된 수동 작업인지, 실수로 저장된 asset churn인지 리뷰자가 판단할 수 없다.
- 수정 방향:
  - 이번 C++/문서 변경 세트에서는 위 `Content/` 변경을 제외한다.
  - 해당 uasset 수정이 의도된 Editor 수동 작업이면 별도 변경 세트로 분리하고, 어떤 BP/레벨 작업을 수행했는지 별도 검증 기록을 남긴다.
  - 의도치 않은 변경이면 사용자 승인 없이 다른 변경을 되돌리지 말고, 작업자가 직접 정리하거나 명시 승인을 받은 뒤 정리한다.

## 검증 방법

- 상태 확인:
  - `git status --short -- Content`
- 기대:
  - C++/문서 리뷰 변경 세트에서는 `Content/` modified 항목이 없어야 한다.
- UBT:
  - `BeekeepingSimEditor Win64 Development`

## 문서 반영 필요 여부

- 불필요. `.md/USER_UNREAL.md`에는 `BP_UncappingTableCombSlot`에서 수동으로 구현/compile/save해야 할 작업이 이미 기록되어 있다.
