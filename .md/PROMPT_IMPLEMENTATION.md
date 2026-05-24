# DynamicSky SkyAtmosphere Scattering 수정 구현 프롬프트

## 목표

이미 구현된 `ADynamicSky`의 SkyAtmosphere `RayleighScattering`, `MultiScattering` 설정 방식을 변경한다.

현재 방식:

- `RayleighScatteringCurve`
- `MultiScatteringCurve`
- `FallbackRayleighScattering`
- `FallbackMultiScattering`
- `Hour24 / 24.0` 기반 curve 평가

변경할 방식:

- curve를 사용하지 않는다.
- Details 패널에서 조절하는 4개 값으로 SkyAtmosphere scattering을 결정한다.
  - RayleighScattering 2개는 `FLinearColor`
  - MultiScattering 2개는 `float`
- 일출/일몰 주변 `GapTime` 구간에서는 두 상태 값을 보간한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

## 구현 범위

수정 대상:

- `Source/BeekeepingSim/Public/Environment/DynamicSky.h`
- `Source/BeekeepingSim/Private/Environment/DynamicSky.cpp`
- 필요 시 `Source/BeekeepingSim/Public/Environment/DynamicSkyTypes.h`
- 문서:
  - `.md/Architecture/EnvironmentSystem.md`
  - `.md/USER_UNREAL.md`

수정하지 말 것:

- `AGameTimeOfDayActor`
- `ITimeOfDayProvider`
- `UGameTimeBucketSubsystem`
- `ABeekeeperController`
- 기존 `AEnvironmentTimeOfDayActor` legacy curve 구조
- Content asset
- Config

## 확정 요구사항

`ADynamicSky`에서 아래 property와 관련 로직을 제거한다.

```cpp
RayleighScatteringCurve
MultiScatteringCurve
FallbackRayleighScattering
FallbackMultiScattering
```

아래 4개 값을 새 source of truth로 둔다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
FLinearColor SunLightRayleighScattering = FLinearColor::White;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
FLinearColor NoSunLightRayleighScattering = FLinearColor::White;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
float SunLightMultiScattering = 1.0f;

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dynamic Sky|Sky Atmosphere", meta = (AllowPrivateAccess = "true"))
float NoSunLightMultiScattering = 1.0f;
```

## 평가 규칙

기존 태양빛 존재시간 정의를 그대로 사용한다.

- 태양빛 존재시간 시작: `SunriseHour - GapTime`
- 태양빛 존재시간 종료: `SunsetHour + GapTime`
- `SunDirectionalLight`와 `MoonDirectionalLight` component visibility는 runtime에서 토글하지 않는다.
- `SunDirectionalLight`는 `AtmosphereSunLight=true`, `AtmosphereSunLightIndex=0`이어야 한다.
- `MoonDirectionalLight`는 `AtmosphereSunLight=true`, `AtmosphereSunLightIndex=1`이어야 한다.

SkyAtmosphere scattering은 다음 규칙으로 평가한다.

### 안정 구간

태양빛 존재시간 내부의 안정 구간:

- `SunLightRayleighScattering`
- `SunLightMultiScattering`

태양빛 존재시간 외부의 안정 구간:

- `NoSunLightRayleighScattering`
- `NoSunLightMultiScattering`

### 전환 구간

일출 전환:

- 구간: `SunriseHour - GapTime`부터 `SunriseHour + GapTime`
- 보간: `NoSunLight* -> SunLight*`

일몰 전환:

- 구간: `SunsetHour - GapTime`부터 `SunsetHour + GapTime`
- 보간: `SunLight* -> NoSunLight*`

보간 방식:

- RayleighScattering은 `FLinearColor` 채널별 linear lerp
- MultiScattering은 float linear lerp
- HSV 보간 사용하지 않음
- alpha는 `0.0..1.0` clamp

`GapTime`은 SkyAtmosphere scattering 전환 폭이며, sky sphere material의 `IsStarVisible` 판정에 쓰는 태양빛 존재시간 폭이다. `IsMoonVisible`은 항상 `1.0`으로 설정한다.

## 권장 함수 구조

`ADynamicSky` private helper로 추가한다.

```cpp
float EvaluateSunLightBlendAlpha(float Hour24) const;
FLinearColor EvaluateRayleighScattering(float Hour24) const;
float EvaluateMultiScattering(float Hour24) const;
```

`EvaluateSunLightBlendAlpha` 의미:

- `0.0`: 태양빛 존재시간 외부 값
- `1.0`: 태양빛 존재시간 값
- `0.0..1.0`: 일출/일몰 전환 보간

예상 로직:

1. `Hour24`, `SunriseHour`, `SunsetHour`를 `[0, 24)`로 normalize한다.
2. `GapTime`을 기존 태양빛 존재시간 계산과 같은 방식으로 clamp한다.
3. 일출 전환 구간이면 `NoSunLight -> SunLight` alpha를 반환한다.
4. 일몰 전환 구간이면 `SunLight -> NoSunLight` alpha를 반환한다.
5. 태양빛 존재시간이면 `1.0`을 반환한다.
6. 그 외는 `0.0`을 반환한다.

자정 wrap을 반드시 처리해야 한다. 기존 `IsTimeWithinWrappedRange` helper를 재사용하되, transition alpha 계산에 필요한 elapsed 계산은 별도 helper로 분리해도 된다.

## DynamicSky 평가 변경

`ADynamicSky::EvaluateSkyState(float Hour24) const`에서 제거:

```cpp
const float CurveTime = SkyState.Hour24 / 24.0f;
SkyState.RayleighScattering = RayleighScatteringCurve ? RayleighScatteringCurve->GetLinearColorValue(CurveTime) : FallbackRayleighScattering;
SkyState.MultiScattering = MultiScatteringCurve ? MultiScatteringCurve->GetLinearColorValue(CurveTime) : FallbackMultiScattering;
```

대체:

```cpp
SkyState.RayleighScattering = EvaluateRayleighScattering(SkyState.Hour24);
SkyState.MultiScattering = EvaluateMultiScattering(SkyState.Hour24);
```

`ApplySkyStateInternal`의 실제 적용 방식은 기존과 동일하게 유지한다.

```cpp
SkyAtmosphere->SetRayleighScattering(SkyState.RayleighScattering);
SkyAtmosphere->SetMultiScatteringFactor(FMath::Max(0.0f, SkyState.MultiScattering));
```

`MultiScattering`은 `USkyAtmosphereComponent::SetMultiScatteringFactor(float)`에 들어가는 float 값이다. `FLinearColor`로 노출하거나 `.R` 채널만 사용하는 구조를 만들지 않는다.

## Editor Preview 반영

`PostEditChangeProperty`에서 기존 curve property 감지를 제거한다.

제거:

```cpp
RayleighScatteringCurve
MultiScatteringCurve
```

추가:

```cpp
SunLightRayleighScattering
NoSunLightRayleighScattering
SunLightMultiScattering
NoSunLightMultiScattering
```

프리뷰 동작:

- `bUseEditorPreviewTime` true
- `PreviewHour24` 또는 위 4개 값, `SunriseHour`, `SunsetHour`, `GapTime`, `OrbitYaw` 변경 시 즉시 `ApplyPreviewTime()`
- PIE 없이 에디터 뷰포트에서 SkyAtmosphere 값이 반영되어야 한다.

## 문서 반영

구현 후 갱신:

- `.md/Architecture/EnvironmentSystem.md`
  - DynamicSky는 Rayleigh/MultiScattering curve를 사용하지 않음
  - 4개 Details 값과 `GapTime` 보간 규칙 기록
- `.md/USER_UNREAL.md`
  - curve asset 연결 안내 제거
  - 4개 scattering 값 설정 안내 추가

이미 문서가 같은 내용으로 갱신되어 있으면 중복 추가하지 말고 현재 Source와 문서가 일치하는지만 확인한다.

## 검증 기준

### 빌드

가능하면 아래 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

Live Coding이 활성화되어 빌드가 실패하면 Editor를 종료하거나 Live Coding을 끄고 다시 빌드한다.

### 코드 검증

- `ADynamicSky`에 `RayleighScatteringCurve` 문자열이 남지 않아야 한다.
- `ADynamicSky`에 `MultiScatteringCurve` 문자열이 남지 않아야 한다.
- `ADynamicSky`에 `FallbackRayleighScattering` 문자열이 남지 않아야 한다.
- `ADynamicSky`에 `FallbackMultiScattering` 문자열이 남지 않아야 한다.
- `DynamicSky.cpp`에서 `CurveTime` 기반 scattering 평가가 없어야 한다.
- `UCurveLinearColor` forward declaration/include가 DynamicSky 전용으로만 쓰였다면 제거한다.
- `SunriseHour - GapTime`, `SunriseHour + GapTime`, `SunsetHour - GapTime`, `SunsetHour + GapTime` 전환이 자정 wrap에서도 동작해야 한다.
- `SunDirectionalLight`와 `MoonDirectionalLight` component visibility를 runtime에서 토글하지 않아야 한다.
- Sun/Moon atmosphere light index가 각각 0/1로 설정되어야 한다.
- `IsStarVisible`은 태양빛 존재시간 기준으로 설정한다.
- `IsMoonVisible`은 항상 `1.0`으로 설정한다.

### 수동 검증

Editor에서 확인:

1. `ADynamicSky::bUseEditorPreviewTime`을 켠다.
2. `PreviewHour24`를 일출 전/중/후로 움직였을 때 scattering 값이 `NoSunLight -> SunLight`로 부드럽게 변한다.
3. `PreviewHour24`를 일몰 전/중/후로 움직였을 때 scattering 값이 `SunLight -> NoSunLight`로 부드럽게 변한다.
4. `GapTime`을 키우면 보간 구간이 넓어진다.
5. `GapTime`을 줄이면 보간 구간이 좁아진다.
6. 낮 안정 구간에서는 SunLight 값이 적용된다.
7. 밤 안정 구간에서는 NoSunLight 값이 적용된다.

## QnA 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `MultiScattering` 적용 API가 float가 아닌 다른 타입을 요구하는 것으로 확인된다.
- `GapTime`이 0일 때 보간 구간 처리 방식이 태양빛 존재시간 판정과 충돌한다.
- `GapTime`이 하루 전체에 가까울 때 alpha 정책을 확정해야 한다.
- 기존 Blueprint asset에 curve property 참조가 있어 C++ property 삭제만으로 compile/save 문제가 발생한다.
- Core Redirect가 필요한 rename 작업이 필요하다.

## 주의사항

- 이번 작업은 DynamicSky scattering 설정 방식 변경만 수행한다.
- 시간 source, bucket subsystem, clock UI, sun/moon orbit 구조는 변경하지 않는다.
- `AEnvironmentTimeOfDayActor` legacy curve 구조는 건드리지 않는다.
- Content asset은 직접 수정하지 않는다.
- Config는 수정하지 않는다.
