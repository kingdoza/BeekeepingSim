# Unreal Editor 수동 작업 목록 - 꿀 용기 / 꿀 소분 작업대

이 문서는 C++ 구현 후 Unreal Editor에서 직접 설정, compile/save, PIE 검증해야 하는 항목만 정리한다.

`Content/` asset은 Codex가 수정하지 않는다. 아래 작업은 Editor에서 수동으로 수행한다.

## 1. Gameplay Tag 준비

프로젝트의 기존 Gameplay Tag authoring 방식에 맞춰 꿀 용기/소분 작업대용 tag를 추가하거나 확인한다.

권장 tag:

- `Item.HoneyContainer`
- `Item.HoneyContainer.Source`
- `Item.HoneyContainer.Target`
- `Item.UseArea.HoneyTransfer.SourceSlot`
- `Item.UseArea.HoneyTransfer.TargetSlot`

주의:

- source/target 구분은 C++ enum 이름이 아니라 item definition gameplay tag와 slot `AcceptedItemTagQuery`로 authoring한다.
- tag asset/config 수정이 필요하면 Editor에서 수정 후 저장한다.

## 2. 꿀 용기 Item Definition 설정

source 용기와 target 용기 item definition을 `UHoneyContainerItemDefinition` 기반 DataAsset으로 만들거나 기존 DataAsset parent를 전환한다.

각 DataAsset에서 확인한다.

- `MaxStack = 1`
- `MaxVolumeMl >= 0`
- `DefaultCurrentVolumeMl`은 `0..MaxVolumeMl`
- `DefaultHoneyDensity`는 `0..1`
- `DefaultHoneyRipeness`는 `0..1`
- `DefaultHoneyDensity < 1.0`이면 runtime에서 `DefaultHoneyRipeness`는 `0.0`으로 정규화된다.
- `GameplayTags`에 꿀 용기 공통 tag와 source/target 구분 tag를 넣는다.

예:

- source 말통 item: `Item.HoneyContainer`, `Item.HoneyContainer.Source`
- target 꿀통 item: `Item.HoneyContainer`, `Item.HoneyContainer.Target`

## 3. 꿀 용기 Placement Action 설정

꿀 용기 item의 placement action을 기존 `UItemPlacementUseAction` 경로로 설정한다.

확인 항목:

- action spec에 placement action이 있다.
- `PlacedActorClass`가 `AHoneyContainerActor` 기반 BP로 지정되어 있다.
- placement action의 `UseAreaTagQuery`가 source/target slot tag를 허용한다.
- generic placement action query가 비어 있는 정책이면 별도 query 수정은 필요 없지만, 의도치 않은 다른 slot 배치가 가능한지 확인한다.

## 4. `BP_HoneyContainerActor` 설정

`AHoneyContainerActor` 기반 Blueprint를 만든다.

native component 확인:

- `ContainerMesh`
- `HoneyVisualMesh`
- `NozzleHitComponent`
- `NozzleOrigin`
- `PourTarget`
- `HoneyStreamNiagara`
- `PlacementOccupant`
- `RetrieveAction`
- `NozzleAction`

설정:

- `ContainerMesh`에 용기 mesh/material을 지정한다.
- `HoneyVisualMesh`에 내부 꿀 표시 mesh/material을 지정한다.
- `HoneyVisualMesh`의 authored relative scale은 full 상태 기준으로 맞춘다.
- `NozzleHitComponent`에 nozzle hover/click용 mesh 또는 collision mesh를 지정한다.
- `NozzleHitComponent`는 visibility trace에 block되도록 collision을 확인한다.
- `NozzleOrigin`을 nozzle hover/click 기준 위치에 둔다.
- `HoneyStreamNiagara`에 꿀 줄기 Niagara System을 지정하고, 실제 꿀 줄기 시작 위치에 둔다.
- `PourTarget`을 target으로 쓰일 때 꿀이 떨어질 위치에 둔다.
- DropLength 목표 길이는 source `HoneyStreamNiagara` world Z와 target `PourTarget` world Z 차이로 계산된다.
- honey material에 scalar parameter를 만든다.
  - `HoneyDensity`
  - `HoneyRipeness`

Compile/Save:

- `BP_HoneyContainerActor`
- honey material/material instance

## 5. Source / Target Slot Blueprint 설정

필요하면 `AHoneyContainerSlotActor` 기반 BP를 source/target용으로 만든다.

slot 공통 확인:

- `SlotMeshComponent`에 placement use-area mesh/material을 지정한다.
- `AttachComponent` transform을 용기 배치 위치에 맞춘다.
- slot mesh collision이 item-use-area trace에 잡히는지 확인한다.

source slot:

- `SlotRole = Source`
- `AcceptedItemTagQuery`가 source 꿀 용기 tag를 만족하도록 설정한다.
- `SlotMeshComponent.AreaTags`에 source slot use-area tag를 넣는다.

target slot:

- `SlotRole = Target`
- `AcceptedItemTagQuery`가 target 꿀 용기 tag를 만족하도록 설정한다.
- `SlotMeshComponent.AreaTags`에 target slot use-area tag를 넣는다.

Compile/Save:

- source slot BP
- target slot BP

## 6. `BP_HoneyDecantingTable` 설정

`AHoneyDecantingTable` 기반 Blueprint를 만들거나 native actor를 레벨에 배치한다.

native component 확인:

- `TableMesh`
- `FocusAnchor`
- `CharacterAnchor`
- `FocusTarget`
- `FocusAction`
- `CursorPartFocusScope`
- `CursorPartFocusRegistration`
- `ChildCursorPartFocusProvider`
- `ItemUseAreaScope`
- `ItemUseAreaMeshProvider`
- `SourceSlotRoot`
- `SourceSlotChildActor`
- `TargetSlotRoot`
- `TargetSlotChildActor`
- `HoneyTransferComponent`
- `HoneyStreamNiagara` (legacy fallback component; 신규 authoring은 source container `HoneyStreamNiagara`를 사용)

설정:

- `TableMesh`에 작업대 mesh/material을 지정한다.
- `FocusAnchor`를 작업대 관찰 기준 위치에 맞춘다.
- `CharacterAnchor`를 플레이어 고정 위치에 맞춘다.
- `SourceSlotRoot` / `TargetSlotRoot` transform을 용기 배치 위치에 맞춘다.
- `SourceSlotActorClass`를 source slot BP 또는 `AHoneyContainerSlotActor` subclass로 지정한다.
- `TargetSlotActorClass`를 target slot BP 또는 `AHoneyContainerSlotActor` subclass로 지정한다.
- 신규 소분 작업에서는 작업대 `HoneyStreamNiagara`가 아니라 source container `HoneyStreamNiagara`에 Niagara System/transform을 authoring한다.
- `HoneyTransferComponent` 값을 gameplay 의도에 맞게 조정한다.
  - `TransferRateMlPerSecond`
  - `DropLengthGrowSpeedCmPerSecond`
  - `DefaultDropLengthCm`

Niagara parameter:

- `User.HoneyDensity` float
- `User.HoneyRipeness` float
- `User.DropLength` float

Compile/Save:

- `BP_HoneyDecantingTable`
- Niagara System
- 테스트 레벨

## 7. 입력 / Focus 확인

기존 입력 정책을 확인한다.

- FocusConfirm 입력으로 `BP_HoneyDecantingTable`에 FocusEngaged 진입해야 한다.
- FocusEngaged 진입 시 hotbar 선택이 비워져야 한다.
- item-use-area placement는 LMB press/release 경로로 동작해야 한다.
- PartFocus primary click은 LMB로 동작해야 한다.
- PartFocus secondary retrieve는 RMB 또는 프로젝트 기존 secondary input으로 동작해야 한다.

## 8. Compile / Save 대상

작업 후 아래 asset을 compile/save한다.

- source 꿀 용기 item definition
- target 꿀 용기 item definition
- 꿀 용기 placement action이 포함된 item/action DataAsset
- `BP_HoneyContainerActor`
- honey visual material/material instance
- source slot BP
- target slot BP
- `BP_HoneyDecantingTable`
- Honey stream Niagara System
- 테스트 레벨

Editor 재시작 후 다시 열어 component, tag, material parameter, Niagara parameter, DataAsset 설정이 유지되는지 확인한다.

## 9. PIE 검증 체크리스트

1. `BP_HoneyDecantingTable`에 FocusConfirm으로 FocusEngaged 진입되는지 확인한다.
2. FocusEngaged 진입 시 hotbar 선택이 비워지는지 확인한다.
3. source slot에는 source tag를 가진 꿀 용기 item만 배치되는지 확인한다.
4. target slot에는 target tag를 가진 꿀 용기 item만 배치되는지 확인한다.
5. 잘못된 용기 tag로 slot 배치가 실패하고 item stack이 소비되지 않는지 확인한다.
6. source container nozzle hover/click prompt가 표시되는지 확인한다.
7. nozzle click으로 배출 시작 시 Niagara가 활성화되는지 확인한다.
8. 배출 시작 시 source container `HoneyStreamNiagara`에 `HoneyDensity`, `HoneyRipeness`, `DropLength=0`이 적용되는지 확인한다.
9. `DropLength`가 source `HoneyStreamNiagara` Z와 target `PourTarget` Z 차이까지 증가하는 동안 실제 volume이 이동하지 않는지 확인한다.
10. 목적값 도달 후 volume이 `TransferRateMlPerSecond`에 맞춰 이동하는지 확인한다.
11. source volume은 감소하지만 source density/ripeness가 유지되는지 확인한다.
12. target에 기존 꿀이 없으면 source density/ripeness가 target에 적용되는지 확인한다.
13. target에 기존 꿀이 있으면 volume-weighted average가 적용되는지 확인한다.
14. 유입 꿀 또는 혼합 결과의 density가 1.0 미만이면 target ripeness가 0.0으로 정규화되는지 확인한다.
15. source empty에서 자동 정지하고 Niagara가 즉시 사라지는지 확인한다.
16. target full에서 자동 정지하고 Niagara가 즉시 사라지는지 확인한다.
17. 배출 중 source 또는 target container를 회수/clear하면 자동 정지하는지 확인한다.
18. nozzle을 다시 클릭하면 진행 중 transfer가 정지되는지 확인한다.
19. 배치된 꿀 용기를 회수하면 hotbar item instance에 volume/density/ripeness가 보존되는지 확인한다.
20. 회수 시 hotbar acquire 실패 상황에서는 actor/slot 상태가 유지되는지 확인한다.
21. hotbar/storage 이동 후 꿀 용기 state가 보존되는지 확인한다.
22. 다른 작업대 actor에 `UHoneyTransferComponent`와 source/target slot을 붙여도 같은 이송 규칙을 재사용할 수 있는지 구조적으로 확인한다.

## 10. 이번 범위에서 하지 않는 작업

- Codex/C++가 `Content/` asset을 직접 수정하거나 저장하지 않는다.
- Core Redirect를 추가하지 않는다.
- `Config/DefaultEngine.ini`를 수정하지 않는다.
- `APlacedItemActor`에 꿀 용기 전용 기능을 추가하지 않는다.
- `AHoneyDecantingTable` Blueprint에서 transfer 계산을 별도로 구현하지 않는다.
- `HoneyRipness` 오탈자 parameter를 만들지 않는다. 정본은 `HoneyRipeness`다.
