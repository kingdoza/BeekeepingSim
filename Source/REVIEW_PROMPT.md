다음 Unreal Engine C++ 변경을 코드 리뷰해줘. 우선순위는 버그, 상태 전이 오류, null 안정성, 멀티플레이 로컬 플레이어 안전성, Tick 비용이다.

변경 파일:
- Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h
- Source/BeekeepingSim/Private/BeekeeperCameraShakeComponent.cpp
- Source/ARCHITECTURE.md

기존 기능:
- `UBeekeeperCameraShakeComponent` 가 `Idle`, `Walk`, `Sprint` 이동 상태에 따라 지속형 카메라 셰이크를 전환한다.
- 로컬 플레이어에게만 셰이크가 재생되도록 처리한다.

이번 요구사항:
- 캐릭터가 공중 상태(`IsFalling() == true`)일 때는 이동 상태 기반 셰이크를 모두 정지한다.
- 공중에서는 `Idle`, `Walk`, `Sprint` 셰이크를 새로 시작하지 않는다.
- 이전 프레임은 공중이고 현재 프레임은 지상이면 착지로 간주한다.
- 착지 순간에만 `LandingCameraShakeClass` 를 1회 재생한다.
- 착지 후에는 기존 이동 상태 계산 로직이 다시 정상 동작해야 한다.
- 비로컬 플레이어는 셰이크 관련 Tick 낭비를 최소화해야 한다.

집중 리뷰 포인트:
1. 공중 진입 시 이동 셰이크가 확실히 멈추는지
2. 착지 셰이크가 실제 착지 순간에만 1회 재생되는지
3. 착지 직후 이동 셰이크 재개가 기존 로직과 충돌하지 않는지
4. `bWasFalling` 상태 전이가 BeginPlay, 점프, 착지, 스폰 직후 상황에서 안전한지
5. `StopMoveShake`, `PlayLandingShake`, `ApplyMoveState` 호출 순서가 부작용 없는지
6. `PlayerController`, `PlayerCameraManager`, `LandingCameraShakeClass` null 처리 누락이 없는지
7. 로컬 플레이어가 아닌 경우 Tick 비활성화 로직이 적절한지

가능하면 파일/라인 기준으로 severity 순으로 지적해줘. 리뷰 결과에는 버그, 회귀 위험, 테스트가 필요한 케이스를 분리해서 적어줘.
