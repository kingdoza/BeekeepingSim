# Active-use 내구도 감소 구현 프롬프트

## 목표

FocusEngaged item-use-area에서 특정 hold-use 아이템이 active use 중일 때, 매 Tick 선택 아이템의 durability를 감소시키는 기능을 구현한다.

이번 설계의 핵심은 다음과 같다.

- 기존 `UItemDefinition` base class는 active-use durability 설정으로 확장하지 않는다.
- active-use 내구도 감소가 필요한 아이템은 전용 하위 item definition class를 사용한다.
- 실제 selected item durability mutation은 `UBeekeeperHotbarComponent` authority API로 처리한다.
- Focus scope는 입력/영역/결과 라우팅만 담당한다.
- 훈연기와 소독약만 active-use durability drain 적용 대상이다. 벌솔은 기존 소모 없음 정책을 유지한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/QNA_IMPLEMENTATION.md`

## 확정 QnA

반드시 `.md/QNA_ARCHITECTURE.md`의 `[사용영역 active 중 아이템 내구도 Tick 감소]` 답변을 기준으로 한다.

- 설정 위치: `UItemDefinition` base class가 아니라 전용 subclass
- 권장 class: `UActiveUseDurabilityItemDefinition : public UItemDefinition`
- active 조건은 전용 item definition의 `DrainPolicy`가 결정한다.
  - `WhenUseEffectSucceeded`: matching active use-area 위에서 `ApplyUseEffect`가 `bSucceeded=true`를 반환한 Tick에만 감소
  - `WhileOverValidUseArea`: matching active use-area 위에서 `CanApplyUseEffect`가 true이면 effect success와 무관하게 감소
  - `WhileUseSessionActive`: `BeginUse` 성공 후 LMB active use session 동안 use-area hover/target과 무관하게 감소
- `WhenUseEffectSucceeded`의 `bSucceeded=true` 의미: 실제 수치 변화가 있었는지가 아니라, action이 유효한 target을 찾아 성공 결과를 반환했는지
- 0 도달 처리: `bRemoveItemWhenDepleted`가 단일 기준
  - true: selected slot item 제거 + active use session 종료
  - false: durability 0 상태로 item 유지 + 이후 use begin/effect 차단
- active-use durability drain 대상 item invariant:
  - `bUsesDurability=true`
  - `MaxDurability>0`
  - `MaxStack==1`
- mutation authority: `FItemActionExecutionResult`가 durability delta를 전달하고, `UCursorItemUseAreaScopeComponent`가 Hotbar API에 위임
- 감소량: `DurabilityDrainPerSecond * DeltaTime` float delta, 별도 accumulator 없음

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/Inventory/ActiveUseDurabilityItemDefinition.h` 신규
- `Source/BeekeepingSim/Public/Inventory/ItemActionTypes.h`
- `Source/BeekeepingSim/Public/Inventory/HoldItemUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/HoldItemUseAction.cpp`
- `Source/BeekeepingSim/Public/Inventory/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`

필요 시:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/USER_UNREAL.md`
- `.md/PROMPT_REVIEW.md`

수정 금지:

- `UItemDefinition` base class에 active-use durability drain property 추가
- 기존 UCLASS/USTRUCT/UENUM rename
- 기존 BlueprintCallable/Public API 삭제 또는 rename
- Core Redirect가 필요한 rename
- Content `.uasset` 직접 수정/저장
- 벌솔 durability 소모 적용

## 전용 ItemDefinition subclass

`UActiveUseDurabilityItemDefinition`을 추가한다.

권장 header:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Inventory/ItemDefinition.h"
#include "ActiveUseDurabilityItemDefinition.generated.h"

UENUM(BlueprintType)
enum class EActiveUseDurabilityDrainPolicy : uint8
{
	WhenUseEffectSucceeded,
	WhileOverValidUseArea,
	WhileUseSessionActive
};

UCLASS(BlueprintType)
class BEEKEEPINGSIM_API UActiveUseDurabilityItemDefinition : public UItemDefinition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Active Use Durability", meta = (ClampMin = "0.0"))
	float DurabilityDrainPerSecond = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Active Use Durability")
	EActiveUseDurabilityDrainPolicy DrainPolicy = EActiveUseDurabilityDrainPolicy::WhenUseEffectSucceeded;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item|Active Use Durability")
	bool bRemoveItemWhenDepleted = true;
};
```

주의:

- cpp 파일은 필요하지 않으면 만들지 않는다.
- `UItemDefinition` base class에는 이 spec을 넣지 않는다.
- class 추가이므로 Core Redirect는 필요 없다.
- 기존 item asset을 이 subclass로 바꾸는 작업은 Content 수동 작업이다.

## Action Result 확장

`FItemActionExecutionResult`에 durability delta를 추가한다.

권장 필드:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Action")
float DurabilityDelta = 0.0f;
```

정책:

- 음수는 durability 감소, 양수는 회복으로 해석한다.
- 이번 기능은 음수만 사용한다.
- `bConsumedItem`/`StackDelta`와 독립적으로 해석한다.
- `DurabilityDelta == 0`이면 durability mutation 없음.

## Hold-use Action 공통 helper

`UHoldItemUseAction`에 active-use durability drain 계산 helper를 추가한다.

권장 public API:

```cpp
UFUNCTION(BlueprintPure, Category = "Item Action|Use Area")
virtual float ResolveActiveUseDurabilityDelta(
	const FItemActionContext& Context,
	const FItemActionExecutionResult& EffectResult,
	float DeltaTime,
	bool bIsOverValidUseArea
) const;
```

동작:

- `Context.SourceItemInstance`가 없으면 0.
- source definition이 `UActiveUseDurabilityItemDefinition`이 아니면 0.
- invalid config면 0.
  - `bUsesDurability=false`
  - `MaxDurability<=0`
  - `MaxStack!=1`
  - `DurabilityDrainPerSecond<=0`
- source item durability가 이미 0 이하이면 0.
- `DrainPolicy == WhileUseSessionActive`이면 use-area/effect result와 무관하게 감소한다.
- `DrainPolicy == WhileOverValidUseArea`이고 `bIsOverValidUseArea=false`이면 0.
- `DrainPolicy == WhenUseEffectSucceeded`이고 `bIsOverValidUseArea=false` 또는 `EffectResult.bSucceeded=false`이면 0.
- 그 외에는 `-DurabilityDrainPerSecond * Max(0, DeltaTime)` 반환.

`UHoldItemUseAction`의 사용 가능 판정도 보강한다.

권장 private/protected helper:

```cpp
bool HasUsableActiveUseDurability(const FItemActionContext& Context) const;
```

정책:

- source definition이 `UActiveUseDurabilityItemDefinition`이 아니면 true.
- active-use durability definition이면 config가 유효해야 하고, current durability가 0보다 커야 true.
- 이 helper를 기본 `ReceiveCanBeginUse_Implementation`과 `ReceiveCanApplyUseEffect_Implementation` 경로에 반영한다.
- 기존 subclass가 `CanBeginUse`/`CanApplyUseEffect`를 override하지 않는 현재 구조를 깨지 않는다.

## Hotbar durability mutation API

`UBeekeeperHotbarComponent`에 selected item durability mutation API를 추가한다.

권장 result struct:

```cpp
USTRUCT(BlueprintType)
struct FHotbarItemDurabilityMutationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	bool bApplied = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	bool bItemDepleted = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	bool bItemRemoved = false;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	float PreviousDurability = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	float NewDurability = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Hotbar")
	FText Message;
};
```

권장 API:

```cpp
UFUNCTION(BlueprintCallable, Category = "Hotbar")
FHotbarItemDurabilityMutationResult ApplySelectedItemDurabilityDelta(
	float DurabilityDelta,
	bool bRemoveWhenDepleted
);
```

동작:

- `DurabilityDelta`가 거의 0이면 no-op 실패 결과.
- selected slot이 없거나 selected item이 없으면 실패.
- selected item이 durability를 사용하지 않으면 실패.
- `PreviousDurability` 기록.
- `NewDurability = Clamp(PreviousDurability + DurabilityDelta, 0, MaxDurability)`.
- 값이 변하지 않으면 실패 또는 no-op으로 처리하되 delegate broadcast는 하지 않는다.
- 값이 변하면 `SelectedItem->SetDurability(NewDurability)`.
- `NewDurability <= 0`이면 `bItemDepleted=true`.
- depleted이고 `bRemoveWhenDepleted=true`이면:
  - `RememberSelectedIndex()`
  - selected slot item clear
  - `bItemRemoved=true`
- mutation 발생 시 `ReevaluateSlotsInternal()` 후 `BroadcastHotbarChanged()`.
- stack count는 변경하지 않는다.
- storage item durability 변경 API는 이번 범위에 넣지 않는다.

## CursorItemUseAreaScope result 해석

`UCursorItemUseAreaScopeComponent::TickComponent`의 `ApplyUseEffect` 결과 처리 흐름을 확장한다.

권장 흐름:

```cpp
FItemActionExecutionResult Result;
bool bIsOverValidUseArea = false;
FItemActionContext DurabilityContext = BuildItemActionContext(INDEX_NONE);

if (ActiveDescriptorIndices.Contains(HoveredDescriptorIndex))
{
	const FItemActionContext EffectContext = BuildItemActionContext(HoveredDescriptorIndex);
	if (CachedHoldAction->CanApplyUseEffect(EffectContext))
	{
		bIsOverValidUseArea = true;
		DurabilityContext = EffectContext;
		Result = CachedHoldAction->ApplyUseEffect(EffectContext, DeltaTime);
	}
}

Result.DurabilityDelta += CachedHoldAction->ResolveActiveUseDurabilityDelta(
	DurabilityContext,
	Result,
	DeltaTime,
	bIsOverValidUseArea
);
ApplyUseEffectResultToSelectedItem(Result);
```

`ApplyUseEffectResultToSelectedItem`는 stack mutation과 durability mutation을 독립 처리해야 한다.

주의:

- 현재 구현은 `!Result.bConsumedItem`이면 즉시 return한다. 이 구조를 유지하면 durability-only result가 무시된다. 반드시 수정한다.
- stack mutation은 기존 `bConsumedItem`/`StackDelta`/placement rollback 정책을 보존한다.
- durability mutation은 `Result.DurabilityDelta`가 0이 아닐 때만 Hotbar API를 호출한다.
- `DrainPolicy == WhileUseSessionActive`인 경우에는 hovered active descriptor가 없어도 durability-only result가 처리되어야 한다.
- durability mutation에 넘기는 `bRemoveWhenDepleted` 값은 selected item definition의 `UActiveUseDurabilityItemDefinition::bRemoveItemWhenDepleted`에서 읽는다.
- durability가 0에 도달하면 `EndUseSession(false)`를 호출해 active use를 자연 종료한다.
  - item이 제거되는 경우에도 먼저 세션을 종료해 held presentation active end 이벤트가 누락되지 않게 한다.
  - item을 유지하는 경우에도 현재 use session은 종료하고 이후 begin/effect는 durability 0 차단에 맡긴다.
- mutation 후 `RefreshSelectedItemAndAction()`과 `RebuildItemUseAreaDescriptors()`를 호출해 area/selection 상태를 갱신한다.

## 훈연기/소독약 적용 방식

C++ action class 자체에 item-specific durability field를 추가하지 않는다.

- `USmokerUseAction`과 `UDisinfectantUseAction`은 기존처럼 domain effect를 적용하고 `Result.bSucceeded=true`를 반환한다.
- durability drain 여부는 source item definition이 `UActiveUseDurabilityItemDefinition`인지로 결정한다.
- 벌솔(`UBeeBrushUseAction`)은 이번 적용 대상이 아니다. 벌솔 asset을 전용 definition으로 바꾸지 않는 한 durability drain이 발생하지 않아야 한다.

Content 수동 작업은 C++ 구현에서 하지 않는다. 구현 완료 후 `.md/USER_UNREAL.md` 또는 최종 보고에 아래 작업을 명시한다.

- 훈연기 item definition asset을 `UActiveUseDurabilityItemDefinition` 기반 asset으로 생성/교체
- 소독약 item definition asset을 `UActiveUseDurabilityItemDefinition` 기반 asset으로 생성/교체
- 각 asset에서:
  - `bUsesDurability=true`
  - `MaxDurability>0`
  - `MaxStack=1`
  - `DurabilityDrainPerSecond` 설정
  - `DrainPolicy` 설정
  - `bRemoveItemWhenDepleted` 설정
- 기존 ActionSpec은 유지한다.

## 문서 갱신

구현 후 실제 API 이름과 일치하도록 아래 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - active-use durability drain 요약
  - 훈연기/소독약 적용 대상, 벌솔 제외
- `.md/Architecture/InventorySystem.md`
  - `UActiveUseDurabilityItemDefinition`
  - `FItemActionExecutionResult::DurabilityDelta`
  - `UBeekeeperHotbarComponent::ApplySelectedItemDurabilityDelta`
  - `UHoldItemUseAction::ResolveActiveUseDurabilityDelta`
- `.md/Architecture/FocusSystem.md`
  - `UCursorItemUseAreaScopeComponent`가 stack delta와 durability delta를 독립 해석한다는 내용
- `.md/USER_UNREAL.md`
  - 훈연기/소독약 DataAsset 수동 전환과 설정 절차

문서 갱신은 구현 범위에 포함한다. 단, Content asset은 직접 수정하지 않는다.

## 검색 검증

```powershell
rg "ActiveUseDurabilityItemDefinition|EActiveUseDurabilityDrainPolicy|DurabilityDrainPerSecond|DrainPolicy|bRemoveItemWhenDepleted" Source/BeekeepingSim .md
rg "DurabilityDelta|ApplySelectedItemDurabilityDelta|FHotbarItemDurabilityMutationResult|ResolveActiveUseDurabilityDelta" Source/BeekeepingSim .md
rg "ApplyUseEffectResultToSelectedItem|bConsumedItem|StackDelta" Source/BeekeepingSim/Private/Focus Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory
rg "BeeBrushUseAction" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory .md
```

확인할 것:

- `UItemDefinition` base class에 active-use durability property가 추가되지 않았다.
- `UPollenPattyItemDefinition` 기존 동작과 충돌하지 않는다.
- `FItemActionExecutionResult::DurabilityDelta`가 `bConsumedItem`과 독립적으로 처리된다.
- selected item durability mutation은 Hotbar API를 통해서만 수행된다.
- durability 0 도달 시 active use session이 종료된다.
- 벌솔에는 별도 durability drain 코드가 추가되지 않았다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `UItemDefinition` base class를 수정하지 않으면 구현이 불가능한 경우
- 기존 `FItemActionExecutionResult` 확장이 Blueprint serialization 또는 기존 BP 노드와 충돌하는 경우
- `UBeekeeperHotbarComponent` public API 추가만으로 selected item durability 상태 갱신이 불가능한 경우
- `bRemoveItemWhenDepleted=false` 정책을 구현하려면 별도 repair/broken item 시스템이 필요해지는 경우
- 훈연기/소독약 Content asset 전환이 C++ 구현 완료 조건이 되는 경우
- UCLASS/USTRUCT/UENUM rename이 필요해지는 경우
- Core Redirect 필요 여부가 불명확한 경우

## 최종 보고 요구사항

구현 완료 보고에는 반드시 아래를 포함한다.

- 변경한 Source 파일
- 변경한 문서 파일
- UBT 빌드 결과 또는 미수행 사유
- Content 수동 작업 목록
- Core Redirect 불필요 여부
- 훈연기/소독약은 opt-in DataAsset 전환 전까지 실제 drain이 발생하지 않을 수 있다는 점
