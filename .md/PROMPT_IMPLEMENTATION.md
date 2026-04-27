# 구현 프롬프트: 독립 Hotbar 호환 Item Slot Drag/Drop 라우팅

## 작업 목표

Hotbar UI 는 StorageBox 상호작용 여부와 무관하게 항상 표시되는 독립 UI 다.

따라서 drag/drop 라우팅이 `UStorageBoxWidget` 에 종속되면 안 된다. 기존 `UStorageBoxWidget::HandleSlotDrop()` 중심 구조를 제거하고, source/target component 참조를 기반으로 동작하는 중립 C++ 라우터로 변경한다.

변경 목표:

- `WBP_Hotbar` 는 `UStorageBoxWidget` 을 몰라도 hotbar 내부 slot swap 을 처리할 수 있어야 한다.
- `WBP_Hotbar` 는 storage 에서 넘어온 drag/drop 도 `UStorageBoxWidget` 참조 없이 처리할 수 있어야 한다.
- `WBP_StorageBox` / storage slot UI 는 hotbar UI 를 몰라도 storage 내부 swap, storage -> hotbar, hotbar -> storage 를 처리할 수 있어야 한다.
- source/target 구분은 `EStorageSlotContainerType` 으로 유지한다.
- drag/drop operation 은 source container type/index 뿐 아니라 source component 참조도 보관한다.
- drop 라우팅은 `UBlueprintFunctionLibrary` 기반 중립 함수가 담당한다.
- `UStorageBoxWidget` 은 storage UI root 역할만 수행하고, cross-container drop 라우터 역할을 갖지 않는다.

## 현재 문제

현재 구현 또는 직전 설계에는 아래 문제가 있다.

- `UStorageBoxWidget::HandleSlotDrop()` 이 source/target 조합 라우팅을 담당한다.
- 독립 HUD `WBP_Hotbar` 가 storage item drop 을 처리하려면 `UStorageBoxWidget` 을 알아야 한다.
- 이는 hotbar UI 를 storage UI 에 종속시킨다.
- 향후 StorageBox 상호작용 없이 hotbar 내부 item 위치 swap 을 지원할 때도 구조가 어색해진다.

이번 작업은 이 종속성을 제거한다.

## 대상 파일 목록

신규 파일:

- `Source/BeekeepingSim/Public/ItemSlotDragDropLibrary.h`
- `Source/BeekeepingSim/Private/ItemSlotDragDropLibrary.cpp`

수정 대상:

- `Source/BeekeepingSim/Public/StorageSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/StorageSlotDragDropOperation.cpp`
- `Source/BeekeepingSim/Public/StorageBoxWidget.h`
- `Source/BeekeepingSim/Private/StorageBoxWidget.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/PROMPT_IMPLEMENTATION.md`

확인 대상:

- `Source/BeekeepingSim/Public/StorageSlotDragDropTypes.h`
- `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Public/StorageBoxComponent.h`
- `Source/BeekeepingSim/Private/StorageBoxComponent.cpp`
- `Source/BeekeepingSim/BeekeepingSim.Build.cs`

## 전제

아래 API 는 이미 존재한다고 가정한다. 없으면 먼저 추가한다.

- `EStorageSlotContainerType`
  - `None`
  - `Hotbar`
  - `Storage`
- `UBeekeeperHotbarComponent::SwapSlots(int32 FromIndex, int32 ToIndex)`
- `UStorageBoxComponent`
  - `MoveHotbarItemToStorage(UBeekeeperHotbarComponent* HotbarComponent, int32 HotbarIndex, int32 StorageIndex)`
  - `MoveStorageItemToHotbar(UBeekeeperHotbarComponent* HotbarComponent, int32 StorageIndex, int32 HotbarIndex)`
  - `SwapStorageSlots(int32 FromIndex, int32 ToIndex)`
  - `SwapHotbarAndStorage(UBeekeeperHotbarComponent* HotbarComponent, int32 HotbarIndex, int32 StorageIndex)`

## 구현 요구사항

### 1. Drag/drop operation 에 source component 참조 추가

`UStorageSlotDragDropOperation` 에 source component 참조를 추가한다.

header 요구사항:

```cpp
class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UItemInstance;
```

property 예시:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
EStorageSlotContainerType SourceType = EStorageSlotContainerType::None;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
int32 SourceIndex = INDEX_NONE;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
TObjectPtr<UBeekeeperHotbarComponent> SourceHotbarComponent = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
TObjectPtr<UStorageBoxComponent> SourceStorageComponent = nullptr;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Storage Drag Drop")
TObjectPtr<UItemInstance> ItemInstance = nullptr;
```

의미:

- `SourceType == Hotbar`
  - `SourceHotbarComponent` 가 유효해야 한다.
  - `SourceStorageComponent` 는 null 이어도 된다.
- `SourceType == Storage`
  - `SourceStorageComponent` 가 유효해야 한다.
  - `SourceHotbarComponent` 는 null 이어도 된다.
- `ItemInstance` 는 drag visual 표시 보조 데이터다.
- 실제 이동/교환의 기준은 source/target type, source/target index, source/target component 참조다.

금지:

- `UStorageBoxWidget* SourceStorageWidget` 참조를 operation 에 넣지 않는다.
- `UDragDropOperation::Payload` 에 source metadata 를 넣는 방식으로 구현하지 않는다.

### 2. 중립 drag/drop 라우터 추가

신규 `UItemSlotDragDropLibrary` 를 추가한다.

상속:

```cpp
UItemSlotDragDropLibrary : public UBlueprintFunctionLibrary
```

header 예시:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Public/StorageSlotDragDropTypes.h"
#include "ItemSlotDragDropLibrary.generated.h"

class UBeekeeperHotbarComponent;
class UStorageBoxComponent;
class UStorageSlotDragDropOperation;

UCLASS()
class BEEKEEPINGSIM_API UItemSlotDragDropLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category = "Item Slot|Drag Drop")
    static bool HandleItemSlotDrop(
        UStorageSlotDragDropOperation* Operation,
        EStorageSlotContainerType TargetType,
        int32 TargetIndex,
        UBeekeeperHotbarComponent* TargetHotbarComponent,
        UStorageBoxComponent* TargetStorageComponent);
};
```

이름은 `UItemSlotDragDropLibrary` 를 사용한다. Storage 전용 widget 함수가 아니라 item slot 간 이동 라우터라는 의미를 명확히 하기 위함이다.

### 3. Drop 라우팅 규칙

`UItemSlotDragDropLibrary::HandleItemSlotDrop()` 은 아래 source/target 조합을 처리한다.

#### Hotbar -> Hotbar

조건:

- `Operation->SourceType == EStorageSlotContainerType::Hotbar`
- `TargetType == EStorageSlotContainerType::Hotbar`
- `Operation->SourceHotbarComponent` 유효
- `TargetHotbarComponent` 유효
- source hotbar 와 target hotbar 가 같은 component

동작:

```cpp
return TargetHotbarComponent->SwapSlots(Operation->SourceIndex, TargetIndex);
```

주의:

- 서로 다른 hotbar component 간 이동은 현재 범위 밖이다.
- source/target hotbar component 가 다르면 false 반환한다.

#### Hotbar -> Storage

조건:

- `Operation->SourceType == Hotbar`
- `TargetType == Storage`
- `Operation->SourceHotbarComponent` 유효
- `TargetStorageComponent` 유효

동작:

```cpp
return TargetStorageComponent->MoveHotbarItemToStorage(
    Operation->SourceHotbarComponent,
    Operation->SourceIndex,
    TargetIndex);
```

기존 `MoveHotbarItemToStorage()` 내부 정책을 따른다.

- 대상 storage 슬롯이 비어 있으면 이동
- 대상 storage 슬롯에 아이템이 있으면 hotbar/storage 교환

#### Storage -> Hotbar

조건:

- `Operation->SourceType == Storage`
- `TargetType == Hotbar`
- `Operation->SourceStorageComponent` 유효
- `TargetHotbarComponent` 유효

동작:

```cpp
return Operation->SourceStorageComponent->MoveStorageItemToHotbar(
    TargetHotbarComponent,
    Operation->SourceIndex,
    TargetIndex);
```

기존 `MoveStorageItemToHotbar()` 내부 정책을 따른다.

- 대상 hotbar 슬롯이 비어 있으면 이동
- 대상 hotbar 슬롯에 아이템이 있으면 hotbar/storage 교환

#### Storage -> Storage

조건:

- `Operation->SourceType == Storage`
- `TargetType == Storage`
- `Operation->SourceStorageComponent` 유효
- `TargetStorageComponent` 유효
- source storage 와 target storage 가 같은 component

동작:

```cpp
return TargetStorageComponent->SwapStorageSlots(Operation->SourceIndex, TargetIndex);
```

주의:

- 서로 다른 storage component 간 이동은 현재 범위 밖이다.
- source/target storage component 가 다르면 false 반환한다.

#### 공통 실패 조건

아래 경우 false 를 반환한다.

- `Operation == nullptr`
- `SourceType == None`
- `TargetType == None`
- `SourceIndex == INDEX_NONE`
- `TargetIndex == INDEX_NONE`
- 필요한 source component 가 null
- 필요한 target component 가 null
- 같은 컨테이너 타입 간 drop 인데 source component 와 target component 가 서로 다름
- 지원하지 않는 source/target 조합

### 4. StorageBoxWidget 라우팅 책임 제거

`UStorageBoxWidget` 은 더 이상 drag/drop source/target 조합 라우팅을 담당하지 않는다.

수정 요구사항:

- `UStorageBoxWidget::HandleSlotDrop()` 은 제거한다.
- `UStorageBoxWidget::SwapHotbarSlots()` wrapper 도 제거한다.
- 기존 `MoveHotbarItemToStorage`, `MoveStorageItemToHotbar`, `SwapStorageSlots`, `SwapHotbarAndStorage` wrapper 는 기존 Blueprint 호환을 위해 유지해도 된다.
- 신규 Blueprint 작업에서는 cross-container drag/drop 처리를 `UItemSlotDragDropLibrary::HandleItemSlotDrop()` 으로만 수행한다.

의도:

- 독립 `WBP_Hotbar` 가 `UStorageBoxWidget` 을 몰라도 된다.
- `UStorageBoxWidget` 은 storage UI root, storage component access, storage UI refresh 책임만 가진다.
- item slot 간 이동 라우팅은 widget class 가 아니라 중립 library 가 담당한다.

### 5. Blueprint 사용 흐름

#### 공통 slot widget 변수

Hotbar slot widget 과 storage slot widget 은 아래 변수를 가진다.

- `ContainerType`: `EStorageSlotContainerType`
- `SlotIndex`: `int32`
- `ItemInstance`: `UItemInstance` reference, optional

Hotbar slot widget 은 추가로 아래 참조를 가진다.

- `HotbarComponent`: `UBeekeeperHotbarComponent` reference

Storage slot widget 은 추가로 아래 참조를 가진다.

- `StorageComponent`: `UStorageBoxComponent` reference

`OwnerStorageWidget` 은 hotbar slot 에 요구하지 않는다.

#### Hotbar slot drag 시작

`OnDragDetected` 에서:

```text
Create Drag Drop Operation
Class = UStorageSlotDragDropOperation 또는 BP 파생 class

Operation.SourceType = Hotbar
Operation.SourceIndex = SlotIndex
Operation.SourceHotbarComponent = HotbarComponent
Operation.SourceStorageComponent = None
Operation.ItemInstance = ItemInstance
```

#### Storage slot drag 시작

`OnDragDetected` 에서:

```text
Create Drag Drop Operation
Class = UStorageSlotDragDropOperation 또는 BP 파생 class

Operation.SourceType = Storage
Operation.SourceIndex = SlotIndex
Operation.SourceHotbarComponent = None
Operation.SourceStorageComponent = StorageComponent
Operation.ItemInstance = ItemInstance
```

#### Hotbar slot drop

`OnDrop` 에서:

```text
Operation -> Cast to UStorageSlotDragDropOperation

UItemSlotDragDropLibrary::HandleItemSlotDrop(
    Operation,
    TargetType = Hotbar,
    TargetIndex = SlotIndex,
    TargetHotbarComponent = HotbarComponent,
    TargetStorageComponent = None)
```

처리 가능 케이스:

- Hotbar -> Hotbar
- Storage -> Hotbar

#### Storage slot drop

`OnDrop` 에서:

```text
Operation -> Cast to UStorageSlotDragDropOperation

UItemSlotDragDropLibrary::HandleItemSlotDrop(
    Operation,
    TargetType = Storage,
    TargetIndex = SlotIndex,
    TargetHotbarComponent = None,
    TargetStorageComponent = StorageComponent)
```

처리 가능 케이스:

- Hotbar -> Storage
- Storage -> Storage

### 6. UI 갱신 정책

Drop 성공 후 UI 갱신은 기존 delegate 기반 갱신을 우선 사용한다.

- hotbar 변경:
  - `UBeekeeperHotbarComponent::OnHotbarChanged`
- storage 변경:
  - `UStorageBoxComponent::OnStorageChanged`

Blueprint 에서 즉시 refresh 가 필요하면 `HandleItemSlotDrop()` 반환값이 true 일 때 해당 slot list refresh 이벤트를 호출해도 된다.

단, C++ 라우터는 UI refresh 를 직접 호출하지 않는다.

## 구현 원칙

- Hotbar UI 는 StorageBox UI 에 의존하지 않는다.
- StorageBox UI 는 Hotbar UI 에 의존하지 않는다.
- drag/drop operation 은 source component 참조를 보관한다.
- drop target slot 은 자기 target component 참조만 전달한다.
- cross-container 이동/교환은 component API 로 처리한다.
- 라우팅은 `UItemSlotDragDropLibrary` 에서 수행한다.
- `UStorageBoxWidget` 은 drop 라우터가 아니다.
- 서로 다른 hotbar 간 이동, 서로 다른 storage 간 이동은 현재 범위 밖이며 false 반환한다.
- 실제 UMG slot layout, hover, drag visual, item icon 표시, 빈 슬롯 표시 등은 Blueprint 구현 범위다.
- 기존 `Public` / `Private` 구조와 Unreal coding style 을 따른다.

## 문서 반영 요구사항

구현 완료 후 `.md/0_ARCHITECTURE.md` 의 변경된 부분만 갱신한다.

반영할 내용:

- `UStorageSlotDragDropOperation` 은 source container type/index 뿐 아니라 source hotbar/storage component 참조를 보관한다.
- `UItemSlotDragDropLibrary` 는 widget 에 종속되지 않는 item slot drag/drop 라우터다.
- `UStorageBoxWidget::HandleSlotDrop()` 중심 구조는 제거된다.
- Hotbar UI 는 StorageBoxWidget 참조 없이 hotbar 내부 swap 과 storage -> hotbar drop 을 처리할 수 있다.
- Storage UI 는 HotbarWidget 참조 없이 hotbar -> storage drop 과 storage 내부 swap 을 처리할 수 있다.
- `UStorageBoxWidget` 은 storage UI root 및 refresh/API 표면 역할만 담당한다.

## 검증 요구사항

- Unreal Build Tool 로 `BeekeepingSimEditor Win64 Development` 빌드를 시도한다.
- 빌드가 불가능하면 최소한 아래를 정적으로 점검한다.
  - `UStorageSlotDragDropOperation` 의 component forward declaration/include
  - `UItemSlotDragDropLibrary` 의 `BlueprintFunctionLibrary` include
  - `UMG` module dependency 여부
  - `HandleItemSlotDrop()` 의 모든 source/target 조합 처리 여부
  - `UStorageBoxWidget::HandleSlotDrop()` 제거 또는 미사용 여부
  - hotbar slot Blueprint 가 `UStorageBoxWidget` 참조 없이 hotbar->hotbar, storage->hotbar 를 처리 가능한지
  - storage slot Blueprint 가 `WBP_Hotbar` 참조 없이 hotbar->storage, storage->storage 를 처리 가능한지

## 출력 요구사항

작업 완료 보고는 아래 형식을 따른다.

```
[상태] 완료
[요약] 중립 ItemSlotDragDropLibrary 기반 drag/drop 라우팅과 Hotbar 독립성 유지 요약
[변경 파일] 수정/추가/제거한 API 목록
[검증] 빌드 또는 정적 확인 결과
[주의] Blueprint slot widget 에서 source/target component 참조를 설정해야 하는 지점
```

---

## 구현 상태 (2026-04-27)

- 구현 완료
- `UStorageSlotDragDropOperation` 에 source hotbar/storage component 참조 추가
- `UItemSlotDragDropLibrary` 추가 및 `HandleItemSlotDrop()` 라우팅 구현
- `UStorageBoxWidget::HandleSlotDrop()`, `SwapHotbarSlots()` 제거
