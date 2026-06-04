# 리뷰 프롬프트: 벌통 질병 VFX 단일화 구현

## 리뷰 목적

이번 리뷰는 `ABeehive`의 위생도(`SanitationValue`) 기반 disease 계산과 colony population 감소 정책은 유지하면서, disease 시각 표현이 `ABeehive::DiseaseVfxNiagara` 단일 경로로만 적용되는지 검증한다.

핵심:
- disease ratio 계산 owner는 `ABeehive`
- population 감소 계산 owner는 `ABeehive::CalculateBeeDecreaseAmount()`
- disease visual 전파 owner는 `ABeehive::RefreshHiveDiseaseVisuals()`
- active disease visual target은 `ABeehive::DiseaseVfxNiagara.User.Disease`
- attraction/outgoing/ingoing/comb/queen 직접 Disease 적용 코드는 삭제가 아니라 주석처리된 legacy path

제외:
- `Content/` asset 직접 수정/저장
- UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect 추가
- Environment bucket 구조 변경
- disease 전용 Tick/subsystem/bucket subscription 추가

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/EnvironmentSystem.md`
- `.md/QNA_REVIEW.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

---

## 리뷰 범위 파일

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeeSwarmTypes.h`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveDualSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveDualSwarmActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`
- `Source/BeekeepingSim/Public/WorldActors/QueenBeeActor.h`
- `Source/BeekeepingSim/Private/WorldActors/QueenBeeActor.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

주의:
- `.md/PROMPT_IMPLEMENTATION.md`는 구현 요청 기록으로 워크트리에 남아 있을 수 있으나 리뷰 대상의 source of truth는 위 파일과 정본 문서다.

---

## 핵심 검증 질문

1. `ABeehive`에 `SanitationDiseaseThreshold`, `MaxSanitationBeeDecreaseMultiplier`가 additive UPROPERTY로 추가되었는가?
2. `GetSanitationDiseaseRatio()`가 `EffectiveThreshold = Clamp(SanitationDiseaseThreshold, 0, MaxSanitationValue)` 기준으로 `0..1`을 반환하는가?
3. threshold가 `0`이면 disease ratio가 항상 `0`인가?
4. `GetSanitationBeeDecreaseMultiplier()`가 `MaxSanitationBeeDecreaseMultiplier`를 최소 `1.0f`로 sanitize하는가?
5. `CalculateBeeDecreaseAmount()`에서 `SanitationDecreaseMultiplier`가 `BaseDecrease` 계산 단계에 곱해지는가?
6. `Decrease = Min(ColonyBeeCount, BaseDecrease)` clamp가 유지되는가?
7. `ApplyColonyPopulationUpdate()`의 최종 `RoundToInt` 위치가 유지되는가?
8. `SetSanitationValue()`가 clamp 후 `RefreshHiveDiseaseVisuals()`를 호출하는가?
9. `ABeehive` 생성자에서 `DiseaseVfxNiagara`가 `CreateDefaultSubobject<UNiagaraComponent>`로 생성되고 `Root`에 attach되는가?
10. `RefreshHiveDiseaseVisuals()`가 `DiseaseVfxNiagara->SetVariableFloat("User.Disease", DiseaseRatio)`만 수행하는가?
11. OnConstruction/BeginPlay/PostEdit/SetSanitationValue 경로에서 `DiseaseVfxNiagara.User.Disease`가 즉시 갱신되는가?
12. attraction/outgoing/ingoing/comb front/back/queen material의 직접 Disease 적용 라인이 주석처리되어 있는가?
13. `FBeehiveDualSwarmNiagaraParameters::Disease`, `SetBeeDiseaseValue(...)`, `SetDiseaseValue(...)`, `DiseaseMaterialParameterName` 등 기존 API/property가 삭제 또는 rename되지 않았는가?
14. `ApplyCombBeeParameters(...)`와 `SetTotalSpawnAmount...` 시그니처가 유지되는가?
15. `BaseEggLayingPower`와 tick yaw jitter 정책이 변경되지 않았는가?
16. Content asset 수정 없이 `BP_Beehive.DiseaseVfxNiagara` 수동 설정 항목만 `.md/USER_UNREAL.md`에 기록되었는가?
17. Core Redirect가 필요한 rename/delete가 없는가?

---

## 검색 검증

```powershell
rg "DiseaseVfxNiagara|BeehiveDiseaseVfxNames|RefreshHiveDiseaseVisuals|User.Disease" Source/BeekeepingSim .md
rg "SetBeeDiseaseValue|ApplyDiseaseToCombActors|ApplyDiseaseToQueenBee|SetDiseaseValue|DiseaseMaterialParameterName" Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "SetVariableFloat\\(.*Disease|SetScalarParameterValue\\(.*Disease|Parameters\\.Disease" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors
rg "CalculateBeeDecreaseAmount|SanitationDecreaseMultiplier|BaseDecrease|BeeDecreaseAbsoluteAmountPerBucket" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors .md
rg "ApplyCombBeeParameters|SetTotalSpawnAmountAndResetTargetBeeCounts|SetTotalSpawnAmountPreservingTargetRatios" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
```

확인할 것:
- 기존 BlueprintCallable/Public API 삭제 또는 rename 없음
- `ColonyPopulation` subscription과 `LatestOnly` 유지
- disease 전용 bucket/subsystem/tick actor 없음
- `RefreshHiveDiseaseVisuals()`의 active output은 `DiseaseVfxNiagara.User.Disease` 단일 경로
- attraction/outgoing/ingoing/comb front/back/queen 직접 Disease 적용은 주석처리된 legacy path

---

## 빌드 검증

권장:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

구현 시점 빌드 결과:
- `BeekeepingSimEditor Win64 Development`
- `Result: Succeeded`

---

## 수동 검증 포인트

1. `BP_Beehive.DiseaseVfxNiagara`에 질병 VFX Niagara System이 지정되어 있는지 확인
2. 질병 VFX Niagara System에 float user parameter `User.Disease`가 있는지 확인
3. `SanitationValue >= SanitationDiseaseThreshold`이면 `DiseaseVfxNiagara.User.Disease`가 0인지 확인
4. `SanitationValue`가 threshold 아래로 내려가면 `DiseaseVfxNiagara.User.Disease`가 `0..1`로 증가하는지 확인
5. 소독약 hold-use로 `SanitationValue`가 증가하면 `DiseaseVfxNiagara.User.Disease`가 즉시 낮아지는지 확인
6. `MaxSanitationBeeDecreaseMultiplier > 1`일 때 population bucket 감소량이 증가하는지 확인
7. 감소량이 커져도 기존 `ColonyBeeCount`를 초과하지 않는지 확인
8. 벌떼 Niagara/소비장 Niagara/QueenBee material이 더 이상 Disease 직접 표현 경로에 의존하지 않는지 확인

---

## 리뷰 결과 출력 형식

`.md/AGENT_REVIEW.md`의 출력 형식을 따른다.

특히:
- Findings를 우선 제시하고 `High -> Medium -> Low` 순서로 정렬
- 각 Finding에 파일/라인, 원인, 영향, 수정 제안 포함
- 이슈가 없으면 `No blocking issues found.`를 명시
- Blueprint/API 영향과 Core Redirect 불필요 여부를 별도 확인
- 남은 검증 공백은 Editor/PIE 수동 검증 항목으로 분리
