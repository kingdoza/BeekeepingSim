# Camera System

## Scope

- `Source/BeekeepingSim/Public/Camera/BeekeeperCameraShakeComponent.h`
- `Source/BeekeepingSim/Private/Camera/BeekeeperCameraShakeComponent.cpp`

## Responsibilities

- 이동 상태 기반 카메라 셰이크 재생/중지
- 착지 셰이크 처리 및 1회 억제(suppress) 처리
- 로컬 플레이어 여부 기반 Tick 비활성화 최적화

## Key Classes

- `UBeekeeperCameraShakeComponent`: idle/walk/sprint/landing 셰이크 상태 기계

## Dependencies

- Character
- Focus

## Refactoring Notes

- `IdleSpeedThreshold` 공개 설정은 유지
- 포커스 재배치 시 landing shake 억제 연동 정책 유지
- public Blueprint API 및 enum 이름 유지

## Manual Review Points

- focus 입력 잠금 중 shake 정지 정책
- falling -> 착지 전이 프레임에서 suppress 동작
- 비로컬 플레이어에서 Tick disable 경로
