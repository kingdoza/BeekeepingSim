# 구현 수정 프롬프트: swarm cluster native FocusCollision 리뷰 Finding

## 우선순위

1. Important: 이번 C++ FocusCollision 변경 범위에 포함되지 않아야 하는 `Content/` asset 변경 제거

## 발견 문제

### 1. `Content/` asset 변경이 working tree에 남아 있음

- 대상 파일:
  - `Content/Niagaras/NS_Part.uasset`
  - `Content/BeeSwarmCluster/BP_SwarmTest.uasset`
  - `Content/__ExternalActors__/Beekeeper/Lvl_BeekeeperTest/C/BF/`
- 문제: 리뷰 프롬프트의 금지 변경 조건은 `Content/` 수정 금지인데, 현재 working tree에 modified/untracked asset 변경이 남아 있다.
- 영향: native `FocusCollision` 전환은 C++ component와 문서/수동 migration note로 처리해야 하며, binary asset 변경이 섞이면 Blueprint/level/Niagara serialized state가 의도치 않게 commit될 수 있다.
- 수정 방향: 이번 구현 changelist에서 `Content/` 변경을 제거한다. 기존 swarm cluster Blueprint의 legacy `SphereCollision` 제거 또는 `NoCollision` 변경은 수동 PIE/Blueprint migration 항목으로 보고하고, 이 코드 변경 PR/patch에는 포함하지 않는다.

## 검증 방법

```powershell
git status --short -- Content Config/DefaultEngine.ini
```

기대 결과:

- `Config/DefaultEngine.ini` 변경 없음
- `Content/` 변경 없음

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "FocusCollision|SphereCollision|SetFocusCollision|RefreshFocusCollision|AliveRadius \+ 5|OnFocusEngagedStarted|OnFocusReturnCompleted|OnFocusActionAborted" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 문서/수동 migration

- `.md/Architecture/WorldActorsSystem.md`의 migration note처럼 기존 swarm cluster Blueprint의 authored `SphereCollision`은 수동으로 제거하거나 `NoCollision`으로 변경한다.
- 수동 migration 과정에서 asset을 저장해야 한다면 별도 change로 분리한다.
