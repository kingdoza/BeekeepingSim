# WBP_FocusPrompt C++ 이관 구현 프롬프트

## 목표

`Content/UI/WBP_FocusPrompt`의 런타임 동작을 C++ base widget으로 이관한다.

`BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름은 유지하고, `WBP_FocusPrompt`는 레이아웃과 스타일만 담당하게 만든다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/CoreSystem.md`

위 문서와 현재 Source/Blueprint 구조가 충돌하면 구현하지 말고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

## 현재 확인된 Blueprint 동작

`WBP_FocusPrompt`는 현재 EventGraph에서 다음을 수행한다.

1. `PreConstruct`에서 design preview를 위해 `Visible` 처리한다.
2. `Construct`에서 자기 자신을 `Collapsed`로 숨긴다.
3. `GetOwningPlayerPawn()`을 `ABeekeeperCharacter`로 cast한다.
4. `BeekeeperFocus` 컴포넌트의 `OnFocusPromptChanged`에 custom event를 바인딩한다.
5. `OnFocusPromptChanged(PromptData)` 수신 시 `UpdateFocusPrompt(PromptData)`를 호출한다.
6. `PromptData.bIsValid == false`이면 위젯을 `Collapsed`로 숨긴다.
7. `PromptData.bIsValid == true`이면 `TargetNameText`, `KeyText`를 갱신하고 위젯을 `Visible`로 보인다.

현재 위젯 트리의 필수 이름:

- `TargetNameText`
- `KeyText`

이 이름은 C++ `BindWidget`에 사용하므로 변경하지 않는다.

## 확정 설계

1. `BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름은 변경하지 않는다.
2. `WBP_FocusPrompt`의 parent class를 새 C++ 클래스 `UFocusPromptWidget`으로 변경한다.
3. `WBP_FocusPrompt`는 레이아웃, 폰트, 색상, 이미지, 스페이서 등 외형만 담당한다.
4. 프롬프트 데이터 바인딩, 텍스트 갱신, visibility 갱신은 모두 C++ 책임이다.
5. C++은 `NativeConstruct()`에서 자동으로 `OwningPlayerPawn`을 통해 `ABeekeeperCharacter`를 찾는다.
6. `ABeekeeperCharacter::GetBeekeeperFocus()`로 `UBeekeeperFocusComponent`를 얻고 `OnFocusPromptChanged`에 바인딩한다.
7. 바인딩 직후 `GetCurrentPromptData()`를 호출해 초기 상태를 즉시 반영한다.
8. `NativeDestruct()`에서 델리게이트를 해제한다.
9. `NativePreConstruct()`에서는 design-time preview가 보이도록 `Visible` 처리한다.
10. 런타임 `NativeConstruct()` 시작 시 기본 상태는 `Collapsed`다.
11. `TargetNameText`, `KeyText`는 `BindWidget` 필수로 둔다.
12. 애니메이션이나 추가 Blueprint 반응 여지를 위해 `BlueprintImplementableEvent OnPromptDataApplied(PromptData, bVisible)`를 제공한다.
13. 기본 텍스트 및 visibility 갱신 책임은 C++에서 유지한다.

## 구현 파일

추가:

- `Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`
- `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`

수정:

- `Content/UI/WBP_FocusPrompt`

수정 금지:

- `BP_BeekeeperCharacter`의 `WBP_FocusPrompt` 생성/viewport 추가 흐름
- `Content/UI/WBP_FocusPrompt` 외의 다른 `Content/` asset
- Focus 판정, prompt 생성 정책, focus action 정책

## C++ 클래스 요구사항

`UFocusPromptWidget : public UUserWidget`를 추가한다.

권장 header 형태:

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Focus/FocusTargetComponent.h"
#include "FocusPromptWidget.generated.h"

class UBeekeeperFocusComponent;
class UTextBlock;

UCLASS(BlueprintType, Blueprintable)
class BEEKEEPINGSIM_API UFocusPromptWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void BindToFocusComponent(UBeekeeperFocusComponent* InFocusComponent);

	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void UnbindFromFocusComponent();

	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void SetPromptData(const FFocusPromptData& InPromptData);

	UFUNCTION(BlueprintCallable, Category = "Focus Prompt")
	void ClearPrompt();

	UFUNCTION(BlueprintPure, Category = "Focus Prompt")
	const FFocusPromptData& GetCurrentPromptData() const { return CurrentPromptData; }

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Focus Prompt")
	void OnPromptDataApplied(const FFocusPromptData& PromptData, bool bVisible);

private:
	UFUNCTION()
	void HandleFocusPromptChanged(FFocusPromptData PromptData);

private:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> TargetNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyText;

	UPROPERTY(Transient)
	TObjectPtr<UBeekeeperFocusComponent> BoundFocusComponent;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "Focus Prompt", meta = (AllowPrivateAccess = "true"))
	FFocusPromptData CurrentPromptData;
};
```

필요하면 private helper를 추가해도 된다.

## C++ 동작 상세

### `NativePreConstruct()`

- `Super::NativePreConstruct()`를 호출한다.
- `IsDesignTime()`이면 `SetVisibility(ESlateVisibility::Visible)`을 호출한다.
- design-time 기본 텍스트는 현재 Blueprint 기본값을 유지해도 된다.

### `NativeConstruct()`

- `Super::NativeConstruct()`를 호출한다.
- 런타임 기본 상태로 `SetVisibility(ESlateVisibility::Collapsed)`를 호출한다.
- `GetOwningPlayerPawn()`을 `ABeekeeperCharacter`로 cast한다.
- 성공하면 `BeekeeperCharacter->GetBeekeeperFocus()`로 focus component를 얻는다.
- `BindToFocusComponent(FocusComponent)`를 호출한다.
- cast 또는 component resolve 실패 시 crash하지 말고 숨김 상태를 유지한다.

### `NativeDestruct()`

- `UnbindFromFocusComponent()`를 호출한다.
- `Super::NativeDestruct()`를 호출한다.

### `BindToFocusComponent(UBeekeeperFocusComponent* InFocusComponent)`

- 기존 `BoundFocusComponent`가 있으면 먼저 `UnbindFromFocusComponent()`로 정리한다.
- `InFocusComponent`가 null이면 `ClearPrompt()` 또는 숨김 상태를 유지한다.
- 새 focus component를 저장한다.
- `OnFocusPromptChanged.AddUniqueDynamic(this, &UFocusPromptWidget::HandleFocusPromptChanged)`로 바인딩한다.
- 바인딩 직후 `SetPromptData(BoundFocusComponent->GetCurrentPromptData())`를 호출해 초기 상태를 즉시 반영한다.

### `UnbindFromFocusComponent()`

- `BoundFocusComponent`가 있으면 `OnFocusPromptChanged.RemoveDynamic(this, &UFocusPromptWidget::HandleFocusPromptChanged)`를 호출한다.
- `BoundFocusComponent`를 null로 만든다.

### `HandleFocusPromptChanged(FFocusPromptData PromptData)`

- `SetPromptData(PromptData)`를 호출한다.

### `SetPromptData(const FFocusPromptData& InPromptData)`

- `CurrentPromptData = InPromptData`로 저장한다.
- `CurrentPromptData.bIsValid == false`이면:
  - `SetVisibility(ESlateVisibility::Collapsed)`
  - `OnPromptDataApplied(CurrentPromptData, false)`
  - return
- `CurrentPromptData.bIsValid == true`이면:
  - `TargetNameText->SetText(CurrentPromptData.DisplayName)`
  - `KeyText->SetText(CurrentPromptData.InteractionKeyText)`
  - `SetVisibility(ESlateVisibility::Visible)`
  - `OnPromptDataApplied(CurrentPromptData, true)`

`TargetNameText` 또는 `KeyText`가 null이어도 crash하지 않게 guard한다. 다만 `BindWidget` 필수 위젯이므로 누락이 확인되면 warning log를 남기는 것은 허용한다.

### `ClearPrompt()`

- `SetPromptData(FFocusPromptData())`를 호출한다.

## Blueprint 에셋 작업

`Content/UI/WBP_FocusPrompt`만 수정한다.

필수 작업:

1. Parent Class를 `UFocusPromptWidget`으로 변경한다.
2. Designer tree의 `TargetNameText`, `KeyText` 이름을 유지한다.
3. 기존 EventGraph의 다음 로직을 제거하거나 더 이상 실행되지 않게 정리한다.
   - `PreConstruct` visibility 처리
   - `Construct`의 `GetOwningPlayerPawn` / `Cast To BeekeeperCharacter` / `OnFocusPromptChanged` 바인딩
   - `OnFocusPromptChanged` custom event
   - `UpdateFocusPrompt`
   - `SetText`, `SetVisibility` 갱신 노드
4. 레이아웃, 폰트, 색상, 이미지, 스페이서 등 외형은 유지한다.

`BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름은 변경하지 않는다.

## 문서 갱신

구현 후 아래 문서를 갱신한다.

- `.md/0_ARCHITECTURE.md`
  - `WBP_FocusPrompt`의 런타임 바인딩과 표시 갱신이 `UFocusPromptWidget` C++ base class 책임이라고 기록한다.
  - `BP_BeekeeperCharacter`는 생성/viewport 추가만 유지한다고 기록한다.
- `.md/Architecture/UISystem.md`
  - `UFocusPromptWidget`을 UI system scope/key class에 추가한다.
  - prompt widget이 `UBeekeeperFocusComponent::OnFocusPromptChanged`를 구독하고 `FFocusPromptData`를 표시한다고 기록한다.
- `.md/Architecture/FocusSystem.md`
  - focus prompt의 데이터 source는 계속 `UBeekeeperFocusComponent`와 `FFocusPromptData`이며, UI는 표시만 담당한다고 확인한다.

## 검증

### 검색 검증

```powershell
rg "UFocusPromptWidget|FocusPromptWidget|OnPromptDataApplied|BindToFocusComponent|HandleFocusPromptChanged" Source/BeekeepingSim .md
rg "OnFocusPromptChanged|UpdateFocusPrompt|TargetNameText|KeyText" Source/BeekeepingSim Content/UI .md
```

확인할 것:

- `UFocusPromptWidget`가 `Source/BeekeepingSim/Public/UI`와 `Source/BeekeepingSim/Private/UI`에 추가되었다.
- `TargetNameText`, `KeyText`는 C++ `BindWidget` 이름과 Blueprint designer 이름이 일치한다.
- `WBP_FocusPrompt`의 EventGraph가 더 이상 prompt binding/update 책임을 갖지 않는다.
- `BP_BeekeeperCharacter`의 생성/viewport 추가 흐름은 유지된다.

### 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

### Blueprint 검증

가능하면 Unreal Editor 또는 commandlet에서 `WBP_FocusPrompt`를 compile/save한다.

확인할 것:

1. `WBP_FocusPrompt` parent class가 `UFocusPromptWidget`이다.
2. `TargetNameText`, `KeyText`가 정상적으로 bind된다.
3. Blueprint compile error가 없다.
4. `BP_BeekeeperCharacter`가 기존처럼 `WBP_FocusPrompt`를 생성하고 viewport에 추가한다.

### 런타임 시나리오 검증

가능하면 PIE에서 확인한다.

1. 플레이 직후 focus target이 없으면 prompt는 숨김 상태다.
2. focus target 진입 시 `TargetNameText`와 `KeyText`가 `FFocusPromptData` 값으로 갱신되고 prompt가 보인다.
3. focus target 이탈 또는 invalid prompt 수신 시 prompt가 `Collapsed`가 된다.
4. engaged focus prompt override가 들어오면 override의 `DisplayName`과 `InteractionKeyText`가 표시된다.
5. 위젯 제거 또는 PIE 종료 시 delegate 해제 관련 오류가 없다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `WBP_FocusPrompt`의 designer tree에서 `TargetNameText` 또는 `KeyText`가 없거나 이름 변경이 필요한 경우
- `WBP_FocusPrompt` parent class 변경이 다른 Blueprint 참조를 깨는 경우
- `BP_BeekeeperCharacter`가 `CreateWidget(WBP_FocusPrompt)`에 올바른 owning player를 전달하지 않아 `GetOwningPlayerPawn()` 자동 바인딩이 성립하지 않는 경우
- `UBeekeeperFocusComponent::OnFocusPromptChanged` 또는 `GetCurrentPromptData()`의 계약이 문서와 현재 Source에서 다르게 확인되는 경우
- Blueprint asset compile/save 자동화가 불가능해 수동 Unreal Editor 작업이 필요한 경우
