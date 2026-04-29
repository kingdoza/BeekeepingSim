# Refactoring QnA

## 상태

- 답변 완료

---

### [질문 1]

1. 시스템 분할 기준 확정
- 질문 내용: 현재 평면 배치된 60개 파일을 어떤 시스템 경계로 나눌지 결정이 필요하다.
- 필요한 이유: `Hotbar`, `Storage`, `Item`, `UI Drag/Drop`, `Focus Action`이 서로 강하게 연결되어 있어 폴더 이동과 시스템별 Architecture 문서 분할 기준을 확정해야 한다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
  - `Source/BeekeepingSim/Public/StorageBoxComponent.h`
  - `Source/BeekeepingSim/Public/StorageSlotDragDropOperation.h`
  - `Source/BeekeepingSim/Public/ItemSlotWidget.h`
  - `Source/BeekeepingSim/Public/FocusActionComponent.h`
  - `Source/BeekeepingSim/Public/StorageBoxFocusActionComponent.h`
- 선택지:
  - 옵션 A: 큰 시스템 기준으로 `Character`, `Camera`, `Focus`, `Interaction`, `Inventory`, `UI`, `WorldActors`, `Core`로 분리한다. Hotbar/Storage/Item 런타임 상태는 `Inventory`, Widget/DragDropOperation/DragDropLibrary는 `UI`, Pickup/Storage focus action은 `Interaction`에 둔다.
  - 옵션 B: 더 세분화해 `Hotbar`, `Storage`, `Item`, `Focus`, `UI`, `Character`, `Camera`, `WorldActors`, `Core`로 분리한다.
  - 옵션 C: 기능 액터 기준으로 `Beekeeper`, `Beehive`, `Pickup`, `Storage`, `Item`, `UI`, `Focus`처럼 게임 오브젝트 중심으로 분리한다.
- 권장 옵션: 옵션 A. 시스템 수를 과도하게 늘리지 않으면서 현재 의존 방향을 가장 안정적으로 정리할 수 있다.
- 사용자 답변: 옵션A

---

### [질문 2]

1. Content 미참조 Blueprint 노출 API 처리
- 질문 내용: Blueprint 참조 정확검사로 참조가 확인된 API는 유지 대상으로 확정하고 QnA에서 제외했다. 남은 문제는 현재 Content에서 직접 참조되지 않는 Blueprint 노출 API를 public 호환성 차원에서 유지할지, 단계적으로 폐기할지, 삭제 후보로 볼지 결정하는 것이다.
- 필요한 이유: 현재 Content 참조가 없어도 `BlueprintCallable`/`BlueprintPure` public API 삭제는 외부 사용, 미저장 Blueprint, 향후 마이그레이션 흐름에 영향을 줄 수 있다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/ItemSlotWidget.h`
  - `Source/BeekeepingSim/Private/ItemSlotWidget.cpp`
  - `Source/BeekeepingSim/Public/StorageBoxWidget.h`
  - `Source/BeekeepingSim/Private/StorageBoxWidget.cpp`
- 검토 대상:
  - `GetDragPreviewDisplayStackCount()`
  - `RefreshDragPreviewFromOperation()`
  - `RefreshPartialDragPreviewFromOperation()`
  - `InitializeStorageWidget()`
  - `MoveHotbarItemToStorage()`
  - `MoveStorageItemToHotbar()`
  - `SwapStorageSlots()`
  - `SwapHotbarAndStorage()`
- 선택지:
  - 옵션 A: 이번 리팩토링에서는 유지하고 문서상 "Content 미참조 public API"로만 기록한다.
  - 옵션 B: `DeprecatedFunction` 메타데이터 등을 붙이는 단계적 폐기 설계를 포함한다.
  - 옵션 C: 삭제 후보로 확정한다.
- 권장 옵션: 옵션 A. 이번 작업은 구조 설계 중심이며 public API 삭제는 별도 마이그레이션 단계가 안전하다.
- 사용자 답변: 옵션A

---

### [질문 3]

1. `UItemDragVisualWidget` 잔존 심볼 처리
- 질문 내용: `UItemDragVisualWidget`은 활성 Blueprint 네이티브 부모로는 확인되지 않았고 함수 심볼 참조도 없다. 다만 `Content/UI/WBP_ItemVisual.uasset` 안에 `ItemDragVisualWidget` 잔존 심볼이 있어 삭제 후보로 확정할지 결정이 필요하다.
- 필요한 이유: 활성 참조는 아니더라도 직렬화된 잔존 심볼이 있는 상태에서 UCLASS를 삭제하면 Blueprint 로드/컴파일 중 경고나 복구 작업이 발생할 수 있다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/ItemDragVisualWidget.h`
  - `Source/BeekeepingSim/Private/ItemDragVisualWidget.cpp`
  - `Source/BeekeepingSim/Public/ItemVisualWidget.h`
  - `Source/BeekeepingSim/Private/ItemVisualWidget.cpp`
- 관련 에셋:
  - `Content/UI/WBP_ItemVisual.uasset`
- 선택지:
  - 옵션 A: 이번 리팩토링에서는 유지하고 `UI` 시스템 문서에 "C++ 미참조, WBP_ItemVisual 잔존 심볼 있음"으로 기록한다.
  - 옵션 B: 구현 단계에서 `WBP_ItemVisual` 열기/컴파일/저장 후 재검사하고, 잔존 심볼이 사라질 때만 삭제 후보로 확정한다.
  - 옵션 C: 삭제 후보로 확정하되 Core Redirect 또는 Blueprint 수동 복구 계획을 포함한다.
- 권장 옵션: 옵션 A. Content 수정이 이번 리팩토링 구현 범위 밖이므로 삭제 확정은 아직 위험하다.
- 사용자 답변: 옵션A

---

### [질문 4]

1. `UFocusTargetComponent::bClearFocusOnConfirm` 처리
- 질문 내용: `bClearFocusOnConfirm`과 `ShouldClearFocusOnConfirm()`은 현재 C++ 구현에서 읽히지 않고, Content 심볼 참조도 확인되지 않았다. public API로 유지할지, 삭제 후보로 확정할지, 실제 동작으로 구현할지 결정이 필요하다.
- 필요한 이유: Blueprint 참조 위험은 낮아졌지만 public/Blueprint 노출 속성 제거는 API 변경이고, 실제 동작 구현은 리팩토링을 넘어서는 행동 변경이다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/FocusTargetComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperFocusComponent.cpp`
- 선택지:
  - 옵션 A: 유지하고 `FocusSystem` 문서에 "현재 미사용 정책 필드, 후속 구현 여부 수동 검토"로 기록한다.
  - 옵션 B: 삭제 후보로 확정한다.
  - 옵션 C: 삭제하지 않고 `ConfirmFocus()` 흐름에서 실제로 사용하도록 후속 기능 구현 대상으로 분리한다.
- 권장 옵션: 옵션 A. 이번 작업은 리팩토링 설계이며 동작 변경을 포함하지 않아야 한다.
- 사용자 답변: 옵션A

---

### [질문 5]

1. 슬롯 이동/부분 스택 이동 로직 통합 방식
- 질문 내용: Hotbar/Storage/QuickMove에 같은 아이템 병합, 빈 슬롯 탐색, 부분 수량 이동, 새 `UItemInstance` 생성 로직이 반복된다. 어떤 방식으로 통합할지 결정이 필요하다.
- 필요한 이유: 중복 제거는 유지보수성에 중요하지만, 소유자 outer, delegate 브로드캐스트, hotbar focus rule 재평가가 얽혀 있어 동작 변경 위험이 있다.
- 관련 파일:
  - `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
  - `Source/BeekeepingSim/Private/StorageBoxComponent.cpp`
  - `Source/BeekeepingSim/Private/ItemSlotWidget.cpp`
  - `Source/BeekeepingSim/Private/ItemSlotDragDropLibrary.cpp`
- 선택지:
  - 옵션 A: public API는 유지하고, 내부 private helper 또는 새 `Inventory` 내부 유틸로 중복 계산만 통합한다.
  - 옵션 B: `IItemContainer` 같은 새 컨테이너 추상화를 도입해 Hotbar/Storage 이동 API를 재설계한다.
  - 옵션 C: 이번 리팩토링에서는 폴더/문서 분리만 수행하고 로직 통합은 후속 작업으로 미룬다.
- 권장 옵션: 옵션 A. 동작 변경을 최소화하면서 중복을 줄일 수 있다.
- 사용자 답변: 옵션A

---

### [질문 6]

1. Content 미참조 public 타입 rename 허용 범위
- 질문 내용: Blueprint 참조가 확인된 `EStorageSlotContainerType` rename은 제외한다. 남은 문제는 Content 심볼 참조가 확인되지 않은 public 타입/파일의 이름을 더 정확한 이름으로 바꿀지 결정하는 것이다.
- 필요한 이유: Content 참조가 없어도 Unreal의 UCLASS/USTRUCT/UENUM 이름 변경은 C++ public API, UHT generated include, Core Redirect, 빌드 경로에 영향을 줄 수 있다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/StorageSlotDragDropOperation.h`
  - `Source/BeekeepingSim/Private/StorageSlotDragDropOperation.cpp`
  - `Source/BeekeepingSim/Public/StorageSlotDragDropTypes.h`
  - `Source/BeekeepingSim/Public/ItemSlotDragDropLibrary.h`
  - `Source/BeekeepingSim/Private/ItemSlotDragDropLibrary.cpp`
- 검토 대상:
  - `UStorageSlotDragDropOperation`
  - `EItemSlotDragMode`
  - `FItemSlotMoveResult`
  - `StorageSlotDragDropOperation.h/.cpp`
  - `StorageSlotDragDropTypes.h`
- 선택지:
  - 옵션 A: 이번 리팩토링에서는 클래스명/파일명 rename을 하지 않고 폴더 이동과 문서 정리만 수행한다.
  - 옵션 B: 더 정확한 이름으로 UCLASS/USTRUCT/UENUM까지 rename하고 Core Redirect 및 빌드/에셋 재검사 계획을 포함한다.
  - 옵션 C: 파일명만 rename하고 UCLASS/USTRUCT/UENUM 이름은 유지한다.
- 권장 옵션: 옵션 A. Blueprint 참조 위험은 낮아졌지만 public 타입 rename은 구조 이동보다 리스크가 크다.
- 사용자 답변: 옵션A

---

### [질문 7]

1. 분석 범위 밖 C++ include 수정 허용 여부
- 질문 내용: 파일을 시스템 하위 폴더로 이동하면 분석 범위 밖의 `Source/BeekeepingSim` 루트 또는 Variant 코드가 기존 헤더 경로를 include하고 있을 경우 빌드가 깨질 수 있다. 구현 단계에서 범위 밖 C++ include 수정까지 허용할지 결정이 필요하다.
- 필요한 이유: 이번 분석 제외 항목은 분석 대상 밖 코드를 제외하지만, Unreal 빌드는 같은 모듈 내 다른 코드의 include에도 영향을 받는다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/*`
  - `Source/BeekeepingSim/Private/*`
  - 분석 범위 밖이지만 영향 가능성이 있는 `Source/BeekeepingSim` 루트 및 Variant 코드
- 선택지:
  - 옵션 A: 구현 범위를 이번 분석 대상 파일로 제한하고, 범위 밖 include 문제는 수동 검토 항목으로만 남긴다.
  - 옵션 B: 구현 담당 에이전트에게 빌드 오류 해결에 필요한 범위 밖 include 수정만 제한적으로 허용한다.
  - 옵션 C: 리팩토링 설계 전에 범위 밖 C++까지 추가 분석 대상으로 확장한다.
- 권장 옵션: 옵션 B. 폴더 이동 후 빌드 성공을 위해 최소한의 include 수정 권한이 필요할 수 있다.
- 사용자 답변: 옵션B

---

### [질문 8]

1. 주석 처리된 레거시 코드와 템플릿 주석 삭제 여부
- 질문 내용: 현재 코드에는 템플릿 주석, 주석 처리된 이전 구현, 무관한 예시 주석이 남아 있다. 이를 삭제 후보로 확정할지 결정이 필요하다.
- 필요한 이유: 가독성 개선 대상이지만, 저작권/프로젝트 템플릿 주석 유지 정책이 있을 수 있다.
- 관련 파일:
  - `Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperCharacter.cpp`
  - `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
  - `Source/BeekeepingSim/Private/BeekeeperMovementComponent.cpp`
  - 여러 파일 상단의 `Fill out your copyright notice...` 주석
- 선택지:
  - 옵션 A: 동작과 무관한 주석 처리 코드/무관 예시 주석은 삭제 후보로 확정하되, 파일 상단 copyright 템플릿 주석은 유지한다.
  - 옵션 B: 주석 정리는 이번 리팩토링 범위에서 제외한다.
  - 옵션 C: 템플릿 copyright 주석까지 포함해 전부 정리한다.
- 권장 옵션: 옵션 A. 런타임 위험 없이 가독성을 개선하되, 프로젝트 정책 가능성이 있는 상단 주석은 보존한다.
- 사용자 답변: 옵션A
