# Character System

## Scope

- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperController.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperController.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperMovementComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperMovementComponent.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperHeldItemVisualizerComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperHeldItemVisualizerComponent.cpp`

## Responsibilities

- 로컬 플레이어 입력 라우팅
- 캐릭터 하위 컴포넌트 조립 지점
- 이동/스프린트 상태 위임
- focus camera override 진입/갱신/복구
- active storage 및 active drag operation의 controller-level UI 세션 상태 제공
- 선택 아이템의 held/on-cursor presentation actor 표시
- runtime time-of-day clock widget 생성, 환경 시간 actor 구독, widget 시간 주입

## Key Classes

- `ABeekeeperCharacter`: 입력 바인딩, 컴포넌트 소유, focus camera override 제어
- `ABeekeeperController`: Enhanced Input mapping 등록, active storage/drag operation 저장, local clock widget binding
- `UBeekeeperMovementComponent`: 스프린트 상태와 이동 조건 판정
- `UBeekeeperHeldItemVisualizerComponent`: 선택 아이템 presentation actor 생성/부착/갱신

## Runtime Flow

1. `ABeekeeperController`가 기본 input mapping context를 등록한다.
2. local controller는 `TimeOfDayClockWidgetClass`가 있으면 `UTimeOfDayClockWidget`을 생성해 viewport에 추가한다.
3. local controller는 world에서 `AEnvironmentTimeOfDayActor`를 resolve하고 `OnTimeOfDayChanged`를 구독해 widget에 `Hour24`를 push한다.
4. `ABeekeeperCharacter`가 이동, 시점, 점프, 스프린트, focus, hotbar 입력을 하위 컴포넌트로 라우팅한다.
5. Focus engaged 중에는 `SetFocusInteractionInputLocked()`로 일반 입력을 제한한다.
6. `UBeekeeperHeldItemVisualizerComponent`는 hotbar/focus delegate를 구독하고, 로컬 플레이어에서만 presentation actor를 유지한다.
7. Storage UI가 열리면 controller가 active storage와 active drag operation을 저장해 UI slot resolution의 기준점이 된다.
8. controller `EndPlay`에서 time-of-day delegate, clock widget, active transient UI references를 정리한다.

## Dependencies

- Camera
- Focus
- Inventory
- UI
- Environment

## Design Notes

- Character는 도메인 상태를 직접 소유하지 않는다. Hotbar, Focus, CameraShake, HeldItemVisualizer가 각자 상태를 가진다.
- Controller의 active storage/drag operation은 transient UI context다. 저장 데이터가 아니며 interaction 종료 시 반드시 정리되어야 한다.
- Controller의 time clock binding도 local UI context다. 환경 시간의 source of truth는 `AEnvironmentTimeOfDayActor`이고, clock widget은 시간을 표시만 한다.
- `ABeekeeperController::FindTimeOfDayActor()`는 local clock widget 편의 경로다. gameplay bucket logic은 이 경로가 아니라 Environment의 `UGameTimeBucketSubsystem`을 사용한다.
- Held item visualizer는 `AItemPresentationActor`를 transient로 spawn하고 카메라에 attach한다. replication 대상이 아니다.
- On-cursor presentation은 커서 deprojection 후 카메라 앞 평면과의 교차점으로 위치를 계산한다.

## Blueprint/API Contracts

- `ABeekeeperCharacter`와 `ABeekeeperController`는 Blueprint native parent로 사용된다.
- 컴포넌트 property 이름 변경은 Blueprint 파손 가능성이 있으므로 Core Redirect 또는 수동 migration 계획이 필요하다.

## Manual Review Points

- focus camera override 종료 시 카메라 재부착, 제어 회전 복구, 입력 잠금 해제 순서
- storage interaction 종료 시 active storage와 active drag operation 정리 여부
- controller `EndPlay` 시 `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged` delegate 해제 여부
- 레벨에 `AEnvironmentTimeOfDayActor`가 여러 개 있을 때 clock widget이 첫 번째 actor만 사용한다는 warning 정책
- 비로컬 플레이어에서 held item presentation actor가 남지 않는지 확인
