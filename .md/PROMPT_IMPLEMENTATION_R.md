# 구현 수정 프롬프트: swarming pressure / queen cell 리뷰 Finding

## 우선순위

1. Important: queen cell placement runtime state를 transient로 고정
2. Improvement: queen cell Editor 수동 작업 문서 보강

## 발견 문제

### 1. `QueenCellPlacements`가 runtime-only 계약인데 `Transient`가 아님

- 대상 파일:
  - `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- 문제:
  - queen cell은 `FBeehiveCombItemState`에 저장하지 않는 runtime state로 설계되어 있다.
  - 하지만 `ABeehiveCombActor::QueenCellPlacements`는 `UPROPERTY(VisibleInstanceOnly, Category = "Beehive|Queen Cell")`로 선언되어 있고 `Transient`가 없다.
- 영향:
  - runtime placement state가 actor instance/editor serialization 대상으로 남을 수 있어 "runtime component group, item state에 저장하지 않음" 계약이 약해진다.
- 수정 방향:
  - `QueenCellPlacements`에 `Transient`를 추가한다.
  - `QueenCellRuntimeComponents`와 `QueenCellUseAreaToId`는 계속 runtime-only mapping으로 유지한다.

수정 예:

```cpp
UPROPERTY(VisibleInstanceOnly, Transient, Category = "Beehive|Queen Cell")
TArray<FQueenCellPlacement> QueenCellPlacements;
```

### 2. `.md/USER_UNREAL.md`가 queen cell 수동 설정을 안내하지 않음

- 대상 파일:
  - `.md/USER_UNREAL.md`
- 문제:
  - `Content/` asset은 수정하지 않는 범위가 맞다.
  - 그러나 queen cell visual mesh/material, use-area mesh/material, spawn area authoring, queen cell removal item DataAsset/action 연결은 Editor/BP 수동 작업이 필요하다.
  - 현재 `.md/USER_UNREAL.md`는 colony swarming site selection과 기존 swarm capture 중심이고 queen cell 수동 설정/PIE 체크가 없다.
- 영향:
  - C++는 빌드되어도 queen cell이 실제로 보이거나 제거 action으로 hit/use 되는 검증 경로가 빠진다.
- 수정 방향:
  - `.md/USER_UNREAL.md`에 swarming pressure / queen cell 섹션을 추가한다.
  - 포함할 항목:
    - `Config/DefaultGameplayTags.ini`의 `Item.UseArea.Beehive.QueenCell` 확인
    - comb Blueprint child에서 `QueenCellSpawnArea` 위치/크기 조정
    - `QueenCellVisualMesh`, `QueenCellVisualMaterial`, `QueenCellUseAreaMesh`, `QueenCellUseAreaMaterial` 지정
    - 제거용 item DataAsset에 `UQueenCellRemovalUseAction` 연결
    - PIE에서 pressure 상승, threshold 전/후 생성, lifted comb 제외, 제거 시 pressure 감소, queen cell 존재 중 comb retrieval 차단 검증
  - `Content/` asset은 이번 코드 변경 changelist에 포함하지 않는다.

## 검증 방법

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "QueenCellPlacements|Transient|QueenCellRemovalUseAction|Item.UseArea.Beehive.QueenCell|QueenCellSpawnArea" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory .md/USER_UNREAL.md
```

```powershell
git status --short -- Content Config/DefaultEngine.ini Config/DefaultGameplayTags.ini
```

기대 결과:

- `Content/` 변경 없음
- `Config/DefaultEngine.ini` 변경 없음
- `Config/DefaultGameplayTags.ini`에는 `Item.UseArea.Beehive.QueenCell` 추가만 있음

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 아키텍처 문서 반영 필요 여부

- `.md/0_ARCHITECTURE.md`와 `.md/Architecture/WorldActorsSystem.md`는 swarming pressure / queen cell 설계를 이미 반영했다.
- `.md/USER_UNREAL.md` 보강은 필요하다.
