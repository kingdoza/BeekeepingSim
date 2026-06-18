# Queen cell spawn relative transform 리뷰 프롬프트

## 리뷰 목표

`ABeehiveCombActor`에 추가된 `QueenCellSpawnRelativeTransform`과 `QueenCellUseAreaScaleMultiplier`가 queen cell sampled placement 위에 authoring-time root offset과 use-area-only scale로만 적용되고, 기존 queen cell lifecycle/removal/retrieval/persistence 계약을 바꾸지 않는지 검토한다.

기대 결과:

- `QueenCellSpawnRelativeTransform`은 `EditAnywhere, BlueprintReadOnly`, category `Beehive|Queen Cell|Spawn`, default `FTransform::Identity`다.
- `QueenCellUseAreaScaleMultiplier`는 `EditAnywhere, BlueprintReadOnly`, category `Beehive|Queen Cell|Use Area`, default `FVector::OneVector`다.
- `FQueenCellPlacement` fields와 의미는 변경되지 않는다.
- sampled base placement는 계속 `UQueenCellSpawnAreaComponent`가 만든 face + area-local YZ + local rotation + scale이다.
- spawned `QueenCellRoot` relative transform은 `SanitizedQueenCellSpawnRelativeTransform * BaseTransform` 순서로 합성된다.
- `QueenCellVisual`은 identity relative transform child로 남는다.
- `QueenCellUseArea`는 identity location/rotation과 `QueenCellUseAreaScaleMultiplier` relative scale을 사용한다.
- runtime transform 변경 후 already-spawned queen cell refresh는 구현하지 않는다.
- Content asset, `Config/DefaultEngine.ini`, Core Redirect, Blueprint API rename/delete가 없어야 한다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/PROMPT_REVIEW.md`

## 중점 리뷰 항목

### Authoring property

- `ABeehiveCombActor`에 아래 property가 queen cell visual/use-area authoring property 근처에 있는지 확인한다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Spawn")
FTransform QueenCellSpawnRelativeTransform = FTransform::Identity;
```

- `ABeehiveCombActor`에 아래 property가 queen cell use-area authoring property 근처에 있는지 확인한다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Queen Cell|Use Area")
FVector QueenCellUseAreaScaleMultiplier = FVector::OneVector;
```

- `FQueenCellPlacement`에 새 field가 추가되지 않았는지 확인한다.
- item state 또는 `FBeehiveCombItemState` persistence에 transform offset이 추가되지 않았는지 확인한다.

### Transform composition

- `BuildQueenCellSpawnAreaRelativeTransform(...)`의 base transform이 기존 sampled placement 값을 그대로 사용하는지 확인한다.
- composition order가 정확히 아래 형태인지 확인한다.

```cpp
return SanitizedQueenCellSpawnRelativeTransform * BaseTransform;
```

- offset이 `QueenCellRoot` relative transform에 적용되고, visual-only child transform으로 빠지지 않았는지 확인한다.
- default identity에서 기존 placement 결과가 변경되지 않는지 확인한다.
- front/back 모두 같은 authored offset을 sampled face-local frame에서 사용하는지 확인한다.

### Scale sanitization

- authoring transform scale component가 non-finite이면 `1.0f`로 정리되는지 확인한다.
- finite scale component가 `0.01f`보다 작으면 `0.01f`로 clamp되는지 확인한다.
- use-area scale multiplier도 같은 scale sanitization을 사용하는지 확인한다.
- finite location/rotation은 별도 정책 없이 authored value를 유지하는지 확인한다.
- placement scale은 기존처럼 `FMath::Max(0.01f, Placement.Scale)`로 유지되는지 확인한다.

### Runtime contract preservation

- `CreateQueenCellRuntimeComponents(...)`에서 `Visual->SetRelativeTransform(FTransform::Identity)`가 유지되는지 확인한다.
- `UseArea->SetRelativeTransform(...)`는 location/rotation identity와 sanitized `QueenCellUseAreaScaleMultiplier` scale만 적용하는지 확인한다.
- `TrySpawnQueenCell()`이 계속 `UQueenCellSpawnAreaComponent::TrySampleQueenCellPlacement(...)`를 사용하는지 확인한다.
- `CanSpawnQueenCell()` 조건이 max count, mesh availability, spawn area availability, sample availability 기반으로 유지되는지 확인한다.
- `RemoveQueenCell(...)`, `ResolveQueenCellIdFromUseArea(...)`, `QueenCellUseAreaToId` mapping이 변경되지 않았는지 확인한다.
- queen cell use-area tag가 `Item.UseArea.Beehive.QueenCell`로 유지되는지 확인한다.
- `ApplyStateFromItemInstance()`가 runtime queen cells를 계속 clear하고, queen cell state를 item state와 섞지 않는지 확인한다.
- comb retrieval blocking 조건에서 `QueenCellCount > 0` 계약이 유지되는지 확인한다.

### Scope and docs

- `Content/` asset과 `Config/DefaultEngine.ini`가 수정되지 않았는지 확인한다.
- UCLASS/USTRUCT/UENUM rename, Blueprint API delete/rename, Core Redirect 변경이 없는지 확인한다.
- architecture docs가 authoring offset, sampled base placement, no `FQueenCellPlacement` persistence, root-only application, visual identity child, use-area scale multiplier, no post-spawn refresh, identity/one-vector default preservation을 정확히 설명하는지 확인한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md
```

```powershell
rg -n "QueenCellSpawnRelativeTransform|QueenCellUseAreaScaleMultiplier|BuildQueenCellSpawnAreaRelativeTransform|FQueenCellPlacement|QueenCellRoot|QueenCellVisual|QueenCellUseArea" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md
```

```powershell
git status --short -- Content Config/DefaultEngine.ini
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 Editor/PIE 확인

- comb Blueprint에서 `QueenCellSpawnRelativeTransform`이 `Beehive|Queen Cell|Spawn` 아래 보이는지 확인한다.
- comb Blueprint에서 `QueenCellUseAreaScaleMultiplier`가 `Beehive|Queen Cell|Use Area` 아래 보이는지 확인한다.
- visible location/rotation/scale offset을 지정한 뒤 queen cell spawn을 유도한다.
- spawned queen cell visual이 root offset을 따르는지 확인한다.
- removal use-area가 같은 중심/방향을 유지하면서 추가 scale multiplier를 적용하는지 확인한다.
- front/back cells가 같은 authored offset을 각 face orientation 기준으로 적용하는지 확인한다.
- identity/one-vector 값에서 기존 placement와 동일한지 확인한다.
- queen cell removal, pressure 감소, retrieval blocking, `ApplyStateFromItemInstance()`의 runtime queen cell clear 동작이 유지되는지 확인한다.
