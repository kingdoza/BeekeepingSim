# Camera System

## Scope

- `Source/BeekeepingSim/Public/Camera/BeekeeperCameraShakeComponent.h`
- `Source/BeekeepingSim/Private/Camera/BeekeeperCameraShakeComponent.cpp`

## Responsibilities

- 이동 상태 기반 idle/walk/sprint camera shake 전환
- falling/landing 상태 감지와 landing shake 재생
- focus 강제 이동 후 가짜 landing shake 1회 억제
- focus interaction input lock 중 shake 중단
- 비로컬 플레이어에서 Tick 비용 저감

## Key Classes

- `UBeekeeperCameraShakeComponent`: 카메라 셰이크 상태 기계
- `EBeekeeperMoveState`: idle/walk/sprint 이동 상태 enum

## Runtime Flow

1. BeginPlay에서 owner character, movement component, player camera manager를 캐시한다.
2. Tick에서 로컬 플레이어 여부를 먼저 확인한다.
3. focus input lock 또는 falling 상태에서는 이동 shake를 중지한다.
4. falling에서 grounded로 전환되는 프레임에 landing shake를 재생하거나 suppress flag를 소비한다.
5. 수평 속도와 sprint 상태로 이동 shake class를 전환한다.

## Dependencies

- Character
- Focus

## Design Notes

- Camera system은 focus action을 직접 알지 않는다. Character의 input lock 및 suppress API를 통해 간접 연동한다.
- 비로컬 최적화는 actor/component 제거가 아니라 Tick interval 조정과 shake 중단으로 처리한다.
- `IdleSpeedThreshold`와 shake class 설정은 tuning 값이며 아키텍처 경계가 아니다.

## Manual Review Points

- anchored focus 이동 직후 landing shake가 한 번만 억제되는지 확인
- focus cancel/abort 후 shake가 정상 재개되는지 확인
- 로컬/비로컬 possession 전환 시 stale camera manager 참조가 남지 않는지 확인
