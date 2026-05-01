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

### 확인 포인트

- 태양이 떠 있는 동안 moon light intensity가 0인지
- 태양이 지면 moon light가 켜지는지
- 새벽/낮/저녁에서 fog density, ambient, sky color가 curve 또는 fallback대로 바뀌는지
- `bRecaptureSkyLightOnApply`를 켠 경우에만 recapture가 발생하는지
