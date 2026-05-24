# 구현 리뷰 프롬프트: DynamicSky SkyAtmosphere Scattering 방식 변경

## 우선순위

1. High: DynamicSky scattering source of truth 교체 정확성
2. High: 일출/일몰 `GapTime` 전환 보간 규칙 정확성 (자정 wrap 포함)
3. High: 기존 시간 소스/구독/visibility 규칙 회귀 없음
4. Medium: Editor preview 즉시 반영 정확성
5. Medium: 문서 본문/USER_UNREAL 정합성

---

## 리뷰 대상 파일

### 코드
- `Source/BeekeepingSim/Public/Environment/DynamicSky.h`
- `Source/BeekeepingSim/Private/Environment/DynamicSky.cpp`
- `Source/BeekeepingSim/Public/Environment/DynamicSkyTypes.h` (변경 시)

### 문서
- `.md/Architecture/EnvironmentSystem.md`
- `.md/USER_UNREAL.md`

---

## 핵심 검증 항목

### High 1: Curve 제거 / 4개 값 전환
- `ADynamicSky`에서 아래 식별자가 완전히 제거되었는가:
  - `RayleighScatteringCurve`
  - `MultiScatteringCurve`
  - `FallbackRayleighScattering`
  - `FallbackMultiScattering`
- 대신 아래 4개가 source of truth로 존재하는가:
  - `SunLightRayleighScattering`
  - `NoSunLightRayleighScattering`
  - `SunLightMultiScattering`
  - `NoSunLightMultiScattering`
- `CurveTime = Hour24 / 24.0` 기반 scattering 평가가 제거되었는가

### High 2: 보간 규칙 정확성
- `EvaluateSunLightBlendAlpha(float Hour24)`가 다음 의미를 지키는가:
  - `0.0`: no-sunlight 안정 구간
  - `1.0`: sunlight 안정 구간
  - `0..1`: 일출/일몰 전환 구간
- 일출 전환:
  - `SunriseHour - GapTime` ~ `SunriseHour + GapTime`
  - `NoSunLight* -> SunLight*` 보간
- 일몰 전환:
  - `SunsetHour - GapTime` ~ `SunsetHour + GapTime`
  - `SunLight* -> NoSunLight*` 보간
- 자정 wrap 구간에서도 alpha가 끊기지 않는가
- RayleighScattering 보간은 `FLinearColor` 채널별 linear lerp인지 확인 (HSV 보간 없음)
- MultiScattering 보간은 float linear lerp인지 확인

### High 3: 기존 동작 회귀 없음
- Sun/Moon visibility 배타 규칙 유지:
  - `bSunLightVisible` 와 `bMoonLightVisible`이 동시에 true가 되는 경로 없음
- MID 파라미터 규칙:
  - `IsStarVisible`은 태양빛 존재시간 기준으로 설정되는가
  - `IsMoonVisible`은 항상 `1.0`으로 설정되는가
- `ApplySkyStateInternal` 적용 방식 유지:
  - `SetRayleighScattering(...)`
  - `SetMultiScatteringFactor(FMath::Max(0.0f, MultiScattering))`
- 시간 source / bucket / clock 관련 클래스(`AGameTimeOfDayActor`, `ITimeOfDayProvider`, `UGameTimeBucketSubsystem`, `ABeekeeperController`) 변경 없음

### Medium 1: Editor Preview 반영
- `PostEditChangeProperty`에서 preview 즉시 반영 트리거가 아래를 포함하는가:
  - `SunLightRayleighScattering`
  - `NoSunLightRayleighScattering`
  - `SunLightMultiScattering`
  - `NoSunLightMultiScattering`
  - `SunriseHour`, `SunsetHour`, `GapTime`, `OrbitYaw`, `PreviewHour24`
- `bUseEditorPreviewTime` 조건 하에서 PIE 없이 적용되는가

### Medium 2: include/타입 정리
- `DynamicSky`에서 `UCurveLinearColor` forward declaration/include가 제거되었는가
- `#include "Public/..."` 패턴이 없는가

### Medium 3: 문서 정합성
- `EnvironmentSystem.md`에 curve 방식 제거 + 4개 값 + `GapTime` 보간 규칙이 기록되었는가
- `USER_UNREAL.md`에서 curve asset 연결 안내가 제거되고 4개 scattering 값 설정 안내가 반영되었는가

---

## 빌드/검색 검증

- UBT:
  - `BeekeepingSimEditor Win64 Development`

- 검색:
  - `rg "RayleighScatteringCurve|MultiScatteringCurve|FallbackRayleighScattering|FallbackMultiScattering|CurveTime" Source/BeekeepingSim/Public/Environment/DynamicSky.h Source/BeekeepingSim/Private/Environment/DynamicSky.cpp -n`
  - `rg "SunLightRayleighScattering|NoSunLightRayleighScattering|SunLightMultiScattering|NoSunLightMultiScattering|EvaluateSunLightBlendAlpha|EvaluateRayleighScattering|EvaluateMultiScattering" Source/BeekeepingSim/Public/Environment/DynamicSky.h Source/BeekeepingSim/Private/Environment/DynamicSky.cpp -n`
  - `rg "UCurveLinearColor" Source/BeekeepingSim/Public/Environment/DynamicSky.h Source/BeekeepingSim/Private/Environment/DynamicSky.cpp -n`
  - `rg '#include \"Public/' Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`

---

## 수동 검증 가이드 (Editor)

1. `ADynamicSky::bUseEditorPreviewTime` ON
2. `PreviewHour24`를 일출 전/중/후로 이동
   - scattering이 `NoSunLight -> SunLight`로 연속 보간되는지
3. `PreviewHour24`를 일몰 전/중/후로 이동
   - scattering이 `SunLight -> NoSunLight`로 연속 보간되는지
4. `GapTime` 증가/감소
   - 전환 구간 폭이 함께 증가/감소하는지
5. 낮/밤 안정 구간 확인
   - 낮: `SunLight*` 값 고정
   - 밤: `NoSunLight*` 값 고정

---

## 리뷰 결과 출력 형식

1. Findings (High / Medium / Low)
2. Open Questions / Assumptions
3. Regression Risks
4. 최종 판단: Pass / Conditional Pass / Fail
