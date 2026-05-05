# 구현 프롬프트: Beehive Comb Lift FacingAxis 리뷰 수정

## 발견 문제

### High: 제거된 `CombLiftRotationOffset`가 `BP_Beehive.uasset`에 직렬화 참조로 남아 있음

- 대상:
  - `Content/Beehive/BP_Beehive.uasset`
  - 필요 시 `.md/PROMPT_IMPLEMENTATION.md`, `.md/PROMPT_REVIEW.md`
- 현재 상태:
  - C++ `UBeehiveCombLiftComponent`에서는 `CombLiftRotationOffset` UPROPERTY가 제거되고 `CombLiftFacingAxis`로 대체됨.
  - 하지만 `rg -a "CombLiftRotationOffset" Content Config Source .md -n` 결과 `Content/Beehive/BP_Beehive.uasset` 내부에 `CombLiftRotationOffset` 문자열이 남아 있음.
  - `.md/PROMPT_IMPLEMENTATION.md`, `.md/PROMPT_REVIEW.md`에도 오래된 용어가 남아 있어 전체 `.md` 검색 검증을 통과하지 못함.
- 영향:
  - 에디터 로드/Blueprint compile 시 제거된 property override 관련 warning이 발생할 수 있음.
  - 사용자는 Details에서 실제 source of truth가 `CombLiftFacingAxis`인지 혼동할 수 있음.
- 수정 방향:
  - Unreal Editor에서 `BP_Beehive`를 열고 compile/save하여 제거된 property override를 정리한다.
  - `CombLiftComponent` Details에 `CombLiftFacingAxis`가 표시되고 `CombLiftRotationOffset`가 더 이상 표시되지 않는지 확인한다.
  - 필요하면 `BP_Beehive`의 stale component template 데이터를 재저장한다.
  - 프롬프트/리뷰 문서에서 `CombLiftRotationOffset`는 제거 지시 목적 외에는 남기지 않거나, 현재 검증 명령 기준을 만족하도록 정리한다.

## 검증

- 검색:
  - `rg -a "CombLiftRotationOffset" Content Config Source .md -n`
  - 결과가 없거나, 의도적으로 보존한 prompt 문서만 남는지 확인한다.
- UBT:
  - `BeekeepingSimEditor Win64 Development`
- Editor:
  - `BP_Beehive` compile/save 후 Output Log에 missing/deprecated property warning이 없는지 확인한다.
  - `CombLiftFacingAxis=+Y/-Y/+X/-X/+Z/-Z` 변경 시 선택한 local 면이 카메라를 향하는지 PIE에서 수동 검증한다.
