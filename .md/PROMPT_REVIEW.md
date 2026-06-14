# ABeehiveSwarmRouteActor Spline 자동 중간점 리뷰 프롬프트

## 리뷰 목표

이번 리뷰는 이미 구현된 분봉 테스트 기능 중 `ABeehiveSwarmRouteActor` route spline 생성 개선만 검토한다.

검토 대상은 고정 start/mid/end 3-point route를 거리 기반 자동 중간점 route로 바꾼 C++ 변경과, route actor spawn rotation을 `SwarmExitPoint` rotation으로 맞춘 `ABeehive` 변경이다.

이번 리뷰 범위에서 제외한다.

- `ABeeSwarmClusterActor` 신규/중복 구현 여부가 아닌 기존 동작 전체 재검토
- `UBeeCarrierUseAction` 포획 rate, item state, Focus 경로
- 자동 분봉 발생 조건
- colony/queen/comb simulation 반영
- `Content/` asset 설정/저장

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/PROMPT_IMPLEMENTATION.md`

## 리뷰 대상 파일

Source:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

Docs:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/PROMPT_REVIEW.md`

## 기대 구현

### 1. Route actor API/UPROPERTY

- 기존 `ABeehiveSwarmRouteActor` class를 유지해야 한다.
- 새 route actor class를 추가하지 않아야 한다.
- 기존 `RouteMidPointHeightOffset` 이름을 변경하지 않아야 한다.
- 기존 public Blueprint API인 `ConfigureRoute`, `ConfigureRouteToCluster`, `GetRouteEndWorldLocation`을 삭제/rename하지 않아야 한다.
- 다음 tuning property가 Blueprint에서 조정 가능해야 한다.
  - `ForwardLeadDistance`
  - `AutoMiddlePointSpacing`
  - `MaxAutoMiddlePointCount`

### 2. 거리 기반 자동 중간점 계산

- `ConfigureRoute(StartWorldLocation, EndWorldLocation)`는 actor location을 start로 맞춰야 한다.
- route world point 순서는 다음이어야 한다.
  - `P0 = StartWorldLocation`
  - `P1 = StartWorldLocation + GetActorForwardVector() * ForwardLeadDistance`
  - `A0..An = P1`과 `EndWorldLocation` 사이의 자동 중간점
  - `Pend = EndWorldLocation`
- 자동 중간점 개수는 최소 1개여야 한다.
- 자동 중간점 개수는 항상 홀수여야 한다.
- `AutoMiddlePointSpacing`은 최소 1.0으로 방어되어야 한다.
- `MaxAutoMiddlePointCount` clamp 후에도 홀수 보정이 유지되어야 한다.

### 3. 중앙점 높이 invariant

자동 중간점 높이는 아래 형태여야 한다.

```cpp
Alpha = static_cast<float>(Index + 1) / static_cast<float>(AutoMiddlePointCount + 1);
HeightRatio = FMath::Sin(Alpha * UE_PI);
Point = Base + FVector::UpVector * RouteMidPointHeightOffset * HeightRatio;
```

리뷰에서 확인할 invariant:

- `AutoMiddlePointCount`가 홀수이면 중앙 index는 `AutoMiddlePointCount / 2`다.
- 중앙 index의 `Alpha`는 정확히 `0.5f`다.
- 중앙 index의 `HeightRatio`는 `1.0f`다.
- 따라서 자동 중간점 중 중앙점 1개만 `RouteMidPointHeightOffset` 높이에 도달해야 한다.
- 양쪽 중간점은 `sin(alpha * pi)` 곡선으로 대칭에 가깝게 올라갔다 내려가야 한다.

### 4. Spline 적용

- spline point 계산은 world space에서 수행해야 한다.
- `SwarmSpline`에 넣을 때는 route actor local space로 변환해야 한다.
- 첫 point는 actor location과 같으므로 local zero여야 한다.
- 나머지는 `GetActorTransform().InverseTransformPosition(WorldPoint)` 경로를 사용해야 한다.
- 모든 spline point는 `ESplinePointType::Curve`로 설정해야 한다.
- point 추가 후 `SwarmSpline->UpdateSpline()`과 `ApplySplineLengthParameter()`가 호출되어야 한다.
- 기존 Niagara parameter 계약인 `User.SwarmSpline`, `User.SplineLength` 갱신 흐름을 깨지 않아야 한다.

### 5. Beehive route spawn rotation

- `ABeehive::BeginSwarmingAtTransform`에서 route actor spawn transform은 `SwarmExitPoint` transform을 기준으로 해야 한다.
- `SwarmExitPoint`가 있으면 location과 rotation을 모두 사용해야 한다.
- `SwarmExitPoint`가 없으면 `GetActorTransform()` fallback을 사용해야 한다.
- route actor의 `GetActorForwardVector()`가 첫 lead 방향으로 쓰이므로 `FRotator::ZeroRotator` 고정 spawn이면 문제로 본다.
- route end는 계속 cluster actor origin이 아니라 `ClusterCenter` world location이어야 한다.

### 6. 변경 금지 사항

이번 route 개선에서 아래 변경이 보이면 문제로 본다.

- `ABeeSwarmClusterActor` 중복 구현 또는 불필요한 동작 변경
- `UBeeCarrierUseAction` 수정
- 새 Focus 경로 추가
- 기존 UCLASS/UFUNCTION/UPROPERTY rename
- Core Redirect 추가
- `Content/` asset 수정
- 분봉 시작 시 `ColonyBeeCount`, 기존 벌통 `QueenBeeChildActor`, active comb bee count/target count 변경

## 권장 검색 검증

```powershell
rg -n "ForwardLeadDistance|AutoMiddlePointSpacing|MaxAutoMiddlePointCount|RouteMidPointHeightOffset" Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp .md
rg -n "FRotator::ZeroRotator.*Route|RouteSpawnTransform" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp
rg -n "BeeCarrierUseAction|BeeSwarmClusterActor|SetColonyBeeCount\\(|DetachFromActor|QueenBeeChildActor.*Swarm" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory
```

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 PIE 확인

1. `SwarmExitPoint` rotation을 바꾸면 route 첫 lead 방향이 같이 바뀌는지 확인한다.
2. 짧은 거리에서도 자동 중간점이 최소 1개 생성되는지 확인한다.
3. 긴 거리에서는 `AutoMiddlePointSpacing`에 따라 중간점 수가 증가하는지 확인한다.
4. 자동 중간점 개수가 항상 홀수인지 spline visualizer 또는 디버그 로그로 확인한다.
5. 자동 중간점 중 중앙점 1개만 `RouteMidPointHeightOffset` 높이에 도달하는지 확인한다.
6. 중앙점 양쪽 점들이 부드러운 arc로 이어지는지 확인한다.
7. spline end가 계속 분봉 본진 `ClusterCenter`에 도달하는지 확인한다.
8. route Niagara가 새 spline을 따라가는지 확인한다.

## 리뷰 결과 작성 형식

리뷰 결과는 `.md/AGENT_REVIEW.md` 기준으로 작성한다.

- Findings first: severity, file/line, 문제, 영향, 수정 방향
- blocking/major issue가 없으면 "검토 범위에서 blocking/major issue 없음"을 명확히 적는다.
- 남은 리스크는 UBT/PIE/BP 수동 확인 여부와 연결해 적는다.
- 구현 요약은 findings 뒤에 짧게 적는다.
- 구현 에이전트에게 넘길 수정이 있으면 `.md/PROMPT_IMPLEMENTATION_R.md`에 작성한다.
