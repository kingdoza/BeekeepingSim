# 리뷰 프롬프트: Generic ItemPlacementSlotActor + Mesh/Transform 확장

## 리뷰 대상

이번 리뷰는 `AItemPlacementSlotActor` 기반 generic item placement 시스템과, 추가된 아래 속성까지 포함한다.

- `SlotMeshMaterial`
- `SlotMeshRelativeTransform`
- `AttachMeshRelativeTransform`

리뷰 범위에는 다음이 포함된다.

- `IItemPlacementSlot` 인터페이스
- `AItemPlacementSlotActor` 구현
- `UItemPlacementUseAction` / `UPollenPattyUseAction` 경로
- `UCursorItemUseAreaScopeComponent` stack delta 실패 rollback 경로
- `ABeehive`에서 pollen 직접 소유 로직 제거 상태
- 관련 아키텍처/사용자 문서 반영 상태

---

## 우선순위

1. High: SlotMesh/Attach transform 적용 시점과 attach 안정성
2. High: placement 성공 후 stack delta 실패 rollback의 정확성
3. Medium: use-area descriptor 활성/비활성 조건의 일관성
4. Medium: SlotMeshMaterial 적용 방식의 런타임/에디터 안정성
5. Low: 문서/에디터 작업 절차와 실제 코드 계약 정합성

---

## 검토 포인트

### 1) ItemPlacementSlotActor 구조/책임 검증

- `AItemPlacementSlotActor`가 아래 구조를 실제로 만족하는지 확인
  - `Root`
  - `SlotMeshComponent` (hit + visual 겸용)
  - `AttachComponent`
- `HitBox`, 별도 `HitComponent` UPROPERTY, 별도 `VisualComponents` UPROPERTY가 남아있지 않은지 확인
- `SlotMeshAsset` 미지정 시 기존 BP 기본 mesh를 보존하는지 확인
- `SlotMeshMaterial` 적용 시 material slot index/동적 머티리얼 충돌 가능성 점검

### 2) Transform 적용 정책 검증

- `SlotMeshRelativeTransform`이 `SlotMeshComponent`에 정확히 적용되는지
- `AttachMeshRelativeTransform`이 `AttachComponent`에 정확히 적용되는지
- 적용 시점이 `OnConstruction`/초기화 경로에서 일관적인지
- PIE 재시작, BP compile 후 transform 드리프트/중복 누적이 없는지
- ChildActor instance override 시 class default와 instance override 우선순위가 의도대로 동작하는지

### 3) Descriptor 생성 조건 검증

`GetItemUseAreaDescriptors_Implementation()`에서 아래 조건이 정확한지 확인

- `PlacedActor == nullptr`
- `AreaId` 유효
- `SlotMeshComponent` 유효
- `SlotMeshComponent->GetStaticMesh()` 유효

descriptor 값 검증

- `HitComponent = SlotMeshComponent`
- `VisualComponents.Add(SlotMeshComponent)`
- `EffectTargetObject = this`

### 4) Placement / Occupied / Clear 검증

- `TryPlaceItem()`:
  - occupied/invalid class/invalid attach/world null 실패 처리
  - spawn 후 attach 실패 시 destroy cleanup
  - 성공 시 `PlacedActor` 저장
- `IsPlacementOccupied()`:
  - `PlacedActor != nullptr` 기준 유지
- `ClearPlacedItem()`:
  - destroy + nullptr 복귀
- `AttachSocketName` 유효 시 socket attach 경로 정상 여부

### 5) Action 경로 검증

- `UItemPlacementUseAction::ApplyUseEffect()`가 `IItemPlacementSlot` 인터페이스만 통해 배치하는지
- 성공 시 `bSucceeded=true`, `bConsumedItem=true`, `StackDelta=-1`
- `UPollenPattyUseAction`이 남아 있어도 `ABeehive::TryInstallPollenPatty` 같은 제거된 API를 참조하지 않는지

### 6) Rollback 경로 검증

`UCursorItemUseAreaScopeComponent`에서

- placement 성공 + stack delta 적용 실패 시
  - `IItemPlacementSlot::ClearPlacedItem()` rollback 호출 여부
  - active descriptor refresh 여부
- stack delta 적용 성공 시 occupied slot descriptor가 즉시 사라지는지

### 7) Beehive 분리 검증

아래가 완전히 제거되었는지 확인

- `FPollenPattyInstallSlot`
- `PollenPattyInstallSlots`
- `PollenPattyActorClass` (Beehive 소유)
- `TryInstallPollenPatty`
- `IsPollenPattySlotOccupied`
- beehive 내부 pollen descriptor 생성 루프

그리고 소독약 descriptor(lid/comb + `Item.UseArea.Beehive.Disinfectant`)는 유지되는지 확인

---

## 코드 검색 체크리스트

- 없어야 함:
  - `FPollenPattyInstallSlot`
  - `TryInstallPollenPatty`
  - `IsPollenPattySlotOccupied`
  - `HitBox` (ItemPlacementSlotActor 내부)
  - ItemPlacementSlotActor의 `VisualComponents` UPROPERTY
- 있어야 함:
  - `SlotMeshComponent`
  - `SlotMeshAsset`
  - `SlotMeshMaterial`
  - `SlotMeshRelativeTransform`
  - `AttachMeshRelativeTransform`
  - descriptor에서 `HitComponent = SlotMeshComponent`
  - descriptor에서 `VisualComponents.Add(SlotMeshComponent)`

---

## 빌드/실행 검증

### 빌드
- `BeekeepingSimEditor Win64 Development`

### PIE 수동 시나리오
1. 슬롯 mesh에 use-area 표시가 정상 표시/hover 전환되는지
2. 슬롯별 `SlotMeshAsset` 차이에 따라 영역 모양이 달라지는지
3. `SlotMeshRelativeTransform` 조정이 즉시 반영되는지
4. `AttachMeshRelativeTransform` 조정 후 배치 actor 부착 위치가 의도대로인지
5. 배치 성공 시 stack 1 감소
6. 배치 직후 descriptor 비활성화(재사용 불가)
7. stack delta 실패를 유도했을 때 placed actor rollback
8. 소독약 기존 경로 정상 동작

---

## 리뷰 결과 출력 형식

- Findings를 **심각도 순(High → Medium → Low)** 으로 제시
- 각 항목에 반드시 포함:
  - 파일 경로
  - 원인
  - 영향
  - 수정 방향
- 마지막에
  - 회귀 위험
  - 추가 테스트 필요 항목
  - 문서 보강 필요 여부
  를 요약
