# Beehive comb Blueprint editor delay 리뷰 프롬프트

## 리뷰 목표

`ABeehiveCombActor`의 wax capping mask runtime state가 comb Blueprint class defaults에 직렬화되거나 editor construction/details-change 경로에서 transient texture allocation/update를 유발하지 않는지 검토한다.

기대 결과:

- `RuntimeCappingMaskWidth`, `RuntimeCappingMaskHeight`, `RuntimeFrontWaxCappingMask`, `RuntimeBackWaxCappingMask`는 transient runtime actor state다.
- 기존 serialized `CappingMaskWidth`, `CappingMaskHeight`, `FrontWaxCappingMask`, `BackWaxCappingMask` class-default payload와 연결되지 않도록 Core Redirect 없이 runtime property 이름을 분리한다.
- `FBeehiveCombItemState`가 inventory/item 이동 사이의 capping mask persistence path로 유지된다.
- editor `OnConstruction()`/`PostEditChangeProperty()` 경로는 scalar sanitize, Niagara parameter, honey visual safe update를 유지하되 capping mask texture refresh를 피한다.
- runtime capping/uncapping, regeneration, item state restore/write, material parameter `WaxCappingMask` 적용은 기존 동작을 유지한다.
- Public Blueprint API rename/delete, UCLASS/USTRUCT/UENUM rename, Core Redirect 변경이 없어야 한다.

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

### Runtime state serialization

- 아래 properties가 `VisibleInstanceOnly, Transient`인지 확인한다.

```cpp
RuntimeCappingMaskWidth
RuntimeCappingMaskHeight
RuntimeFrontWaxCappingMask
RuntimeBackWaxCappingMask
```

- `FrontWaxCappingMaskTexture`/`BackWaxCappingMaskTexture`가 계속 `Transient`로 GC 보호되는지 확인한다.
- `FBeehiveCombItemState` 구조와 `UItemInstance` persistence path가 변경되지 않았는지 확인한다.
- `WriteStateToItemInstance()`가 width/height/front/back masks를 계속 저장하는지 확인한다.
- `ApplyStateFromItemInstance()`가 valid stored mask를 복원하고 invalid dimension은 full mask fallback하는지 확인한다.

### Editor-time guard

- `OnConstruction()`이 sanitize, Niagara parameter, honey visual update를 유지하면서 `BeehiveCombActorNames::IsGameWorldContext(this)`일 때만 `EnsureCappingMaskState()`와 `RefreshCappingMaskTextures()`를 호출하는지 확인한다.
- `PostEditChangeProperty()`가 editor details 변경에서 transient capping texture allocation/update를 수행하지 않는지 확인한다.
- `ApplyHoneyVisualState()`가 editor world에서 capping texture를 간접 생성하지 않는지 확인한다.
- `ApplyHoneyCappingVisualState()`가 editor world에서 mask 배열을 scan하지 않고 즉시 반환하는지 확인한다.
- `ApplyWaxCappingMaskMaterialParameters()`가 front/back capping dynamic material instance가 둘 다 없으면 `EnsureCappingMaskTextures()` 전에 return하는지 확인한다.
- `EnsureHoneyMaterialInstances()`의 editor-world dynamic material instance 회피 정책과 새 guard가 서로 맞는지 확인한다.

### Runtime behavior preservation

- 다음 runtime paths에서 capping mask state/texture refresh가 제거되지 않았는지 확인한다.

```text
BeginPlay()
ApplyCombBeeParameters(...)
SetTotalSpawnAmountAndResetTargetBeeCounts(...)
SetTotalSpawnAmountPreservingTargetRatios(...)
ApplyWaxCappingBrush(...)
TryRegenerateWaxCapping()
ApplyStateFromItemInstance(...)
```

- `ApplyWaxCappingBrush(...)`가 현재 visible face의 mask만 수정하고 texture/material/visibility를 갱신하는지 확인한다.
- `TryRegenerateWaxCapping()`이 full honey와 ripeness threshold 조건에서 제거된 face mask를 `255`로 복원하는지 확인한다.
- capping material texture parameter 이름 `WaxCappingMask`와 honey scalar parameters `HoneyAmount`, `HoneyRipeness`가 변경되지 않았는지 확인한다.
- `IsWaxCappingFaceComplete()`/`ApplyHoneyCappingVisualState()` 조건이 기존 visibility behavior를 유지하는지 확인한다.

### Scope and asset safety

- `Content/Beehive/BP_HoneyComb.uasset` 변경은 stale serialized capping mask payload 제거를 위한 resave 결과인지 확인한다.
- 다른 `Content/` asset이 수정되지 않았는지 확인한다.
- `Config/DefaultEngine.ini` Core Redirect 변경이 없는지 확인한다.
- Public Blueprint API rename/delete가 없는지 확인한다.
- architecture docs가 capping mask transient runtime state, `FBeehiveCombItemState` persistence, editor-time texture allocation avoidance, runtime `WaxCappingMask` material contract를 정확히 반영하는지 확인한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md
```

```powershell
rg -n "RuntimeCappingMaskWidth|RuntimeCappingMaskHeight|RuntimeFrontWaxCappingMask|RuntimeBackWaxCappingMask|RefreshCappingMaskTextures|ApplyWaxCappingMaskMaterialParameters|ApplyHoneyCappingVisualState|PostEditChangeProperty|OnConstruction" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md .md/PROMPT_REVIEW.md
```

```powershell
git status --short -- Content Config/DefaultEngine.ini
```

```powershell
$bytes=[IO.File]::ReadAllBytes("Content\Beehive\BP_HoneyComb.uasset"); $max=0; $cur=0; for($i=0; $i -lt $bytes.Length; $i++){ if($bytes[$i] -eq 255){ $cur++ } else { if($cur -gt $max){ $max=$cur }; $cur=0 } }; if($cur -gt $max){ $max=$cur }; "AssetLength=$($bytes.Length) MaxFFRun=$max"
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 Editor/PIE 확인

- `Content/Beehive/BP_HoneyComb`를 열 때 기존 delay가 사라졌거나 크게 줄었는지 확인한다.
- ordinary details 값을 바꿔도 기존 delay가 재현되지 않는지 확인한다.
- Blueprint를 다시 Compile/Save해도 serialized mask payload가 재생성되지 않는지 확인한다.
- `BP_HoneyComb.uasset` 크기가 다시 512x512 mask payload 포함 수준으로 증가하지 않는지 확인한다.
- PIE에서 honey fill/ripeness visuals, full honey capping visuals, uncapping brush, inventory 회수/재배치 capping mask restore, wax capping regeneration이 기존처럼 동작하는지 확인한다.
