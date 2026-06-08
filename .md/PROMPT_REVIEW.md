# 리뷰 프롬프트: 밀랍 plane HoneyRipeness material parameter 주입

## 리뷰 목적

이번 리뷰는 `ABeehiveCombActor`가 이미 소유한 앞/뒤 wax capping plane material에 꿀 숙성도 값을 올바르게 주입하는지 검증한다.

핵심:
- 대상 actor는 `ABeehiveCombActor`
- 대상 component는 `FrontWaxCappingPlane`, `BackWaxCappingPlane`
- 대상 material parameter는 기존 `HoneyRipenessMaterialParameterName` (`HoneyRipeness`)
- 값 source는 `ABeehiveCombActor::GetHoneyRipenessRatio()`
- 값 범위는 `0..1`
- capping visibility 조건은 기존 `IsHoneyFull()` 파생 정책을 유지
- honey production/ripeness bucket과 `FBeehiveCombItemState` 계약은 유지

제외:
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

---

## 반드시 읽을 문서

- `.md/AGENT_REVIEW.md`
- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_REVIEW.md`
- `.md/QNA_IMPLEMENTATION.md`
- `.md/USER_UNREAL.md`

참고:
- `.md/QNA_IMPLEMENTATION.md`의 FocusPrompt 관련 질문은 이번 작업과 직접 관련이 없다.
- `.md/0_ARCHITECTURE.md`, `.md/Architecture/WorldActorsSystem.md`에는 wax capping plane과 `HoneyRipeness` material 주입 설계가 이미 반영되어 있을 수 있다. 구현이 해당 설계와 일치하는지 확인한다.

---

## 리뷰 범위 파일

주요 변경:
- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp`

문서/수동 검증 확인:
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

계약 확인:
- `Source/BeekeepingSim/Public/Inventory/ItemInstance.h`
- `Source/BeekeepingSim/Private/Inventory/ItemInstance.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/WorldActorsSystem.md`

주의:
- `.md/PROMPT_IMPLEMENTATION.md`는 구현 요청 기록으로 워크트리에 남아 있을 수 있으나, 리뷰 대상의 source of truth는 정본 문서와 위 변경 파일이다.
- `Content/` 변경이 워크트리에 있더라도 이번 C++ 리뷰 범위에는 포함하지 않는다. 단, 수동 검증 항목으로 material parameter 존재 여부는 확인한다.

---

## 핵심 검증 질문

1. `ABeehiveCombActor` header에 wax capping plane용 transient dynamic material instance가 2개 추가되었는가?
   - `FrontWaxCappingMaterialInstance`
   - `BackWaxCappingMaterialInstance`
2. 새 material parameter name UPROPERTY를 만들지 않고 기존 `HoneyRipenessMaterialParameterName`을 재사용하는가?
3. `EnsureHoneyMaterialInstances()`의 기존 honey plane material instance 생성 로직이 유지되는가?
4. `EnsureHoneyMaterialInstances()`가 `FrontWaxCappingPlane->GetMaterial(0)` 기준으로 dynamic material instance를 생성/캐시하는가?
5. `EnsureHoneyMaterialInstances()`가 `BackWaxCappingPlane->GetMaterial(0)` 기준으로 dynamic material instance를 생성/캐시하는가?
6. 각 wax capping component가 없으면 대응 cached pointer를 `nullptr`로 정리하는가?
7. `ApplyHoneyVisualState()`가 기존 `const float RipenessRatio = GetHoneyRipenessRatio();` 값을 사용해 wax material에 주입하는가?
8. `FrontWaxCappingMaterialInstance`에 `SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio)`를 호출하는가?
9. `BackWaxCappingMaterialInstance`에 `SetScalarParameterValue(HoneyRipenessMaterialParameterName, RipenessRatio)`를 호출하는가?
10. wax capping material에는 `HoneyAmount`를 새로 주입하지 않는가?
11. `HoneyRipeness` 값이 face별 값이 아니라 소비장 전체 숙성도 값으로 유지되는가?
12. full 미만으로 hidden-in-game 상태여도 material parameter 갱신 경로가 막히지 않는가?
13. `ApplyHoneyCappingVisualState()`의 표시 조건이 `IsHoneyFull()` 그대로인가?
14. `SetHiddenInGame` / `SetVisibility` 정책이 이번 변경으로 바뀌지 않았는가?
15. `FrontWaxCappingPlane` / `BackWaxCappingPlane` component 생성 방식, attach, collision 설정이 이번 변경으로 바뀌지 않았는가?
16. `ABeehive`의 `ApplyHoneyProductionUpdate()`, `ApplyHoneyRipenessUpdate()`, `DistributeHoneyIncreaseToCombs(...)`가 변경되지 않았는가?
17. `FBeehiveCombItemState`에 새 필드가 추가되지 않았고 `HoneyAmount`, `HoneyRipeness`, visible face 계약이 유지되는가?
18. `ApplyStateFromItemInstance(...)`와 `WriteStateToItemInstance(...)`의 상태 복원/저장 계약이 유지되는가?
19. 기존 BlueprintCallable/Public API 삭제 또는 rename이 없는가?
20. UCLASS/USTRUCT/UENUM rename 또는 Core Redirect가 필요한 변경이 없는가?
21. 새 Tick, subsystem, 별도 actor state owner가 추가되지 않았는가?
22. `.md/USER_UNREAL.md`에 capping material scalar parameter `HoneyRipeness`와 PIE 검증 항목이 기록되어 있는가?

---

## 검색 검증

```powershell
rg "FrontWaxCappingMaterialInstance|BackWaxCappingMaterialInstance" Source/BeekeepingSim/Public/WorldActors/BeehiveCombActor.h Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
rg "HoneyRipenessMaterialParameterName|SetScalarParameterValue" Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
rg "ApplyHoneyCappingVisualState|SetHiddenInGame|SetVisibility" Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
rg "FBeehiveCombItemState|SetBeehiveCombStateWithRipeness|HoneyAmount|HoneyRipeness" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory Source/BeekeepingSim/Public/WorldActors Source/BeekeepingSim/Private/WorldActors
rg "ApplyHoneyProductionUpdate|ApplyHoneyRipenessUpdate|DistributeHoneyIncreaseToCombs|AddHoneyAmount" Source/BeekeepingSim/Public/WorldActors/Beehive.h Source/BeekeepingSim/Private/WorldActors/Beehive.cpp Source/BeekeepingSim/Private/WorldActors/BeehiveCombActor.cpp
```

확인할 것:
- 밀랍 plane용 dynamic material instance 2개가 추가되어 있다.
- `EnsureHoneyMaterialInstances()`가 `FrontWaxCappingPlane` / `BackWaxCappingPlane` material instance를 만든다.
- `ApplyHoneyVisualState()`가 두 밀랍 material instance에 `HoneyRipenessMaterialParameterName` 값을 `GetHoneyRipenessRatio()` 결과로 주입한다.
- `HoneyAmount`를 밀랍 material에 새로 주입하지 않는다.
- `SetHiddenInGame` / `SetVisibility` 정책은 변경하지 않았다.
- `FBeehiveCombItemState`에 새 필드가 없다.
- `ABeehive` honey production/ripeness 로직이 변경되지 않았다.

---

## 빌드 검증

권장:

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

구현 시점 빌드 결과:
- `BeekeepingSimEditor Win64 Development`: compile 단계 통과, link 실패
- 실패 사유: 실행 중인 `UnrealEditor.exe`가 `Binaries/Win64/UnrealEditor-BeekeepingSim.dll`을 점유
- 추가 검증: `BeekeepingSim Win64 Development` 빌드 성공 (`Result: Succeeded`)

리뷰어는 가능하면 Unreal Editor를 종료한 뒤 Editor target 빌드를 다시 수행한다.

---

## 수동 검증 포인트

Editor/PIE에서 확인:

1. `FrontWaxCappingPlane` / `BackWaxCappingPlane` material에 scalar parameter `HoneyRipeness`가 있는지 확인한다.
2. 소비장이 full 상태에서 숙성도가 증가하면 밀랍 material 표현이 `0..1` 범위로 변하는지 확인한다.
3. 회수 후 재배치 시 `HoneyRipeness`가 복원되고 밀랍 material 표현도 복원되는지 확인한다.
4. 밀랍 plane의 표시/숨김 정책은 기존과 동일한지 확인한다.
5. full 미만 상태에서도 material parameter 갱신이 문제가 되지 않는지 확인한다. 시각적으로는 plane이 hidden-in-game이라 보이지 않을 수 있다.

---

## 리뷰 결과 출력 형식

`.md/AGENT_REVIEW.md`의 출력 형식을 따른다.

특히:
- Findings를 우선 제시하고 `High -> Medium -> Low` 순서로 정렬
- 각 Finding에 파일/라인, 원인, 영향, 수정 제안 포함
- 이슈가 없으면 `No blocking issues found.`를 명시
- Blueprint/API 영향과 Core Redirect 불필요 여부를 별도 확인
- wax capping material `HoneyRipeness` 주입 여부를 명시
- visibility 정책 미변경 여부를 명시
- `FBeehiveCombItemState` 미변경 여부와 `ABeehive` honey production/ripeness 미변경 여부를 명시
- 남은 검증 공백은 Editor/PIE 수동 검증 항목으로 분리

