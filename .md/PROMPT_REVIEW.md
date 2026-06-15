# SwarmCluster density-based radius 리뷰 프롬프트

## 리뷰 목표

분봉 본진 생성 시 디자이너가 radius 값을 직접 authoring하지 않고, `SpawnAmount`와 `SwarmClusterBeeDensityPerCubicMeter`만으로 `InitialAliveRadius`, `AliveRadius`, `SphereRadius`가 내부 파생되는지 리뷰한다.

핵심 기대는 `ABeehive`의 분봉 테스트 입력 surface에서 radius authoring 변수가 제거되고, `ABeeSwarmClusterActor`는 runtime radius state를 유지하되 bee count와 density를 source of truth로 삼는 것이다. BeeCarrier capture rate, queen cage/queen capture, route spline generation, Niagara parameter 이름은 변경되면 안 된다.

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 리뷰 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmClusterActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSwarmClusterActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

참고: `.md/PROMPT_IMPLEMENTATION.md`는 작업 시작 전부터 dirty였고 이번 구현 대상이 아니다.

## 기대 구현 요약

### `ABeehive`

- `SwarmClusterInitialAliveRadius`와 `SwarmClusterSphereRadius`가 `UPROPERTY` authoring 입력에서 제거되어야 한다.
- `SwarmClusterSpawnAmount`는 유지되어야 한다.
- `SwarmClusterBeeDensityPerCubicMeter`가 추가되어야 한다.
  - `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beehive|Swarming Test", meta = (ClampMin = "0.0001"))`
  - 기본값 `8000.0f`
- `BeginSwarmingAtTransform`은 cluster spawn 후 explicit radius를 넘기지 않아야 한다.
- expected call shape:

```cpp
ClusterActor->InitializeSwarmClusterFromDensity(
    SwarmClusterSpawnAmount,
    SwarmClusterBeeDensityPerCubicMeter);
```

### `ABeeSwarmClusterActor`

- 기존 radius 기반 BlueprintCallable initializer `InitializeSwarmCluster(float, int32, float)`는 제거되어야 한다.
- 새 initializer가 BlueprintCallable이어야 한다.

```cpp
void InitializeSwarmClusterFromDensity(int32 InSpawnAmount, float InBeeDensityPerCubicMeter);
```

- `AliveRadius`와 `SphereRadius`는 `EditAnywhere`가 아니어야 한다.
- `InitialAliveRadius`, `AliveRadius`, `SphereRadius`는 runtime state로 유지될 수 있다.
- `BeeDensityPerCubicMeter` runtime state가 있어야 하며 기본값은 `8000.0f`다.
- density sanitize는 `0.0001f` 같은 작은 양수 clamp를 사용해야 한다.
- `SpawnAmount <= 0`이면 radius는 `0.0f`여야 한다.
- radius 계산은 meter 단위 부피에서 산출 후 cm로 변환해야 한다.

```cpp
VolumeM3 = SpawnAmount / BeeDensityPerCubicMeter
RadiusM = cbrt((3 * VolumeM3) / (4 * PI))
RadiusCm = RadiusM * 100
```

- `InitializeSwarmClusterFromDensity`는 최소 다음 상태를 reset/set 해야 한다.
  - `bCaptured=false`
  - `bBeesCaptured=false`
  - `bQueenCaptured=false`
  - `SpawnAmount=max(0, InSpawnAmount)`
  - `CapturedBeeAmount=0.0f`
  - sanitized density 저장
  - `InitialAliveRadius=CalculateRadiusCmFromBeeDensity(...)`
  - `AliveRadius=InitialAliveRadius`
  - `SphereRadius=InitialAliveRadius`
- queen setup, capture use-area activation, Niagara parameter application, events, captured-state handling은 기존 의미가 유지되어야 한다.
- `OnConstruction`/`BeginPlay`는 manually authored `AliveRadius`를 source로 쓰면 안 된다.
- `BeginPlay`가 runtime initializer 이후 capture progress를 깨면 finding이다.

### Capture Math

- Bee count가 계속 source of truth여야 한다.
- `CaptureBees`, `SetCapturedBeeAmount`, `RefreshAliveRadiusFromBeeAmounts`, `CalculateAliveRadiusFromRemainingBees`는 `SpawnAmount`, `CapturedBeeAmount`, `InitialAliveRadius`를 기준으로 동작해야 한다.
- `AliveRadius = InitialAliveRadius * cbrt(RemainingBeeAmount / TotalBeeAmount)` 공식은 유지되어야 한다.
- `SetAliveRadius`/`DecreaseAliveRadius`는 legacy/manual visual adjustment API로 남아도 된다.
- `SetAliveRadius`/`DecreaseAliveRadius`가 bee count 없이 bees captured 상태를 완료 처리하면 finding이다.
- BeeCarrier capture gameplay가 `SetAliveRadius`/`DecreaseAliveRadius`를 사용하면 finding이다.

### Niagara Contract

다음 parameter 이름은 변경되면 안 된다.

- `User.AliveRadius`
- `User.SpawnAmount`
- `User.SphereRadius`

`User.AliveRadius`와 `User.SphereRadius` 값은 density-derived runtime radius에서 와야 한다.

### Blueprint/API/Core Redirect

- expected Blueprint-facing removals:
  - `ABeehive::SwarmClusterInitialAliveRadius`
  - `ABeehive::SwarmClusterSphereRadius`
  - `ABeeSwarmClusterActor::InitializeSwarmCluster(float, int32, float)`
- expected Blueprint-facing additions:
  - `ABeehive::SwarmClusterBeeDensityPerCubicMeter`
  - `ABeeSwarmClusterActor::InitializeSwarmClusterFromDensity(int32, float)`
- `Config/DefaultEngine.ini` Core Redirect 추가는 없어야 한다.
- radius-to-density property redirect는 units/semantics가 다르므로 추가하면 finding이다.
- 기존 Blueprint asset의 old properties/nodes는 수동 migration 대상으로 보고해야 하며, `Content/`를 수정하면 finding이다.

### 문서

- `.md/0_ARCHITECTURE.md`는 direct radius start configuration 대신 `SwarmClusterSpawnAmount`와 `SwarmClusterBeeDensityPerCubicMeter` 생성 입력을 설명해야 한다.
- `.md/Architecture/WorldActorsSystem.md`는 `ABeehive` composition, swarming test success flow, `ABeeSwarmClusterActor` source-of-truth, Niagara contract를 density 기반 설명으로 갱신해야 한다.
- `.md/USER_UNREAL.md`의 수동 설정 안내에서 old radius fields가 남아 있으면 finding이다.

## 반드시 확인할 불변조건

- BeeCarrier capture rate logic 변경 없음.
- Queen cage / queen capture logic 변경 없음.
- Route actor spline generation logic 변경 없음.
- Existing Niagara parameter names 변경 없음.
- `Content/` 수정 없음.
- `Config/DefaultEngine.ini` 수정 없음.
- UCLASS/USTRUCT/UENUM rename 없음.
- `#include "Public/..."` 신규 추가 없음.

## 구현자가 수행한 검증

- `git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md`
  - 통과. Git line-ending warning만 출력.
- `rg -n "SwarmClusterInitialAliveRadius|SwarmClusterSphereRadius|InitializeSwarmCluster\\(" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md`
  - Source에는 old symbol 없음.
  - `.md/PROMPT_IMPLEMENTATION.md`와 변경 기록 문구에는 old symbol이 남을 수 있다.
- `rg -n "SwarmClusterBeeDensityPerCubicMeter|BeeDensityPerCubicMeter|InitializeSwarmClusterFromDensity|CalculateRadiusCmFromBeeDensity" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md`
  - expected matches 확인.
- `rg -n "EditAnywhere.*AliveRadius|EditAnywhere.*SphereRadius|bLegacyRadiusDepleted|AliveRadius <= KINDA_SMALL_NUMBER" Source/BeekeepingSim/Public Source/BeekeepingSim/Private`
  - match 없음.
- `BeekeepingSimEditor Win64 Development`
  - 실패. 열린 `UnrealEditor.exe`가 `Binaries/Win64/UnrealEditor-BeekeepingSim.dll`을 lock해서 link 단계 `LNK1104`.
- `BeekeepingSim Win64 Development`
  - 성공.

## 리뷰어 검증 명령

```powershell
git diff --check -- Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

```powershell
rg -n "SwarmClusterInitialAliveRadius|SwarmClusterSphereRadius|InitializeSwarmCluster\\(" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg -n "SwarmClusterBeeDensityPerCubicMeter|BeeDensityPerCubicMeter|InitializeSwarmClusterFromDensity|CalculateRadiusCmFromBeeDensity" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
rg -n "AliveRadius|SphereRadius" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors .md/0_ARCHITECTURE.md .md/Architecture/WorldActorsSystem.md
rg -n "EditAnywhere.*AliveRadius|EditAnywhere.*SphereRadius|bLegacyRadiusDepleted|AliveRadius <= KINDA_SMALL_NUMBER" Source/BeekeepingSim/Public Source/BeekeepingSim/Private
```

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

Editor target link가 DLL lock으로 실패하면 Unreal Editor를 종료한 뒤 재시도한다. 코드 컴파일 대체 확인은 아래 game target으로 한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSim Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

## 수동 PIE 리뷰 포인트

1. Beehive instance Details의 Swarming Test에서 `SwarmClusterInitialAliveRadius`와 `SwarmClusterSphereRadius`가 사라졌는지 확인한다.
2. `SwarmClusterBeeDensityPerCubicMeter`가 보이고 기본값이 `8000.0`인지 확인한다.
3. `SpawnAmount=500`, density `8000.0`으로 분봉을 시작하면 cluster radius가 약 `24.63cm`인지 확인한다.
4. Niagara가 다음 값을 받는지 확인한다.
   - `User.SpawnAmount = 500`
   - `User.AliveRadius ~= 24.63`
   - `User.SphereRadius ~= 24.63`
5. 벌 절반 포획 시 `AliveRadius`가 선형이 아니라 cube-root volume scaling을 따르는지 확인한다.
6. 벌 전량 포획 시 `AliveRadius=0`, BeeCarrier use-area 비활성화, 최종 완료는 queen capture까지 대기하는지 확인한다.
7. 여왕벌 포획까지 끝났을 때 `ReceiveSwarmCaptured`가 1회만 호출되는지 확인한다.
8. route generation end가 여전히 `ClusterCenter`인지 확인한다.
9. `Content/` asset이 수정되지 않았는지 확인한다.

## 리뷰 출력 형식

- Findings first. 심각도 순으로 파일/라인을 포함한다.
- 합당한 finding이 없으면 "중요 finding 없음"이라고 명시한다.
- 이후 검증 결과와 남은 수동 PIE/Blueprint migration 항목을 짧게 정리한다.
