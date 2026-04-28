# 구현 프롬프트: Storage Widget 초기 Refresh 전 ActiveStorage 등록

## 문제

저장소를 처음 열었을 때 `WBP_StorageBox.RefreshAllSlots` 는 실행되지만, storage slot 들이 모두 빈 아이템/비활성 상태로 표시된다.  
이후 저장소에 아이템을 하나라도 넣으면 슬롯 표시가 정상 갱신된다.

원인은 `UStorageBoxFocusActionComponent::BeginFocusAction()` 의 호출 순서다.

현재 흐름은 widget 초기화가 active storage context 등록보다 먼저 실행된다.

```cpp
ActiveWidget->InitializeStorageWidget(StorageComponent, HotbarComponent);
ActiveWidget->AddToViewport();

if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(PlayerController))
{
    BeekeeperController->SetActiveStorageComponent(StorageComponent);
}
```

그런데 `UItemSlotWidget::RefreshFromData()` 는 storage slot 데이터를 controller context 로 resolve 한다.

```cpp
ResolveStorageComponentForSlot()
→ ABeekeeperController::GetActiveStorageComponent()
```

따라서 `InitializeStorageWidget()` 내부의 `OnStorageWidgetInitialized()` 에서 Blueprint 가 `RefreshAllSlots()` 를 호출하면, 이 시점에는 아직 active storage 가 등록되지 않아 storage slot 들이 null context 로 refresh 된다.

## 목표

storage widget 의 최초 refresh 가 실행되기 전에 `ABeekeeperController::SetActiveStorageComponent(StorageComponent)` 를 먼저 호출하도록 순서를 수정한다.

## 대상 파일

- `Source/BeekeepingSim/Private/StorageBoxFocusActionComponent.cpp`

필요 시 참조:

- `Source/BeekeepingSim/Public/BeekeeperController.h`
- `Source/BeekeepingSim/Private/BeekeeperController.cpp`
- `Source/BeekeepingSim/Private/ItemSlotWidget.cpp`

## 구현 요구사항

### 1. ActiveStorage 등록 순서 변경

`UStorageBoxFocusActionComponent::BeginFocusAction()` 에서 `ActiveWidget` 생성 성공 후, `InitializeStorageWidget()` 호출 전에 active storage 를 등록한다.

변경 후 흐름:

```cpp
ActiveWidget = CreateWidget<UStorageBoxWidget>(PlayerController, StorageWidgetClass);
if (!ActiveWidget)
{
    CleanupInteractionUI();
    bIsActionEngaged = false;
    return false;
}

if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(PlayerController))
{
    BeekeeperController->SetActiveStorageComponent(StorageComponent);
}

ActiveWidget->InitializeStorageWidget(StorageComponent, HotbarComponent);
ActiveWidget->AddToViewport();
```

### 2. 기존 cleanup 정책 유지

`CleanupInteractionUI()` 에서 active storage 를 해제하는 기존 정책은 유지한다.

요구사항:

- 현재 active storage 가 이 focus action 의 `StorageComponent` 와 같을 때만 clear
- 다른 storage context 를 실수로 지우지 않도록 기존 방어 로직 유지

예상 형태:

```cpp
if (ABeekeeperController* BeekeeperController = Cast<ABeekeeperController>(PlayerController))
{
    if (BeekeeperController->GetActiveStorageComponent() == StorageComponent)
    {
        BeekeeperController->ClearActiveStorageComponent();
    }
}
```

### 3. Blueprint workaround 추가 금지

이 문제는 C++ 초기화 순서 문제이므로 Blueprint 에 `Delay 0.0` 같은 우회 처리를 추가하지 않는다.

`WBP_StorageBox.OnStorageWidgetInitialized -> RefreshAllSlots` 흐름은 그대로 유지되어야 한다.

## 검증 항목

- 저장소를 처음 열자마자 기존 저장소 아이템이 바로 표시된다.
- 저장소를 처음 열자마자 빈 슬롯도 활성 슬롯 상태로 표시된다.
- 저장소에 아이템을 새로 넣지 않아도 `RefreshAllSlots` 결과가 정상이다.
- `WBP_StorageBox.OnStorageWidgetInitialized` 에 `Delay` 없이 동작한다.
- 저장소를 닫으면 active storage context 가 기존처럼 해제된다.
- 다른 storage box 를 연속으로 열어도 이전 storage context 가 남지 않는다.
- UBT 빌드 통과.
