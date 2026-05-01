# 코드 리뷰 요청 프롬프트 (Environment 24시간 하늘/조명/태양/달 시스템)

아래 변경은 Unreal Engine 5.7 기준 `Environment` 신규 시스템 구현이다.  
리뷰 목표는 **시간 모델 정확성, 태양/달 활성 정책, Blueprint 계약 안정성, 문서-코드 일치성** 검증이다.

---

## 리뷰 전제 (구현 사실)

- 신규 Source 추가:
  - `Source/BeekeepingSim/Public/Environment/TimeOfDayTypes.h`
  - `Source/BeekeepingSim/Public/Environment/EnvironmentTimeOfDayActor.h`
  - `Source/BeekeepingSim/Private/Environment/EnvironmentTimeOfDayActor.cpp`
- 신규 타입:
  - `FTimeOfDayCurveSet`
  - `FTimeOfDayVisualState`
  - `AEnvironmentTimeOfDayActor`
- Blueprint 계약:
  - `SetCurrentHour24(float)`
  - `GetCurrentHour24() const`
  - `SetTimeProgressionEnabled(bool)`
  - `ApplyPreviewTime()`
  - `EvaluateCurrentVisualState() const`
  - `OnTimeOfDayChanged(float, const FTimeOfDayVisualState&)`

---

## 리뷰 목표

1. `CurrentHour24` wrap/진행 로직이 24시간 모델에 맞는지
2. 태양/달 궤도와 light on/off 정책이 요구사항과 일치하는지
3. curve null 시 fallback 경로가 안전한지
4. editor preview 경로(`PostEditChangeProperty`, `CallInEditor`)가 runtime clock과 분리되어 있는지
5. tick/로그/skylight recapture 정책이 과도하지 않은지
6. 문서 반영(`0_ARCHITECTURE`, `EnvironmentSystem`, `USER_UNREAL`)이 구현과 일치하는지

---

## 리뷰 대상 경로

- `Source/BeekeepingSim/Public/Environment/**`
- `Source/BeekeepingSim/Private/Environment/**`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/USER_UNREAL.md`

---

## 필수 검증 포인트

### A. 시간/궤도 계산
- `DayLengthSeconds` 기반 `HoursPerSecond = 24 / DayLengthSeconds` 적용 여부
- `CurrentHour24` normalize/wrap 안전성
- `SunriseHour`, `SunsetHour`, `MaxSunAltitudeDegrees`, `CelestialYawOffsetDegrees` 반영 여부
- `MoonHour = CurrentHour24 + 12` wrap 평가 여부

### B. light 활성 정책
- 태양이 지평선 아래면 `SunIntensity = 0` 강제 여부
- 태양이 떠 있으면 moon intensity가 0으로 강제되는지
- `bMoonLightActive = !bSunAboveHorizon && bMoonAboveHorizon` 정책 일치 여부

### C. curve/fallback 안전성
- 모든 curve pointer가 optional로 동작하는지
- null curve에서 fallback이 사용되는지
- 음수 강도/밀도 clamp 처리 적절성

### D. 적용 경로 안전성
- `SkyLight` recapture가 tick마다 발생하지 않는지 (`bRecaptureSkyLightOnApply` opt-in)
- `SkyParameterCollection` 미설정/instance 실패 시 crash 없이 동작하는지
- warning 로그가 tick마다 반복되지 않는지

### E. Blueprint/API 안정성
- 요구된 함수/프로퍼티/델리게이트 이름이 정확히 일치하는지
- `#include "Public/..."` 위반이 없는지

---

## 권장 확인 명령

- `rg '#include "Public/' Source/BeekeepingSim`
- `rg "EnvironmentTimeOfDayActor|TimeOfDay|OnTimeOfDayChanged" Source/BeekeepingSim/Public Source/BeekeepingSim/Private .md`
- `git diff -- Source/BeekeepingSim/Public/Environment Source/BeekeepingSim/Private/Environment .md/0_ARCHITECTURE.md .md/Architecture/EnvironmentSystem.md .md/USER_UNREAL.md .md/PROMPT_REVIEW.md`

---

## 출력 형식 (반드시 준수)

1. **Findings (High -> Medium -> Low)**  
   - 각 이슈에 파일/심볼/근거 포함
2. **Open Questions / Assumptions**
3. **Regression Risk Checklist**
4. **최종 판정: Pass / Conditional Pass / Fail**

이슈가 없으면 **“High/Medium 이슈 없음”**을 명시하고, 남은 테스트 공백을 짧게 정리한다.
