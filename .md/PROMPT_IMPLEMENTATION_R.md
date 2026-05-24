# 구현 수정 프롬프트: DynamicSky / GameTimeOfDay 분리 리뷰 Findings

## 우선순위

1. High: provider 선택 정책에서 `AGameTimeOfDayActor`를 canonical source of truth로 우선 고정
2. Medium: `ADynamicSky` legacy provider 구독 경로 보완
3. Medium: 아키텍처 문서 본문을 split 이후 구조와 일치하도록 갱신

## 발견 문제

### 1. 기존 `AEnvironmentTimeOfDayActor`가 신규 시간 소유자를 덮어쓸 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/Environment/EnvironmentTimeOfDayActor.cpp`
  - `Source/BeekeepingSim/Private/Environment/GameTimeBucketSubsystem.cpp`
  - `Source/BeekeepingSim/Private/Character/BeekeeperController.cpp`
  - `Source/BeekeepingSim/Private/Environment/DynamicSky.cpp`
- 원인:
  - `AEnvironmentTimeOfDayActor::BeginPlay()`가 항상 `SetTimeOfDayProvider(this)`를 호출한다.
  - `UGameTimeBucketSubsystem::BindTimeProviderActor()`는 기존 provider가 `AGameTimeOfDayActor`인지 확인하지 않고 새 provider로 교체한다.
  - controller / dynamic sky / subsystem auto-find가 `TActorIterator<AActor>` 첫 번째 `ITimeOfDayProvider`를 사용한다.
- 영향:
  - 레벨에 legacy `AEnvironmentTimeOfDayActor`와 신규 `AGameTimeOfDayActor`가 공존하면 BeginPlay 순서 또는 iterator 순서에 따라 bucket, clock, sky가 legacy actor를 시간 기준으로 사용할 수 있다.
  - `AGameTimeOfDayActor`를 source of truth로 분리한다는 핵심 설계가 런타임에서 보장되지 않는다.
- 수정 방향:
  - provider resolve는 2-pass 또는 helper 함수로 통일한다.
  - 1순위: valid `AGameTimeOfDayActor`
  - 2순위: legacy `AEnvironmentTimeOfDayActor`
  - `UGameTimeBucketSubsystem::SetTimeOfDayProvider()`는 기존 bound provider가 `AGameTimeOfDayActor`이면 legacy provider가 덮어쓰지 못하게 한다.
  - `AEnvironmentTimeOfDayActor::BeginPlay()`는 compatibility wrapper 등록을 유지하더라도, 신규 game time actor가 이미 존재하거나 bound 된 경우 override하지 않게 한다.

### 2. `ADynamicSky` legacy transition path가 현재 시간만 1회 적용하고 이후 갱신을 받지 않음

- 대상 파일:
  - `Source/BeekeepingSim/Public/Environment/DynamicSky.h`
  - `Source/BeekeepingSim/Private/Environment/DynamicSky.cpp`
- 원인:
  - `ResolveAndBindTimeSource()`는 `AGameTimeOfDayActor::OnGameTimeOfDayChanged`에만 bind한다.
  - legacy `AEnvironmentTimeOfDayActor`는 `ApplySkyState(LegacyActor->GetCurrentHour24())`만 호출하고 `OnTimeOfDayChanged` 또는 `OnGameTimeOfDayChanged`를 구독하지 않는다.
- 영향:
  - transition level에서 legacy provider로 `ADynamicSky`를 구동하면 BeginPlay 시각만 반영되고 시간 진행에 따른 sky visual update가 멈춘다.
- 수정 방향:
  - legacy actor용 transient pointer를 추가하고 `OnTimeOfDayChanged` 또는 새 `OnGameTimeOfDayChanged`에 bind/unbind한다.
  - handler는 visual-only 원칙에 맞게 `HandleProviderHour(Hour24)`만 호출한다.
  - 가능하면 legacy도 provider priority fallback에만 사용한다.

### 3. 문서 본문이 split 이후 구조와 충돌함

- 대상 파일:
  - `.md/0_ARCHITECTURE.md`
  - `.md/Architecture/EnvironmentSystem.md`
  - `.md/Architecture/CharacterSystem.md`
  - `.md/Architecture/UISystem.md`
- 원인:
  - 기존 본문은 여전히 `AEnvironmentTimeOfDayActor`가 시간 source of truth이고 controller/bucket이 `OnTimeOfDayChanged`를 직접 구독한다고 설명한다.
  - 하단 update 섹션만 신규 구조를 설명해 동일 문서 안에서 기준이 충돌한다.
- 영향:
  - 이후 구현/리뷰 agent가 오래된 본문을 정본으로 읽으면 신규 provider 기반 설계와 반대 결론을 낼 수 있다.
- 수정 방향:
  - Environment key classes/runtime flow/time model/bucket model/time clock integration을 `AGameTimeOfDayActor` + `ITimeOfDayProvider` + `ADynamicSky` 기준으로 갱신한다.
  - Character/UI clock flow는 provider 기반 resolve와 `AGameTimeOfDayActor` 우선 구독을 본문에 반영한다.
  - legacy `AEnvironmentTimeOfDayActor`는 compatibility/transition 경로로 명시한다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg "ITimeOfDayProvider|OnGameTimeOfDayChanged|SetTimeOfDayProvider|AGameTimeOfDayActor|ADynamicSky" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
  - `rg "#include \"Public/" Source/BeekeepingSim/Public Source/BeekeepingSim/Private -n`
  - `rg "AEnvironmentTimeOfDayActor.*source of truth|OnTimeOfDayChanged.*clock|SetTimeOfDayActor\\(" .md/0_ARCHITECTURE.md .md/Architecture -n`
- PIE:
  - 신규 레벨 구성: `AGameTimeOfDayActor` 1개 + `ADynamicSky` 1개에서 sky, clock, bucket listener가 같은 hour를 따르는지 확인
  - 공존 구성: legacy `AEnvironmentTimeOfDayActor`가 남아 있어도 `AGameTimeOfDayActor`가 bucket/clock/sky 기준으로 우선되는지 확인
  - legacy-only 구성: `AEnvironmentTimeOfDayActor`만 있을 때 기존 clock/bucket 호환 경로가 유지되는지 확인

## 문서 반영 필요 여부

- 필요.
- 시간 소유자 분리는 아키텍처 source of truth 변경이므로 `.md/0_ARCHITECTURE.md`와 관련 system 문서 본문까지 갱신해야 한다.
