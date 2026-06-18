# Unreal Editor 수동 작업 목록 - Swarming Pressure / Queen Cell

이 문서는 C++ 구현 후 Unreal Editor에서 직접 설정, Compile/Save, PIE 검증해야 하는 swarming pressure / queen cell 항목만 정리한다.

`Content/` asset은 Codex가 수정하지 않는다. 아래 작업은 Editor에서 수동으로 수행한다.

## 1. Gameplay Tag 확인

Project Settings 또는 Gameplay Tags 창에서 아래 tag가 보이는지 확인한다.

- `Item.UseArea.Beehive.QueenCell`

보이지 않으면 `Config/DefaultGameplayTags.ini`에 tag가 반영되어 있는지 확인한 뒤 Editor를 재시작한다.

Core Redirect나 Gameplay Tag Redirect는 추가하지 않는다.

## 2. Comb Blueprint Queen Cell Authoring

`ABeehiveCombActor` 기반 comb Blueprint child를 연다.

native component 확인:

- `QueenCellSpawnArea`

`QueenCellSpawnArea` 설정:

- local `+X`가 comb front surface를 향하는지 확인한다.
- local `-X`가 comb back surface를 향하는지 확인한다.
- local `Y/Z`가 queen cell을 배치할 rectangular surface 영역을 덮도록 box 위치와 크기를 조정한다.
- box는 front/back 양면을 모두 포함하는 하나의 spawn area로 사용한다.
- edge band가 comb 바깥으로 나가지 않도록 `EdgeInsetCm`와 box extent를 함께 조정한다.
- 기본값 확인:
  - `EdgeBandWidthCm = 8.0`
  - `EdgeInsetCm = 2.0`
  - `MinQueenCellSpacingCm = 6.0`
  - `MaxPlacementAttempts = 32`
  - `BottomEdgeWeight = 3.0`
  - `LeftEdgeWeight = 1.0`
  - `RightEdgeWeight = 1.0`
  - `TopEdgeWeight = 0.5`

queen cell visual/use-area 설정:

- `QueenCellVisualMesh`를 지정한다.
- `QueenCellVisualMaterial`을 지정한다.
- `QueenCellUseAreaMesh`를 지정한다. 비워두면 C++가 visual mesh를 fallback으로 사용할 수 있지만, trace/hit 검증이 쉬운 별도 use-area mesh를 권장한다.
- `QueenCellUseAreaMaterial`을 지정한다.
- `MaxQueenCellCountPerComb`을 의도한 per-comb cap으로 설정한다. 기본값은 `4`다.

Compile/Save:

- comb Blueprint child
- queen cell visual material/material instance
- queen cell use-area material/material instance

## 3. Beehive Swarming Pressure Settings

`ABeehive` 기반 벌통 Blueprint를 연다.

swarming pressure 설정 기본값을 확인한다.

- `SwarmingPressure = 0.0`
- `SwarmingLifecycleBucketMinutes = 30`
- `bApplySwarmingLifecycleOnBeginPlayBucket = false`
- `ComfortBeeCountPerComb = 100.0`
- `PopulationStartRatio = 0.7`
- `PopulationTriggerRatio = 1.1`
- `QueenCellSpawnPressureThreshold = 0.7`
- `SwarmingTriggerPressure = 1.0`
- `QueenCellRemovalPressureDelta = 0.1`

queen cell hive target 설정 기본값을 확인한다.

- `MaxQueenCellCountPerHive = 10`
- `QueenCellSpawnExponent = 1.5`
- `MaxQueenCellsSpawnPerBucket = 2`

튜닝 기준:

- pressure target:

```cpp
ComfortBeeCapacity = ActiveCombCount * ComfortBeeCountPerComb;
PopulationRatio = ColonyBeeCount / ComfortBeeCapacity;
TargetPressure = (PopulationRatio - PopulationStartRatio)
    / (PopulationTriggerRatio - PopulationStartRatio);
```

- queen cell target:

```cpp
Alpha = Clamp(
    (SwarmingPressure - QueenCellSpawnPressureThreshold)
    / (SwarmingTriggerPressure - QueenCellSpawnPressureThreshold),
    0.0f,
    1.0f);

DesiredQueenCellCount =
    RoundToInt(MaxQueenCellCountPerHive * Pow(Alpha, QueenCellSpawnExponent));
```

Compile/Save:

- `BP_Beehive` 또는 사용하는 벌통 Blueprint
- 테스트 레벨

## 4. Queen Cell Removal Item DataAsset

queen cell 제거에 사용할 item definition 또는 관련 DataAsset을 연다.

설정:

- hold/use action 목록에 `UQueenCellRemovalUseAction`을 추가한다.
- action tag는 프로젝트의 기존 item action authoring 규칙에 맞춰 지정한다.
- 이 action은 `Item.UseArea.Beehive.QueenCell` use-area hit에서만 적용된다.
- 별도 mesh나 material asset을 C++에 하드코딩하지 않는다.

Compile/Save:

- queen cell 제거용 item definition/DataAsset
- 관련 UI 또는 hotbar 테스트 asset이 있다면 함께 저장

## 5. PIE 검증 체크리스트

1. 테스트 벌통에 queen이 있고 active comb가 배치되어 있는지 확인한다.
2. `ColonyBeeCount`를 높이거나 `ComfortBeeCountPerComb`을 낮춰 `SwarmingPressure`가 상승하는 조건을 만든다.
3. `SwarmingLifecycleBucketMinutes = 30` 기준으로 lifecycle update가 실행되는지 확인한다.
4. `SwarmingPressure < QueenCellSpawnPressureThreshold`에서는 queen cell이 생성되지 않는지 확인한다.
5. `SwarmingPressure >= QueenCellSpawnPressureThreshold` 이후 queen cell이 생성되는지 확인한다.
6. queen cell 생성 수가 hive-wide target을 따르고, bucket당 `MaxQueenCellsSpawnPerBucket`를 넘지 않는지 확인한다.
7. queen cell이 comb local `+X/-X` surface에만 생성되는지 확인한다.
8. queen cell이 `QueenCellSpawnArea`의 `Y/Z` edge band 안에만 생성되는지 확인한다.
9. center area에는 queen cell이 생성되지 않는지 확인한다.
10. 같은 face에서 `MinQueenCellSpacingCm`보다 가까운 queen cell 배치가 거부되는지 확인한다.
11. 여러 active comb가 있을 때 queen cell이 적은 comb가 더 자주 선택되는지 반복 확인한다.
12. lifted comb에는 새 queen cell이 생성되지 않는지 확인한다.
13. queen cell 제거용 item을 hotbar에서 선택하면 queen cell use-area가 hit/use 가능한지 확인한다.
14. queen cell 제거 성공 시 해당 runtime component group이 사라지는지 확인한다.
15. queen cell 제거 성공 시 `SwarmingPressure`가 `QueenCellRemovalPressureDelta`만큼 감소하고 0 미만으로 내려가지 않는지 확인한다.
16. queen cell이 남아 있는 comb는 retrieval이 차단되는지 확인한다.
17. bees target count가 0이고 queen이 붙어 있지 않더라도 queen cell이 있으면 comb retrieval이 실패하는지 확인한다.
18. 모든 queen cell을 제거한 뒤 bees/queen 조건도 clear되면 comb retrieval이 가능해지는지 확인한다.
19. queen이 없는 벌통에서는 swarming lifecycle이 queen cell을 생성하지 않는지 확인한다.
20. `SwarmingPressure > SwarmingTriggerPressure`에서 기존 `BeginColonySwarming()` flow가 호출되는지 확인한다.
21. 실제 colony swarming 성공 후 `SwarmingPressure`가 `0.0`으로 리셋되는지 확인한다.
22. `BeginSwarmingAtTransform`/`BeginSwarmingAtActor` 테스트 API 호출이 queen, `ColonyBeeCount`, `SwarmingPressure`, queen cell state를 변경하지 않는지 확인한다.

## 6. 이번 범위에서 하지 않는 작업

- Codex/C++가 `Content/` asset을 직접 수정하거나 저장하지 않는다.
- queen cell state를 `FBeehiveCombItemState`에 저장하지 않는다.
- queen cell을 actor로 만들지 않는다.
- `Config/DefaultEngine.ini` Core Redirect를 추가하지 않는다.
- 제거용 item DataAsset 연결을 C++에서 자동 생성하지 않는다.
