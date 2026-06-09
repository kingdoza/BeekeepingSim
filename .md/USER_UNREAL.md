# Unreal Editor 수동 작업 목록 - 밀도 작업대 / 소비장 밀도질

이 문서는 C++ 구현 후 Unreal Editor에서 직접 설정, 컴파일, 저장, PIE 검증해야 하는 항목만 정리한다.

## 1. `BP_BeehiveComb` capping plane 설정

- `ABeehiveCombActor` 기반 소비장 Blueprint에서 아래 native component가 보이는지 확인한다.
  - `FrontWaxCappingPlane`
  - `BackWaxCappingPlane`
- 각 capping plane에 밀랍/capping mesh 또는 plane mesh를 지정한다.
- 각 capping plane에 밀랍/capping material을 지정한다.
- 각 capping plane의 relative transform을 `FrontHoneyPlane` / `BackHoneyPlane`과 맞춘다.
- full honey 미만 상태에서는 capping plane이 보이지 않아야 한다.
- full honey 상태에서는 완료되지 않은 face의 capping plane만 보여야 한다.

## 2. `BP_BeehiveComb` capping use-area mesh 설정

- `ABeehiveCombActor` 기반 소비장 Blueprint에서 아래 native component가 보이는지 확인한다.
  - `FrontWaxCappingUseAreaMesh`
  - `BackWaxCappingUseAreaMesh`
- 두 use-area mesh의 local X/Y가 capping 작업면의 `PlaneSize.X/Y`에 대응하도록 mesh와 relative transform을 조정한다.
- local origin은 작업면 중심에 둔다.
- component collision은 기본적으로 비활성/ignore 상태로 둔다.
- PIE에서 runtime active descriptor가 선택된 use-area에만 query collision을 켜는지 확인한다.
- `AreaTags`에는 `Item.UseArea.UncappingTable.Comb`가 포함되어야 한다.
- `EffectTargetPolicy`는 `ComponentOwner`로 유지한다.

## 3. capping material graph 연결

- `FrontWaxCappingPlane` / `BackWaxCappingPlane`에 사용하는 material에 texture parameter `WaxCappingMask`를 추가한다.
- `WaxCappingMask`를 alpha, opacity mask, opacity 또는 동등한 밀랍 visibility mask 입력으로 연결한다.
- mask 값 의미는 `255 = 밀랍 남음`, `0 = 밀랍 제거됨`이다.
- 제거된 영역 아래의 기존 honey plane이 보여야 한다.
- 기존 scalar parameter `HoneyRipeness` 연결은 유지한다.
- material이 masked/translucent 표현을 사용한다면 blend mode, opacity mask clip value, two-sided 여부를 실제 mesh에 맞게 조정한다.

## 4. 소비장 item / placement DataAsset 확인

- 소비장 item definition은 `MaxStack = 1`을 유지한다.
- 소비장 placement action은 기존 `UItemPlacementUseAction` 경로를 사용한다.
- placement action의 `PlacedActorClass`는 `ABeehiveCombActor` 기반 BP로 지정한다.
- placement action의 `UseAreaTagQuery`가 제한되어 있다면 empty 밀도 작업대 slot tag `Item.UseArea.UncappingTable`을 허용한다.
- query가 비어 있는 generic placement action이면 추가 tag 수정은 필요 없다.

## 5. 밀도 도구 DataAsset 설정

- 밀도 도구 item definition의 action spec에 `UCombUncappingUseAction`을 추가한다.
- `UCombUncappingUseAction`은 `Item.UseArea.UncappingTable.Comb` use-area tag를 대상으로 한다.
- 필요하면 아래 값을 item action spec에서 조정한다.
  - `BrushRadiusCm`
  - `MinStampInterval`
  - `MinStampDistanceCm`
- 이번 범위에서는 밀도 도구 active-use durability drain을 설정하지 않는다.

## 6. 밀도 작업대 Blueprint / 레벨 배치

- `AUncappingTable` 기반 Blueprint를 만들거나 native actor를 레벨에 배치한다.
- `TableMesh`에 작업대 mesh/material을 지정한다.
- `FocusAnchor` transform을 작업대 관찰 기준 위치에 맞춘다.
- `CharacterAnchor` transform을 플레이어 고정 위치에 맞춘다.
- `CombSlotRoot` / `CombSlotChildActor` transform을 소비장이 놓일 위치에 맞춘다.
- 필요하면 `CombSlotActorClass`를 `AUncappingTableCombSlot` subclass로 지정한다.
- `CombSlotChildActor`가 정상 생성되고 작업대 slot 위치에 놓이는지 확인한다.

## 7. 입력 설정 확인

- FocusConfirm 입력이 기존 focus 진입 키로 유지되어야 한다.
- PartFocus click 입력은 LMB로 동작해야 한다.
- Focus secondary 입력은 occupied comb 회수용으로 RMB에 연결되어 있어야 한다.
- item-use-area hold-use는 LMB press/release 경로로 동작해야 한다.

## 8. Compile / Save

- 아래 asset을 compile/save한다.
  - `BP_BeehiveComb`
  - capping material / material instance
  - 소비장 item definition
  - 밀도 도구 item definition
  - `AUncappingTable` 기반 Blueprint가 있다면 해당 BP
  - 테스트 레벨
- Editor 재시작 후 다시 열어 component, tag, material parameter, DataAsset 설정이 유지되는지 확인한다.

## 9. PIE 검증 체크리스트

- 밀도 작업대에 FocusConfirm으로 FocusEngaged 진입되는지 확인한다.
- FocusEngaged 진입 시 hotbar 선택이 비워지는지 확인한다.
- 소비장을 선택한 뒤 empty 작업대 slot에 LMB로 배치되는지 확인한다.
- occupied comb가 PartFocus secondary 입력으로 회수되는지 확인한다.
- 회수 후 hotbar item instance에 honey amount, ripeness, visible face, capping mask가 보존되는지 확인한다.
- 재배치하면 capping mask가 복원되는지 확인한다.
- 밀도 도구를 선택하면 현재 visible face의 capping use-area만 표시되는지 확인한다.
- LMB hold로 커서 중심 원형 영역의 밀랍만 제거되는지 확인한다.
- 제거된 영역 아래의 honey plane이 보이는지 확인한다.
- 이미 제거된 영역을 문질러도 추가 변화가 없는지 확인한다.
- horizontal drag flip으로 visible face가 바뀌는지 확인한다.
- face가 바뀐 뒤 새 visible face만 밀도질 가능한지 확인한다.
- full honey가 아닌 소비장은 밀도질 use-area가 active가 아닌지 확인한다.
- 한 face가 threshold 이하로 제거되면 해당 face capping plane과 use-area가 inactive 되는지 확인한다.
- 양면 완료 시 `IsWaxCappingComplete()` 또는 동등 API 결과가 true인지 확인한다.

## 10. 이번 범위에서 하지 않는 작업

- Content asset을 C++/Codex가 직접 수정하거나 저장하지 않는다.
- 채밀, 수확, 꿀 아이템 생산은 구현/검증 대상이 아니다.
- 밀도 도구 active-use durability drain은 구현/검증 대상이 아니다.
- Core Redirect 작업은 필요하지 않다.
