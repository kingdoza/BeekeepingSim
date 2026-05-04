## 10. Environment 24시간 하늘/조명 시스템 수동 연결

### 대상 C++ 클래스

- `AEnvironmentTimeOfDayActor`
- `FTimeOfDayCurveSet`
- `FTimeOfDayVisualState`

### 사용자가 할 일

1. 레벨에 `AEnvironmentTimeOfDayActor`를 1개 배치한다.
2. `DirectionalLight` 2개를 배치하고 각각 Sun/Moon으로 세팅한 뒤 `SunLight`, `MoonLight` 참조에 연결한다.
3. `SkyLight`, `ExponentialHeightFog` 액터를 참조에 연결한다.
4. `Material Parameter Collection` 에셋을 생성하고 아래 파라미터를 추가한다.
   - `SkyZenithColor` (Vector)
   - `SkyHorizonColor` (Vector)
   - `SunDirection` (Vector)
   - `MoonDirection` (Vector)
   - `SunIntensity` (Scalar)
   - `MoonIntensity` (Scalar)
   - `FogDensity` (Scalar)
   - `AmbientIntensity` (Scalar)
5. sky material/Blueprint에서 위 MPC 파라미터를 읽어 하늘 색/방향/강도를 반영하도록 연결한다.
6. 시간대 curve asset을 생성해 `CurveSet`에 연결한다. (없으면 fallback으로 동작)
7. 에디터에서 `PreviewHour24`를 `0`, `5.5`, `8`, `12`, `18`, `21`로 바꿔 `ApplyPreviewTime()` 실행 후 시각 상태를 확인한다.
8. `DayLengthSeconds`를 낮게 설정한 뒤 PIE에서 시간이 24시간 wrap되는지 확인한다.
9. 각 시간 반응 actor를 Environment delegate에 직접 bind하지 않고, Game Time Bucket listener/subsystem 경로를 사용한다.

### 확인 포인트

- 태양이 떠 있는 동안 moon light intensity가 0인지
- 태양이 지면 moon light가 켜지는지
- 새벽/낮/저녁에서 fog density, ambient, sky color가 curve 또는 fallback대로 바뀌는지
- `bRecaptureSkyLightOnApply`를 켠 경우에만 recapture가 발생하는지

## 11. Beehive Spline Bee Swarm 수동 연결

### 대상 C++ 클래스

- `ABeehiveDualSwarmActor`
- `ABeehive`

### 사용자가 할 일

1. `BP_BeehiveDualSwarmActor`를 만들고 parent class를 `ABeehiveDualSwarmActor`로 지정한다.
2. Niagara User Parameter 이름을 아래와 같이 정확히 맞춘다.
   - `User.SwarmSpline` (Object)
   - `User.SplineLength` (Float)
   - `User.StartShapeExtent` (Vector3)
   - `User.EndShapeExtent` (Vector3)
   - `User.SpawnAmount` (Float)
   - `User.SpeedMin` (Float)
   - `User.SpeedMax` (Float)
   - `User.bIsReverse` (Bool)
3. `OutgoingNiagara`, `IngoingNiagara` 컴포넌트에 Niagara System asset을 지정한다.
4. Niagara Spline Data Interface의 `Spline User Parameter`를 `User.SwarmSpline`으로 지정한다.
5. `BP_Beehive`의 `BeeSplineSwarmActorClass`를 `BP_BeehiveDualSwarmActor`로 지정한다.
6. `BP_Beehive` 인스턴스의 `SwarmSpline` point를 레벨마다 원하는 경로로 편집한다.
7. `BP_Beehive`에서 `ColonyBeeCount`, `DualSwarmCommonSettings`, `OutgoingSwarmSettings`, `IngoingSwarmSettings`, `BeeSwarmHour24`를 설정한다.
8. Environment 시간 변경 delegate(예: `OnTimeOfDayChanged`)를 Blueprint에서 받아 `ABeehive::ApplyBeeSwarmHour24()`를 호출하도록 연결한다.
9. `ABeehiveDualSwarmActor` 및 해당 actor 소유 Niagara component details에서 User Parameter override 편집 UI(`OverrideParameters`)가 숨겨졌는지 확인한다.
10. `BP_Beehive`의 `BeeSwarmBucketMinutes`(기본 10)와 `bApplyBeeSwarmOnBeginPlayBucket` 설정을 확인한다.

### 확인 포인트

- `BP_Beehive` 인스턴스마다 `SwarmSpline`을 다르게 편집했을 때 Outgoing/Ingoing Niagara가 그 인스턴스 spline을 공유하는지
- Outgoing에 `User.bIsReverse=false`, Ingoing에 `User.bIsReverse=true`가 적용되는지
- actor details와 component details 양쪽에서 User Parameter override 편집 UI가 숨겨지는지
- `ABeehive` 설정 변경 시 child dual swarm actor의 양쪽 Niagara에 계산된 `SpawnAmount`가 즉시 적용되는지
- `ApplyBeeSwarmHour24()` 반복 호출 시 child actor instance가 재생성되지 않는지
- `ColonyBeeCount = 0`일 때 양쪽 spawn amount가 0인지
- `MaxSpawnAmount` clamp가 적용되는지
- spline point 자동 생성/삭제/재배치가 발생하지 않는지
- BeginPlay 즉시 1회 + 이후 n분 bucket 경계에서만 `ApplyBeeSwarmHour24`가 호출되는지
## Time Clock Widget Setup (Manual)

1. Create `WBP_TimeOfDayClock` and set parent class to `UTimeOfDayClockWidget`.
2. Add `TextBlock_Time` and implement `OnDisplayedTimeChanged(NewTimeText, Hour, Minute)`.
3. Set `TextBlock_Time` text from `NewTimeText`.
4. Assign `TimeOfDayClockWidgetClass` on `BP_BeekeeperController`.
5. Ensure exactly one `AEnvironmentTimeOfDayActor` (or its BP child) exists in level.
## Beehive Attraction Swarm Setup (Manual)

1. Open `BP_Beehive`.
2. Set Niagara system asset on `AttractionSwarmNiagara` component.
3. Move `AttractionSwarmNiagara` component to place attraction center.
4. Configure `AttractionSwarmSettings`:
   - `AttractionPower`
   - `NoisePower`
   - `SpawnSphereRadius`
   - `SpawnAmountScale`
   - `MaxSpawnAmount`
5. Verify Niagara user parameter types:
   - `User.AttractionPower` Float
   - `User.NoisePower` Float
   - `User.SpawnSphereRadius` Float
   - `User.SpawnAmount` Int32
