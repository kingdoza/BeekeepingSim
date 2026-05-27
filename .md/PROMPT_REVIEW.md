# 리뷰 프롬프트: Generic Placement Occupant + Beehive Comb Slot 전환

## 리뷰 목적

이번 리뷰는 다음 변경의 정합성과 회귀 위험을 검증한다.

- generic 배치 점유/회수 구조(`UPlacementOccupantComponent`, `UPlacementSlotRetrievePartFocusActionComponent`)
- `AItemPlacementSlotActor`의 preplaced occupant claim + generic occupied actor 처리
- `APlacedItemActor` migration(기존 API wrapper 유지)
- 벌통 소비장 slot 구조 전환(`ABeehiveCombSlotActor` + `ABeehive` active comb 조회 변경)
- 소비장 회수 조건(`TargetBeeCount`, queen attach) 및 회수 상태 보존(꿀양/visible face)

중요: 워크트리에 다른 변경이 섞여 있을 수 있으므로, 반드시 **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md` (벌통 소비장 슬롯/Generic Placement Occupant 섹션)
- `.md/QNA_IMPLEMENTATION.md`

---

## 핵심 검증 질문

1. generic occupant/retrieve 계약이 component 중심으로 일원화되었는가?
2. `AItemPlacementSlotActor`가 spawn 경로와 preplaced claim 경로 모두에서 occupant ownership을 올바르게 주입하는가?
3. `ClearPlacedItem`에서 occupant pre-clear hook 호출 후 destroy 순서가 보장되는가?
4. `APlacedItemActor`의 기존 getter/초기화 API 호환을 유지하면서 내부는 새 component 경로를 사용하는가?
5. `ABeehive`가 comb child actor 직접 참조 대신 comb slot의 placed comb를 active comb로 사용하도록 전환되었는가?
6. 소비장 회수 차단 조건(`TargetBeeCount > 0`, queen attached)이 실제 회수 경로에서 강제되는가?
7. 소비장 회수 시 꿀양/visible face가 `UItemInstance` state에 저장되고 재배치 시 복원되는가?

---

## 리뷰 범위 (우선 파일)

### WorldActors
- `Public/WorldActors/PlacementOccupantComponent.h`
- `Private/WorldActors/PlacementOccupantComponent.cpp`
- `Public/WorldActors/PlacementSlotRetrievePartFocusActionComponent.h`
- `Private/WorldActors/PlacementSlotRetrievePartFocusActionComponent.cpp`
- `Public/WorldActors/ItemPlacementSlotActor.h`
- `Private/WorldActors/ItemPlacementSlotActor.cpp`
- `Public/WorldActors/PlacedItemActor.h`
- `Private/WorldActors/PlacedItemActor.cpp`
- `Public/WorldActors/PlacedItemRetrievePartFocusActionComponent.h`
- `Private/WorldActors/PlacedItemRetrievePartFocusActionComponent.cpp`
- `Private/WorldActors/PlacedItemRetrieveFocusActionComponent.cpp`
- `Public/WorldActors/BeehiveCombSlotActor.h`
- `Private/WorldActors/BeehiveCombSlotActor.cpp`
- `Public/WorldActors/BeehiveCombPlacementOccupantComponent.h`
- `Private/WorldActors/BeehiveCombPlacementOccupantComponent.cpp`
- `Public/WorldActors/Beehive.h`
- `Private/WorldActors/Beehive.cpp`
- `Public/WorldActors/BeehiveCombActor.h`
- `Private/WorldActors/BeehiveCombActor.cpp`
- `Public/WorldActors/BeehiveCombPartFocusActionComponent.h`
- `Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`

### Inventory
- `Public/Inventory/ItemInstance.h`
- `Private/Inventory/ItemInstance.cpp`
- `Public/Inventory/BeekeeperHotbarComponent.h`
- `Private/Inventory/BeekeeperHotbarComponent.cpp`

---

## 상세 체크리스트

### 1) Generic Occupant/Retrieve
- `GetReturnItemDefinition` 우선순위가 runtime -> authored fallback인지
- `CanRetrievePlacementOccupant` 기본 조건(반환 definition + owning slot) 검증
- 회수 성공 판정이 `TryAcquireItem`의 `bSuccess && AddedQuantity == 1`인지
- 회수 실패 시 actor/slot 상태가 유지되는지

### 2) Placement Slot Generic 확장
- `InitialOccupantActor` claim이 BeginPlay에서 수행되는지(OnConstruction mutation 회피)
- claim 대상에 occupant component 없으면 실패/로그 처리되는지
- `bAttachInitialOccupantToSlot`, `bSnapInitialOccupantToAttachPoint` 정책 반영 여부
- occupied/empty 전환 시 part focus + item-use-area rebuild 호출 여부

### 3) PlacedItem migration 호환성
- `InitializePlacedItem`, `GetItemDefinition`, `GetOwningPlacementSlotActor`, `GetPartFocusActionComponent`이 BP 호환 경로를 유지하는지
- `UPlacedItemRetrievePartFocusActionComponent`가 새 generic retrieve 기반 wrapper인지
- legacy `UPlacedItemRetrieveFocusActionComponent`가 깨지지 않고 generic 경유로 동작하는지

### 4) Beehive Comb Slot 전환
- `ABeehive`의 slot child actor class가 `CombSlotActorClass` 기반인지
- `FindManagedCombSlotIndex`, `GetLiftedCombActor`, honey/colony/queen 관련 순회가 slot placed comb 기준인지
- empty slot skip 정책이 각 계산/등록 경로에서 일관적인지
- `CurrentCombCount`가 legacy/test 용도로만 유지되고 실제 active comb 수와 혼동되지 않는지

### 5) Comb Retrieve 조건 + 상태 보존
- comb 회수 조건:
  - `TargetBeeCount == 0`
  - queen 미부착
- 회수 성공 후 item instance state 기록:
  - 꿀양(`CurrentHoney`)
  - visible face(front/back)
- 재배치 시 item state 적용:
  - `SetCurrentHoney`
  - `SetVisibleCombFace`

### 6) Focus 입력 정책 보존
- comb LMB lift/return/drag 기존 정책 유지 여부
- secondary retrieve가 comb action handler 하나에서 bridge되는지(Descriptor action 1개 제약 준수)
- comb slot이 occupied descriptor를 중복 등록하지 않는지

---

## 코드 검색 기준

### 있어야 함
- `UPlacementOccupantComponent`
- `UPlacementSlotRetrievePartFocusActionComponent`
- `ABeehiveCombSlotActor`
- `InitialOccupantActor`
- `AuthoredReturnItemDefinition`
- `RuntimeReturnItemDefinition`
- `PreClearPlacementOccupant`
- `CanRetrievePlacementOccupant`
- `SetBeehiveCombState`
- `GetBeehiveCombState`

### 줄어들어야 함 / 축소되어야 함
- `APlacedItemActor` 하드캐스트 중심 retrieve 로직
- placed-item 전용 컴포넌트에만 회수 규칙이 묶이는 구조
- 벌통에서 comb child actor 직접 참조를 active comb source로 쓰는 경로

---

## 검증 방법

1. 코드 리뷰 + 검색 근거 제시
2. UBT 빌드 확인
   - `BeekeepingSimEditor Win64 Development`
3. PIE 논리 검증(가능 시)
   - generic slot 배치/회수 성공/실패 케이스
   - preplaced claim + authored fallback 회수
   - comb 회수 차단 조건
   - 회수 후 재배치 상태 복원

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순서로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- Findings 이후:
  - 가정/불확실성
  - 테스트 공백
  - 문서 동기화 누락 여부
