# 밀랍 plane HoneyRipeness 파라미터 구현 프롬프트

## 목표

이미 존재하는 소비장 밀랍/capping plane material에 꿀 숙성도 값을 주입한다.

이번 작업은 **밀랍 plane material parameter 갱신만** 구현한다.

- 대상 actor: `ABeehiveCombActor`
- 대상 component:
  - `FrontWaxCappingPlane`
  - `BackWaxCappingPlane`
- 대상 material parameter: `HoneyRipeness`
- 값 source: `ABeehiveCombActor::GetHoneyRipenessRatio()`
- 값 범위: `0..1`

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

필요 시 함께 확인한다.

- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

## 현재 Source 전제

현재 Source에는 아래 구현이 이미 있다고 본다.

- `ABeehiveCombActor`
  - `FrontWaxCappingPlane`
  - `BackWaxCappingPlane`
  - `HoneyRipenessMaterialParameterName = "HoneyRipeness"`
  - `GetHoneyRipenessRatio()`
  - `EnsureHoneyMaterialInstances()`
  - `ApplyHoneyVisualState()`
- 기존 honey plane material instance:
  - `FrontHoneyMaterialInstance`
  - `BackHoneyMaterialInstance`
- 기존 honey plane에는 이미 `HoneyRipenessMaterialParameterName`으로 `RipenessRatio`를 주입한다.

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

문서:

- 필요 시 `.md/USER_UNREAL.md`
- 필요 시 `.md/PROMPT_REVIEW.md`

## 수정 금지

- `Content/` asset 직접 수정/저장
- `FrontWaxCappingPlane` / `BackWaxCappingPlane` component 생성 방식 변경
- 밀랍 plane visibility / hidden-in-game 정책 변경
- `IsHoneyFull()` 표시 조건 변경
- `ABeehive` honey production/ripeness bucket 변경
- `FBeehiveCombItemState` 필드 추가/삭제
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- UCLASS/USTRUCT/UENUM rename
- Core Redirect 추가
- 별도 Tick/subsystem/actor 추가

## 구현 지시

### 1. 밀랍 plane용 dynamic material instance 추가

`ABeehiveCombActor` header에 transient material instance를 추가한다.

권장 위치:

- `FrontHoneyMaterialInstance`
- `BackHoneyMaterialInstance`

바로 아래.

권장 선언:

```cpp
UPROPERTY(Transient)
TObjectPtr<UMaterialInstanceDynamic> FrontWaxCappingMaterialInstance;

UPROPERTY(Transient)
TObjectPtr<UMaterialInstanceDynamic> BackWaxCappingMaterialInstance;
```

새 material parameter name UPROPERTY는 만들지 않는다. 기존 `HoneyRipenessMaterialParameterName`을 재사용한다.

### 2. `EnsureHoneyMaterialInstances()` 확장

기존 honey plane dynamic material instance 생성 로직은 유지한다.

추가로 아래 두 component에 대해 material index 0 dynamic material instance를 생성/캐시한다.

- `FrontWaxCappingPlane`
- `BackWaxCappingPlane`

정책:

- component가 있으면 `GetMaterial(0)` 기준으로 `CreateDynamicMaterialInstance(0, CurrentMaterial)` 호출
- component가 없으면 대응 cached pointer를 `nullptr`로 설정
- 기존 honey plane 처리와 같은 패턴을 사용한다.

### 3. `ApplyHoneyVisualState()`에서 `HoneyRipeness` 주입

`ApplyHoneyVisualState()`의 기존 값 계산을 그대로 사용한다.

```cpp
const float RipenessRatio = GetHoneyRipenessRatio();
```

기존 honey material parameter 주입 뒤 또는 근처에서 밀랍 plane material에도 같은 값을 넣는다.

권장 구현:

```cpp
if (FrontWaxCappingMaterialInstance)
{
	FrontWaxCappingMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio);
}

if (BackWaxCappingMaterialInstance)
{
	BackWaxCappingMaterialInstance->SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio);
}
```

주의:

- 밀랍 plane material에는 `HoneyAmount`를 주입하지 않는다.
- `HoneyRipeness` 값은 face별 값이 아니라 소비장 전체 숙성도다.
- full 미만으로 hidden-in-game 상태여도 material parameter 갱신은 수행해도 된다.
- visibility helper인 `ApplyHoneyCappingVisualState()`의 조건은 변경하지 않는다.

## 검색 검증

```powershell
rg "FrontWaxCappingMaterialInstance|BackWaxCappingMaterialInstance" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
rg "HoneyRipenessMaterialParameterName|SetScalarParameterValue" Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
rg "ApplyHoneyCappingVisualState|SetHiddenInGame|SetVisibility" Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
```

확인할 것:

- 밀랍 plane용 dynamic material instance 2개가 추가되어 있다.
- `EnsureHoneyMaterialInstances()`가 `FrontWaxCappingPlane` / `BackWaxCappingPlane` material instance를 만든다.
- `ApplyHoneyVisualState()`가 두 밀랍 material instance에 `HoneyRipenessMaterialParameterName` 값을 `GetHoneyRipenessRatio()` 결과로 주입한다.
- `HoneyAmount`를 밀랍 material에 새로 주입하지 않는다.
- `SetHiddenInGame` / `SetVisibility` 정책은 변경하지 않았다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 수동 검증 항목

Editor/PIE에서 확인할 항목:

- `FrontWaxCappingPlane` / `BackWaxCappingPlane` material에 scalar parameter `HoneyRipeness`가 있다.
- 소비장이 full 상태에서 숙성도가 증가하면 밀랍 material 표현이 `0..1` 범위로 변한다.
- 회수 후 재배치 시 `HoneyRipeness`가 복원되고 밀랍 material 표현도 복원된다.
- 밀랍 plane의 표시/숨김 정책은 기존과 동일하다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- 밀랍 material에 별도 parameter name이 필요하다고 판단되는 경우
- `FBeehiveCombItemState` 변경이 필요하다고 판단되는 경우
- visibility 정책 변경이 필요하다고 판단되는 경우
- `ABeehive` 생산/숙성 bucket 변경이 필요하다고 판단되는 경우
- Content asset 수정/저장이 C++ 구현 완료 조건이 되는 경우
- Core Redirect 필요성이 생기는 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Core Redirect 불필요 여부
- Blueprint/API 영향
- 밀랍 plane `HoneyRipeness` scalar parameter 주입 확인
- visibility 정책 미변경 확인
- `FBeehiveCombItemState` 미변경 확인
