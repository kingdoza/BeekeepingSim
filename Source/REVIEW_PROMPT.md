다음 Unreal Engine C++ 변경을 코드 리뷰해줘. 우선순위는 버그, 상태 전이 오류, null 안정성, 입력 연결, 포커스 해제 누락, 태그 필터링 정확성, 성능(Tick/trace)이다.

변경 파일:
- Source/BeekeepingSim/BeekeepingSim.Build.cs
- Source/BeekeepingSim/Public/BeekeeperCharacter.h
- Source/BeekeepingSim/Private/BeekeeperCharacter.cpp
- Source/BeekeepingSim/Public/BeekeeperFocusComponent.h
- Source/BeekeepingSim/Private/BeekeeperFocusComponent.cpp
- Source/BeekeepingSim/Public/FocusTargetComponent.h
- Source/BeekeepingSim/Private/FocusTargetComponent.cpp
- Source/BeekeepingSim/Public/FocusInteractable.h
- Source/BeekeepingSim/Public/Beehive.h
- Source/BeekeepingSim/Private/Beehive.cpp
- Source/ARCHITECTURE.md

요구사항:
- 플레이어는 화면 중앙 기준으로 포커스 대상을 감지해야 한다.
- 포커스 진입 시 외곽선 활성, UI용 `F` 팝업 데이터 제공.
- 포커스 이탈 시 외곽선 해제, UI 데이터 해제.
- `F` 상호작용 확정과 `ESC` 포커스 해제를 분리하되 구조상 둘 다 대상 행동 훅을 지원.
- 포커스 대상은 `AllowedItemTags` 기반으로 핫바 활성/비활성 규칙을 제공.
- `AllowedItemTags` 가 비어 있으면 모든 아이템 비활성화.
- `AllowedItemTags` 에 모든 아이템 최상위 태그가 있으면 모든 아이템 활성화.
- 그 외에는 아이템 태그가 `HasAny` 를 만족할 때만 활성화.
- 외곽선 대상 메시 지정은 명시 배열 우선, 비어 있으면 owner primitive fallback.
- 벌통은 이 공통 시스템을 사용하는 예시 액터여야 한다.

집중 리뷰 포인트:
1. `UBeekeeperFocusComponent` 의 라인트레이스 기반 포커스 전이가 중복 호출 없이 안전한지
2. 이전 대상 `OnFocusExit` / 새 대상 `OnFocusEnter` 호출 순서가 적절한지
3. `ConfirmFocus` 와 `CancelFocus` 의 동작이 분리되어 있고, `bClearFocusOnConfirm` 로 확장 가능성이 맞는지
4. `AllowedItemTags` 해석 로직이 요구사항과 정확히 일치하는지
5. 포커스 대상이 없을 때 기본 상태 복구가 가능한 API/델리게이트 구조인지
6. `UFocusTargetComponent` 의 외곽선 처리에서 fallback primitive 수집이 부작용 없는지
7. `ABeehive` 예시가 인터페이스/컴포넌트 조합 예시로 충분한지
8. non-local 플레이어에서 포커스 Tick/trace 낭비가 과한지
9. `GameplayTags` 모듈 추가 외에 빌드 깨질 가능성이 있는 include/generated/UHT 문제가 없는지

가능하면 파일/라인 기준으로 severity 순으로 지적해줘. 버그, 회귀 위험, 추가 테스트 필요 항목을 나눠서 적어줘.
