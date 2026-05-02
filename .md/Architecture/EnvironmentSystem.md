# Environment System

## Scope

- Source 폴더:
  - `Source/BeekeepingSim/Public/Environment`
  - `Source/BeekeepingSim/Private/Environment`
- 핵심 파일:
  - `TimeOfDayTypes.h`
  - `EnvironmentTimeOfDayActor.h`
  - `EnvironmentTimeOfDayActor.cpp`

## Responsibilities

- 24시간 가속 월드 시간의 환경 연출 기준점 제공
- 시간대별 하늘 색상, 전체 조명, 색온도, 안개 밀도 평가
- 태양과 달의 동쪽-서쪽 천구 궤도 계산
- 태양이 지평선 아래로 내려갔을 때 달빛 활성화, 태양 상승 시 달빛 비활성화
- 에디터에서 시각 슬라이더로 결과를 미리 보는 preview 경로 제공
- 레벨 디자이너가 light/fog/sky actor와 curve asset을 연결할 수 있는 최소 Blueprint 계약 제공

## Key Classes

- `AEnvironmentTimeOfDayActor`: 레벨에 1개 배치되는 환경 director
  - 가속 시간 상태를 소유한다.
  - 태양/달 light, skylight, fog, sky material parameter를 참조한다.
  - 단일 `Hour24` 값에서 모든 연출 값을 평가하고 적용한다.
  - `OnTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)`를 broadcast한다.
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
  - `AEnvironmentTimeOfDayActor::OnTimeOfDayChanged`를 구독한다.
  - listener subscription별로 00:00 기준 n분 bucket 경계 변경 시점에만 이벤트를 발행한다.
  - `LatestOnly` / `CatchUp` 정책과 BeginPlay 즉시 적용 옵션을 처리한다.

## Runtime Flow

1. `AEnvironmentTimeOfDayActor`는 runtime time progression이 켜져 있을 때만 Tick에서 시간을 진행한다.
2. `DeltaTime`과 `DayLengthSeconds`로 `CurrentHour24`를 wrap 갱신한다.
3. `CurrentHour24 / 24.0`을 normalized time으로 변환한다.
4. curve set을 평가해 `FTimeOfDayVisualState`를 만든다.
5. `SkyParameterCollection`이 설정된 경우 material parameter collection instance를 갱신한다.
6. 태양/달 directional light의 회전, intensity, color temperature를 적용한다.
7. skylight/ambient 값과 exponential height fog density를 적용한다.
8. `OnTimeOfDayChanged` Blueprint delegate를 broadcast한다.
9. `UGameTimeBucketSubsystem`은 매 broadcast를 그대로 전달하지 않고 bucket 경계 변경 시점만 listener에 dispatch한다.

## Time Model

- `CurrentHour24`가 환경 시간의 단일 기준 값이다.
- 유효 범위는 normalize 기준 `[0.0, 24.0)`이다.
- `DayLengthSeconds`는 하루 길이를 실제 초 단위로 정의하며 `1.0` 미만이면 Tick 진행을 중단하고 warning을 1회 출력한다.
- 예시: `DayLengthSeconds = 600`이면 게임 내 하루가 실제 10분에 진행된다.
- runtime clock은 24시간에서 wrap된다.
- 하늘/조명/안개/태양/달 상태는 항상 `CurrentHour24`에서 결정되어야 하며, 개별 actor가 별도 시간을 누적하면 안 된다.
- bucket 경계 계산도 같은 시간 기준을 사용한다:
  - `MinuteOfDay = FloorToInt(NormalizeHour24(Hour24) * 60)`
  - `BucketIndex = MinuteOfDay / BucketMinutes`
  - 마지막 bucket은 `Min(BucketStart + BucketMinutes, 1440)`로 짧아질 수 있다.

## Sky And Lighting Curves

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

태양과 달은 같은 천구 궤도 계산을 사용한다.

태양 기본 정책:

- `SunriseHour = 6.0`
- `SunsetHour = 18.0`
- 아침에는 동쪽 지평선에서 떠오른다.
- 정오에는 `MaxSunAltitudeDegrees`에 도달한다. (`FRotator(-Altitude, Yaw, 0)`)
- 저녁에는 서쪽 지평선으로 진다.
- 태양이 지평선 아래에 있으면 curve 값과 무관하게 sun light intensity를 0으로 강제한다.

달 기본 정책:

- `MoonHour = CurrentHour24 + 12.0`, 24시간 wrap 적용
- 달은 태양과 반대 위상에서 같은 궤도를 이동한다.
- 달은 `MoonHour = CurrentHour24 + 12.0 (wrap)`으로 같은 궤도 함수를 평가한다.
- moon light 활성 조건은 `!bSunAboveHorizon && bMoonAboveHorizon`이다.
- 태양이 올라오면 moon light intensity를 0으로 강제한다.

## Scene Actor Binding

`AEnvironmentTimeOfDayActor`는 다음 참조를 노출한다.

- `ADirectionalLight* SunLight`
- `ADirectionalLight* MoonLight`
- `ASkyLight* SkyLight`
- `AExponentialHeightFog* HeightFog`
- `UMaterialParameterCollection* SkyParameterCollection`

권장 sky 연결 방식:

- C++은 `UMaterialParameterCollection`에 sky color와 sun/moon direction을 쓴다.
- sky material 또는 sky Blueprint는 해당 parameter collection을 읽는다.
- 특정 sky mesh Blueprint 클래스에 C++이 직접 의존하지 않는다.

권장 MPC parameter 이름:

- `SkyZenithColor`
- `SkyHorizonColor`
- `SunDirection`
- `MoonDirection`
- `SunIntensity`
- `MoonIntensity`
- `FogDensity`
- `AmbientIntensity`

## Editor Preview

에디터 프리뷰용 property/API:

- `bUseEditorPreviewTime`
- `PreviewHour24`: `0.0-24.0` slider
- `ApplyPreviewTime()`: `CallInEditor`
- `bUpdatePreviewOnPropertyChange`
- `bRecaptureSkyLightOnApply`: opt-in

프리뷰 동작:

1. 에디터에서 `bUseEditorPreviewTime`이 true이면 `PreviewHour24`를 기준으로 visual state를 평가한다.
2. slider 변경 또는 `ApplyPreviewTime()` 호출 시 runtime과 동일한 평가/적용 로직을 사용한다.
3. 프리뷰는 runtime clock을 진행하지 않는다.
4. 프리뷰는 scene actor를 생성/삭제하지 않는다.
5. 프리뷰는 연결된 light/fog/sky material parameter만 갱신한다.

## Blueprint/API Contracts

초기 Blueprint-safe API:

- `SetCurrentHour24(float NewHour)`
- `GetCurrentHour24() const`
- `SetTimeProgressionEnabled(bool bEnabled)`
- `ApplyPreviewTime()`
- `OnTimeOfDayChanged`
- `EvaluateCurrentVisualState() const`

위 이름이 Blueprint에 노출된 뒤에는 Blueprint 계약으로 취급한다. rename/delete 시 Blueprint 참조 검사와 Core Redirect 필요 여부를 먼저 검토한다.

## Dependencies

- Core
- WorldActors는 레벨 composition 관계로만 연결하며 C++ 의존성은 두지 않는다.
- 벌떼 SpawnAmount 연동은 `AEnvironmentTimeOfDayActor`를 WorldActors가 직접 include/탐색하지 않고, `UGameTimeBucketSubsystem` + listener interface 경로로 연결한다.

Environment system은 Character, Camera, Focus, Interaction, Inventory, UI에 의존하지 않는다.

## Design Notes

- Environment는 레벨 전역 연출 authority다. gameplay actor나 UI가 별도 시간 흐름을 소유하지 않는다.
- `ABeehiveDualSwarmActor`는 환경 concrete class를 모른 채 상위 actor(`ABeehive`)가 계산한 최종 parameter를 전달받는다.
- `AEnvironmentTimeOfDayActor`는 BeginPlay에서 `UGameTimeBucketSubsystem::SetTimeOfDayActor(this)`를 호출해 수동 지정 우선 경로를 제공한다.
- 시간대별 연출 값은 curve asset 데이터로 authoring하고, C++은 평가/적용만 담당한다.
- 새벽 안개, 아침 주황, 낮 하늘색, 저녁 노을, 밤 남색은 `Sky*ColorCurve`와 `FogDensityCurve`로 표현한다.
- 태양/달 활성 정책은 light intensity curve만 믿지 않고 지평선 판정으로 한 번 더 강제한다.
- 이후 농작물 성장, 벌 활동량, 야간 이벤트 같은 gameplay hook은 light actor를 polling하지 말고 `OnTimeOfDayChanged` 또는 별도 time event를 구독한다.
- Warning 로그는 반복 스팸을 막기 위해 참조별 1회 출력 플래그를 사용한다.

## Manual Review Points

- 레벨에 활성 `AEnvironmentTimeOfDayActor`가 1개만 있는지 확인한다.
- 태양/달 directional light가 다른 Blueprint Tick에서 동시에 제어되지 않는지 확인한다.
- sky material parameter 이름이 C++에서 쓰는 이름과 일치하는지 확인한다.
- curve wraparound에서 `24.0 -> 0.0` 전환이 튀지 않는지 확인한다.
- 새벽 fog density가 낮에 정상적으로 감소하는지 확인한다.
- 태양이 지평선 아래일 때 sun light가 남지 않는지 확인한다.
- 태양이 올라왔을 때 moon light가 꺼지는지 확인한다.
- editor preview가 의도치 않게 level asset을 더럽히는지 확인한다.
## Bucket Payload Note

- For catch-up dispatch, `bWrappedDay` is true only on the midnight boundary bucket event (`BucketStartMinute == 0`).
- Bucket minute conversion uses `FloorToInt(NormalizeHour24(Hour24) * 60.0f)` with no epsilon offset.
