# BeekeepingSim Architecture

## 프로젝트 개요

- 이 문서는 아래 경로의 실제 C++ 파일만 기준으로 정리한다.
  - `Source/BeekeepingSim/Public`
  - `Source/BeekeepingSim/Private`
- 질문 11 답변 기준으로, 현재 공식 아키텍처 범위는 기존 이동/카메라 구조뿐 아니라 Focus/Action/Hotbar 계층까지 포함한다.
- 현재 공식 분석 대상은 아래 구조 전체다.
  - `ABeekeeperCharacter`
  - `ABeekeeperController`
  - `UBeekeeperMovementComponent`
  - `UBeekeeperCameraShakeComponent`
  - `UBeekeeperFocusComponent`
  - `UBeekeeperHeldItemVisualizerComponent`
  - `UFocusTargetComponent`
  - `UFocusActionComponent`
  - `UAnchoredFocusActionComponent`
  - `UAnchoredFocusCursorActionComponent`
  - `UBeekeeperHotbarComponent`
  - `UItemDefinition`
  - `UItemInstance`
  - `UItemAction`
  - `IFocusInteractable`
  - `IHotbarItemInterface`
  - `ABeehive`
  - `AWorldItemPickup`
  - `UPickupFocusActionComponent`

## 최신 설계 결정

### 2026-04-22 Update: PreviewFocus / EngagedFocus and FocusActionComponent

- `UBeekeeperFocusComponent` 는 포커스 상태를 두 단계로 분리한다.
  - `PreviewFocus`: 화면 중앙 트레이스 기반 감지, 아웃라인, 프롬프트 UI
  - `EngagedFocus`: 확정된 상호작용 상태, preview 비활성화
- `UFocusActionComponent` 는 confirm/cancel 상호작용의 재사용 가능한 베이스다.
- `UAnchoredFocusActionComponent` 는 앵커 기반 상호작용 구현이다.
  - confirm 시 캐릭터를 캐릭터 앵커로 이동시킨다.
  - confirm 시 카메라를 포커스 앵커로 블렌드한다.
  - cancel 시 캐릭터는 유지하고 카메라만 기본 1인칭 시점으로 되돌린다.
  - 입력 복구는 카메라 복귀 블렌드 완료 후 수행한다.
- `UAnchoredFocusCursorActionComponent` 는 앵커 기반 흐름에 커서/UI 정책을 추가한다.
  - Engaged 동안 크로스헤어 숨김 정책을 제공한다.
  - Engaged 동안 마우스 커서를 표시하고 `FInputModeGameAndUI` 로 전환한다.
  - cancel 시작 시 크로스헤어를 즉시 복구한다.
  - 커서 숨김과 기본 입력 모드 복구는 카메라 복귀 블렌드 완료 후 수행한다.
- `UBeekeeperFocusComponent` 는 크로스헤어 가시성의 단일 기준점이다.
  - 현재 engaged action 정책으로 크로스헤어 숨김 여부를 계산한다.
  - Blueprint UI 는 `ShouldHideCrosshair()` 를 읽거나 `OnCrosshairVisibilityChanged` 를 구독하면 된다.
- 크로스헤어 WBP 직접 제어는 하지 않는다.
  - 액션 컴포넌트는 상태만 제공하고, Blueprint UI/HUD 가 delegate 를 받아 표시를 갱신한다.
- Hotbar 필터링은 `PreviewFocus` 에서는 브로드캐스트하지 않고 `EngagedFocus` 에서만 브로드캐스트한다.
- `UBeekeeperCameraShakeComponent` 는 강제 포커스 재배치 시 가짜 착지 셰이크가 발생하지 않도록 1회성 착지 셰이크 억제를 지원한다.

### 2026-04-22 Update: Reusable Focus System

- 재사용 가능한 포커스 구조가 추가되었다.
  - `UBeekeeperFocusComponent`
  - `UFocusTargetComponent`
  - `IFocusInteractable`
  - `UFocusActionComponent`
  - `UAnchoredFocusActionComponent`
  - `UAnchoredFocusCursorActionComponent`
  - `ABeehive`
- `ABeekeeperCharacter` 는 `UBeekeeperFocusComponent` 를 소유하고 focus confirm/cancel 입력을 라우팅한다.
- `UBeekeeperFocusComponent` 는 아래를 담당한다.
  - 화면 중앙 라인트레이스 기반 포커스 탐지
  - focus enter/exit/confirm/cancel 전환
  - 프롬프트 데이터 브로드캐스트
  - item tag 필터링 평가
  - 크로스헤어 가시성 브로드캐스트
- `UFocusTargetComponent` 는 아래를 담당한다.
  - 프롬프트 텍스트 데이터
  - `FFocusItemRule` 기반 허용 아이템 태그
  - outline 대상 primitive 결정과 custom depth outline 토글
  - `IFocusInteractable` 구현 Actor 로의 이벤트 전달
- `ABeehive` 는 예시 포커스 액터이며 `UFocusTargetComponent` 와 `UAnchoredFocusCursorActionComponent` 를 함께 사용한다.

### 2026-04-23 Update: Beekeeper Hotbar Component

- `UBeekeeperHotbarComponent` 가 고정 8슬롯 hotbar 상태 오너다.
- 선택 상태는 `SelectedIndex` 하나로 관리한다.
  - `INDEX_NONE`: 선택 없음, 빈손 상태
  - 유효 인덱스: 해당 슬롯 선택 및 활성 상태
- 같은 슬롯을 다시 선택하면 선택을 해제한다.
- Hotbar 표시 모드는 포커스 상태로부터 파생된다.
  - `None`
  - `InHand`: 일반 플레이
  - `OnCursor`: `EngagedFocus`
- `UBeekeeperHotbarComponent` 는 `UBeekeeperFocusComponent::OnFocusRuleChanged` 를 구독한다.
  - `PreviewFocus` 에서는 필터링하지 않는다.
  - `EngagedFocus` 에서만 필터링한다.
  - `EngagedFocus` 진입 시 항상 선택을 해제하고 빈손 상태로 강제한다.
- `IHotbarItemInterface` 로 hotbar 아이템이 gameplay tag 를 제공한다.

### 2026-04-24 Update: Item Definition / Instance / Action and Held Item Visualizer

- 아이템 구조는 정적 데이터, 런타임 상태, 행동 객체를 분리한다.
  - `UItemDefinition`: 에셋 기반 정적 데이터
  - `UItemInstance`: 런타임 소유 아이템 인스턴스
  - `UItemAction`: 선택적 행동 객체 베이스
- `UItemDefinition` 은 아래 데이터를 가진다.
  - `ItemId`, `DisplayName`, `Description`, `Icon`, `WorldMesh`
  - `GameplayTags`, `MaxStack`, `ActionSpecs`
- `UItemInstance` 는 `IHotbarItemInterface` 를 구현하며 아래 상태를 가진다.
  - `Definition`, `StackCount`, `Durability`, `InstanceId`, `Actions`
- action 객체의 outer 는 `UItemInstance` 다.
- hotbar 는 여전히 `UObject* ItemInstance` 와 `IHotbarItemInterface` 기반으로 슬롯 필터링을 수행한다.
  - 실제 런타임 아이템은 `UItemInstance` 를 수용한다.
  - UI 편의를 위해 선택 아이템 이름, 아이콘, 스택 수량 조회 함수를 제공한다.
- `UBeekeeperHeldItemVisualizerComponent` 는 선택 아이템의 `WorldMesh` 표시를 담당한다.
  - hotbar/focus 상태 오너와 분리된 별도 컴포넌트다.
  - `ABeekeeperCharacter` 가 이를 소유한다.
  - 로컬 플레이어 기준으로만 Tick 한다.
  - `EHotbarPresentationMode::InHand` 에서는 카메라 기준 로컬 오프셋으로 표시한다.
  - `EHotbarPresentationMode::OnCursor` 에서는 같은 메시를 유지한 채 마우스 커서의 정규화 화면 좌표를 카메라 앞 평면 오프셋으로 변환해 커서 추종시킨다.
  - 선택 해제, 비유효 아이템, 메시 없음, 비로컬 상태에서는 즉시 숨긴다.

### 2026-04-24 Update: Hotbar Item Pickup

- `UBeekeeperHotbarComponent` 는 hotbar 직접 저장 구조를 유지한 채 아이템 획득 API를 제공한다.
  - `UItemDefinition` 과 수량을 받아 기존 stack 병합을 우선 시도한다.
  - 남는 수량은 첫 번째 빈 슬롯부터 새 `UItemInstance` 를 생성해 배치한다.
  - 새 `UItemInstance` 의 outer 는 `UBeekeeperHotbarComponent` 이며, `InitializeFromDefinition()` 경로로 초기화한다.
  - 결과는 성공 여부, 부분 성공 여부, 요청 수량, 추가 수량, 남은 수량, 메시지를 포함한 구조체로 반환한다.
- `UFocusTargetComponent` 는 런타임 표시명/상호작용 키 텍스트 setter 를 제공해 pickup 류 액터가 프롬프트를 갱신할 수 있다.
- `AWorldItemPickup` 는 월드에 배치되는 단일 아이템 pickup actor 다.
  - `USceneComponent` 루트, `UStaticMeshComponent`, `UFocusTargetComponent`, `UPickupFocusActionComponent` 를 소유한다.
  - `UItemDefinition` 하나를 보관하고, `WorldMesh` 와 `DisplayName` 을 시각화/프롬프트 기본값으로 사용한다.
- `UPickupFocusActionComponent` 는 `UFocusActionComponent` 파생 단발성 상호작용이다.
  - confirm 시 소유 pickup 과 상호작용 캐릭터를 검증한다.
  - 캐릭터의 `UBeekeeperHotbarComponent::TryAcquireItem()` 을 호출한다.
  - 획득 성공 시 pickup actor 를 제거하고, 실패 시 actor 를 유지한 채 로그/디버그 메시지만 남긴다.

## 주요 모듈/파일 구조

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
- 포커스 상태 컴포넌트
  - `Source/BeekeepingSim/Public/BeekeeperFocusComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperFocusComponent.cpp`
- 포커스 타겟 컴포넌트
  - `Source/BeekeepingSim/Public/FocusTargetComponent.h`
  - `Source/BeekeepingSim/Private/FocusTargetComponent.cpp`
- 포커스 액션 베이스
  - `Source/BeekeepingSim/Public/FocusActionComponent.h`
  - `Source/BeekeepingSim/Private/FocusActionComponent.cpp`
- 앵커 기반 포커스 액션
  - `Source/BeekeepingSim/Public/AnchoredFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/AnchoredFocusActionComponent.cpp`
- 커서 포함 앵커 포커스 액션
  - `Source/BeekeepingSim/Public/AnchoredFocusCursorActionComponent.h`
  - `Source/BeekeepingSim/Private/AnchoredFocusCursorActionComponent.cpp`
- 핫바 컴포넌트
  - `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- 들고 있는 아이템 시각화 컴포넌트
  - `Source/BeekeepingSim/Public/BeekeeperHeldItemVisualizerComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`
- 아이템 정의
  - `Source/BeekeepingSim/Public/ItemDefinition.h`
  - `Source/BeekeepingSim/Private/ItemDefinition.cpp`
- 아이템 인스턴스
  - `Source/BeekeepingSim/Public/ItemInstance.h`
  - `Source/BeekeepingSim/Private/ItemInstance.cpp`
- 아이템 액션
  - `Source/BeekeepingSim/Public/ItemAction.h`
  - `Source/BeekeepingSim/Private/ItemAction.cpp`
- 아이템 액션 공용 타입
  - `Source/BeekeepingSim/Public/ItemActionContext.h`
  - `Source/BeekeepingSim/Public/ItemActionTypes.h`
- 포커스 인터페이스
  - `Source/BeekeepingSim/Public/FocusInteractable.h`
- 핫바 아이템 인터페이스
  - `Source/BeekeepingSim/Public/HotbarItemInterface.h`
- 예시 상호작용 액터
  - `Source/BeekeepingSim/Public/Beehive.h`
  - `Source/BeekeepingSim/Private/Beehive.cpp`
- 월드 아이템 pickup 액터
  - `Source/BeekeepingSim/Public/WorldItemPickup.h`
  - `Source/BeekeepingSim/Private/WorldItemPickup.cpp`
- pickup 전용 포커스 액션
  - `Source/BeekeepingSim/Public/PickupFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/PickupFocusActionComponent.cpp`

## 핵심 클래스 역할

### `ABeekeeperCharacter`

- 플레이어 캐릭터 본체다.
- `UBeekeeperMovementComponent` 를 기본 이동 컴포넌트로 사용한다.
- 아래 컴포넌트를 소유한다.
  - `FirstPersonCamera`
  - `UBeekeeperMovementComponent`
  - `UBeekeeperCameraShakeComponent`
  - `UBeekeeperFocusComponent`
  - `UBeekeeperHotbarComponent`
  - `UBeekeeperHeldItemVisualizerComponent`
- 이동, 시점, 점프, 스프린트, 포커스 confirm/cancel, hotbar 슬롯, hotbar wheel 입력을 바인딩한다.
- 포커스 상호작용 중 입력 잠금을 관리한다.
- 포커스 카메라 override 시작/갱신/복구를 담당한다.
- 캐릭터 앵커 위치로 순간 이동하고 제어 회전과 이동 모드를 동기화한다.

### `ABeekeeperController`

- `APlayerController` 기반 컨트롤러다.
- `DefaultMappingContext` 를 `EnhancedInputLocalPlayerSubsystem` 에 등록한다.
- `PlayerCameraManagerClass` 를 `ABeekeepingSimCameraManager` 로 설정한다.

### `UBeekeeperMovementComponent`

- 걷기/달리기 속도와 스프린트 상태를 관리한다.
- `StartSprinting`, `StopSprinting`, `SwitchSprinting` 을 제공한다.
- 전방 가속 여부와 낙하 상태를 기준으로 스프린트 가능 여부를 판정한다.
- Tick 에서 입력 가속이 사라지면 스프린트를 해제한다.

### `UBeekeeperCameraShakeComponent`

- 이동 상태 기반 카메라 셰이크를 담당하는 `UActorComponent` 다.
- 상태 enum `EBeekeeperMoveState` 는 `Idle`, `Walk`, `Sprint` 다.
- 수평 속도와 `UBeekeeperMovementComponent::IsSprinting()` 로 상태를 계산한다.
- 공중 상태에서는 이동 셰이크를 중단한다.
- 착지 시 일회성 착지 셰이크를 재생한다.
- 포커스 강제 이동 직후에는 다음 착지 셰이크를 1회 억제할 수 있다.
- 포커스 상호작용 중에는 모든 셰이크를 중지한다.
- 로컬 플레이어가 아니면 Tick 을 느리게 돌리며 셰이크를 중지한다.

### `UBeekeeperFocusComponent`

- 포커스 시스템의 상태 오너다.
- `PreviewFocus` 와 `EngagedFocus` 상태를 구분한다.
- 카메라 전방 라인트레이스로 현재 포커스 타겟을 찾는다.
- 포커스 enter/exit/confirm/cancel 흐름을 관리한다.
- 프롬프트 데이터, 아이템 필터 규칙, 크로스헤어 가시성 변화를 delegate 로 브로드캐스트한다.
- engaged 중인 타겟에서 `UFocusActionComponent` 를 찾아 액션 시작/취소/해제를 위임한다.
- 로컬 플레이어가 아닐 경우 포커스 상태를 정리하고 Tick 빈도를 낮춘다.

### `UFocusTargetComponent`

- 포커스 가능한 액터의 상호작용 메타데이터를 제공한다.
- `DisplayName`, `InteractionKeyText`, `FFocusItemRule`, outline 설정을 보관한다.
- 런타임 setter 로 pickup 류 액터가 표시명과 상호작용 키 텍스트를 갱신할 수 있다.
- 포커스 진입/이탈 시 custom depth outline 을 토글한다.
- owner 가 `IFocusInteractable` 을 구현한 경우 focus enter/exit/confirm/cancel 이벤트를 전달한다.
- `bClearFocusOnConfirm` 값은 포커스 타겟이 confirm 시 preview 를 유지할지 여부를 나타내는 설정으로 존재한다.

### `UFocusActionComponent`

- 포커스 confirm/cancel 상호작용의 공통 베이스다.
- `CanBeginFocusAction`, `BeginFocusAction`, `CancelFocusAction`, `AbortFocusAction` 을 제공한다.
- engaged 상태 여부를 내부에서 관리한다.
- 크로스헤어 숨김 여부와 cancel 시작 시 복구 여부를 UI 정책으로 노출한다.

### `UAnchoredFocusActionComponent`

- 앵커 기반 상호작용의 기본 구현이다.
- owner actor 내부의 `FocusAnchorTag`, `CharacterAnchorTag` 를 가진 `USceneComponent` 를 찾아 사용한다.
- confirm 시 아래 순서로 동작한다.
  - 입력 잠금
  - 카메라 override 시작
  - 캐릭터 앵커 위치로 이동
  - 포커스 카메라 블렌드 시작
- cancel 시 카메라를 기본 1인칭 카메라 transform 으로 블렌드 백한다.
- 블렌드 완료 후 제어 회전 복구, 카메라 재부착, 입력 잠금 해제를 수행한다.
- `OnFocusEngagedStarted`, `OnFocusCancelStarted`, `OnFocusReturnCompleted`, `OnFocusActionAborted` 훅으로 파생 컴포넌트 확장을 허용한다.

### `UAnchoredFocusCursorActionComponent`

- 앵커 기반 상호작용에 커서 및 입력 모드 정책을 추가한다.
- Engaged 동안 크로스헤어를 숨겨야 함을 선언한다.
- cancel 시작 시 크로스헤어 즉시 복구가 필요함을 선언한다.
- engaged 시작 시 로컬 `APlayerController` 에 대해 아래를 수행한다.
  - `bShowMouseCursor = true`
  - `FInputModeGameAndUI`
- 복귀 블렌드 완료 또는 abort 시 아래를 수행한다.
  - `bShowMouseCursor = false`
  - `FInputModeGameOnly`

### `UBeekeeperHotbarComponent`

- 8 슬롯 hotbar 상태 오너다.
- `FHotbarSlotData` 배열과 `SelectedIndex` 로 상태를 관리한다.
- 슬롯 선택/해제, 휠 순환, 포커스 규칙 적용, 슬롯 재평가를 담당한다.
- engaged focus 중에는 선택 슬롯을 강제로 비우고 허용 아이템 태그 규칙으로 슬롯 활성화를 갱신한다.
- 선택된 아이템이 있을 때만 표시 모드가 `InHand` 또는 `OnCursor` 로 바뀐다.
- `OnHotbarChanged` delegate 로 UI 갱신 지점을 제공한다.
- 선택 아이템이 `UItemInstance` 인 경우 표시 이름, 아이콘, 스택 수량 조회를 제공한다.
- `TryAcquireItem()` 으로 동일 정의 stack 병합, 빈 슬롯 자동 배치, 부분 성공 결과 반환을 처리한다.
- 슬롯 아이템은 `UObject` 로 유지되며, concrete class 고정 없이 `IHotbarItemInterface` 와 선택적 `UItemInstance` 접근을 함께 지원한다.

### `UBeekeeperHeldItemVisualizerComponent`

- 선택된 hotbar 아이템의 `WorldMesh` 시각화를 담당한다.
- 로컬 플레이어 카메라에 메시 컴포넌트를 붙여 1인칭 뷰 모델처럼 동작시킨다.
- hotbar 의 `OnHotbarChanged` 와 focus 상태 변화를 구독해 메시/표시 상태를 갱신한다.
- 일반 상태에서는 우하단 held item 오프셋을 사용한다.
- engaged focus 상태에서는 같은 메시를 카메라 앞 로컬 공간에서 커서 위치에 따라 이동시킨다.

### `UItemDefinition`

- 디자이너가 설정하는 아이템 정적 데이터 에셋이다.
- UI 텍스트, 아이콘, 월드 메시, 태그, 최대 스택, 행동 스펙 목록을 가진다.

### `UItemInstance`

- 플레이 중 소유되는 런타임 아이템 오브젝트다.
- `Definition` 을 참조해 표시 데이터와 태그를 읽는다.
- `StackCount`, `Durability`, `InstanceId` 를 관리한다.
- 정의의 액션 스펙을 기반으로 `UItemAction` 객체들을 생성하고 소유한다.

### `UItemAction`

- 아이템별 선택적 행동의 공통 UObject 베이스다.
- `CanExecute`, `Execute`, `GetActionDisplayName`, `GetActionTypeTag` API 를 제공한다.
- 실제 행동 실행은 파생 action 이 확장한다.

### `IFocusInteractable`

- 포커스 가능한 actor 가 구현하는 인터페이스다.
- `OnFocusEnter`, `OnFocusExit`, `OnFocusConfirm`, `OnFocusCancel` 훅을 제공한다.

### `IHotbarItemInterface`

- hotbar 아이템이 자신의 gameplay tag 집합을 제공하는 인터페이스다.
- hotbar 필터링은 concrete item class 대신 이 인터페이스를 통해 수행한다.

### `ABeehive`

- 포커스 상호작용 예시 actor 다.
- `USceneComponent` 루트, `UStaticMeshComponent`, `UFocusTargetComponent`, `UAnchoredFocusCursorActionComponent` 를 소유한다.
- `IFocusInteractable` 를 구현한다.
- focus confirm 시 `bIsLidOpen = true`, cancel 시 `bIsLidOpen = false` 로 상태를 전환한다.
- 세부 연출은 Blueprint implementable event 로 위임한다.

### `AWorldItemPickup`

- 단일 아이템 획득용 월드 actor 다.
- `UItemDefinition` 하나를 보관하고 `WorldMesh` 와 `DisplayName` 을 시각화/프롬프트에 반영한다.
- 획득 성공 시 actor 를 제거하고, 실패 시 그대로 유지된다.

### `UPickupFocusActionComponent`

- 단일 pickup confirm 시점에 즉시 hotbar 획득을 시도하는 `UFocusActionComponent` 파생이다.
- 별도 앵커 이동이나 장기 engaged 상태 없이 단발성으로 종료된다.
- 획득 실패 시 로그 또는 온스크린 디버그 메시지로 최소 피드백만 제공한다.

## 주요 실행 흐름

### 1. 플레이어 입력 라우팅

1. `ABeekeeperController::SetupInputComponent`
   - `DefaultMappingContext` 를 로컬 플레이어 Enhanced Input 서브시스템에 등록한다.
2. `ABeekeeperCharacter::SetupPlayerInputComponent`
   - 이동, 시점, 점프, 스프린트, focus confirm/cancel, hotbar 입력을 바인딩한다.
3. 포커스 입력
   - confirm 입력은 `UBeekeeperFocusComponent::ConfirmFocus()` 로 전달된다.
   - cancel 입력은 `UBeekeeperFocusComponent::CancelFocus()` 로 전달된다.
4. hotbar 입력
   - 숫자 입력은 슬롯 인덱스로 변환되어 `HandleSlotInput()` 으로 전달된다.
   - 휠 입력은 방향만 추출해 `HandleWheelInput()` 으로 전달된다.

### 2. PreviewFocus 흐름

1. `UBeekeeperFocusComponent::TickComponent`
   - 로컬 플레이어 기준으로 카메라 전방 라인트레이스를 수행한다.
2. 타겟 변경 시 `SetPreviewFocusTarget()`
   - 이전 타겟 outline 해제와 `OnFocusExit` 전달
   - 새 타겟 outline 활성화와 `OnFocusEnter` 전달
3. 프롬프트 UI
   - `OnFocusPromptChanged` 로 현재 prompt data 를 브로드캐스트한다.
4. Preview 상태에서는 hotbar 필터를 브로드캐스트하지 않는다.

### 3. EngagedFocus 진입 흐름

1. `ConfirmFocus()`
   - 현재 preview target 에서 `UFocusActionComponent` 를 찾는다.
   - `CanBeginFocusAction()` 통과 시 preview 상태를 정리한다.
2. `BeginFocusAction()`
   - engaged target/action 을 고정한다.
   - 액션 시작 실패 시 preview 상태를 복원한다.
3. 성공 시 후속 처리
   - target 에 `NotifyFocusConfirm()` 전달
   - `OnFocusRuleChanged(true, Rule)` 브로드캐스트
   - 현재 action 정책으로 크로스헤어 가시성을 갱신한다.

### 4. Anchored focus 카메라/입력 흐름

1. `UAnchoredFocusActionComponent::BeginFocusAction`
   - 포커스 앵커와 캐릭터 앵커를 찾는다.
   - 캐릭터 입력을 잠그고 카메라 override 를 시작한다.
   - 캐릭터를 앵커 위치로 이동시킨다.
   - 필요 시 다음 착지 셰이크를 억제한다.
   - 상태를 `BlendingToFocus` 로 전환한다.
2. `TickComponent`
   - 포커스 앵커까지 카메라 블렌드
   - 도착 후 `Focused` 상태 유지
3. `CancelFocusAction`
   - 상태를 `BlendingBack` 으로 전환한다.
4. 복귀 완료 시
   - 기본 카메라 transform 으로 복귀
   - 컨트롤 회전 동기화
   - 카메라 재부착
   - 입력 잠금 해제

### 5. 크로스헤어/커서/UI 흐름

1. 크로스헤어 숨김 정책은 action component 가 제공한다.
2. 최종 가시성 판단과 브로드캐스트는 `UBeekeeperFocusComponent` 가 수행한다.
3. UI/HUD 는 `ShouldHideCrosshair()` 또는 `OnCrosshairVisibilityChanged` 에만 의존한다.
4. `UAnchoredFocusCursorActionComponent`
   - engaged 시작 시 커서 표시 + `FInputModeGameAndUI`
   - cancel 시작 시 크로스헤어 즉시 복구
   - 카메라 복귀 완료 후 커서 숨김 + `FInputModeGameOnly`

### 6. Hotbar 필터링 흐름

1. `UBeekeeperHotbarComponent::BeginPlay`
   - 슬롯을 8개로 초기화한다.
   - focus component 의 `OnFocusRuleChanged` 를 구독한다.
2. `HandleFocusRuleChanged`
   - engaged 여부와 `FFocusItemRule` 을 `ApplyFocusRule()` 로 전달한다.
3. engaged 진입 시
   - 기존 선택 해제
   - 각 슬롯의 enabled 상태 재평가
4. 슬롯 필터링 규칙
   - engaged 가 아니면 모든 슬롯 허용
   - 빈 슬롯은 허용
   - 허용 태그가 비어 있으면 아이템 슬롯은 비허용
   - `AllItemsRootTag` 가 허용 태그에 있으면 모든 아이템 허용
   - 그 외에는 `IHotbarItemInterface::GetHotbarItemTags()` 결과와 허용 태그를 비교한다.

### 7. 카메라 셰이크 흐름

1. `BeginPlay`
   - owner 캐릭터와 이동 컴포넌트를 캐시한다.
   - 초기 falling 상태를 기록한다.
2. `TickComponent`
   - 포커스 입력 잠금 중이면 모든 셰이크를 중지한다.
   - falling 중이면 이동 셰이크를 중지한다.
   - 착지 프레임이면 landing shake 또는 suppress 로직을 처리한다.
   - 이동 상태가 바뀌면 해당 셰이크 클래스로 전환한다.

## 컴포넌트/의존 관계

- `ABeekeeperCharacter`
  - `UBeekeeperMovementComponent`
  - `UBeekeeperCameraShakeComponent`
  - `UBeekeeperFocusComponent`
  - `UBeekeeperHotbarComponent`
  - `UBeekeeperHeldItemVisualizerComponent`
  - `UCameraComponent`
- `ABeekeeperController`
  - `UInputMappingContext`
  - `UEnhancedInputLocalPlayerSubsystem`
- `UBeekeeperFocusComponent`
  - `ABeekeeperCharacter`
  - `UCameraComponent`
  - `UFocusTargetComponent`
  - `UFocusActionComponent`
- `UAnchoredFocusActionComponent`
  - `ABeekeeperCharacter`
  - `USceneComponent` anchor 들
  - `UBeekeeperCameraShakeComponent`
- `UAnchoredFocusCursorActionComponent`
  - 로컬 `APlayerController`
- `UBeekeeperHotbarComponent`
  - `ABeekeeperCharacter`
  - `UBeekeeperFocusComponent`
  - `IHotbarItemInterface`
- `UBeekeeperHeldItemVisualizerComponent`
  - `ABeekeeperCharacter`
  - `UBeekeeperHotbarComponent`
  - `UBeekeeperFocusComponent`
  - `UCameraComponent`
- `UItemInstance`
  - `UItemDefinition`
  - `UItemAction`
- `UFocusTargetComponent`
  - owner actor 의 `UPrimitiveComponent`
  - 선택적으로 `IFocusInteractable`
- `ABeehive`
  - `UFocusTargetComponent`
  - `UAnchoredFocusCursorActionComponent`
- `AWorldItemPickup`
  - `UFocusTargetComponent`
  - `UPickupFocusActionComponent`

## 확인된 설계 특징

- 실제 코드 기준으로 `Public` / `Private` 구조를 일관되게 사용한다.
- 캐릭터 본체는 입력 라우팅과 카메라 기준점 역할에 집중하고, 이동/포커스/핫바/카메라 연출은 컴포넌트로 분리되어 있다.
- 포커스 시스템은 preview 와 engaged 를 명확히 분리한다.
- 상호작용 실행 방식은 `UFocusActionComponent` 계층으로 일반화되어 있어 다른 상호작용 액터로 확장 가능하다.
- UI 는 액션 컴포넌트가 직접 제어하지 않고 delegate 기반 상태 브로드캐스트를 통해 분리된다.
- hotbar 필터링은 engaged 상호작용 중에만 적용되어 일반 탐색 중 UI 제약을 만들지 않는다.
- `UBeekeeperFocusComponent` 와 `UBeekeeperCameraShakeComponent` 는 로컬 플레이어 기준으로만 빠른 Tick 을 유지하고, 비로컬에서는 Tick 부담을 낮춘다.
- `UBeekeeperMovementComponent` 와 focus/camera 일부 흐름은 여전히 Tick 기반 상태 갱신을 사용한다.

## 불명확한 부분

- `Source/BeekeepingSim/Public/BeekeeperCharacter.h` 는 `BeekeepingSimCharacter.h` 를 include 하지만 실제 상속은 `ACharacter` 다. 이 include 가 의도적인 의존성인지 정리되지 않은 잔재인지는 문서만으로 확정할 수 없다.
- `Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h` 의 `IdleSpeedThreshold = 3.0f` 는 주석상 safe default 로 적혀 있다. 실제 프로젝트 감각에 맞는 최종 밸런스 값인지는 코드만으로 확정할 수 없다.
- `UFocusTargetComponent` 의 `bClearFocusOnConfirm` 설정은 현재 public API 에 존재하지만, 현 구현 흐름에서는 명시적으로 사용되지 않는다. 향후 설계 의도인지 미사용 잔재인지 추가 확인이 필요하다.
