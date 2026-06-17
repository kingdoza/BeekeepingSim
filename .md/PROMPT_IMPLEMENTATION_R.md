# 구현 수정 프롬프트: colony swarming site selection 리뷰 Finding

## 우선순위

1. Important: 제거된 colony swarming Blueprint node 수동 migration 안내 보강

## 발견 문제

### 1. `BP_Beehive`에 제거 API 참조가 남아 있으나 수동 작업 문서가 최신 flow를 안내하지 않음

- 대상 파일:
  - `.md/USER_UNREAL.md`
  - `Content/Beehive/BP_Beehive.uasset`는 직접 수정하지 않는다.
- 문제:
  - `rg -a` 결과 `Content/Beehive/BP_Beehive.uasset`에 `BeginColonySwarmingAtActor` 참조가 남아 있다.
  - C++에서는 `BeginColonySwarmingAtTransform`/`BeginColonySwarmingAtActor`가 제거되고 `BeginColonySwarming()`만 남았다.
  - 현재 `.md/USER_UNREAL.md`는 이전 분봉 테스트 포획 workflow 중심이라, 새 `ABeeSwarmClusterSiteActor` 배치와 `BP_Beehive` node 교체/Compile/Save 절차를 지속 문서로 안내하지 않는다.
- 영향:
  - Editor에서 `BP_Beehive`를 열 때 제거된 Blueprint node가 missing/unknown 상태가 될 수 있다.
  - 실제 colony swarming은 site actor 없이는 selectable site 없음으로 실패하므로, 레벨 배치와 PIE 검증 절차가 문서화되어야 한다.
- 수정 방향:
  - `.md/USER_UNREAL.md`에 colony swarming site selection 수동 작업 섹션을 추가한다.
  - 포함할 내용:
    - `Content/Beehive/BP_Beehive.uasset`의 `BeginColonySwarmingAtActor` node를 `BeginColonySwarming` node로 수동 교체한다.
    - `BP_Beehive`를 Compile/Save한다.
    - 레벨에 여러 `ABeeSwarmClusterSiteActor` 또는 해당 BP child를 배치하고 `OccupantSpawnPoint`, selection weight 설정을 확인한다.
    - selectable site 없음, reservation 실패, route start 실패, route arrival occupation, cluster destroy auto-release PIE 체크를 추가한다.
  - `Content/` asset은 이번 구현 changelist에서 수정하지 않는다. 실제 asset migration은 Editor 수동 작업으로 분리한다.

## 검증 방법

```powershell
rg -a -n "BeginColonySwarmingAtTransform|BeginColonySwarmingAtActor" Source Content Config .md
```

기대 결과:

- `Source`에는 제거 API 선언/정의가 없어야 한다.
- `Content/Beehive/BP_Beehive.uasset` 참조는 수동 migration 전까지 남을 수 있으며, `.md/USER_UNREAL.md`가 해당 작업을 명시해야 한다.
- `Config/DefaultEngine.ini` Core Redirect 변경은 없어야 한다.

```powershell
git status --short -- Content Config/DefaultEngine.ini
```

기대 결과:

- 이번 코드/문서 변경에는 `Content/` 변경이 포함되지 않는다.
- `Config/DefaultEngine.ini` 변경이 없다.

## 아키텍처 문서 반영 필요 여부

- `.md/0_ARCHITECTURE.md`와 `.md/Architecture/WorldActorsSystem.md`는 이미 새 API/site selection flow를 반영했다.
- 추가 반영 대상은 Editor 수동 작업 문서인 `.md/USER_UNREAL.md`다.
