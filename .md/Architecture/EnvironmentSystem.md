# Environment System

## Scope

- `Source/BeekeepingSim/Public/Environment`
  - `GameTimeOfDayActor.h`
  - `DynamicSky.h`
  - `DynamicSkyTypes.h`
  - `TimeOfDayProvider.h`
  - `TimeOfDayTypes.h`
  - `GameTimeBucketTypes.h`
  - `GameTimeBucketListener.h`
  - `GameTimeBucketSubsystem.h`
  - `EnvironmentTimeOfDayActor.h`
- `Source/BeekeepingSim/Private/Environment`
  - `GameTimeOfDayActor.cpp`
  - `DynamicSky.cpp`
  - `GameTimeBucketSubsystem.cpp`
  - `EnvironmentTimeOfDayActor.cpp`

## Responsibilities

- 24시간 가속 월드 시간의 source of truth 제공
- `ITimeOfDayProvider` 기반 시간 공급 계약 제공
- 시간 source와 하늘/조명 visual consumer 분리
- 시간대별 하늘/조명/태양/달/안개 visual state 평가 및 적용
- 에디터에서 preview hour로 결과를 미리 보는 preview 경로 제공
- 레벨 디자이너가 sky/light/fog component와 tuning 값을 연결할 수 있는 최소 Blueprint 계약 제공
- gameplay actor가 분 단위 bucket 경계 이벤트를 구독할 수 있는 world subsystem 제공

## Canonical Runtime Model (2026-05-24+)

- Runtime 시간 source of truth는 `AGameTimeOfDayActor`다.
- 공통 시간 계약은 `ITimeOfDayProvider`다.
- `UGameTimeBucketSubsystem`은 provider를 구독해 bucket 이벤트를 dispatch한다.
- `ADynamicSky`는 provider hour를 받아 visual만 갱신한다(시간 누적/변이 금지).
- `AEnvironmentTimeOfDayActor`는 compatibility/transition actor다.

## Key Classes

- `AGameTimeOfDayActor`: runtime 시간 source of truth
  - `CurrentHour24`, `DayLengthSeconds`, progression enabled 상태를 소유한다.
  - `ITimeOfDayProvider::GetCurrentHour24`를 구현한다.
  - `OnGameTimeOfDayChanged(float Hour24)`를 broadcast한다.
  - BeginPlay에서 `UGameTimeBucketSubsystem::SetTimeOfDayProvider(this)`를 호출한다.
  - `ADynamicSky` preview start hour를 resolve해 runtime 시작 시간을 맞출 수 있다.
- `ITimeOfDayProvider`: runtime 소비자가 concrete time actor를 몰라도 현재 시간을 조회하는 공통 interface
- `ADynamicSky`: provider hour를 소비해 sky/light/fog visual만 평가/적용하는 actor
  - 시간 누적/진행 상태를 소유하지 않는다.
  - `AGameTimeOfDayActor`를 우선 bind하고, legacy `AEnvironmentTimeOfDayActor`를 fallback으로 bind한다.
  - `FDynamicSkyState`를 평가하고 sky atmosphere, directional light, skylight, fog, sky sphere material에 적용한다.
- `FDynamicSkyState`: `ADynamicSky`가 특정 hour에서 평가한 visual state DTO
- `FTimeOfDayVisualState`: 특정 시각에서 평가된 결과 구조체
  - sky color
  - sun/moon intensity
  - sun/moon color temperature
  - fog density
  - ambient intensity
  - sun/moon rotation
  - `bSunAboveHorizon`
  - `bMoonLightActive`
- `FTimeOfDayCurveSet`: 시간대별 연출 curve 묶음
  - sky color curves
  - light intensity curves
  - color temperature curves
  - fog density curve
  - ambient/skylight intensity curve
- `UGameTimeBucketSubsystem` (`UWorldSubsystem`):
  - `ITimeOfDayProvider` actor를 canonical 정책으로 resolve/bind한다.
  - `AGameTimeOfDayActor::OnGameTimeOfDayChanged`를 우선 구독하고, legacy actor delegate는 fallback으로만 사용한다.
  - listener subscription별로 00:00 기준 n분 bucket 경계 변경 시점에만 이벤트를 발행한다.
  - `LatestOnly` / `CatchUp` 정책과 BeginPlay 즉시 적용 옵션을 처리한다.
- `IGameTimeBucketListener`:
  - Actor가 구현하는 BlueprintNativeEvent interface다.
  - `GetGameTimeBucketSubscriptions()`로 여러 bucket subscription을 제공할 수 있다.
  - `OnGameTimeBucketEvent()`로 `FGameTimeBucketEvent`를 받는다.
- `FGameTimeBucketSubscription`:
  - `BucketMinutes`, `bApplyImmediatelyOnBeginPlay`, `CatchUpPolicy`, `SubscriptionTag`를 가진다.
- `FGameTimeBucketEvent`:
  - `Hour24`, bucket index/start/end minute, wrap 여부, initial/catch-up 여부, subscription tag를 전달한다.
- `AEnvironmentTimeOfDayActor`: compatibility/transition actor
  - 기존 Blueprint API와 delegate를 유지한다.
  - `ITimeOfDayProvider`를 구현한다.
  - `AGameTimeOfDayActor`가 없는 legacy level에서만 시간/visual source 역할을 한다.

## Runtime Flow

1. `AGameTimeOfDayActor`는 runtime progression이 켜져 있을 때만 Tick에서 시간을 진행한다.
2. `DeltaTime`과 `DayLengthSeconds`로 `CurrentHour24`를 `[0, 24)` 범위로 wrap 갱신한다.
3. 변경된 hour를 `OnGameTimeOfDayChanged(float)`로 broadcast한다.
4. `ADynamicSky`는 provider delegate를 받아 현재 hour 기준 `FDynamicSkyState`를 평가한다.
5. `ADynamicSky`는 sky atmosphere scattering, sun/moon directional light, skylight, fog, sky sphere material parameter를 적용한다.
6. `UGameTimeBucketSubsystem`은 provider hour를 받아 매 broadcast를 그대로 전달하지 않고 bucket 경계 변경 시점만 listener에 dispatch한다.
7. Runtime clock UI는 bucket event를 사용하지 않고, `ABeekeeperController`가 provider delegate를 직접 구독해 `UTimeOfDayClockWidget`에 `Hour24`를 주입한다.
8. `AEnvironmentTimeOfDayActor`는 legacy level에서만 기존 visual/time actor 역할을 유지한다.

## Time Model

- `AGameTimeOfDayActor::CurrentHour24`가 runtime 환경 시간의 단일 기준 값이다.
- 유효 범위는 normalize 기준 `[0.0, 24.0)`이다.
- `DayLengthSeconds`는 하루 길이를 실제 초 단위로 정의하며 `1.0` 미만이면 Tick 진행을 중단하고 warning을 1회 출력한다.
- 예시: `DayLengthSeconds = 600`이면 게임 내 하루가 실제 10분에 진행된다.
- runtime clock은 24시간에서 wrap된다.
- 하늘/조명/안개/태양/달 상태는 항상 `CurrentHour24`에서 결정되어야 하며, 개별 actor가 별도 시간을 누적하면 안 된다.
- bucket 경계 계산도 같은 시간 기준을 사용한다:
  - `MinuteOfDay = FloorToInt(NormalizeHour24(Hour24) * 60)`
  - `BucketIndex = MinuteOfDay / BucketMinutes`
  - 마지막 bucket은 `Min(BucketStart + BucketMinutes, 1440)`로 짧아질 수 있다.
- catch-up dispatch에서 `bWrappedDay`는 자정 bucket(`BucketStartMinute == 0`) event에만 true다.
- bucket minute conversion은 epsilon 없이 floor 변환을 사용한다.

## Game Time Bucket Model

- `UGameTimeBucketSubsystem`은 world begin play에서 listener scan을 수행하고, `ITimeOfDayProvider`를 canonical 정책(`AGameTimeOfDayActor` 우선, legacy fallback)으로 bind한다.
- Listener는 runtime spawn 시 직접 `RegisterListener()`를 호출할 수 있고, `EndPlay`에서 `UnregisterListener()`를 호출해야 한다.
- 하나의 actor는 `SubscriptionTag`가 다른 여러 subscription을 반환할 수 있다.
- `LatestOnly`는 경계가 여러 개 지나도 현재 bucket만 dispatch한다.
- `CatchUp`은 마지막 처리 bucket 이후 경계를 순서대로 dispatch하되 subscription당 최대 512개로 제한한다.
- Bucket dispatch는 gameplay용이다. 시계 UI처럼 매 minute 표시가 필요한 UI는 `OnTimeOfDayChanged`를 직접 구독하는 owner가 값을 주입한다.

## Sky And Lighting

현재 runtime visual authoring의 기본 경로는 `ADynamicSky`다.

- `SunriseHour`, `SunsetHour`, `GapTime`, `OrbitYaw`로 태양/달 궤도와 전환 구간을 설정한다.
- `SunLightRayleighScattering` / `NoSunLightRayleighScattering`으로 sunlight presence 전환 전후의 SkyAtmosphere Rayleigh 값을 설정한다.
- `SunLightMultiScattering` / `NoSunLightMultiScattering`으로 sunlight presence 전환 전후의 MultiScattering 값을 설정한다.
- sky sphere material에는 현재 `IsStarVisible`, `IsMoonVisible` scalar parameter를 주입한다.
- `ADynamicSky`는 시간 progression을 하지 않고 provider hour만 소비한다.

아래 curve set 정책은 legacy `AEnvironmentTimeOfDayActor` compatibility 경로에만 적용한다.

하늘과 조명 변화는 hard-coded if 구간이 아니라 curve asset으로 작성한다.

권장 normalized time key:

- `0.00` 자정: 짙은 남색 하늘, 태양 꺼짐, 달빛 활성, 낮은 안개
- `0.18` 새벽 전: 남색 유지, 안개 증가
- `0.23` 새벽: 남색에서 주황으로 전환, 낮은 각도의 따뜻한 태양광, 안개 최대
- `0.32` 아침: 주황이 옅은 하늘색으로 전환, 안개 감소
- `0.50` 정오: 밝은 하늘색, 강한 중립 daylight, 안개 최소
- `0.72` 저녁: 따뜻한 노을색, 태양광 감소
- `0.80` 황혼: 주황에서 남색으로 전환, 달빛 시작
- `1.00` 자정: 짙은 남색 하늘로 loop

필수 curve 의미:

- `SkyZenithColorCurve`: 상단 하늘색
- `SkyHorizonColorCurve`: 지평선 하늘색
- `SunIntensityCurve`: 낮에는 밝고 지평선 아래에서는 0
- `SunTemperatureCurve`: 새벽/저녁은 따뜻하게, 정오는 중립적으로
- `MoonIntensityCurve`: 밤에는 활성, 낮에는 0
- `MoonTemperatureCurve`: 밤의 차가운 청색 계열
- `FogDensityCurve`: 새벽에 가장 짙고 낮에는 낮음
- `AmbientIntensityCurve`: 밤의 최소 가시성과 낮의 간접광 보정

## Sun And Moon Movement

`ADynamicSky` 현재 정책:

- 태양과 달은 `SunriseHour`, `SunsetHour`, `GapTime`, `OrbitYaw`를 사용해 world rotation을 계산한다.
- 태양은 `SunriseHour -> SunsetHour` 구간을 `0 -> -180` degrees로 매핑한다.
- 달은 `SunsetHour -> next SunriseHour` 구간을 `0 -> -180` degrees로 매핑한다.
- gap 구간에서는 같은 각속도를 유지해 orbit을 unclamped extrapolation한다.
- `bSunLightVisible`은 `SunriseHour - GapTime`부터 `SunsetHour + GapTime`까지 true다.
- `bMoonLightVisible`은 `!bSunLightVisible`로 계산된다.
- 현재 `ADynamicSky`는 sun/moon directional light component visibility 또는 intensity를 runtime에서 토글하지 않는다.
- sky sphere material에서 `IsStarVisible`은 sunlight window의 반대로 설정하고, `IsMoonVisible`은 항상 `1.0`으로 설정한다.

legacy `AEnvironmentTimeOfDayActor` 정책:

- `MoonHour = CurrentHour24 + 12.0`, 24시간 wrap 적용
- 태양이 지평선 아래에 있으면 curve 값과 무관하게 sun light intensity를 0으로 강제한다.
- moon light 활성 조건은 `!bSunAboveHorizon && bMoonAboveHorizon`이다.
- 태양이 올라오면 moon light intensity를 0으로 강제한다.

## Scene Actor Binding

현재 visual binding owner는 `ADynamicSky`다. `AGameTimeOfDayActor`는 scene visual component를 직접 제어하지 않는다.

`ADynamicSky`는 다음 component/asset 연결을 소유한다.

- sun/moon `UDirectionalLightComponent`
- `USkyAtmosphereComponent`
- `USkyLightComponent`
- `UExponentialHeightFogComponent`
- sky sphere `UStaticMeshComponent`
- sky sphere mesh/material
- optional time source actor 또는 auto-find provider 설정

`AEnvironmentTimeOfDayActor`는 legacy compatibility actor로 기존 light/fog/MPC 참조 API를 유지한다. 새 runtime visual authoring은 `ADynamicSky` 경로를 우선한다.

권장 sky 연결 방식:

- C++은 `ADynamicSky` component와 sky sphere material parameter를 갱신한다.
- time progression은 `AGameTimeOfDayActor`가 소유하고, visual actor는 provider hour만 소비한다.
- gameplay actor는 sky/light actor를 직접 polling하지 않는다.

legacy `AEnvironmentTimeOfDayActor` MPC parameter 이름:

- `SkyZenithColor`
- `SkyHorizonColor`
- `SunDirection`
- `MoonDirection`
- `SunIntensity`
- `MoonIntensity`
- `FogDensity`
- `AmbientIntensity`

## Editor Preview

`ADynamicSky` 에디터 프리뷰용 property/API:

- `bUseEditorPreviewTime`
- `PreviewHour24`: `0.0-24.0` slider
- `ApplyPreviewTime()`: `CallInEditor`
- `bUpdatePreviewOnPropertyChange`
- `bStartGameTimeFromPreviewHour`: runtime 시작 시 `AGameTimeOfDayActor` 시작 hour로 preview hour를 사용할지 결정

legacy `AEnvironmentTimeOfDayActor`는 `bRecaptureSkyLightOnApply` opt-in 프리뷰 옵션을 유지한다.

프리뷰 동작:

1. 에디터에서 `bUseEditorPreviewTime`이 true이면 `PreviewHour24`를 기준으로 visual state를 평가한다.
2. slider 변경 또는 `ApplyPreviewTime()` 호출 시 runtime과 동일한 평가/적용 로직을 사용한다.
3. 프리뷰는 runtime clock을 진행하지 않는다.
4. 프리뷰는 scene actor를 생성/삭제하지 않는다.
5. 프리뷰는 연결된 light/fog/sky material parameter만 갱신한다.

## Blueprint/API Contracts

Blueprint-safe API:

- `AGameTimeOfDayActor::SetCurrentHour24(float NewHour)`
- `AGameTimeOfDayActor::GetCurrentHour24() const`
- `AGameTimeOfDayActor::SetTimeProgressionEnabled(bool bEnabled)`
- `AGameTimeOfDayActor::SetDayLengthSeconds(float NewDayLengthSeconds)`
- `AGameTimeOfDayActor::OnGameTimeOfDayChanged`
- `ADynamicSky::ApplyPreviewTime()`
- `AEnvironmentTimeOfDayActor::SetCurrentHour24(float NewHour)` (legacy)
- `AEnvironmentTimeOfDayActor::GetCurrentHour24() const` (legacy)
- `AEnvironmentTimeOfDayActor::SetTimeProgressionEnabled(bool bEnabled)` (legacy)
- `AEnvironmentTimeOfDayActor::ApplyPreviewTime()` (legacy)
- `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged` (legacy)
- `AEnvironmentTimeOfDayActor::EvaluateCurrentVisualState() const` (legacy)
- `ITimeOfDayProvider::GetCurrentHour24`
- `UGameTimeBucketSubsystem::SetTimeOfDayProvider`
- `UGameTimeBucketSubsystem::SetTimeOfDayActor`
- `UGameTimeBucketSubsystem::RegisterListener`
- `UGameTimeBucketSubsystem::UnregisterListener`
- `UGameTimeBucketSubsystem::RefreshListeners`
- `IGameTimeBucketListener::GetGameTimeBucketSubscriptions`
- `IGameTimeBucketListener::OnGameTimeBucketEvent`

위 이름이 Blueprint에 노출된 뒤에는 Blueprint 계약으로 취급한다. rename/delete 시 Blueprint 참조 검사와 Core Redirect 필요 여부를 먼저 검토한다.

## Dependencies

- Core
- WorldActors는 레벨 composition 관계로만 연결하며 C++ 의존성은 두지 않는다.
- 벌떼 SpawnAmount 연동은 concrete time actor를 WorldActors가 직접 include/탐색하지 않고, `UGameTimeBucketSubsystem` + listener interface 경로로 연결한다.

- Environment system은 Character, Camera, Focus, Interaction, Inventory, UI에 의존하지 않는다.
- Character/Controller는 runtime clock 표시를 위해 Environment actor를 참조하지만, 이 의존은 Character 쪽 단방향 UI binding이다.

## Design Notes

- Environment의 runtime 시간 authority는 `AGameTimeOfDayActor`다. gameplay actor나 UI가 별도 시간 흐름을 소유하지 않는다.
- Environment의 visual authority는 `ADynamicSky`다. sky/light/fog actor가 별도 runtime clock을 진행하면 안 된다.
- `ABeehiveDualSwarmActor`는 환경 concrete class를 모른 채 상위 actor(`ABeehive`)가 계산한 최종 parameter를 전달받는다.
- `AEnvironmentTimeOfDayActor`는 transition 호환을 위해 BeginPlay에서 provider 등록을 시도하되, `AGameTimeOfDayActor`가 존재하면 canonical source를 덮어쓰지 않는다.
- 시간대별 연출 값은 curve asset 데이터로 authoring하고, C++은 평가/적용만 담당한다.
- 새벽 안개, 아침 주황, 낮 하늘색, 저녁 노을, 밤 남색은 `Sky*ColorCurve`와 `FogDensityCurve`로 표현한다.
- 태양/달 활성 정책은 light intensity curve만 믿지 않고 지평선 판정으로 한 번 더 강제한다.
- 이후 농작물 성장, 벌 활동량, 야간 이벤트 같은 gameplay hook은 light actor를 polling하지 말고 `ITimeOfDayProvider` 또는 별도 time event를 구독한다.
- Gameplay actor의 분 단위 반응은 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem`을 우선 사용한다. Environment actor를 직접 찾아 polling하지 않는다.
- Warning 로그는 반복 스팸을 막기 위해 참조별 1회 출력 플래그를 사용한다.

## Manual Review Points

- 레벨에 활성 `AGameTimeOfDayActor`가 1개만 있는지 확인한다.
- legacy `AEnvironmentTimeOfDayActor`가 함께 있어도 bucket/clock provider는 `AGameTimeOfDayActor`를 우선 사용하는지 확인한다.
- `ADynamicSky`가 시간 값을 직접 증가시키지 않고 provider delegate만 소비하는지 확인한다.
- 태양/달 directional light가 다른 Blueprint Tick에서 동시에 제어되지 않는지 확인한다.
- sky material parameter 이름이 C++에서 쓰는 이름과 일치하는지 확인한다.
- curve wraparound에서 `24.0 -> 0.0` 전환이 튀지 않는지 확인한다.
- 새벽 fog density가 낮에 정상적으로 감소하는지 확인한다.
- 태양이 지평선 아래일 때 sun light가 남지 않는지 확인한다.
- 태양이 올라왔을 때 moon light가 꺼지는지 확인한다.
- editor preview가 의도치 않게 level asset을 더럽히는지 확인한다.
- runtime-spawned bucket listener가 register/unregister를 수행하는지 확인한다.

## Time Clock UI Integration

- Runtime clock UI owner는 controller를 통해 provider 값을 주입받는다. (`AGameTimeOfDayActor::OnGameTimeOfDayChanged` 우선)
- Clock UI는 `UGameTimeBucketSubsystem`을 사용하지 않는다. Bucket dispatch는 gameplay bucket logic 전용이다.
- 현재 흐름은 `ABeekeeperController`가 `ITimeOfDayProvider` actor를 resolve하고 `UTimeOfDayClockWidget`에 `Hour24`를 주입하는 방식이다.

## Update 2026-05-24 (DynamicSky/GameTimeOfDay Split)

- New `ITimeOfDayProvider` interface is the canonical runtime time-source contract.
- New `AGameTimeOfDayActor` owns:
  - `Hour24` source of truth
  - day-length progression
  - `OnGameTimeOfDayChanged(float Hour24)` delegate broadcast
- New `ADynamicSky` owns environment visual components and applies visuals from provider hour only.
- `ADynamicSky` configures its directional light components for SkyAtmosphere ownership:
  - `SunDirectionalLight`: `AtmosphereSunLight=true`, `AtmosphereSunLightIndex=0`
  - `MoonDirectionalLight`: `AtmosphereSunLight=true`, `AtmosphereSunLightIndex=1`
- `ADynamicSky` does not toggle Sun/Moon directional light component visibility at runtime.
- Sky sphere material visibility parameters:
  - `IsStarVisible` follows the sun-light presence window.
  - `IsMoonVisible` is always set to `1.0`.
- `ADynamicSky` can optionally use its editor preview hour as the runtime start hour when `bUseEditorPreviewTime` and `bStartGameTimeFromPreviewHour` are both true.
  - Canonical path: `AGameTimeOfDayActor` resolves `ADynamicSky::PreviewHour24` before its first time broadcast.
  - Legacy path: `ADynamicSky` pushes `PreviewHour24` to `AEnvironmentTimeOfDayActor` on BeginPlay.
- `UGameTimeBucketSubsystem` now binds providers through `SetTimeOfDayProvider(AActor*)`.
- `AEnvironmentTimeOfDayActor` is retained as transition-compatible actor; existing public Blueprint APIs remain available.

## Update 2026-05-24 (DynamicSky SkyAtmosphere Scattering Simplification)

- `ADynamicSky` no longer uses curve assets for SkyAtmosphere `RayleighScattering` and `MultiScattering`.
- DynamicSky scattering source of truth is four Details-panel values:
  - `SunLightRayleighScattering` (`FLinearColor`)
  - `NoSunLightRayleighScattering` (`FLinearColor`)
  - `SunLightMultiScattering` (`float`)
  - `NoSunLightMultiScattering` (`float`)
- The same `GapTime` used by the sun-light presence window is also the scattering transition width.
- Sun/Moon world rotation uses unclamped extrapolation outside the canonical orbit range:
  - Sun: `SunriseHour` to `SunsetHour` maps exactly to `0 -> -180` degrees.
  - Sun gap intervals continue the same angular velocity outside that range.
  - Moon: `SunsetHour` to next `SunriseHour` maps exactly to `0 -> -180` degrees.
  - Moon gap intervals continue the same angular velocity outside that range.
- Sunrise transition:
  - `SunriseHour - GapTime` to `SunriseHour + GapTime`
  - interpolate from no-sunlight values to sunlight values.
- Sunset transition:
  - `SunsetHour - GapTime` to `SunsetHour + GapTime`
  - interpolate from sunlight values to no-sunlight values.
- Stable sunlight window uses sunlight values.
- Stable non-sunlight window uses no-sunlight values.
- Removed DynamicSky curve contract:
  - `RayleighScatteringCurve`
  - `MultiScatteringCurve`
  - curve fallback values and curve evaluation logic.
