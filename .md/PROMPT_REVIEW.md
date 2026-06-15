# Swarm cluster native FocusCollision 리뷰 프롬프트

## 리뷰 목표

`ABeeSwarmClusterActor`의 preview focus hit proxy가 Blueprint-authored `SphereCollision` 의존에서 native C++ `FocusCollision`으로 이동했는지 검토한다.

핵심 기대는 swarm cluster actor가 항상 native `USphereComponent FocusCollision`을 소유하고, 이 component가 preview focus trace에서는 `ECC_Visibility`를 block하지만 FocusEngaged 중에는 BeeCarrier/QueenCage item-use-area cursor trace를 막지 않도록 disabled 되는 것이다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `Source/BeekeepingSim/Public/Focus/AnchoredFocusCursorActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/BeekeeperFocusComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

## 기대 구현

### Native Component

- `ABeeSwarmClusterActor`가 native `USphereComponent* FocusCollision`을 `VisibleAnywhere` component로 가져야 한다.
- 생성자는 `CreateDefaultSubobject<USphereComponent>(TEXT("FocusCollision"))` 후 `ClusterCenter`에 attach해야 한다.
- Blueprint-authored `SphereCollision`을 C++에서 이름으로 찾아 mutation하면 안 된다.

### Collision Contract

- enabled 상태는 `QueryOnly`여야 한다.
- all channels ignore 후 `ECC_Visibility`만 block해야 한다.
- overlap events, physics, navigation 영향이 없어야 한다.
- final captured 상태, FocusEngaged active suppression, actor destroy/end play에서는 disabled 되어야 한다.
- intro growth 중에는 preview focus hit proxy가 enabled 상태를 유지해야 한다. FocusEngaged 시작 차단은 `CanBeginFocusAction(...)`이 담당한다.

### Radius Sync

정확한 공식:

```cpp
FocusCollisionRadius = FMath::Max(0.0f, AliveRadius) + 5.0f;
```

- `AliveRadius` 변경 경로마다 radius가 stale하지 않아야 한다.
- 선호 구현은 `ApplyClusterNiagaraParameters()` 끝에서 `RefreshFocusCollisionState()`를 호출하는 것이다.
- `ClusterNiagara == nullptr`이어도 focus collision refresh는 실행되어야 한다.
- bees fully captured but queen remains 상태에서는 `AliveRadius=0`, `FocusCollision` radius `5`가 되어야 한다.

### FocusEngaged Lifecycle

- `USwarmClusterFocusActionComponent`가 다음 lifecycle을 override해야 한다.
  - `OnFocusEngagedStarted`
  - `OnFocusReturnCompleted`
  - `OnFocusActionAborted`
- `Super` 호출 후 owner cluster의 suppression state를 갱신해야 한다.
- FocusEngaged start에서는 `FocusCollision`을 suppress/disable해야 한다.
- return completed와 abort에서는 suppression을 해제하고 collision state를 refresh해야 한다.
- cancel start만으로 collision을 복구하면 안 된다. Focus return이 완료되거나 abort될 때 복구되어야 한다.

### Trace 상호작용

- `UBeekeeperFocusComponent::FindFocusTargetFromTrace()`는 기존 `FocusTraceChannel` line trace와 `HitActor->FindComponentByClass<UFocusTargetComponent>()` 경로를 유지해야 한다.
- `UCursorItemUseAreaScopeComponent` cursor trace도 Visibility 기반이므로, FocusEngaged 중 `FocusCollision`이 enabled로 남아 있으면 BeeCarrier/QueenCage use-area hit를 막을 수 있다. 이 경우 finding이다.

### Blueprint/API/Core Redirect

- UCLASS/USTRUCT/UENUM rename이 없어야 한다.
- 기존 UFUNCTION/UPROPERTY 삭제가 없어야 한다.
- Core Redirect는 필요하지 않아야 한다.
- 기존 swarm cluster Blueprint에는 새 inherited `FocusCollision` component가 생긴다.
- 기존 BP-authored `SphereCollision`은 수동으로 제거하거나 `NoCollision`으로 변경해야 한다. 이 migration note가 최종 보고와 문서에 있어야 한다.

## 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "FocusCollision|SphereCollision|SetFocusCollision|RefreshFocusCollision|AliveRadius \+ 5|OnFocusEngagedStarted|OnFocusReturnCompleted|OnFocusActionAborted" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md
```

```powershell
rg -n "FindFocusTargetFromTrace|FocusTraceChannel|CursorTraceChannel|ECC_Visibility|ItemUseArea" Source/BeekeepingSim/Private/Focus Source/BeekeepingSim/Public/Focus
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## Manual Editor/PIE 확인

1. Swarm cluster Blueprint를 연다.
2. 기존 BP `SphereCollision`을 제거하거나 `NoCollision`으로 설정한다.
3. Blueprint compile/save를 수행한다.
4. 수동 분봉 route를 시작하고 cluster spawn을 기다린다.
5. inherited native `FocusCollision`이 존재하는지 확인한다.
6. preview focus가 `FocusCollision`으로 swarm cluster를 hit하는지 확인한다.
7. intro growth 중 prompt는 보일 수 있지만 FocusEngaged는 시작되지 않는지 확인한다.
8. intro 완료 후 FocusEngaged가 시작되는지 확인한다.
9. FocusEngaged start 시 `FocusCollision` collision이 disabled 되는지 확인한다.
10. FocusEngaged에서 BeeCarrier use-area hit/use가 가능한지 확인한다.
11. FocusEngaged에서 QueenCage use-area hit/use가 가능한지 확인한다.
12. FocusEngaged cancel/exit 후 final capture 전 `FocusCollision`이 복구되는지 확인한다.
13. 벌 포획 중 `FocusCollision` radius가 `AliveRadius + 5`를 따르는지 확인한다.
14. 벌만 모두 포획하고 queen이 남았을 때 radius가 `5`인지 확인한다.
15. queen capture 후 final captured actor removal이 기존 BP collision 없이 정상 동작하는지 확인한다.

## 리뷰 결론 요구

- 승인 가능 / 수정 후 재검토 / 설계 재검토 필요 중 하나로 결론을 낸다.
- finding이 있으면 파일/라인과 함께 원인, 영향, 수정 방향을 제시한다.
- 추가 구현 프롬프트가 필요하면 `.md/PROMPT_IMPLEMENTATION_R.md`를 작성한다.
