# Unreal Editor 프롬프트: WBP_StorageBox 구현

## 목표

`UStorageBoxWidget` 기반의 `WBP_StorageBox` 를 구현한다.  
Storage Box Focus 진입 시 표시되는 UI이며, storage 슬롯과 hotbar 슬롯 간 아이템 이동/교환을 drag/drop으로 처리한다.

## 전제

- C++ 클래스가 이미 존재한다.
  - `UStorageBoxWidget`
  - `UStorageBoxComponent`
  - `UBeekeeperHotbarComponent`
- `UStorageBoxFocusActionComponent::StorageWidgetClass` 에 `WBP_StorageBox` 를 지정해 사용할 예정이다.

## 구현 범위

1. `WBP_StorageBox` 생성
- Parent Class: `UStorageBoxWidget`
- 화면 중앙 또는 하단 중심에 storage/hotbar 패널을 함께 배치
- 최소 구성:
  - Storage 슬롯 그리드 (예: 4x4)
  - Hotbar 슬롯 라인 (8칸)
  - 닫기 힌트 텍스트(선택)

2. 슬롯 위젯 구성
- 재사용 가능한 슬롯 위젯 BP를 만들어 storage/hotbar 공용으로 사용
- 슬롯은 아래 상태를 시각적으로 표현:
  - Empty
  - Occupied (아이콘, 스택 수량)
  - Hovered / Drag Over
  - Disabled(필요 시)

3. 초기화 로직
- `Event OnStorageWidgetInitialized` 에서:
  - `GetStorageComponent()` / `GetHotbarComponent()` 유효성 검사
  - Storage 슬롯 수와 Hotbar 슬롯 수를 읽어 UI 생성/바인딩
  - `OnStorageChanged`, `OnHotbarChanged` delegate 구독
  - 최초 `RefreshAllSlots()` 호출

4. 데이터 갱신 함수
- `RefreshAllSlots()` 구현:
  - Storage: `GetStorageComponent()->GetSlots()`
  - Hotbar: `GetHotbarComponent()->GetSlots()`
  - 각 슬롯에 아이콘/수량/빈 슬롯 상태 반영
- delegate 수신 시 동일 함수 재호출

5. Drag/Drop 동작
- 슬롯 Drag 시작 시 payload에 출처 타입/인덱스 저장
  - 출처 타입: Storage 또는 Hotbar
  - 출처 인덱스: int32
- Drop 시 목적지 타입/인덱스에 따라 아래 C++ API 호출:
  - Hotbar -> Storage: `MoveHotbarItemToStorage(HotbarIndex, StorageIndex)`
  - Storage -> Hotbar: `MoveStorageItemToHotbar(StorageIndex, HotbarIndex)`
  - Storage -> Storage: `SwapStorageSlots(FromStorageIndex, ToStorageIndex)`
  - Hotbar -> Hotbar는 이번 범위에서 처리하지 않거나 기본 hotbar 로직에 위임
  - Hotbar <-> Storage 직접 교환이 필요하면 `SwapHotbarAndStorage(HotbarIndex, StorageIndex)` 호출

6. UX 요구사항
- 드래그 중 대상 슬롯 하이라이트
- Drop 실패 시 원위치 유지(시각 피드백)
- 커서 모드 UI에서 텍스트/아이콘 겹침 없이 표시
- `Esc` 또는 FocusCancel 입력으로 닫히는 흐름과 충돌하지 않게, 위젯에서 강제 InputMode 변경 금지

## 설정 요구사항

1. `BP_StorageBox`에서 설정:
- `StorageBoxFocusActionComponent.StorageWidgetClass = WBP_StorageBox`

2. FocusItemRule 설정:
- `FocusTargetComponent.FocusItemRule.AllowedItemTags` 에 전체 아이템 허용 루트 태그 추가
- `UBeekeeperHotbarComponent.AllItemsRootTag` 와 동일 루트 태그 사용

## 출력 요구사항

- 생성/수정한 Blueprint 목록
- 구현한 함수 목록 (`OnStorageWidgetInitialized`, `RefreshAllSlots`, DragStart/Drop 핸들러 등)
- 테스트 체크리스트 결과:
  - Storage -> Hotbar 이동
  - Hotbar -> Storage 이동
  - Storage <-> Storage 교환
  - FocusCancel 시 UI 닫힘/커서 복구
