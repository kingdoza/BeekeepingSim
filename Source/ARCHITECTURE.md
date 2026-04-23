# BeekeepingSim Architecture

## 2026-04-22 Update: PreviewFocus / EngagedFocus and FocusActionComponent

- `UBeekeeperFocusComponent` now separates:
  - `PreviewFocus`: center-screen trace, outline, prompt UI
  - `EngagedFocus`: confirmed interaction state with preview disabled
- `UFocusActionComponent` is the reusable base for confirm/cancel interaction behavior.
- `UAnchoredFocusActionComponent` is the reusable anchored interaction implementation:
  - character moves to the character anchor on confirm
  - camera blends to a focus anchor on confirm
  - cancel keeps the character in place and only blends the camera back to the default first-person view
  - camera blends back to the default first-person view before input is restored
- `UAnchoredFocusCursorActionComponent` extends the anchored interaction flow:
  - provides crosshair visibility policy while engaged instead of touching widgets directly
  - shows the mouse cursor and switches to `FInputModeGameAndUI` during engaged focus
  - restores crosshair immediately on cancel, then restores cursor/input mode after the camera return blend completes
- `UBeekeeperFocusComponent` is the single source of truth for crosshair visibility:
  - it calculates whether the crosshair should be hidden from the current engaged action policy
  - Blueprint UI only needs to read `ShouldHideCrosshair()` or subscribe to `OnCrosshairVisibilityChanged`
- `ABeehive` now uses `UAnchoredFocusCursorActionComponent` as the example interaction action.
- Hotbar filtering is only broadcast during `EngagedFocus`, not during `PreviewFocus`.
- `UBeekeeperCameraShakeComponent` supports one-shot landing shake suppression so forced focus repositioning does not trigger a fake landing shake.

## 2026-04-22 Update: Reusable Focus System

- Added reusable focus types under `Source/BeekeepingSim/Public` and `Source/BeekeepingSim/Private`:
  - `UBeekeeperFocusComponent`
  - `UFocusTargetComponent`
  - `IFocusInteractable`
  - `ABeehive`
- `ABeekeeperCharacter` now owns `UBeekeeperFocusComponent` and routes focus confirm/cancel input to it.
- `UBeekeeperFocusComponent` handles:
  - center-screen line trace based focus detection
  - focus enter/exit/confirm/cancel transitions
  - prompt data broadcasting
  - item-tag filtering evaluation for external hotbar systems
- `UFocusTargetComponent` handles:
  - prompt text data
  - `FFocusItemRule` with `AllowedItemTags`
  - outline toggling through explicit primitive list or owner primitive fallback
  - forwarding focus events to actors implementing `IFocusInteractable`
- `ABeehive` is the example focus actor and uses `UFocusTargetComponent` plus `IFocusInteractable` hooks.

## 프로젝트 개요

- 이 문서는 아래 경로의 실제 C++ 파일만 기준으로 정리한다.
  - `Source/BeekeepingSim/Public`
  - `Source/BeekeepingSim/Private`
- 현재 분석 대상 클래스는 아래 4개다.
  - `ABeekeeperCharacter`
  - `ABeekeeperController`
  - `UBeekeeperMovementComponent`
  - `UBeekeeperCameraShakeComponent`

## 주요 모듈/클래스 구조

- 캐릭터
  - `Source/BeekeepingSim/Public/BeekeeperCharacter.h`
  - `Source/BeekeepingSim/Private/BeekeeperCharacter.cpp`
- 컨트롤러
  - `Source/BeekeepingSim/Public/BeekeeperController.h`
  - `Source/BeekeepingSim/Private/BeekeeperController.cpp`
- 이동 컴포넌트
  - `Source/BeekeepingSim/Public/BeekeeperMovementComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperMovementComponent.cpp`
- 카메라 셰이크 컴포넌트
  - `Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperCameraShakeComponent.cpp`

## 핵심 클래스 역할

### `ABeekeeperCharacter`

- 파일:
  - `Source/BeekeepingSim/Public/BeekeeperCharacter.h`
  - `Source/BeekeepingSim/Private/BeekeeperCharacter.cpp`
- 역할:
  - 플레이어 캐릭터 본체다.
  - `UBeekeeperMovementComponent` 를 기본 이동 컴포넌트로 사용한다.
  - `FirstPersonCamera`, `BeekeeperMovement`, `BeekeeperCameraShake` 를 소유한다.
  - 이동, 시점, 점프, 스프린트 입력을 바인딩한다.
  - 입력 값을 `DoMove`, `DoLook`, `DoJumpStart`, `DoJumpEnd` 로 전달한다.

### `ABeekeeperController`

- 파일:
  - `Source/BeekeepingSim/Public/BeekeeperController.h`
  - `Source/BeekeepingSim/Private/BeekeeperController.cpp`
- 역할:
  - `APlayerController` 기반 컨트롤러다.
  - `DefaultMappingContext` 하나를 `EnhancedInputLocalPlayerSubsystem` 에 등록한다.
  - 카메라 매니저 클래스를 설정한다.

### `UBeekeeperMovementComponent`

- 파일:
  - `Source/BeekeepingSim/Public/BeekeeperMovementComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperMovementComponent.cpp`
- 역할:
  - 걷기/달리기 속도와 스프린트 상태를 관리한다.
  - `StartSprinting`, `StopSprinting`, `SwitchSprinting` 을 제공한다.
  - `TickComponent` 에서 입력 가속이 사라지면 스프린트를 해제한다.
  - `IsForwardAccelerating` 으로 전방 입력 여부를 판정한다.
  - `IsSprinting` 으로 현재 스프린트 상태를 외부에 노출한다.

### `UBeekeeperCameraShakeComponent`

- 파일:
  - `Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperCameraShakeComponent.cpp`
- 역할:
  - 이동 상태에 따라 카메라 셰이크를 전환하는 `UActorComponent` 다.
  - 상태 enum `EBeekeeperMoveState` 는 `Idle`, `Walk`, `Sprint` 로 구성된다.
  - 수평 속도와 `UBeekeeperMovementComponent::IsSprinting()` 결과를 사용해 상태를 계산한다.
  - 상태가 바뀔 때만 셰이크를 교체한다.
  - 로컬 플레이어가 아닌 경우 Tick 을 비활성화한다.

## 주요 호출 흐름 또는 실행 흐름

1. `ABeekeeperController::SetupInputComponent`
   - `Source/BeekeepingSim/Private/BeekeeperController.cpp`
   - 입력 매핑 컨텍스트를 로컬 플레이어 서브시스템에 등록한다.
2. `ABeekeeperCharacter::SetupPlayerInputComponent`
   - `Source/BeekeepingSim/Private/BeekeeperCharacter.cpp`
   - 이동, 시점, 점프, 스프린트 액션을 바인딩한다.
3. 이동 입력 처리
   - `MoveInput` 이 입력값에 `MoveSpeedScale` 을 적용한 뒤 `DoMove` 를 호출한다.
   - `DoMove` 는 `AddMovementInput` 으로 실제 이동을 수행한다.
4. 시점 입력 처리
   - `LookInput` 이 입력값에 `LookSpeedScale` 을 적용한 뒤 `DoLook` 을 호출한다.
   - `DoLook` 는 `AddControllerYawInput`, `AddControllerPitchInput` 를 호출한다.
5. 스프린트 처리
   - `SprintStartInput` 은 토글 여부에 따라 `SwitchSprinting` 또는 `StartSprinting` 을 호출한다.
   - `SprintReleaseInput` 은 홀드 방식일 때 `StopSprinting` 을 호출한다.
   - `UBeekeeperMovementComponent` 는 속도를 `WalkingSpeed` 와 `SprintingSpeed` 사이에서 전환한다.
6. 카메라 셰이크 처리
   - `UBeekeeperCameraShakeComponent::BeginPlay` 에서 Owner 캐릭터와 이동 컴포넌트를 캐시한다.
   - `TickComponent` 에서 현재 이동 상태를 다시 계산한다.
   - 상태 변경 시 `ApplyMoveState` 가 기존 셰이크를 멈추고 새 셰이크를 시작한다.

## 컴포넌트/의존 관계

- `ABeekeeperCharacter`
  - `UBeekeeperMovementComponent` 에 의존한다.
  - `UBeekeeperCameraShakeComponent` 를 소유한다.
  - `UCameraComponent` 를 소유한다.
- `UBeekeeperCameraShakeComponent`
  - Owner 를 `ABeekeeperCharacter` 로 가정한다.
  - `UBeekeeperMovementComponent` 상태를 읽는다.
  - `APlayerController` 와 `APlayerCameraManager` 를 통해 카메라 셰이크를 재생한다.
  - 공중 상태에서는 이동 셰이크를 정지하고, 착지 순간에는 일회성 착지 셰이크를 재생한다.
- `ABeekeeperController`
  - `UInputMappingContext` 와 `UEnhancedInputLocalPlayerSubsystem` 에 의존한다.

## 확인된 설계 특징

- `Source/BeekeepingSim/Public` / `Private` 구조를 실제로 사용하고 있다.
- 캐릭터 본체와 이동 처리, 카메라 연출이 분리되어 있다.
- `ABeekeeperCharacter` 는 입력 라우팅 중심이고, 스프린트 속도 관리는 `UBeekeeperMovementComponent` 가 담당한다.
- 카메라 셰이크는 `UBeekeeperCameraShakeComponent` 로 분리되어 있다.
- 카메라 셰이크 상태 전환은 속도와 스프린트 상태 기반이다.
- `UBeekeeperMovementComponent` 와 `UBeekeeperCameraShakeComponent` 모두 Tick 기반으로 상태를 갱신한다.

## 불명확한 부분

- `Source/BeekeepingSim/Public/BeekeeperCharacter.h` 는 `BeekeepingSimCharacter.h` 를 include 하지만 실제 상속은 `ACharacter` 다. 이 include 가 의도적인지 정리되지 않은 잔재인지는 코드만으로 확정할 수 없다.
- `Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h` 의 `IdleSpeedThreshold = 3.0f` 는 주석상 기본값이며, 실제 프로젝트 감각에 맞는 최종 값인지는 코드만으로 확정할 수 없다.
