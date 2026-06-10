# 꿀 용기와 재사용 가능한 꿀 이송 작업대 구현 프롬프트

## 목표

꿀 용기 런타임 상태, 꿀 용기 배치 actor, 재사용 가능한 꿀 이송 컴포넌트, 소분 작업대 native actor를 C++로 구현한다.

이번 구현은 말통->꿀통 소분 작업대가 1차 사용처지만, 같은 꿀 용기 간 이송 기능을 다른 작업대에서도 재사용할 수 있어야 한다. QnA 답변끼리 충돌하는 경우에는 소분 작업대 전용 구현보다 재사용성이 높은 방향을 우선한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

## 핵심 설계 확정 사항

- 꿀 용기 상태는 `UItemInstance` base에 optional `FHoneyContainerItemState`로 저장한다.
- `FHoneyContainerItemState`는 최소 `bHasState`, `CurrentVolumeMl`, `HoneyDensity`, `HoneyRipeness`를 가진다.
- 전체 용량은 `UHoneyContainerItemDefinition::MaxVolumeMl`이 source of truth다.
- 꿀 용기 item은 `MaxStack=1`을 invariant로 둔다.
- 비-용기 item은 `bHasState=false`인 honey container state를 무시한다.
- 꿀 용기 월드 actor는 `AHoneyContainerActor` 전용 class로 구현한다.
- 꿀 용기 슬롯은 `AHoneyContainerSlotActor` 전용 class로 구현한다.
- 슬롯 role enum은 말통/꿀통 고정명이 아니라 재사용 가능한 `Source`, `Target`으로 둔다.
- 어떤 용기 조합을 받을지는 slot의 accepted gameplay tag query/authoring 값으로 결정한다.
- 실제 꿀 이송 규칙과 진행 상태는 `UHoneyTransferComponent`가 소유한다.
- 작업대 actor는 source slot, target slot, VFX anchor/Niagara를 조립하는 host 역할을 한다.
- 노즐 클릭 대상과 action owner는 `AHoneyContainerActor`다.
- 노즐 action은 concrete 작업대 class가 아니라 owning slot/host의 transfer component/interface를 찾아 toggle 요청한다.
- 회수 시 꿀 용기 state write-back은 꿀 용기 전용 retrieve action에서 보장한다.

## 구현 대상

### Inventory

새 파일:

- `Source/BeekeepingSim/Public/Inventory/HoneyContainerItemDefinition.h`

수정:

- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- hotbar/storage/item stack 생성 경로 중 runtime state copy가 필요한 곳

권장 추가 타입/API:

```cpp
USTRUCT(BlueprintType)
struct FHoneyContainerItemState
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container")
    bool bHasState = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container", meta = (ClampMin = "0.0"))
    float CurrentVolumeMl = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HoneyDensity = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item|Honey Container", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float HoneyRipeness = 0.0f;
};
```

`UItemInstance`에 권장 API:

- `SetHoneyContainerState(float CurrentVolumeMl, float HoneyDensity, float HoneyRipeness)`
- `ClearHoneyContainerState()`
- `HasHoneyContainerState() const`
- `GetHoneyContainerState() const`
- `CopyRuntimeStateFrom(const UItemInstance* SourceItemInstance)` 또는 동등한 helper

`SetHoneyContainerState`는 아래 invariant를 강제한다.

- `HoneyDensity < 1.0f`이면 `HoneyRipeness = 0.0f`
- 최종 상태는 `HoneyDensity < 1.0 && HoneyRipeness == 0.0` 또는 `HoneyDensity == 1.0 && HoneyRipeness >= 0.0`

`UHoneyContainerItemDefinition : UItemDefinition`:

- `MaxVolumeMl`
- `DefaultCurrentVolumeMl`
- `DefaultHoneyDensity`
- `DefaultHoneyRipeness`

기본값 sanitize:

- `MaxVolumeMl >= 0`
- `DefaultCurrentVolumeMl`은 `0..MaxVolumeMl`
- density/ripeness는 `0..1`
- default density가 `1.0` 미만이면 default ripeness는 runtime 적용 시 `0.0`

### WorldActors

새 파일:

- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerActor.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerSlotActor.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerSlotActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyTransferComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyTransferComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyNozzlePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyNozzlePartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyContainerRetrievePartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyContainerRetrievePartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/HoneyDecantingTable.h`
- `Source/BeekeepingSim/Private/WorldActors/HoneyDecantingTable.cpp`

구현자가 기존 패턴과 파일 배치를 보고 더 적절한 이름을 선택할 수 있으나, 다음 책임 분리는 유지한다.

## `AHoneyContainerActor`

역할:

- 배치된 꿀 용기 1개를 대표한다.
- 꿀 용기의 runtime state와 visual update를 소유한다.
- 노즐 hit/action을 소유한다.
- 회수 action을 소유한다.

권장 구성:

- `Root`
- `ContainerMesh`
- `HoneyVisualMesh`
- `NozzleHitComponent`
- `NozzleOrigin`
- `PourTarget`
- `UNiagaraComponent HoneyStreamNiagara`
- `UPlacementOccupantComponent`
- `UHoneyContainerRetrievePartFocusActionComponent`
- `UHoneyNozzlePartFocusActionComponent`

권장 API:

- `ApplyStateFromItemInstance(const UItemInstance* SourceItemInstance)`
- `WriteHoneyContainerStateToItemInstance(UItemInstance* TargetItemInstance) const`
- `GetCurrentVolumeMl() const`
- `GetMaxVolumeMl() const`
- `GetFreeVolumeMl() const`
- `GetHoneyDensity() const`
- `GetHoneyRipeness() const`
- `RemoveHoneyVolume(float VolumeMl)`
- `AddHoneyVolume(float VolumeMl, float IncomingDensity, float IncomingRipeness)`
- `GetNozzleOriginComponent() const`
- `GetPourTargetComponent() const`
- `GetHoneyStreamNiagaraComponent() const`
- `RefreshHoneyVisualState()`

visual 규칙:

- fill ratio = `CurrentVolumeMl / MaxVolumeMl`, `0..1`
- `HoneyVisualMesh`의 Z scale을 fill ratio로 조정한다.
- X/Y scale은 유지한다.
- full scale 기준은 BeginPlay 또는 construction 시점의 authored relative scale을 사용한다.
- honey material dynamic instance에 scalar parameter를 설정한다.
  - `HoneyDensity`
  - `HoneyRipeness`
- `HoneyDensity < 1.0`이면 visual/material에도 `HoneyRipeness=0.0`을 적용한다.

## `AHoneyContainerSlotActor`

역할:

- reusable honey container placement slot.
- source/target 역할과 accepted tag query를 소유한다.
- empty 상태에서는 기존 item-use-area placement 경로를 제공한다.
- occupied 상태에서는 꿀 용기 회수 descriptor와, source role인 경우 노즐 descriptor를 제공한다.

권장 enum:

```cpp
UENUM(BlueprintType)
enum class EHoneyContainerSlotRole : uint8
{
    Source,
    Target
};
```

구현 규칙:

- `AItemPlacementSlotActor`를 상속한다.
- `TryPlaceItem_Implementation`에서 `AHoneyContainerActor` class만 허용한다.
- source item definition의 gameplay tags가 slot accepted tag query를 만족해야 한다.
- 배치 성공 후 `AHoneyContainerActor::ApplyStateFromItemInstance(SourceItemInstance)`를 호출한다.
- clear/place 후 owning host의 PartFocus/item-use-area descriptors를 rebuild한다.
- `GetPlacedHoneyContainerActor() const`를 제공한다.
- `GetCursorPartFocusDescriptors_Implementation`에서 occupied container retrieve descriptor를 제공하고, `Role == Source`이면 container의 nozzle descriptor도 append한다.

## `UHoneyTransferComponent`

역할:

- 꿀 이송 진행 상태의 source of truth.
- 다른 작업대에서도 재사용 가능한 domain component.
- source/target validation, source container HoneyStream Niagara control, DropLength grow phase, 실제 transfer phase, auto stop을 소유한다.

권장 state:

```cpp
UENUM(BlueprintType)
enum class EHoneyTransferState : uint8
{
    Idle,
    GrowingDrop,
    Transferring
};
```

권장 설정:

- `TransferRateMlPerSecond`
- `DropLengthGrowSpeedCmPerSecond`
- `DefaultDropLengthCm`
- Niagara parameter names
  - default `User.HoneyDensity`
  - default `User.HoneyRipeness`
  - default `User.DropLength`
- optional material/scalar names은 container actor 쪽에서 소유한다.

권장 API:

- `ConfigureSlots(AHoneyContainerSlotActor* SourceSlot, AHoneyContainerSlotActor* TargetSlot)`
- `SetHoneyStreamNiagara(UNiagaraComponent* NiagaraComponent)`는 legacy fallback/compatibility API로 유지한다. 기본 stream source는 active source container의 `HoneyStreamNiagara`다.
- `CanStartTransfer() const`
- `StartTransfer()`
- `StopTransfer(bool bImmediateVfx)`
- `ToggleTransferFromNozzle(AHoneyContainerActor* SourceContainer)`
- `IsTransferActive() const`
- `GetTransferState() const`

이송 시작:

1. source slot/target slot/containers를 검증한다.
2. source role은 `Source`, target role은 `Target`이어야 한다.
3. source volume > 0, target free volume > 0이어야 한다.
4. source container `HoneyStreamNiagara`를 활성화하고 source container의 `HoneyDensity`, `HoneyRipeness`를 Niagara parameter에 세팅한다.
5. `DropLength`는 0으로 세팅한다.
6. state는 `GrowingDrop`으로 전환한다.

DropLength grow:

- target length는 source container `HoneyStreamNiagara` world Z와 target container 또는 target slot `PourTarget` world Z 차이(`Max(0, SourceStream.Z - TargetPourTarget.Z)`)로 계산한다.
- 계산할 수 없으면 `DefaultDropLengthCm`를 사용한다.
- `CurrentDropLength += DropLengthGrowSpeedCmPerSecond * DeltaTime`
- 목적값 도달 전에는 실제 volume 이동을 하지 않는다.
- 목적값 도달 시 source/target을 재검증하고 state를 `Transferring`으로 전환한다.

실제 이송:

```cpp
MoveAmountMl = Min(
    TransferRateMlPerSecond * DeltaTime,
    SourceVolumeMl,
    TargetFreeVolumeMl);
```

- source는 volume만 감소하며 density/ripeness는 유지한다.
- target의 `HoneyDensity`, `HoneyRipeness`는 기존 target 내용물과 유입량을 volume-weighted average로 계산한다.
- 유입 꿀 또는 혼합 결과의 `HoneyDensity < 1.0`이면 target `HoneyRipeness = 0.0`으로 정규화한다.
- 모든 꿀 용기 state는 다음 invariant를 만족해야 한다.
  - `HoneyDensity < 1.0 && HoneyRipeness == 0.0`
  - 또는 `HoneyDensity == 1.0 && HoneyRipeness >= 0.0`

자동 정지 조건:

- source slot/container missing
- target slot/container missing
- source volume <= 0
- target free volume <= 0
- slot occupant changed
- owning actor EndPlay/destroy
- Focus cancel/deactivate 등 host가 transfer stop을 요청하는 경우

정지:

- 꿀 줄기 Niagara는 즉시 사라져야 한다.
- 가능하면 `DeactivateImmediate()`, 불가능하면 `Deactivate()`와 `SetVisibility(false)` 등 기존 Niagara 사용 패턴에 맞춘다.
- stop 시 `DropLength`는 0으로 되돌린다.

## `UHoneyNozzlePartFocusActionComponent`

역할:

- source container nozzle click을 transfer toggle로 라우팅한다.

구현 규칙:

- concrete `AHoneyDecantingTable` class에 직접 의존하지 않는다.
- owner `AHoneyContainerActor`의 placement occupant에서 owning slot을 찾고, slot의 attach parent/host에서 `UHoneyTransferComponent`를 찾는다.
- role이 `Source`인 slot의 container일 때만 start/toggle 가능하다.
- prompt action text는 상태에 따라 `배출`/`정지` 또는 프로젝트 기존 naming 방식에 맞춰 구현자가 정한다.
- prompt availability와 실제 실행 가능성은 같은 helper를 공유한다.

## `UHoneyContainerRetrievePartFocusActionComponent`

역할:

- 배치된 꿀 용기 회수 시 item instance에 honey state를 write-back한다.

구현 규칙:

- 기존 `UPlacementSlotRetrievePartFocusActionComponent::TryRetrievePlacementOccupant(...)` 경로를 활용한다.
- retrieve 성공 후 `AHoneyContainerActor::WriteHoneyContainerStateToItemInstance(AcquiredItemInstance)`를 호출한다.
- 그 뒤 owning slot `ClearPlacedItem`을 호출한다.
- hotbar acquire 실패 시 actor/slot 상태를 유지한다.
- prompt availability와 실제 retrieve는 같은 helper/API를 공유한다.

## `AHoneyDecantingTable`

역할:

- 1차 사용처인 꿀 소분 작업대 native WorldActor.
- source slot, target slot, reusable transfer component, Niagara를 조립한다.

권장 구성은 `AUncappingTable` 패턴을 따른다.

- `Root`
- `TableMesh`
- `FocusAnchor`
- `CharacterAnchor`
- `UFocusTargetComponent`
- `UAnchoredFocusCursorActionComponent`
- `UCursorPartFocusScopeComponent`
- `UCursorPartFocusRegistrationComponent`
- `UChildCursorPartFocusProviderComponent`
- `UCursorItemUseAreaScopeComponent`
- `UItemUseAreaMeshProviderComponent`
- `SourceSlotRoot`
- `SourceSlotChildActor`
- `TargetSlotRoot`
- `TargetSlotChildActor`
- `UHoneyTransferComponent`
- `UNiagaraComponent HoneyStreamNiagara` legacy fallback component. 신규 VFX authoring은 source container `HoneyStreamNiagara`에서 한다.

구현 규칙:

- FocusEngaged 진입/취소, hotbar 선택 clear, cursor/input mode는 기존 anchored cursor 정책을 따른다.
- source/target slot child actor class는 `AHoneyContainerSlotActor` 기반으로 둔다.
- source slot role은 `Source`, target slot role은 `Target`으로 설정한다.
- slot accepted tag query/mesh/material/transform은 BP/details authoring 가능하게 둔다.
- `RebuildCursorPartFocusDescriptors()`
- `RebuildItemUseAreaDescriptors()`
- BeginPlay/OnConstruction에서 transfer component에 source/target slot과 Niagara를 연결한다.
- table은 transfer 계산을 직접 구현하지 않는다.

## Blueprint/Content 경계

- `Content/` asset은 수정하지 않는다.
- Niagara asset은 BP/details에서 설정한다.
- C++은 Niagara component와 parameter 주입 경로만 제공한다.
- 꿀 용기 mesh, honey visual mesh material, nozzle mesh/hit mesh, slot mesh/material은 BP/details authoring 대상으로 둔다.
- 새 material parameter 계약:
  - `HoneyDensity`
  - `HoneyRipeness`
- 새 Niagara parameter 계약:
  - `User.HoneyDensity`
  - `User.HoneyRipeness`
  - `User.DropLength`

## Gameplay tag 권장

구현자가 기존 tag authoring 방식에 맞춰 확정한다.

권장 예:

- `Item.HoneyContainer`
- `Item.HoneyContainer.Source`
- `Item.HoneyContainer.Target`
- `Item.UseArea.HoneyTransfer.SourceSlot`
- `Item.UseArea.HoneyTransfer.TargetSlot`

Config/Content tag asset 수정이 필요하면 구현하지 말고 최종 보고 또는 `.md/USER_UNREAL.md`에 수동 작업으로 적는다.

## 수정하면 안 되는 것

- 기존 UCLASS/USTRUCT/UENUM rename 금지
- 기존 Blueprint API 삭제/rename 금지
- `Content/` 에셋 저장 금지
- `Config/DefaultEngine.ini` CoreRedirect 추가 금지. 이번 작업은 신규 class/struct 추가 중심이어야 한다.
- `APlacedItemActor`에 꿀 용기 전용 state/nozzle/transfer 기능을 직접 추가하지 않는다.
- `AHoneyDecantingTable`에 transfer 계산을 하드코딩하지 않는다.
- `HoneyRipness` 오탈자 parameter를 새 정본으로 만들지 않는다. 정본은 `HoneyRipeness`다.

## 문서 반영

구현 후 구조 변경 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - Source 구조 counts
  - 시스템 간 책임 흐름
  - 꿀 용기/소분 작업대 요약
- `.md/Architecture/InventorySystem.md`
  - `FHoneyContainerItemState`
  - `UHoneyContainerItemDefinition`
  - state copy/write-back 주의
- `.md/Architecture/WorldActorsSystem.md`
  - `AHoneyContainerActor`
  - `AHoneyContainerSlotActor`
  - `UHoneyTransferComponent`
  - nozzle/retrieve action
  - `AHoneyDecantingTable`
- `.md/Architecture/FocusSystem.md`
  - 노즐 PartFocus descriptor/toggle 경로

Core Redirect 문서는 신규 추가만이면 갱신하지 않는다.

## 검증

공백/패치 검증:

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

UBT 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

검색 검증:

```powershell
rg -n "HoneyRipness|SourceCan|TargetJar" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

위 검색은 새 정본에 맞지 않는 이름이 남지 않았는지 확인하기 위한 것이다. QnA의 과거 선택지에만 남는 경우는 허용되지만, 새 구현 코드와 정본 문서에는 남기지 않는다.

## 수동 PIE 확인

1. `BP_HoneyDecantingTable` 또는 해당 BP child에서 source/target slot class와 slot mesh를 설정한다.
2. source slot에는 source tag를 가진 꿀 용기 item만 배치되는지 확인한다.
3. target slot에는 target tag를 가진 꿀 용기 item만 배치되는지 확인한다.
4. source container nozzle hover/click prompt가 표시되는지 확인한다.
5. 배출 시작 시 source container `HoneyStreamNiagara`가 활성화되고 `HoneyDensity`, `HoneyRipeness`, `DropLength=0`이 적용되는지 확인한다.
6. `DropLength`가 source stream Z와 target pour target Z 차이까지 증가하는 동안 실제 volume이 이동하지 않는지 확인한다.
7. 목적값 도달 후 volume이 `TransferRateMlPerSecond`에 맞춰 이동하는지 확인한다.
8. source empty 또는 target full에서 자동 정지하고 Niagara가 즉시 사라지는지 확인한다.
9. target에 기존 꿀이 있을 때 volume-weighted average와 density/ripeness invariant가 유지되는지 확인한다.
10. 배치된 꿀 용기를 회수한 뒤 hotbar item instance에 volume/density/ripeness가 보존되는지 확인한다.
11. hotbar/storage 이동 후 꿀 용기 state가 보존되는지 확인한다.
12. 다른 작업대 actor에 `UHoneyTransferComponent`와 source/target slot을 붙여도 같은 이송 규칙을 재사용할 수 있는지 구조적으로 확인한다.

## 최종 보고 요구사항

- 변경 파일
- 새 UCLASS/USTRUCT/UENUM 목록
- 추가한 Blueprint/API 계약
- 꿀 이송 state flow 요약
- 회수 state write-back 경로
- `HoneyDensity/HoneyRipeness` invariant 적용 위치
- Niagara/material parameter 이름
- 아키텍처 문서 반영 내용
- 빌드 결과 또는 미수행 사유
- 필요한 수동 BP/Content 작업 목록
