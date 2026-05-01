# 구현 프롬프트: Environment 24시간 하늘/조명/태양/달 시스템

## 배경

BeekeepingSim에 24시간 가속 시간에 연동되는 환경 연출 시스템을 추가한다.

현재 Source에는 시간/하늘/태양/달 전용 시스템이 없다. 정본 아키텍처 문서에는 신규 `Environment` 시스템이 추가되어 있으며, 구현은 이 문서의 경계를 따른다.

반드시 먼저 읽을 문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 구현 목표

1. `CurrentHour24` 기반 24시간 가속 시간 시스템을 추가한다.
2. 시간에 따라 하늘 색상, 태양광, 달빛, ambient/skylight, 안개 밀도가 자연스럽게 변하도록 한다.
3. 태양은 아침 동쪽, 정오 정점, 저녁 서쪽으로 이동한다.
4. 달은 태양과 반대 위상으로 움직이고, 태양이 지평선 아래일 때만 달빛을 활성화한다.
5. 에디터에서 `PreviewHour24` 슬라이더와 `ApplyPreviewTime()`으로 결과를 미리 볼 수 있게 한다.
6. Content asset을 자동 생성하거나 수정하지 않는다. 필요한 curve/material/level 연결은 사용자 수동 작업으로 보고한다.

## 작업 범위

생성할 Source 폴더:

- `Source/BeekeepingSim/Public/Environment`
- `Source/BeekeepingSim/Private/Environment`

생성할 파일:

- `Source/BeekeepingSim/Public/Environment/TimeOfDayTypes.h`
- `Source/BeekeepingSim/Public/Environment/EnvironmentTimeOfDayActor.h`
- `Source/BeekeepingSim/Private/Environment/EnvironmentTimeOfDayActor.cpp`

필요 시 갱신할 문서:

- `.md/Architecture/EnvironmentSystem.md`
- `.md/0_ARCHITECTURE.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

## 금지 작업

- 기존 시스템의 UCLASS/USTRUCT/UENUM rename
- 기존 Blueprint API 삭제 또는 시그니처 변경
- `Content/` asset 자동 생성/수정
- `Config/DefaultEngine.ini` Core Redirect 임의 추가
- Character, Camera, Focus, Interaction, Inventory, UI, WorldActors 시스템에 불필요한 의존성 추가
- 특정 sky Blueprint 클래스에 C++을 직접 의존시키기
- `#include "Public/..."` 형태 include 추가

## 핵심 타입 설계

### `FTimeOfDayCurveSet`

`USTRUCT(BlueprintType)`로 작성한다.

권장 property:

- `TObjectPtr<UCurveLinearColor> SkyZenithColorCurve`
- `TObjectPtr<UCurveLinearColor> SkyHorizonColorCurve`
- `TObjectPtr<UCurveFloat> SunIntensityCurve`
- `TObjectPtr<UCurveFloat> SunTemperatureCurve`
- `TObjectPtr<UCurveFloat> MoonIntensityCurve`
- `TObjectPtr<UCurveFloat> MoonTemperatureCurve`
- `TObjectPtr<UCurveFloat> FogDensityCurve`
- `TObjectPtr<UCurveFloat> AmbientIntensityCurve`

모든 curve는 optional이어야 한다. curve가 비어 있으면 C++ fallback 값을 사용해 actor가 crash 없이 동작해야 한다.

### `FTimeOfDayVisualState`

`USTRUCT(BlueprintType)`로 작성한다.

권장 property:

- `float Hour24`
- `float NormalizedTime`
- `FLinearColor SkyZenithColor`
- `FLinearColor SkyHorizonColor`
- `float SunIntensity`
- `float SunTemperature`
- `float MoonIntensity`
- `float MoonTemperature`
- `float FogDensity`
- `float AmbientIntensity`
- `FRotator SunRotation`
- `FRotator MoonRotation`
- `bool bSunAboveHorizon`
- `bool bMoonLightActive`

## `AEnvironmentTimeOfDayActor` 요구사항

### Class 기본

- `UCLASS(Blueprintable)` actor로 구현한다.
- Tick은 runtime time progression이 켜진 경우에만 실질 작업을 수행한다.
- 생성자에서 `PrimaryActorTick.bCanEverTick = true`로 둔다.
- `ShouldTickIfViewportsOnly()`는 필요 시 editor preview를 위해 override할 수 있으나, preview가 매 프레임 강제 갱신되지 않도록 주의한다.

### 시간 property

필수 property:

- `CurrentHour24`
  - `EditAnywhere`, `BlueprintReadOnly`
  - `ClampMin=0.0`, `ClampMax=24.0`, `UIMin=0.0`, `UIMax=24.0`
- `DayLengthSeconds`
  - `EditAnywhere`, `BlueprintReadOnly`
  - `ClampMin=1.0`
- `bTimeProgressionEnabled`
  - `EditAnywhere`, `BlueprintReadOnly`
- `SunriseHour`
  - 기본값 `6.0`
- `SunsetHour`
  - 기본값 `18.0`
- `MaxSunAltitudeDegrees`
  - 기본값 예: `70.0`
- `CelestialYawOffsetDegrees`
  - 동서 방향 보정용

시간 갱신:

```text
HoursPerSecond = 24.0 / DayLengthSeconds
CurrentHour24 = Wrap(CurrentHour24 + DeltaTime * HoursPerSecond, 0.0, 24.0)
```

### Scene actor reference

필수 reference property:

- `TObjectPtr<ADirectionalLight> SunLight`
- `TObjectPtr<ADirectionalLight> MoonLight`
- `TObjectPtr<ASkyLight> SkyLight`
- `TObjectPtr<AExponentialHeightFog> HeightFog`
- `TObjectPtr<UMaterialParameterCollection> SkyParameterCollection`

가능하면 actor 자체보다 component에 접근해 값을 적용한다.

필요 include는 Unreal 표준 경로를 사용한다. 예:

- `Engine/DirectionalLight.h`
- `Engine/SkyLight.h`
- `Engine/ExponentialHeightFog.h`
- `Materials/MaterialParameterCollection.h`
- `Materials/MaterialParameterCollectionInstance.h`

### Curve/property

필수 property:

- `FTimeOfDayCurveSet CurveSet`
- fallback sky zenith/horizon color
- fallback sun/moon intensity
- fallback sun/moon temperature
- fallback fog density
- fallback ambient intensity

curve가 null이면 fallback 값을 사용한다.

## 하늘/조명 적용 요구사항

### Sky material parameter collection

`SkyParameterCollection`이 설정되어 있으면 world의 collection instance에 값을 쓴다.

권장 parameter 이름:

- `SkyZenithColor`
- `SkyHorizonColor`
- `SunDirection`
- `MoonDirection`
- `SunIntensity`
- `MoonIntensity`
- `FogDensity`
- `AmbientIntensity`

주의:

- parameter가 collection asset에 없을 수 있으므로 실패해도 crash하지 않게 처리한다.
- 실패 시 `UE_LOG` warning은 허용하되 Tick마다 spam하지 않도록 한다.

### Directional light

Sun:

- `SunLight` rotation을 `SunRotation`으로 설정한다.
- `bSunAboveHorizon == false`면 intensity를 0으로 강제한다.
- `bSunAboveHorizon == true`면 `SunIntensityCurve` 또는 fallback intensity를 적용한다.
- color temperature를 적용한다.

Moon:

- `MoonLight` rotation을 `MoonRotation`으로 설정한다.
- `bMoonLightActive == true`일 때만 curve/fallback intensity를 적용한다.
- 태양이 지평선 위이면 moon intensity를 0으로 강제한다.
- color temperature를 적용한다.

### SkyLight / ambient

- `AmbientIntensityCurve` 결과를 skylight intensity scale에 적용한다.
- SkyLight recapture는 매 Tick 호출하지 않는다.
- 필요하면 `bRecaptureSkyLightOnApply` 같은 opt-in bool을 두고 editor/manual preview에서만 사용한다.

### Fog

- `HeightFog`가 있으면 `FogDensityCurve` 결과를 exponential height fog component의 fog density에 적용한다.
- 새벽에 짙고 낮에 걷히는 연출은 curve 데이터로 조정 가능해야 한다.

## 태양/달 궤도 요구사항

태양:

- `SunriseHour`부터 `SunsetHour`까지 horizon 위로 판단한다.
- sunrise에서는 동쪽 지평선, noon에서는 최고 고도, sunset에서는 서쪽 지평선이 되게 rotation을 계산한다.
- 낮 구간 밖에서는 지평선 아래 rotation을 계산해도 되지만 sun intensity는 반드시 0이다.

달:

- `MoonHour = CurrentHour24 + 12.0`을 24시간 wrap한다.
- 같은 궤도 함수를 moon hour로 평가한다.
- 태양과 반대편에서 떠오르는 구조여야 한다.
- 태양이 horizon 위로 올라오면 moon intensity는 0이다.

계산 함수는 private helper로 분리한다.

권장 helper:

- `float NormalizeHour(float Hour) const`
- `float GetNormalizedTime(float Hour) const`
- `bool IsHourInDaylight(float Hour) const`
- `FRotator CalculateCelestialRotation(float Hour, bool& bOutAboveHorizon) const`
- `FTimeOfDayVisualState EvaluateVisualState(float Hour) const`
- `void ApplyVisualState(const FTimeOfDayVisualState& State)`

## Editor preview 요구사항

필수 property/API:

- `bUseEditorPreviewTime`
- `PreviewHour24`
  - `EditAnywhere`
  - `ClampMin=0.0`, `ClampMax=24.0`, `UIMin=0.0`, `UIMax=24.0`
- `bUpdatePreviewOnPropertyChange`
- `ApplyPreviewTime()`
  - `UFUNCTION(CallInEditor, BlueprintCallable)`

구현 방식:

- `ApplyPreviewTime()`은 `PreviewHour24`로 `EvaluateVisualState()` 후 `ApplyVisualState()`를 호출한다.
- editor property 변경 시 `PostEditChangeProperty`에서 `bUpdatePreviewOnPropertyChange`가 true면 preview를 갱신한다.
- `PostEditChangeProperty` 코드는 `#if WITH_EDITOR`로 감싼다.
- preview는 runtime clock을 진행하지 않는다.
- preview는 actor 생성/삭제를 하지 않는다.

## Blueprint/API 계약

필수 UFUNCTION:

- `void SetCurrentHour24(float NewHour)`
- `float GetCurrentHour24() const`
- `void SetTimeProgressionEnabled(bool bEnabled)`
- `void ApplyPreviewTime()`
- `FTimeOfDayVisualState EvaluateCurrentVisualState() const`

필수 delegate:

- `OnTimeOfDayChanged`
  - `BlueprintAssignable`
  - 인자: `float Hour24`, `const FTimeOfDayVisualState& VisualState`

주의:

- 위 API는 노출 후 Blueprint 계약이 된다.
- 이름을 바꾸지 않는다.
- 구현 중 더 좋은 이름이 떠올라도 변경하지 말고 QnA에 남긴다.

## 로깅

새 log category를 추가한다.

권장:

- header: `DECLARE_LOG_CATEGORY_EXTERN(LogBeekeepingEnvironment, Log, All);`
- cpp: `DEFINE_LOG_CATEGORY(LogBeekeepingEnvironment);`

경고 상황:

- Light/Fog/Sky reference가 비어 있는데 apply를 호출한 경우
- DayLengthSeconds가 비정상 값인 경우
- Material parameter collection instance 획득 실패

Tick마다 같은 warning을 반복하지 않는다.

## 문서 반영

구현 후 `.md/Architecture/EnvironmentSystem.md`를 실제 구현과 맞춘다.

확인할 항목:

- 파일명
- class/struct 이름
- Blueprint API 이름
- preview property 이름
- scene actor binding 방식
- 수동 editor 작업

`Content/` 연결이 필요하면 `.md/USER_UNREAL.md`에 사용자 작업을 작성한다.

필수 수동 작업 예시:

- 레벨에 `AEnvironmentTimeOfDayActor` 배치
- Sun/Moon `DirectionalLight` 배치 및 참조 연결
- `SkyLight`, `ExponentialHeightFog` 참조 연결
- `UMaterialParameterCollection` 생성 및 parameter 이름 추가
- sky material/Blueprint에서 parameter collection 읽기
- curve asset 생성 후 `CurveSet`에 연결

## 검증 명령

검색:

```powershell
rg '#include "Public/' Source/BeekeepingSim
rg "EnvironmentTimeOfDayActor|TimeOfDay" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md
```

빌드:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

수동 검증 필요:

- Editor에서 `PreviewHour24`를 0, 5.5, 8, 12, 18, 21로 변경해 하늘/조명/안개/태양/달 상태 확인
- 태양이 떠 있는 동안 moon light intensity가 0인지 확인
- 태양이 지면 moon light가 활성화되는지 확인
- `DayLengthSeconds`를 낮게 설정해 runtime 가속 시간이 wrap되는지 확인

## 완료 기준

- `Environment` Source 폴더와 3개 핵심 파일이 생성된다.
- C++ 빌드가 통과한다.
- `AEnvironmentTimeOfDayActor`가 Blueprint/Editor에서 배치 가능하다.
- `CurrentHour24`와 `DayLengthSeconds`로 runtime 시간이 진행된다.
- curve/fallback 기반으로 sky/light/fog 값이 평가된다.
- 태양/달 rotation과 light 활성 정책이 구현된다.
- `PreviewHour24`와 `ApplyPreviewTime()`으로 에디터 프리뷰가 가능하다.
- Content asset 자동 수정 없이 수동 작업이 문서화된다.
- 구조 변경 내용이 아키텍처 문서에 반영된다.
- `.md/PROMPT_REVIEW.md`에 리뷰 프롬프트가 작성된다.
