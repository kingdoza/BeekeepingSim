# 리뷰 프롬프트: ItemUseAreaMeshComponent 통합 전환

## 리뷰 목적

이번 리뷰는 ItemUseArea 등록 경로를 `UItemUseAreaMeshComponent` + `UItemUseAreaMeshProviderComponent`로 통합한 변경의 정합성과 회귀 위험을 검증한다.

중요: 워크트리에 여러 에이전트 변경이 섞여 있을 수 있으므로, **최종 코드 상태 기준**으로 판단한다.

---

## 반드시 읽을 문서

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/QNA_ARCHITECTURE.md` (ItemUseAreaMeshComponent 통합 설계 QnA)
- `.md/QNA_IMPLEMENTATION.md`

---

## 핵심 검증 질문

1. runtime descriptor source가 `UItemUseAreaMeshProviderComponent` 중심으로 일원화되었는가?
2. `UCursorItemUseAreaScopeComponent`가 구 provider actor/interface/tag fallback을 더 이상 사용하지 않는가?
3. `ABeehive` actor-level `GetItemUseAreaDescriptors_Implementation` 경로가 제거(또는 runtime 미사용)되었는가?
4. `ABeehiveCombActor::BeeBrushUseAreaMesh`가 `UItemUseAreaMeshComponent`로 전환되었고, **`CombMesh` 하위 부착**이 유지되는가?
5. `AItemPlacementSlotActor`가 `UItemUseAreaMeshComponent` + `IItemUseAreaActivationProvider` 정책으로 empty/occupied active를 올바르게 제공하는가?
6. inactive descriptor 등록 시 `AreaTags`를 비우는 정책이 실제 query 매칭/visual/collision 흐름과 충돌하지 않는가?
7. `EffectTargetObject`가 component `EffectTargetPolicy`에 따라 올바르게 전달되는가?

---

## 리뷰 범위 (우선 파일)

### Focus
- `Public/Focus/ItemUseAreaMeshComponent.h`
- `Private/Focus/ItemUseAreaMeshComponent.cpp`
- `Public/Focus/ItemUseAreaActivationProvider.h`
- `Public/Focus/ItemUseAreaMeshProviderComponent.h`
- `Private/Focus/ItemUseAreaMeshProviderComponent.cpp`
- `Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Private/Focus/CursorItemUseAreaScopeComponent.cpp`

### WorldActors
- `Public/WorldActors/Beehive.h`
- `Private/WorldActors/Beehive.cpp`
- `Public/WorldActors/BeehiveCombActor.h`
- `Private/WorldActors/BeehiveCombActor.cpp`
- `Public/WorldActors/ItemPlacementSlotActor.h`
- `Private/WorldActors/ItemPlacementSlotActor.cpp`

### 관련 레거시(참조만)
- `Public/Focus/ItemUseAreaProvider.h`
- `Public/Focus/ChildItemUseAreaProviderComponent.h`
- `Private/Focus/ChildItemUseAreaProviderComponent.cpp`

---

## 상세 체크리스트

### 1) Scope 수집 경로
- `RebuildItemUseAreaDescriptors()`가 `RebuildDescriptorsFromItemUseAreaMeshProviders()`만 사용해 등록하는지
- `RegisterItemUseAreaDescriptor` 유효성 조건과 충돌 없는지
- 기존 helper/경로(`RebuildDescriptorsFromProviderActor`, `...ProviderComponents`, `...DirectComponentTags`) 호출이 runtime에 남아있지 않은지

### 2) Provider component 동작
- owner component + direct child actor 순회 규칙 준수 여부
- `RequiredChildActorComponentTag` 필터 정확성
- `IItemUseAreaActivationProvider` 구현 actor의 false 시 `AreaTags` empty 처리 확인
- `HitComponent`, `VisualComponents`, `VisualSettings`, `EffectTargetObject` 채움 정확성

### 3) Beehive/Comb 통합
- `ABeehive`가 `IItemUseAreaProvider`를 더 이상 상속하지 않는지
- `ItemUseAreaMeshProvider` subobject 추가 여부
- `GetLiftedCombActor()` 구현이 lift 상태 source-of-truth와 일치하는지
- `ABeehiveCombActor`:
  - `BeeBrushUseAreaMesh` 타입 전환
  - `BeeBrushUseAreaMesh->SetupAttachment(CombMesh)` 유지
  - `IsItemUseAreaMeshActive_Implementation`에서 lifted comb일 때만 true

### 4) Placement slot 통합
- `SlotMeshComponent` 타입 전환 여부
- `IItemUseAreaProvider` 제거 및 `IItemUseAreaActivationProvider` 구현 여부
- active 정책: `SlotMeshComponent`는 empty일 때 true, occupied일 때 false
- place/clear 후 host `RebuildItemUseAreaDescriptors` 재호출 여부
- deprecated bridge(`AreaId`, `AreaTags`)가 BP 호환성에 미치는 영향

### 5) 회귀/충돌 포인트
- item-use-area collision 제어가 PartFocus hit과 충돌하지 않는지
- inactive descriptor 유지가 hover 판정/visual 갱신/효과 적용 경로에 부작용 없는지
- `EffectTargetPolicy=ComponentOwner/HostActor/ExplicitObject` fallback이 안전한지

---

## 코드 검색 기준

### 있어야 함
- `UItemUseAreaMeshComponent`
- `EItemUseAreaEffectTargetPolicy`
- `UItemUseAreaMeshProviderComponent`
- `IItemUseAreaActivationProvider`
- `BuildItemUseAreaDescriptors`
- `IsItemUseAreaMeshActive`
- `RebuildDescriptorsFromItemUseAreaMeshProviders`
- `GetLiftedCombActor`

### runtime 경로에서 없어야 함
- `IItemUseAreaProvider::Execute_GetItemUseAreaDescriptors` (scope runtime 경로)
- `RebuildDescriptorsFromProviderActor`
- `RebuildDescriptorsFromProviderComponents`
- `RebuildDescriptorsFromDirectComponentTags`
- `ComponentHasTag(TEXT("ItemUseArea"))` fallback

---

## 검증 방법

1. 코드 리뷰 + 검색 근거 제시
2. UBT 빌드 확인
   - `BeekeepingSimEditor Win64 Development`
3. 가능하면 PIE 시나리오 논리 검증
   - BeeBrush 미선택/comb 미-lift: area inactive
   - comb lift + BeeBrush 선택: lifted comb만 active
   - slot empty/occupied 전환 시 area active 상태 전환

---

## 리뷰 결과 출력 형식

- Findings를 `High -> Medium -> Low` 순으로 제시
- 각 Finding에 포함:
  - 파일/라인
  - 원인
  - 영향
  - 수정 제안
- Findings 이후:
  - 가정/불확실성(멀티 에이전트 변경으로 인한 추정 포함)
  - 테스트 공백
  - 문서 동기화 누락 여부
