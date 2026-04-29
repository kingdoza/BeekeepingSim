# Refactoring Implementation Prompt

## 역할

너는 BeekeepingSim 리팩토링 구현 담당 에이전트다.

이 문서는 설계 담당 에이전트가 확정한 구현 지시서다. 아래 범위와 금지 사항을 그대로 따른다.

## 확정된 사용자 답변

- 시스템 분할 기준: 옵션 A
  - `Character`, `Camera`, `Focus`, `Interaction`, `Inventory`, `UI`, `WorldActors`, `Core`
- Content 미참조 Blueprint 노출 API: 유지
- `UItemDragVisualWidget`: 유지
- `UFocusTargetComponent::bClearFocusOnConfirm`: 유지
- 슬롯 이동/부분 스택 이동 중복: public API 유지, 내부 helper/Inventory 내부 유틸로 중복 계산만 통합
- 클래스명/파일명 rename: 하지 않음
- 분석 범위 밖 C++ include 수정: 빌드 오류 해결에 필요한 최소 include 수정만 허용
- 주석 정리: 동작과 무관한 주석 처리 코드/무관 예시 주석 삭제, 파일 상단 copyright 템플릿 주석 유지

## 작업 목표

1. `Source/BeekeepingSim/Public` / `Source/BeekeepingSim/Private` 의 평면 구조를 시스템 단위 하위 폴더로 재구성한다.
2. 클래스명, UCLASS/USTRUCT/UENUM 이름, public Blueprint API는 유지한다.
3. 파일 이동에 따른 include 경로를 수정해 빌드 가능 상태로 만든다.
4. Hotbar/Storage/QuickMove의 스택 계산 중복은 동작 변경 없이 내부 helper로만 줄인다.
5. `.md/0_ARCHITECTURE.md`는 전체 지도 역할로 축소하고, 시스템별 문서는 `.md/Architecture/{시스템명}.md`로 분리한다.

## 수정 가능 경로

- 기본 수정 범위:
  - `Source/BeekeepingSim/Public/**/*.h`
  - `Source/BeekeepingSim/Private/**/*.cpp`
  - `Source/BeekeepingSim/Private/**/*.h`
  - `.md/0_ARCHITECTURE.md`
  - `.md/Architecture/*.md`
- 제한적 수정 허용:
  - `Source/BeekeepingSim` 하위의 분석 범위 밖 C++ 파일은 파일 이동으로 인한 include 오류를 고치는 경우에만 수정한다.
  - 이 제한적 수정은 include 경로 변경만 허용한다.

## 수정 금지 경로

- `Content/`
- `Config/`
- `Intermediate/`
- `Saved/`
- `Binaries/`
- Unreal Engine 내부 코드
- 플러그인 코드
- 외부 라이브러리 코드

## 금지 사항

- UCLASS/USTRUCT/UENUM 이름 변경 금지
- public Blueprint API 삭제 금지
- 클래스명 변경 금지
- 파일명 변경 금지
- Content asset 수정/저장 금지
- Core Redirect 추가 금지
- 동작 변경을 전제로 한 리팩토링 금지
- 저장/로드, 네트워크, 신규 gameplay 기능 추가 금지
- 분석 범위 밖 C++에 include 변경 외 수정 금지

## 목표 폴더 구조

```text
Source/BeekeepingSim/
  Public/
    Character/
    Camera/
    Focus/
    Interaction/
    Inventory/
    UI/
    WorldActors/
  Private/
    Character/
    Camera/
    Focus/
    Interaction/
    Inventory/
    UI/
    WorldActors/
```

`Core`는 문서상 시스템 경계로만 둔다. 현재 60개 분석 대상 중 Core 전용으로 이동할 파일은 확정하지 않는다. 빈 source 폴더는 만들지 않는다.

## 파일 이동 계획

### Character

- `Public/BeekeeperCharacter.h` -> `Public/Character/BeekeeperCharacter.h`
- `Private/BeekeeperCharacter.cpp` -> `Private/Character/BeekeeperCharacter.cpp`
- `Public/BeekeeperController.h` -> `Public/Character/BeekeeperController.h`
- `Private/BeekeeperController.cpp` -> `Private/Character/BeekeeperController.cpp`
- `Public/BeekeeperMovementComponent.h` -> `Public/Character/BeekeeperMovementComponent.h`
- `Private/BeekeeperMovementComponent.cpp` -> `Private/Character/BeekeeperMovementComponent.cpp`
- `Public/BeekeeperHeldItemVisualizerComponent.h` -> `Public/Character/BeekeeperHeldItemVisualizerComponent.h`
- `Private/BeekeeperHeldItemVisualizerComponent.cpp` -> `Private/Character/BeekeeperHeldItemVisualizerComponent.cpp`

### Camera

- `Public/BeekeeperCameraShakeComponent.h` -> `Public/Camera/BeekeeperCameraShakeComponent.h`
- `Private/BeekeeperCameraShakeComponent.cpp` -> `Private/Camera/BeekeeperCameraShakeComponent.cpp`

### Focus

- `Public/BeekeeperFocusComponent.h` -> `Public/Focus/BeekeeperFocusComponent.h`
- `Private/BeekeeperFocusComponent.cpp` -> `Private/Focus/BeekeeperFocusComponent.cpp`
- `Public/FocusTargetComponent.h` -> `Public/Focus/FocusTargetComponent.h`
- `Private/FocusTargetComponent.cpp` -> `Private/Focus/FocusTargetComponent.cpp`
- `Public/FocusActionComponent.h` -> `Public/Focus/FocusActionComponent.h`
- `Private/FocusActionComponent.cpp` -> `Private/Focus/FocusActionComponent.cpp`
- `Public/FocusInteractable.h` -> `Public/Focus/FocusInteractable.h`
- `Public/AnchoredFocusActionComponent.h` -> `Public/Focus/AnchoredFocusActionComponent.h`
- `Private/AnchoredFocusActionComponent.cpp` -> `Private/Focus/AnchoredFocusActionComponent.cpp`
- `Public/AnchoredFocusCursorActionComponent.h` -> `Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Private/AnchoredFocusCursorActionComponent.cpp` -> `Private/Focus/AnchoredFocusCursorActionComponent.cpp`

### Interaction

- `Public/PickupFocusActionComponent.h` -> `Public/Interaction/PickupFocusActionComponent.h`
- `Private/PickupFocusActionComponent.cpp` -> `Private/Interaction/PickupFocusActionComponent.cpp`
- `Public/StorageBoxFocusActionComponent.h` -> `Public/Interaction/StorageBoxFocusActionComponent.h`
- `Private/StorageBoxFocusActionComponent.cpp` -> `Private/Interaction/StorageBoxFocusActionComponent.cpp`

### Inventory

- `Public/BeekeeperHotbarComponent.h` -> `Public/Inventory/BeekeeperHotbarComponent.h`
- `Private/BeekeeperHotbarComponent.cpp` -> `Private/Inventory/BeekeeperHotbarComponent.cpp`
- `Public/StorageBoxComponent.h` -> `Public/Inventory/StorageBoxComponent.h`
- `Private/StorageBoxComponent.cpp` -> `Private/Inventory/StorageBoxComponent.cpp`
- `Public/ItemDefinition.h` -> `Public/Inventory/ItemDefinition.h`
- `Private/ItemDefinition.cpp` -> `Private/Inventory/ItemDefinition.cpp`
- `Public/ItemInstance.h` -> `Public/Inventory/ItemInstance.h`
- `Private/ItemInstance.cpp` -> `Private/Inventory/ItemInstance.cpp`
- `Public/ItemAction.h` -> `Public/Inventory/ItemAction.h`
- `Private/ItemAction.cpp` -> `Private/Inventory/ItemAction.cpp`
- `Public/ItemActionContext.h` -> `Public/Inventory/ItemActionContext.h`
- `Public/ItemActionTypes.h` -> `Public/Inventory/ItemActionTypes.h`
- `Public/HotbarItemInterface.h` -> `Public/Inventory/HotbarItemInterface.h`
- `Public/HotbarPresentationTypes.h` -> `Public/Inventory/HotbarPresentationTypes.h`
- `Public/ItemPresentationActor.h` -> `Public/Inventory/ItemPresentationActor.h`
- `Private/ItemPresentationActor.cpp` -> `Private/Inventory/ItemPresentationActor.cpp`

### UI

- `Public/StorageBoxWidget.h` -> `Public/UI/StorageBoxWidget.h`
- `Private/StorageBoxWidget.cpp` -> `Private/UI/StorageBoxWidget.cpp`
- `Public/ItemVisualWidget.h` -> `Public/UI/ItemVisualWidget.h`
- `Private/ItemVisualWidget.cpp` -> `Private/UI/ItemVisualWidget.cpp`
- `Public/ItemDragVisualWidget.h` -> `Public/UI/ItemDragVisualWidget.h`
- `Private/ItemDragVisualWidget.cpp` -> `Private/UI/ItemDragVisualWidget.cpp`
- `Public/ItemSlotWidget.h` -> `Public/UI/ItemSlotWidget.h`
- `Private/ItemSlotWidget.cpp` -> `Private/UI/ItemSlotWidget.cpp`
- `Public/ItemSlotDragDropLibrary.h` -> `Public/UI/ItemSlotDragDropLibrary.h`
- `Private/ItemSlotDragDropLibrary.cpp` -> `Private/UI/ItemSlotDragDropLibrary.cpp`
- `Public/StorageSlotDragDropOperation.h` -> `Public/UI/StorageSlotDragDropOperation.h`
- `Private/StorageSlotDragDropOperation.cpp` -> `Private/UI/StorageSlotDragDropOperation.cpp`
- `Public/StorageSlotDragDropTypes.h` -> `Public/UI/StorageSlotDragDropTypes.h`

### WorldActors

- `Public/Beehive.h` -> `Public/WorldActors/Beehive.h`
- `Private/Beehive.cpp` -> `Private/WorldActors/Beehive.cpp`
- `Public/WorldItemPickup.h` -> `Public/WorldActors/WorldItemPickup.h`
- `Private/WorldItemPickup.cpp` -> `Private/WorldActors/WorldItemPickup.cpp`
- `Public/StorageBox.h` -> `Public/WorldActors/StorageBox.h`
- `Private/StorageBox.cpp` -> `Private/WorldActors/StorageBox.cpp`

## Include 수정 지침

1. 모든 `#include "Public/Foo.h"` 형태를 새 public 경로로 고친다.
   - 예: `#include "Public/BeekeeperHotbarComponent.h"` -> `#include "Inventory/BeekeeperHotbarComponent.h"`
   - 예: `#include "Public/FocusActionComponent.h"` -> `#include "Focus/FocusActionComponent.h"`
2. 같은 시스템 내부 파일도 public 헤더는 `System/File.h` 형식을 사용한다.
3. `.generated.h` include는 파일명 그대로 유지하고, 항상 해당 header의 마지막 include로 둔다.
4. 분석 범위 밖 C++ 파일에서 빌드 오류가 발생하면, 파일 이동으로 깨진 include 경로만 수정한다.
5. Build.cs 의존성은 변경하지 않는다. include path 변경만으로 해결한다.

## 중복 기능 통합 계획

### 유지할 public API

아래 API는 삭제하거나 시그니처를 바꾸지 않는다.

- `UBeekeeperHotbarComponent::TryAcquireItem`
- `UBeekeeperHotbarComponent::MovePartialToSlot`
- `UStorageBoxComponent::MovePartialStorageToStorage`
- `UStorageBoxComponent::MovePartialStorageToHotbar`
- `UStorageBoxComponent::MovePartialHotbarToStorage`
- `UStorageBoxComponent::MoveHotbarItemToStorage`
- `UStorageBoxComponent::MoveStorageItemToHotbar`
- `UStorageBoxComponent::SwapHotbarAndStorage`
- `UStorageBoxWidget`의 Blueprint 노출 이동/교환 wrapper

### 허용되는 내부 helper

`Private/Inventory` 아래에 private-only helper를 추가할 수 있다.

권장 파일:

- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.h`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`

권장 책임:

- `UItemDefinition::MaxStack` 기반 stack 여유량 계산
- 같은 item definition 여부 확인
- target stack에 수량 병합
- 새 `UItemInstance` 생성과 `InitializeFromDefinition()` 공통화
- `FItemSlotMoveResult`의 요청/이동/잔여 수량 계산 보조

금지:

- Hotbar/Storage의 delegate broadcast를 helper로 옮기지 않는다.
- Hotbar focus rule 재평가를 helper로 옮기지 않는다.
- Hotbar/Storage public API를 helper 중심으로 재설계하지 않는다.
- `IItemContainer` 같은 새 public abstraction은 도입하지 않는다.

컨테이너별 책임은 유지한다.

- Hotbar 변경 후에는 기존처럼 `ReevaluateSlotsInternal()`과 `BroadcastHotbarChanged()` 흐름을 보존한다.
- Storage 변경 후에는 기존처럼 `BroadcastStorageChanged()` 흐름을 보존한다.
- Hotbar 대상 새 아이템 outer는 `UBeekeeperHotbarComponent`로 유지한다.
- Storage 대상 새 아이템 outer는 `UStorageBoxComponent`로 유지한다.
- QuickMove의 대상 슬롯 선택 순서도 유지한다.
  - 같은 item definition이고 stack 여유가 있는 슬롯 우선
  - 없으면 첫 빈 슬롯

## 삭제/정리 대상

### 삭제하지 않을 항목

- `UItemDragVisualWidget` 파일/클래스 유지
- `bClearFocusOnConfirm` 유지
- `ShouldClearFocusOnConfirm()` 유지
- Content 미참조 Blueprint public API 유지
- `UStorageSlotDragDropOperation`, `EItemSlotDragMode`, `FItemSlotMoveResult` 이름 유지
- `EStorageSlotContainerType` 이름 유지

### 주석 정리 대상

파일 상단 `Fill out your copyright notice...` 주석은 유지한다.

아래 동작과 무관한 주석 처리 코드/예시 주석은 제거한다.

- `Private/Character/BeekeeperCharacter.cpp`
  - `PrimaryActorTick.bCanEverTick` 관련 주석 처리 코드
  - `Called when...` 계열 Unreal 템플릿 주석
  - 주석 처리된 `AProfessorCharacter::GetCharacterMovement()` 예시 코드
- `Private/Inventory/BeekeeperHotbarComponent.cpp`
  - 주석 처리된 이전 `HandleWheelInput()` 구현
- `Private/Character/BeekeeperMovementComponent.cpp`
  - 주석 처리된 sprint stop 블록
  - `Find all actors of class Enemy...` 예시 주석
- `Public/Camera/BeekeeperCameraShakeComponent.h`
  - `IdleSpeedThreshold`의 "추측" 설명 주석

`ItemSlotWidget.h`의 Blueprint compatibility wrapper 주석은 현재 public API 유지 결정의 근거이므로 삭제하지 않는다.

## Rename 계획

이번 리팩토링에서는 rename을 수행하지 않는다.

미래 검토 후보로만 문서에 남긴다.

- `StorageSlotDragDropOperation` -> item slot 범용 drag/drop operation 계열 이름
- `StorageSlotDragDropTypes` -> item slot drag/drop types 계열 이름
- `FItemSlotMoveResult`는 이름이 비교적 범용이므로 유지 가능

## Blueprint / Core Redirect 지침

- Blueprint 참조 감사 결과는 `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`를 참조한다.
- 클래스명/UENUM/USTRUCT 이름을 바꾸지 않으므로 Core Redirect는 추가하지 않는다.
- Header 파일 이동은 Blueprint asset class path(`/Script/BeekeepingSim.ClassName`)를 바꾸지 않는다.
- 다음 항목은 Blueprint 참조가 확인되었으므로 삭제/rename 금지:
  - `UItemSlotWidget`
  - `UItemVisualWidget`
  - `UStorageBoxWidget`
  - `EStorageSlotContainerType`
  - `InitializeSlotContext`
  - `ShouldHideItemVisualForCurrentDrag`
  - `IsPartialDragPreviewActive`
  - `GetPartialDragPreviewDisplayStackCount`
  - `OnStorageWidgetInitialized`
- `UItemDragVisualWidget`은 활성 부모 참조는 없지만 `WBP_ItemVisual.uasset`에 잔존 심볼이 있으므로 유지한다.

## Architecture 문서 분할 계획

### `.md/0_ARCHITECTURE.md`

전체 지도 역할로 축소한다.

유지할 내용:

- 프로젝트 개요
- 분석 범위
- 시스템 목록
- 시스템별 문서 링크
- 시스템 간 주요 의존 관계
- 리팩토링 후 source 폴더 구조 요약
- 분석 범위 밖 코드가 존재한다는 주의사항

상세 클래스 설명과 실행 흐름은 시스템별 문서로 이동한다.

### 새 시스템 문서

`.md/Architecture/` 폴더를 만들고 아래 문서를 작성한다.

- `.md/Architecture/CharacterSystem.md`
- `.md/Architecture/CameraSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/CoreSystem.md`

각 문서는 다음 항목을 포함한다.

- `Scope`: 포함 파일 경로
- `Responsibilities`: 시스템 책임
- `Key Classes`: 주요 클래스와 역할
- `Dependencies`: 의존 시스템
- `Refactoring Notes`: 구현 중 주의점
- `Manual Review Points`: 수동 검토 필요 지점

`CoreSystem.md`에는 현재 60개 분석 대상 중 Core 전용 source 파일은 없으며, 모듈 공통 규칙/문서 허브 역할만 한다고 기록한다.

## 단계별 구현 순서

### 1단계. 작업 전 기준 확인

1. `git status --short`로 기존 변경을 확인한다.
2. 사용자 변경으로 보이는 파일은 되돌리지 않는다.
3. `rg --files Source/BeekeepingSim/Public Source/BeekeepingSim/Private -g "*.h" -g "*.cpp"`로 대상 60개 파일을 확인한다.

### 2단계. 시스템 폴더 생성

1. `Public` / `Private` 아래에 필요한 시스템 폴더를 생성한다.
2. 빈 `Core` source 폴더는 생성하지 않는다.

### 3단계. 파일 이동

1. 위 파일 이동 계획대로 파일을 이동한다.
2. 파일명은 변경하지 않는다.
3. 클래스명은 변경하지 않는다.

### 4단계. include 경로 수정

1. 이동된 파일의 include를 새 시스템 경로로 수정한다.
2. `#include "Public/...` 패턴이 남지 않도록 정리한다.
3. 분석 범위 밖 C++에서 깨지는 include가 있으면 include만 최소 수정한다.

### 5단계. 내부 중복 helper 적용

1. `Private/Inventory/ItemStackMoveUtils.*`를 추가한다.
2. stack 병합/여유 수량/인스턴스 생성 계산만 helper로 옮긴다.
3. Hotbar/Storage public API, delegate broadcast, focus reevaluation 순서는 유지한다.
4. 이동/부분 이동 결과가 기존과 같은지 수동 검토한다.

### 6단계. 주석 정리

1. 지정된 주석 처리 코드와 무관 예시 주석만 삭제한다.
2. 파일 상단 copyright 주석은 유지한다.
3. Blueprint compatibility 설명 주석은 유지한다.

### 7단계. Architecture 문서 갱신

1. `.md/Architecture/*.md` 문서를 작성한다.
2. `.md/0_ARCHITECTURE.md`를 전체 지도 문서로 축소한다.
3. 기존 상세 내용은 시스템 문서로 분산한다.

### 8단계. 검증

1. include 잔여 패턴 확인:
   - `rg "#include \"Public/" Source/BeekeepingSim`
2. forbidden rename 확인:
   - `rg "class BEEKEEPINGSIM_API UStorageSlotDragDropOperation|enum class EStorageSlotContainerType|struct FItemSlotMoveResult" Source/BeekeepingSim`
3. 빌드 확인:
   - UnrealBuildTool로 `BeekeepingSimEditor Win64 Development` 빌드를 수행한다.
4. 빌드 실패 시 파일 이동으로 인한 include 오류만 수정한다.

## 수동 검토 필요 지점

- `UItemDragVisualWidget`은 유지하되, 추후 Content 작업에서 `WBP_ItemVisual`을 열기/컴파일/저장한 뒤 잔존 심볼 제거 여부를 다시 확인한다.
- `bClearFocusOnConfirm`은 유지하되, `FocusSystem.md`에 "현재 미사용 정책 필드"로 기록한다.
- Content 미참조 Blueprint public API는 유지하되, `UISystem.md`에 "Content 미참조 public compatibility API"로 기록한다.
- `StorageSlotDragDropOperation` 이름은 유지하되, `UISystem.md`에 future rename 후보로 기록한다.
- 슬롯 이동 helper 적용 후, Hotbar 선택/활성화 재평가와 Storage UI 갱신 delegate가 기존 순서대로 호출되는지 확인한다.

## 완료 기준

- 대상 파일이 시스템 폴더로 이동되어 있다.
- 모든 include가 새 경로로 정리되어 있다.
- 클래스명/USTRUCT/UENUM/public API는 기존 이름을 유지한다.
- `Content/`는 수정되지 않았다.
- `.md/0_ARCHITECTURE.md`는 지도 문서로 축소되어 있다.
- `.md/Architecture/*.md`가 시스템별 상세 문서를 가진다.
- UnrealBuildTool 빌드가 성공한다.
