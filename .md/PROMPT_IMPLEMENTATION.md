# `ABeehiveSwarmRouteActor` Spline 중간지점 자동생성 구현 프롬프트

## 목표

이미 구현된 분봉 테스트 기능 중 `ABeehiveSwarmRouteActor`의 route spline 생성만 개선한다.

현재 `ABeehiveSwarmRouteActor::ConfigureRoute(Start, End)`는 start/mid/end 3-point spline을 만든다. 이를 거리 기반 자동 중간지점 생성 방식으로 바꾼다. 자동 생성된 중간점들 중 정확히 하나의 중앙점만 `RouteMidPointHeightOffset` 높이를 가져야 하며, 그 양쪽 점들은 같은 부드러운 곡선으로 중앙점까지 올라갔다가 도착점으로 내려가야 한다.

새 분봉 actor, 새 item action, 새 Focus 경로는 만들지 않는다. 이미 구현된 `ABeeSwarmClusterActor`, `UBeeCarrierUseAction`, `ABeehive::BeginSwarmingAtTransform/Actor`는 유지한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

## 현재 코드 기준

이미 존재하는 파일:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`

현재 route actor:

- `ABeehiveSwarmRouteActor : ABeeSplineSwarmActor`
- `ConfigureRoute(FVector StartWorldLocation, FVector EndWorldLocation)`
- `ConfigureRouteToCluster(FVector StartWorldLocation, ABeeSwarmClusterActor* ClusterActor)`
- 기존 property: `RouteMidPointHeightOffset`
- 기존 `SwarmSpline`은 parent `ABeeSplineSwarmActor`의 protected component다.

현재 `ABeehive::BeginSwarmingAtTransform`:

- cluster actor spawn
- route actor spawn
- `RouteActor->ConfigureRoute(RouteStartLocation, RouteEndLocation)`
- `RouteActor->ApplyExternalSwarmParameters(SwarmRouteParameters)`

## 구현 범위

수정 대상:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 구조 문서 갱신이 필요하면 `.md/0_ARCHITECTURE.md`, `.md/Architecture/WorldActorsSystem.md`

수정 금지:

- `ABeeSwarmClusterActor` 중복 구현 금지
- `UBeeCarrierUseAction` 중복 구현 금지
- 새 route actor class 추가 금지
- 기존 UCLASS/UFUNCTION/UPROPERTY rename 금지
- `RouteMidPointHeightOffset` rename 금지
- `Content/` asset 수정/저장 금지
- Core Redirect 추가 금지

## `ABeehiveSwarmRouteActor` 설계

기존 `RouteMidPointHeightOffset`는 유지한다. Blueprint에 노출된 기존 property이므로 이름을 바꾸지 않는다.

추가 권장 UPROPERTY:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route", meta = (ClampMin = "0.0"))
float ForwardLeadDistance = 150.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route", meta = (ClampMin = "1.0"))
float AutoMiddlePointSpacing = 300.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive Swarm Route", meta = (ClampMin = "1"))
int32 MaxAutoMiddlePointCount = 11;
```

선택 helper:

```cpp
int32 CalculateAutoMiddlePointCount(float LeadToEndDistance) const;
void BuildAutoRouteSplinePoints(const FVector& StartWorldLocation, const FVector& EndWorldLocation, TArray<FVector>& OutWorldPoints) const;
```

## Spline Point 생성 규칙

route actor의 world transform 기준:

- `P0 = StartWorldLocation`
- `P1 = StartWorldLocation + GetActorForwardVector() * ForwardLeadDistance`
- `A0..An = P1`과 `EndWorldLocation` 사이의 거리 기반 자동 중간점
- `Pend = EndWorldLocation`

`A0..An` 개수는 반드시 홀수여야 한다.

거리 기반 개수 계산:

```cpp
const float LeadToEndDistance = FVector::Distance(LeadPoint, EndWorldLocation);
const float Spacing = FMath::Max(1.0f, AutoMiddlePointSpacing);

int32 SegmentCount = FMath::Max(2, FMath::CeilToInt(LeadToEndDistance / Spacing));

// AutoMiddlePointCount = SegmentCount - 1 이므로,
// AutoMiddlePointCount를 홀수로 만들려면 SegmentCount는 짝수여야 한다.
if (SegmentCount % 2 != 0)
{
    ++SegmentCount;
}

int32 AutoMiddlePointCount = SegmentCount - 1;
AutoMiddlePointCount = FMath::Clamp(AutoMiddlePointCount, 1, FMath::Max(1, MaxAutoMiddlePointCount));

// clamp 후에도 홀수 유지
if (AutoMiddlePointCount % 2 == 0)
{
    --AutoMiddlePointCount;
}
AutoMiddlePointCount = FMath::Max(1, AutoMiddlePointCount);
```

높이 계산:

```cpp
for (int32 Index = 0; Index < AutoMiddlePointCount; ++Index)
{
    const float Alpha = static_cast<float>(Index + 1) / static_cast<float>(AutoMiddlePointCount + 1);
    const FVector Base = FMath::Lerp(LeadPoint, EndWorldLocation, Alpha);
    const float HeightRatio = FMath::Sin(Alpha * UE_PI);
    const FVector Point = Base + FVector::UpVector * RouteMidPointHeightOffset * HeightRatio;
}
```

중요 invariant:

- `AutoMiddlePointCount`는 항상 홀수다.
- 중앙 index는 `AutoMiddlePointCount / 2`다.
- 중앙점은 `Alpha == 0.5f`이므로 `HeightRatio == 1.0f`다.
- 따라서 자동 생성된 중간점 중 정확히 중앙점 1개만 `RouteMidPointHeightOffset` 높이를 가진다.
- 양쪽 중간점들은 `sin(alpha * pi)` 곡선으로 부드럽게 연결된다.

좌표계:

- point 계산은 world space에서 수행한다.
- `SwarmSpline`에 넣을 때는 route actor local space로 변환한다.
- `P0`는 route actor location과 같아야 하므로 local zero를 사용할 수 있다.
- 나머지는 `GetActorTransform().InverseTransformPosition(WorldPoint)` 사용.

Spline 적용:

```cpp
SwarmSpline->ClearSplinePoints(false);
for each point:
    SwarmSpline->AddSplinePoint(LocalPoint, ESplineCoordinateSpace::Local, false);
    SwarmSpline->SetSplinePointType(Index, ESplinePointType::Curve, false);
SwarmSpline->UpdateSpline();
ApplySplineLengthParameter();
```

## `ABeehive` 변경

`ABeehiveSwarmRouteActor`가 `GetActorForwardVector()`를 첫 lead 방향으로 사용하므로 route actor spawn rotation을 `SwarmExitPoint` rotation으로 맞춘다.

현재 `BeginSwarmingAtTransform`의 route spawn transform이 `FRotator::ZeroRotator`이면 변경한다.

권장:

```cpp
const FTransform RouteStartTransform = SwarmExitPoint
    ? SwarmExitPoint->GetComponentTransform()
    : GetActorTransform();

const FVector RouteStartLocation = RouteStartTransform.GetLocation();
const FTransform RouteSpawnTransform(RouteStartTransform.GetRotation(), RouteStartLocation);
```

이후 기존 흐름 유지:

```cpp
RouteActor->ConfigureRoute(RouteStartLocation, RouteEndLocation);
RouteActor->ApplyExternalSwarmParameters(SwarmRouteParameters);
```

주의:

- `SwarmExitPoint`가 없으면 기존처럼 벌통 actor transform을 fallback으로 사용한다.
- route end는 계속 cluster center world location 기준이다.
- 기존 colony/queen/comb state는 변경하지 않는다.

## 문서 반영

구현 후 아래 문서만 필요한 만큼 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - `ABeehiveSwarmRouteActor` 설명을 3-point 고정 route에서 거리 기반 홀수 자동 중간점 route로 수정
- `.md/Architecture/WorldActorsSystem.md`
  - `ABeehiveSwarmRouteActor` composition/flow 설명 수정
  - route actor spawn rotation이 `SwarmExitPoint` rotation을 사용한다는 점 기록

Inventory/Focus 문서는 수정하지 않는다. 이번 작업은 item action과 Focus 경로를 바꾸지 않는다.

## 검증

공백/패치 검증:

```powershell
git diff --check -- Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

UBT 빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

검색 검증:

```powershell
rg -n "ForwardLeadDistance|AutoMiddlePointSpacing|MaxAutoMiddlePointCount|RouteMidPointHeightOffset" Source/BeekeepingSim/Public/WorldActors/BeehiveSwarmRouteActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveSwarmRouteActor.cpp .md
rg -n "FRotator::ZeroRotator.*Route|RouteSpawnTransform" Source/BeekeepingSim/Private/WorldActors/Beehive.cpp
```

## 수동 PIE 확인

1. `SwarmExitPoint` rotation을 바꾸면 route 첫 lead 방향이 같이 바뀌는지 확인한다.
2. 짧은 거리에서도 자동 중간점이 최소 1개 생성되는지 확인한다.
3. 긴 거리에서는 `AutoMiddlePointSpacing`에 따라 중간점 수가 증가하는지 확인한다.
4. 자동 중간점 개수가 항상 홀수인지 디버그/로그/스플라인 시각화로 확인한다.
5. 자동 중간점 중 중앙점 1개만 `RouteMidPointHeightOffset` 높이를 갖는지 확인한다.
6. 중앙점 양쪽 점들이 부드러운 arc로 이어지는지 확인한다.
7. spline end가 계속 분봉 본진 `ClusterCenter`에 도달하는지 확인한다.
8. route Niagara의 `User.SwarmSpline`/`User.SplineLength`가 갱신되어 벌떼가 새 spline을 따라가는지 확인한다.

## 최종 보고 요구사항

- 변경 파일
- 추가한 UPROPERTY/API
- 거리 기반 중간점 개수 계산 방식
- 홀수 중간점 보정 방식
- 중앙점 1개만 `RouteMidPointHeightOffset`에 도달하는 근거
- `SwarmExitPoint` rotation을 route actor forward로 사용하는 방식
- 빌드 결과 또는 미수행 사유
- 문서 반영 내용
