# 보류 리팩토링 후속 구현 프롬프트

## 역할

너는 BeekeepingSim 1차 구조 리팩토링 이후 보류된 고위험 리팩토링을 처리하는 후속 구현 에이전트다.

이번 작업은 Blueprint 직렬화, Core Redirect, Public C++ API 호환성 때문에 1차 리팩토링에서 의도적으로 보류한 항목을 단계적으로 정리하는 것이다.

## 반드시 먼저 읽을 문서

- `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`
- `.md/REFACTORING_QNA.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`

## 전제

- 1차 리팩토링은 완료된 상태다.
- 현재 C++ 파일은 시스템별 하위 폴더로 이동되어 있다.
- 이번 작업은 1차 리팩토링에서 금지했던 Content/Config 변경이 필요할 수 있다.
- Content 에셋 저장, Blueprint 그래프 수정, `Config/DefaultEngine.ini` Core Redirect 추가는 사용자 승인 후에만 수행한다.
- 승인 없이 진행 가능한 것은 C++/문서 분석, 참조 재검사, 후속 계획 정리뿐이다.

## 최우선 규칙

1. 소스 수정 전에 현재 빌드가 통과하는지 확인한다.
2. 삭제/rename 전에는 반드시 C++ 참조와 Blueprint 심볼 참조를 다시 검사한다.
3. Blueprint에서 참조가 확인된 API는 Blueprint 그래프 수정/컴파일/저장/재검사 전까지 삭제하거나 rename하지 않는다.
4. UCLASS/USTRUCT/UENUM 이름을 변경하면 Core Redirect 계획을 먼저 세운다.
5. 각 보류 항목은 가능한 한 작은 단위로 나누어 처리하고, 항목별로 빌드와 Blueprint 검증을 수행한다.
6. 예상과 다른 Blueprint 참조, 로드 오류, Core Redirect 실패, 에셋 저장 실패가 발생하면 즉시 중단하고 `.md/QNA_IMPLEMENTATION.md`에 질문을 남긴다.

## 빌드 검증

기본 빌드 명령:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

명령 실행 전 `Test-Path`로 UnrealBuildTool 경로를 확인한다. 경로가 없으면 임의로 다른 버전을 사용하지 말고 사용자에게 확인한다.

## 1. Blueprint 참조 재검사

작업 시작 전에 다음 심볼을 현재 Content 기준으로 다시 검사한다.

- `ItemDragVisualWidget`
- `UItemDragVisualWidget`
- `InitializeDragVisual`
- `OnDragVisualInitialized`
- `GetDragPreviewDisplayStackCount`
- `RefreshDragPreviewFromOperation`
- `RefreshPartialDragPreviewFromOperation`
- `InitializeStorageWidget`
- `MoveHotbarItemToStorage`
- `MoveStorageItemToHotbar`
- `SwapStorageSlots`
- `SwapHotbarAndStorage`
- `bClearFocusOnConfirm`
- `ShouldClearFocusOnConfirm`
- `StorageSlotDragDropOperation`
- `UStorageSlotDragDropOperation`
- `EStorageSlotContainerType`
- `EItemSlotDragMode`
- `FItemSlotMoveResult`
- `ShouldHideItemVisualForCurrentDrag`
- `InitializeSlotContext`
- `IsPartialDragPreviewActive`
- `GetPartialDragPreviewDisplayStackCount`
- `OnStorageWidgetInitialized`

필요 시 현재 `Content` 기준으로 Blueprint 참조 검사 스크립트/JSON을 임시 생성한다. 작업 완료 후에는 최종 요약을 `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`에 반영하고, 중간 Python/JSON 산출물은 정리한다.

## 2. `UItemDragVisualWidget` 삭제 후보 정리

### 배경

1차 검사 결과:

- `UItemDragVisualWidget`을 네이티브 부모로 쓰는 Blueprint는 확인되지 않았다.
- `InitializeDragVisual`, `OnDragVisualInitialized` 심볼 참조도 확인되지 않았다.
- 단, `Content/UI/WBP_ItemVisual.uasset` 내부에 `ItemDragVisualWidget` 문자열이 남아 있었다.

그래서 1차 리팩토링에서는 삭제하지 않고 유지했다.

### 대상 파일

- `Source/BeekeepingSim/Public/UI/ItemDragVisualWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemDragVisualWidget.cpp`
- `Content/UI/WBP_ItemVisual.uasset`

### 후속 처리 목표

가능하면 `UItemDragVisualWidget` 클래스를 완전히 제거한다. 단, Blueprint 에셋에서 잔존 심볼이 제거된 것이 확인된 경우에만 삭제한다.

### 실행 절차

1. 사용자에게 Content 에셋 열기/컴파일/저장 승인을 받는다.
2. Unreal Editor에서 `Content/UI/WBP_ItemVisual`을 열고 컴파일/저장한다.
3. 가능하면 UI 관련 Widget Blueprint도 함께 컴파일/저장한다.
   - `Content/UI/WBP_ItemSlot`
   - `Content/UI/WBP_StorageBox`
   - `Content/UI/WBP_Hotbar`
4. Blueprint 심볼 재검사를 실행한다.
5. `ItemDragVisualWidget`, `UItemDragVisualWidget`, `InitializeDragVisual`, `OnDragVisualInitialized` 참조가 모두 사라졌으면 다음을 수행한다.
   - `ItemDragVisualWidget.h/.cpp` 삭제
   - include/build 참조 제거
   - `.md/Architecture/UISystem.md`에서 잔존 심볼 관련 주석 제거
6. 심볼이 여전히 남아 있으면 삭제하지 않는다.
   - 클래스는 유지한다.
   - `UISystem.md`에 현재 상태와 남은 수동 확인 항목을 갱신한다.

### 주의

이 클래스는 대체 대상이 명확하지 않으므로 단순 삭제에 Core Redirect를 붙이는 방식은 기본 선택지가 아니다. 잔존 참조가 있다면 삭제하지 않는 것이 우선이다.

## 3. Content 미참조 Blueprint 노출 API 정리

### 배경

1차 검사에서 다음 API는 Content 심볼 참조가 확인되지 않았지만, Public C++/Blueprint API 안정성 때문에 유지했다.

### 대상 API

`Source/BeekeepingSim/Public/UI/ItemSlotWidget.h`

- `GetDragPreviewDisplayStackCount()`
- `RefreshDragPreviewFromOperation()`
- `RefreshPartialDragPreviewFromOperation()`

`Source/BeekeepingSim/Public/UI/StorageBoxWidget.h`

- `InitializeStorageWidget()`
- `MoveHotbarItemToStorage()`
- `MoveStorageItemToHotbar()`
- `SwapStorageSlots()`
- `SwapHotbarAndStorage()`

### 후속 처리 목표

Blueprint에 노출할 필요가 없는 API는 Blueprint 표면에서 제거하거나, 삭제 전 단계로 `DeprecatedFunction` 메타데이터를 붙인다.

### 실행 절차

1. 현재 C++ 참조를 다시 확인한다.
2. C++ 내부에서 필요한 함수는 삭제하지 말고 다음 중 하나로 정리한다.
   - `UFUNCTION(BlueprintCallable/Pure)` 제거 후 일반 C++ public/protected/private 함수로 유지
   - 더 좁은 private helper로 이동
3. C++ 참조도 없고 Blueprint 참조도 없는 래퍼는 삭제 후보로 처리한다.
4. 외부 호환성이 걱정되는 경우 즉시 삭제하지 말고 먼저 `DeprecatedFunction` 메타데이터를 붙인다.
5. Blueprint compile/save 후 심볼 재검사를 수행한다.
6. 참조가 없고 빌드가 통과하면 최종 삭제한다.

### 현재 주의 지점

- `RefreshDragPreviewFromOperation()`은 현재 `UStorageSlotDragDropOperation` 쪽 C++ 호출이 있을 수 있으므로 바로 삭제하지 않는다.
- `InitializeStorageWidget()`은 `StorageBoxFocusActionComponent`가 C++에서 호출할 수 있으므로 Blueprint 노출 제거와 함수 삭제를 구분한다.
- `MoveHotbarItemToStorage()`, `MoveStorageItemToHotbar()`, `SwapStorageSlots()`, `SwapHotbarAndStorage()`는 Widget 래퍼 성격이 강하므로 실제 호출이 없다면 삭제 후보로 우선 검토한다.

## 4. Blueprint 참조가 확인된 Widget API 마이그레이션

### 배경

다음 API는 1차 검사에서 Blueprint 참조가 확인되어 유지 대상으로 확정했다.

- `InitializeSlotContext`
- `ShouldHideItemVisualForCurrentDrag`
- `IsPartialDragPreviewActive`
- `GetPartialDragPreviewDisplayStackCount`
- `OnStorageWidgetInitialized`

### 후속 처리 목표

이 중 이름이나 책임이 애매한 API를 제거하고 싶다면 먼저 Blueprint 그래프를 명시적인 새 API로 마이그레이션한다.

### 실행 절차

1. `Content/UI/WBP_ItemSlot`, `Content/UI/WBP_Hotbar`, `Content/UI/WBP_StorageBox`의 그래프 참조를 확인한다.
2. `ShouldHideItemVisualForCurrentDrag` 제거를 원하면 먼저 대체 API를 설계한다.
   - 예: full-stack drag 숨김 여부와 partial-stack preview 표시 여부를 분리한 명확한 함수
3. Blueprint 그래프를 새 API로 교체한다.
4. 해당 Blueprint를 컴파일/저장한다.
5. 심볼 재검사에서 기존 함수 참조가 사라졌을 때만 기존 함수를 deprecate 또는 삭제한다.

### 주의

`IsPartialDragPreviewActive`, `GetPartialDragPreviewDisplayStackCount`는 현재 UI 표시 로직의 명확한 API이므로, 제거가 목적이 아니라면 유지해도 된다.

## 5. `bClearFocusOnConfirm` 미사용 정책 필드 정리

### 배경

1차 검사 결과:

- `UFocusTargetComponent` 클래스 자체는 Blueprint/레벨에서 참조된다.
- `bClearFocusOnConfirm`, `ShouldClearFocusOnConfirm` 심볼 참조는 확인되지 않았다.
- C++ 구현에서도 현재 실제로 읽히지 않는다.

### 대상 파일

- `Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusTargetComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`

### 후속 처리 목표

사용하지 않는 정책 필드라면 제거한다. 단, 사용자나 기획 의도가 "Confirm 후 focus clear 정책"을 실제 기능으로 쓰는 것이라면 삭제하지 말고 동작으로 구현한다.

### 기본 권장안

현재 근거만으로는 삭제가 더 적합하다.

### 실행 절차

1. C++ 전체와 Content 심볼에서 `bClearFocusOnConfirm`, `ShouldClearFocusOnConfirm`, `ClearFocusOnConfirm`을 재검사한다.
2. 참조가 없으면 `bClearFocusOnConfirm` 속성과 `ShouldClearFocusOnConfirm()` getter를 제거한다.
3. Blueprint 에셋 로드/컴파일에서 missing property 경고가 없는지 확인한다.
4. 만약 사용자가 기능 구현을 원하면 삭제하지 말고 `ConfirmFocus` 흐름에서 해당 정책을 실제로 적용한다.

### 중단 조건

Blueprint 또는 레벨 에셋에서 해당 property 참조가 새로 확인되면 삭제하지 말고 사용자에게 확인한다.

## 6. Drag/Drop 타입 rename 후보

### 배경

1차 리팩토링에서는 UCLASS/UENUM/USTRUCT rename을 하지 않았다. 이유는 Public API, UHT, Blueprint, Core Redirect 리스크 때문이다.

### 후보

- `UStorageSlotDragDropOperation` -> `UItemSlotDragDropOperation`
- `StorageSlotDragDropOperation.h/.cpp` -> `ItemSlotDragDropOperation.h/.cpp`
- `StorageSlotDragDropTypes.h` -> `ItemSlotDragDropTypes.h`
- `EStorageSlotContainerType` -> `EItemSlotContainerType`
- `FItemSlotMoveResult`는 현재 이름이 충분히 일반적이므로 rename 필요성이 낮다.
- `EItemSlotDragMode`는 현재 이름이 충분히 일반적이므로 rename 필요성이 낮다.

### Blueprint 참조 위험

1차 검사 기준:

- `EStorageSlotContainerType`은 `WBP_Hotbar`, `WBP_StorageBox`에서 참조가 확인되었다.
- `StorageSlotDragDropOperation`, `EItemSlotDragMode`, `FItemSlotMoveResult`는 Content 심볼 참조가 확인되지 않았다.

### 후속 처리 목표

UI 슬롯 전반에서 쓰는 타입명에 `Storage`가 남아 있는 부분을 `ItemSlot` 중심 이름으로 정리한다.

### 실행 절차

1. 먼저 파일명만 바꾸는 방안과 UCLASS/UENUM 이름까지 바꾸는 방안을 분리해 검토한다.
2. UCLASS/UENUM 이름까지 변경할 경우 `Config/DefaultEngine.ini` Core Redirect 추가 승인을 받는다.
3. 예상 Core Redirect 예시는 다음과 같다. 실제 Unreal Core Redirect 문법은 적용 전 공식 문서/프로젝트 관례로 확인한다.

```ini
[/Script/Engine.Engine]
+ClassRedirects=(OldName="/Script/BeekeepingSim.StorageSlotDragDropOperation",NewName="/Script/BeekeepingSim.ItemSlotDragDropOperation")
+EnumRedirects=(OldName="/Script/BeekeepingSim.EStorageSlotContainerType",NewName="/Script/BeekeepingSim.EItemSlotContainerType")
```

4. C++ rename과 include 경로 수정을 수행한다.
5. Blueprint를 열어 enum pin과 drag/drop operation 참조가 정상 복구되는지 확인한다.
6. 관련 Blueprint를 컴파일/저장한다.
7. 심볼 재검사에서 구 이름 참조가 사라졌는지 확인한다.
8. 빌드와 Editor 로드 검증을 수행한다.

### 중단 조건

- Core Redirect 후 Blueprint enum pin이 깨지거나 기본값이 손실되면 즉시 중단한다.
- `WBP_Hotbar`, `WBP_StorageBox`에서 `EStorageSlotContainerType` 참조가 자동 복구되지 않으면 rename을 되돌리거나 수동 마이그레이션 계획을 사용자에게 확인한다.

## 7. Storage Widget 래퍼 API 제거 후보

### 대상

- `UStorageBoxWidget::MoveHotbarItemToStorage`
- `UStorageBoxWidget::MoveStorageItemToHotbar`
- `UStorageBoxWidget::SwapStorageSlots`
- `UStorageBoxWidget::SwapHotbarAndStorage`

### 배경

이 함수들은 Widget에서 Storage/Hotbar 컴포넌트로 단순 위임하는 래퍼다. 1차 검사에서는 Content 참조가 확인되지 않았다.

### 후속 처리 목표

Blueprint 그래프에서 쓰지 않고 C++에서도 호출하지 않는다면 제거한다. 이동 실행은 `UItemSlotDragDropLibrary` 또는 Inventory 컴포넌트 API 쪽으로 일원화한다.

### 실행 절차

1. C++/Blueprint 참조를 다시 검사한다.
2. 참조가 없으면 `StorageBoxWidget.h/.cpp`에서 함수들을 제거한다.
3. 관련 문서에서 "Content 미참조 public API" 기록을 갱신한다.
4. 빌드와 Blueprint compile 검증을 수행한다.

## 8. Public API deprecate 후 삭제 전략

외부 사용 가능성을 완전히 배제하기 어렵다면 한 번에 삭제하지 않는다.

권장 순서:

1. `DeprecatedFunction` 메타데이터 추가
2. 대체 함수 또는 대체 경로 명시
3. Blueprint 컴파일 경고 확인
4. 에셋 저장 후 심볼 재검사
5. 다음 리팩토링 단계에서 삭제

예시:

```cpp
UFUNCTION(BlueprintCallable, meta=(DeprecatedFunction, DeprecationMessage="Use the item slot drag/drop library or inventory component API instead."))
bool MoveHotbarItemToStorage(int32 HotbarIndex, int32 StorageIndex);
```

단, 이번 작업 목표가 완전 삭제라면 사용자에게 "deprecate 단계 없이 삭제" 승인을 받은 뒤 진행한다.

## 9. 문서 갱신

변경 후 다음 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/REFACTORING_BLUEPRINT_REFERENCE_AUDIT.md`

문서에는 다음을 명확히 적는다.

- 삭제한 API
- 유지한 API와 유지 사유
- Blueprint 마이그레이션 여부
- 추가한 Core Redirect
- 남은 수동 검토 항목

## 10. 최종 검증

최종 보고 전 최소 검증:

1. UBT 빌드 성공
2. `rg "#include \"Public/" Source/BeekeepingSim` 결과 없음
3. 삭제한 클래스/함수/enum의 C++ 참조 없음
4. Blueprint 심볼 재검사 결과 정리
5. Editor에서 관련 Blueprint 컴파일 성공
6. Core Redirect를 추가했다면 Editor 재시작 후 에셋 로드 확인

## 최종 보고 형식

다음 형식으로 보고한다.

```text
[상태] 완료 / 보류 / 질문 필요

[처리한 보류 항목]
- ...

[삭제/rename한 항목]
- ...

[유지한 항목과 이유]
- ...

[Blueprint 마이그레이션]
- 수정한 Blueprint
- 컴파일/저장 결과
- 심볼 재검사 결과

[Core Redirect]
- 추가 여부
- 추가한 redirect
- 검증 결과

[빌드 검증]
- 실행 명령
- 결과

[문서 갱신]
- 수정한 문서

[남은 리스크]
- ...
```
