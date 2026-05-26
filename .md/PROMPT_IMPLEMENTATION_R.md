# 구현 수정 프롬프트: Generic ItemPlacementSlotActor 리뷰 Findings

## 우선순위

1. Medium: `AItemPlacementSlotActor` occupied 판정을 `IsValid(PlacedActor)` 기준으로 변경
2. Low: `SlotMeshAsset`을 null로 되돌릴 때 stale mesh가 남지 않도록 기본 mesh 복구 정책 명시

## 발견 문제

### 1. occupied 상태가 raw pointer null 여부만 봐서 외부 destroy 후 슬롯이 계속 점유될 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
- 원인:
  - descriptor 생성, `TryPlaceItem`, `IsPlacementOccupied`가 `PlacedActor != nullptr`만 기준으로 사용한다.
  - placed actor가 외부에서 destroy되면 pointer가 즉시 nullptr이 되지 않을 수 있고, `IsValid(PlacedActor)`는 false지만 raw pointer는 남을 수 있다.
- 영향:
  - 배치 actor가 제거된 뒤에도 slot actor가 descriptor를 반환하지 않아 재배치가 불가능해질 수 있다.
  - `ClearPlacedItem` 호출 전까지 occupied state가 stuck될 수 있다.
- 수정 방향:
  - occupancy helper를 추가해 `IsValid(PlacedActor)` 기준으로 판단한다.
  - invalid pointer가 발견되면 `PlacedActor = nullptr`로 정리한다.
  - `GetItemUseAreaDescriptors`, `TryPlaceItem`, `IsPlacementOccupied`, `ClearPlacedItem` 모두 같은 helper를 사용한다.

### 2. `SlotMeshAsset`을 null로 되돌릴 때 이전 override mesh가 남을 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
- 원인:
  - `ApplySlotAuthoringSettings()`는 `SlotMeshAsset`이 유효할 때만 `SetStaticMesh(SlotMeshAsset)`를 호출한다.
  - 한 번 mesh override가 적용된 뒤 property를 null로 되돌리면 기존 component mesh를 복구하지 않는다.
- 영향:
  - "SlotMeshAsset 미지정 시 BP 기본 mesh 보존"은 초기 상태에서는 만족하지만, 에디터에서 override를 제거하는 경우 이전 mesh가 남아 slot shape가 기대와 달라질 수 있다.
- 수정 방향:
  - 생성 시 CDO/BP 기본 mesh를 저장해 null일 때 복구하거나, `SlotMeshAsset=null`은 현재 component mesh 유지라는 정책을 문서에 명시한다.
  - 인스턴스 authoring UX를 우선하면 복구 helper를 두는 쪽이 안전하다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg "PlacedActor" Source/BeekeepingSim/Public/WorldActors/ItemPlacementSlotActor.h Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp -n`
- PIE/Editor:
  - placed actor를 외부 destroy한 뒤 slot descriptor가 다시 나타나는지 확인
  - `SlotMeshAsset` 설정 후 null로 되돌렸을 때 BP 기본 mesh 복구 또는 문서화된 유지 정책이 작동하는지 확인

## 문서 반영 필요 여부

- 선택.
- `SlotMeshAsset=null`의 의미를 현재 component mesh 유지로 둘 경우 `.md/USER_UNREAL.md`에 명시한다.
