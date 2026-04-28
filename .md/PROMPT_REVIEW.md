# 코드 리뷰 요청 프롬프트 (1~6회차 통합)

아래 변경사항을 Unreal Engine 5.7 C++/UMG 기준으로 리뷰하라.

리뷰 목표:
- 기능 정확성
- 회귀 위험
- 객체 수명/소유권(Outer, GC) 안정성
- 입력 정책 충돌 여부
- 책임 분리(Widget vs Logic Component)

리뷰 스타일:
- 버그/리스크를 심각도 순으로 제시
- 각 이슈에 파일/함수 기준 근거 포함
- 필요한 경우 재현 시나리오와 수정 제안 포함

---

## 리뷰 대상 범위

### 핵심 코드

- `Source/BeekeepingSim/Public/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/StorageBoxFocusActionComponent.h`
- `Source/BeekeepingSim/Private/StorageBoxFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/BeekeeperController.h`
- `Source/BeekeepingSim/Private/BeekeeperController.cpp`
- `Source/BeekeepingSim/Private/BeekeeperCharacter.cpp`
- `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Public/ItemDefinition.h`
- `Source/BeekeepingSim/Public/ItemInstance.h`
- `Source/BeekeepingSim/Private/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/StorageSlotDragDropTypes.h`
- `Source/BeekeepingSim/Public/StorageSlotDragDropOperation.h`
- `Source/BeekeepingSim/Private/StorageSlotDragDropOperation.cpp`
- `Source/BeekeepingSim/Public/ItemSlotWidget.h`
- `Source/BeekeepingSim/Private/ItemSlotWidget.cpp`
- `Source/BeekeepingSim/Public/ItemVisualWidget.h`
- `Source/BeekeepingSim/Private/ItemVisualWidget.cpp`
- `Source/BeekeepingSim/Public/ItemDragVisualWidget.h`
- `Source/BeekeepingSim/Private/ItemDragVisualWidget.cpp`
- `Source/BeekeepingSim/Public/StorageBoxComponent.h`
- `Source/BeekeepingSim/Private/StorageBoxComponent.cpp`
- `Source/BeekeepingSim/Public/ItemSlotDragDropLibrary.h`
- `Source/BeekeepingSim/Private/ItemSlotDragDropLibrary.cpp`
- `Source/BeekeepingSim/BeekeepingSim.Build.cs`

### 문서

- `.md/0_ARCHITECTURE.md`
- `.md/PROMPT_IMPLEMENTATION.md`

---

## 회차별 기대 동작(검증 기준)

1) StorageFocus 입력 정책
- StorageFocus engaged 시 기존 hotbar 선택 해제
- engaged 중 숫자키/wheel hotbar 선택 차단
- 다른 focus action 정책과 불필요하게 결합되지 않을 것

2) Active Context + Wheel 라우팅
- controller 에 active storage / active drag op 등록·해제 일관성
- drag 중 wheel 입력이 hotbar 순환으로 누수되지 않을 것

3) Durability + ItemVisualWidget
- durability getter 안전성(0 division, null definition)
- ItemVisualWidget null-safe fallback
- ItemVisualWidget/ItemDragVisualWidget 이 표시 책임만 갖는지

4) DragDropOperation + ItemSlotWidget 기본 입력
- LMB FullStack / RMB PartialStack 초기 MoveQuantity=1
- source visual hide/restore 경로 누락 없는지

5) Partial Move API + Drop Router
- `FullStack` 기존 경로 회귀 없음
- `PartialStack`에서 swap 금지, merge/split 규칙 준수
- source 0 수량 시 slot clear
- split 생성 인스턴스 Outer 가 target container 소유인지
- merge overflow 시 빈 슬롯 확장 로직의 일관성

6) Double Click Quick Move
- LMB double click만 quick move 처리
- Hotbar -> ActiveStorage, Storage -> Hotbar 동작
- 대상 슬롯 선택 우선순위(merge 가능 슬롯 -> 빈 슬롯)
- active storage 없음 시 실패 처리

---

## 특히 봐야 할 리스크 포인트

- `UItemSlotWidget`:
  - `NativeOnMouseButtonDown`, `NativeOnMouseButtonDoubleClick`
  - drag delegate(`OnDrop`, `OnDragCancelled`) 중복/누락 처리
  - drag 성공/실패/취소 케이스별 source visual 복구 보장
- `UStorageBoxComponent` partial move 계열:
  - hotbar/storage 간 이동 시 Broadcast 호출 타이밍
  - 동일 프레임 다중 `SetSlotItem()` 호출 시 과도한 브로드캐스트
  - `ReevaluateSlots()` 호출 경로 일관성
- `UBeekeeperHotbarComponent::MovePartialToSlot()`:
  - target full + 빈 슬롯 fallback 로직의 정책 적합성
  - source/target 동일 index 방어
- `UItemSlotDragDropLibrary::HandleItemSlotDrop()`:
  - source/target component 동일성 검사 정확성
  - unsupported 조합 false 반환 일관성
- Build 설정:
  - `SlateCore` 추가가 링크 문제 해결 외 부작용 없는지

---

## 리뷰 결과 출력 형식 요청

1. Findings (High -> Medium -> Low)
2. Open Questions / Assumptions
3. Regression Test Checklist
4. Optional Refactor Suggestions (비필수, 동작 변경 없는 범위)
