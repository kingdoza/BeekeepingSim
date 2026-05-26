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
- `Source/BeekeepingSim/Public/Character/BeekeeperFlashlightComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperFlashlightComponent.cpp`

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
- `UBeekeeperFlashlightComponent`: `USpotLightComponent` 기반 카메라 부착 로컬 손전등 상태/설정/토글 관리

## Runtime Flow

1. `ABeekeeperController`가 기본 input mapping context를 등록한다.
2. local controller는 `TimeOfDayClockWidgetClass`가 있으면 `UTimeOfDayClockWidget`을 생성해 viewport에 추가한다.
3. local controller는 world에서 `ITimeOfDayProvider` actor를 resolve하고, `AGameTimeOfDayActor::OnGameTimeOfDayChanged`를 우선 구독해 widget에 `Hour24`를 push한다. (`AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`는 fallback)
4. `ABeekeeperCharacter`가 이동, 시점, 점프, 스프린트, focus, hotbar 입력을 하위 컴포넌트로 라우팅한다.
5. Focus engaged 중에는 `SetFocusInteractionInputLocked()`로 일반 입력을 제한한다.
6. `UBeekeeperHeldItemVisualizerComponent`는 hotbar/focus delegate를 구독하고, 로컬 플레이어에서만 presentation actor를 유지한다.
7. Storage UI가 열리면 controller가 active storage와 active drag operation을 저장해 UI slot resolution의 기준점이 된다.
8. controller `EndPlay`에서 time-of-day delegate, clock widget, active transient UI references를 정리한다.
9. `FlashlightToggleAction` 입력이 시작되면 `ABeekeeperCharacter`는 `UBeekeeperFlashlightComponent::ToggleFlashlight()`를 호출한다.

## Dependencies

- Camera
- Focus
- Inventory
- UI
- Environment

## Design Notes

- Character는 도메인 상태를 직접 소유하지 않는다. Hotbar, Focus, CameraShake, HeldItemVisualizer가 각자 상태를 가진다.
- Controller의 active storage/drag operation은 transient UI context다. 저장 데이터가 아니며 interaction 종료 시 반드시 정리되어야 한다.
- Controller의 time clock binding도 local UI context다. 환경 시간의 canonical source of truth는 `AGameTimeOfDayActor`이고, clock widget은 시간을 표시만 한다.
- `ABeekeeperController::FindTimeOfDayProviderActor()`는 local clock widget 편의 경로다. gameplay bucket logic은 이 경로가 아니라 Environment의 `UGameTimeBucketSubsystem`을 사용한다.
- Held item visualizer는 `AItemPresentationActor`를 transient로 spawn하고 카메라에 attach한다. replication 대상이 아니다.
- On-cursor presentation은 커서 deprojection 후 카메라 앞 평면과의 교차점으로 위치를 계산한다.
- Flashlight는 `FirstPersonCamera` 하위에 부착되는 로컬 `USpotLightComponent` 기반 시각 컴포넌트다. focus input lock 여부와 무관하게 토글 입력을 허용한다.

## Blueprint/API Contracts

- `ABeekeeperCharacter`와 `ABeekeeperController`는 Blueprint native parent로 사용된다.
- 컴포넌트 property 이름 변경은 Blueprint 파손 가능성이 있으므로 Core Redirect 또는 수동 migration 계획이 필요하다.

## Manual Review Points

- focus camera override 종료 시 카메라 재부착, 제어 회전 복구, 입력 잠금 해제 순서
- storage interaction 종료 시 active storage와 active drag operation 정리 여부
- controller `EndPlay` 시 시간 provider delegate 해제 여부 (`AGameTimeOfDayActor` 우선, legacy fallback 포함)
- 레벨에 시간 provider가 여러 개일 때 `AGameTimeOfDayActor` 우선 선택 정책과 warning 정책 확인
- 비로컬 플레이어에서 held item presentation actor가 남지 않는지 확인
- `FlashlightToggleAction`이 `T` 키에 매핑되어 있고 focus interaction input lock 중에도 토글이 동작하는지 확인
- 손전등 방향이 카메라 회전을 따라가고, 밝기/거리/콘각/그림자 값이 Details 조절대로 반영되는지 확인

## Update 2026-05-24

- `ABeekeeperController` clock binding now resolves `ITimeOfDayProvider` actor.
- Preferred runtime path:
  - bind `AGameTimeOfDayActor::OnGameTimeOfDayChanged`
  - immediately push current provider hour to `UTimeOfDayClockWidget`
- Legacy compatibility path to `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged` remains.

## Update 2026-05-25

- `ABeekeeperCharacter` LMB focus 입력 라우팅이 press/release gesture 모델로 변경되었다.
  - `FocusConfirmAction`: `Started -> FocusPrimaryPressedInput`, `Completed -> FocusPrimaryReleasedInput`
  - `PartFocusClickAction`: `Started -> PartFocusPointerPressedInput`, `Completed -> PartFocusPointerReleasedInput`
- 기존 `FocusConfirmInput`, `PartFocusClickInput`, `PartFocusClickReleaseInput` 함수는 삭제하지 않고 새 pointer API wrapper로 유지한다.

## Update 2026-05-27

- `ABeekeeperCharacter`에 `FocusSecondaryAction`(`UInputAction`) 입력 바인딩 경로를 추가했다.
- 입력 `Started` 시 `FocusSecondaryInput()`이 호출되고, 이는 `UBeekeeperFocusComponent::HandleSecondaryInput()`으로 위임된다.

## Update 2026-05-27 (PartFocus Secondary)

- `FocusSecondaryAction` 입력의 의미를 FocusEngaged host 내부 PartFocus secondary 입력으로 사용한다.
- non-engaged preview secondary 동작은 기본 false이며, engaged action이 secondary 처리 경로를 소유한다.

## Update 2026-05-27 (Hotbar Toggle Selection)

- `ABeekeeperCharacter`에 `HotbarToggleSelectionAction` 입력 바인딩(`Started`)을 추가했다.
- 입력 핸들러 `HotbarToggleSelectionInput()`은 도메인 로직 없이 `UBeekeeperHotbarComponent::ToggleSelectionFromLastSelectedSlot()` 호출만 수행한다.
