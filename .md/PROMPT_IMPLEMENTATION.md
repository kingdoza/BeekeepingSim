# Focus Prompt 위치 추적 기능 구현 프롬프트

## 목표

`WBP_FocusPrompt` / `UFocusPromptWidget`에 prompt 위치 정책을 추가한다.

- 전체 Focus 대상 prompt는 화면 중앙 근처에 표시한다.
- PartFocus 대상 prompt는 마우스 커서 근처를 따라다니게 한다.

현재 `UFocusPromptWidget`은 이미 C++ base widget으로 존재하며, prompt text/visibility 갱신을 담당한다. 이번 작업은 그 구조를 유지하면서 위치 갱신 책임을 추가한다.

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/UISystem.md`
- `.md/Architecture/CoreSystem.md`
- `.md/QNA_ARCHITECTURE.md`

특히 `.md/QNA_ARCHITECTURE.md`의 `Focus Prompt 위치 정책 QnA` 답변을 따른다. 1-6번 모두 `옵션 A`로 확정되어 있다.

## 확정 정책

1. Prompt 위치 정책은 `FFocusPromptData`에 싣는다.
2. `FFocusPromptData`에 `EFocusPromptAnchorMode` enum property를 추가한다.
3. 전체 Focus prompt는 `ScreenCenter` mode로 표시한다.
4. PartFocus prompt는 `MouseCursor` mode로 표시한다.
5. 전체 Focus 기준점은 world projection이 아니라 viewport center다.
6. PartFocus prompt는 `UFocusPromptWidget::NativeTick()`에서 visible 상태 동안 매 프레임 위치를 갱신한다.
7. 위치 갱신을 위한 전체 prompt 컨테이너는 `PromptContent` 필수 `BindWidget`으로 참조한다.
8. prompt가 viewport 밖으로 나가지 않도록 `ViewportPadding` 기준 clamp를 적용한다.
9. `ScreenCenterOffset`, `MouseCursorOffset`, `ViewportPadding`은 `UFocusPromptWidget` 소속 UI layout property다.
10. `UBeekeepingSimFocusSettings`에는 이 UI spacing 값을 추가하지 않는다.

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`
- `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`

Content:

- `Content/UI/WBP_FocusPrompt`

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/UISystem.md`
- 필요 시 `.md/USER_UNREAL.md`

수정 금지:

- `BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름
- `Content/UI/WBP_FocusPrompt` 외의 다른 `Content/` asset
- Focus target 판정 방식
- PartFocus hover/trace/action 정책
- `UBeekeepingSimFocusSettings`에 prompt visual spacing 추가

## `FFocusPromptData` 변경

`Source/BeekeepingSim/Public/Focus/FocusTargetComponent.h`에 prompt anchor enum을 추가한다.

권장 형태:

```cpp
UENUM(BlueprintType)
enum class EFocusPromptAnchorMode : uint8
{
	ScreenCenter,
	MouseCursor
};
```

`FFocusPromptData`에 property를 추가한다.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus")
EFocusPromptAnchorMode AnchorMode = EFocusPromptAnchorMode::ScreenCenter;
```

정책:

- 기본값은 `ScreenCenter`다.
- 기존 `UFocusTargetComponent::GetPromptData()`는 별도 anchor 설정 없이 기본값을 사용한다.
- additive USTRUCT property 추가이므로 class/struct rename이나 Core Redirect는 필요하지 않다.

## PartFocus prompt 변환 변경

`Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`의 `BroadcastPartPrompt()`에서 `FCursorPartFocusPromptData`를 `FFocusPromptData`로 변환할 때 anchor mode를 지정한다.

권장:

```cpp
FFocusPromptData FocusPromptData;
FocusPromptData.bIsValid = PromptData.bIsValid;
FocusPromptData.DisplayName = PromptData.DisplayName;
FocusPromptData.InteractionKeyText = PromptData.InteractionKeyText;
FocusPromptData.AnchorMode = EFocusPromptAnchorMode::MouseCursor;
OwnerFocusComponent->SetEngagedFocusPromptOverride(FocusPromptData);
```

정책:

- `FCursorPartFocusPromptData`에는 anchor mode를 추가하지 않는다.
- 모든 PartFocus prompt는 `MouseCursor`로 변환한다.
- invalid prompt의 anchor mode는 표시되지 않으므로 실질 동작에 영향이 없어야 한다.

## `UFocusPromptWidget` Header 변경

`Source/BeekeepingSim/Public/UI/FocusPromptWidget.h`에 prompt content와 layout property를 추가한다.

필요한 forward declaration:

```cpp
class UWidget;
```

필수 `BindWidget`:

```cpp
UPROPERTY(BlueprintReadOnly, meta = (BindWidget, AllowPrivateAccess = "true"))
TObjectPtr<UWidget> PromptContent;
```

layout property:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Prompt|Layout", meta = (AllowPrivateAccess = "true"))
FVector2D ScreenCenterOffset = FVector2D(20.0f, 0.0f);

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Prompt|Layout", meta = (AllowPrivateAccess = "true"))
FVector2D MouseCursorOffset = FVector2D(18.0f, 0.0f);

UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Focus Prompt|Layout", meta = (AllowPrivateAccess = "true"))
FVector2D ViewportPadding = FVector2D(8.0f, 8.0f);
```

tick override:

```cpp
virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
```

권장 private helper:

```cpp
void UpdatePromptPosition();
bool TryGetPromptAnchorPosition(FVector2D& OutPosition) const;
bool TryGetViewportSizeInWidgetUnits(FVector2D& OutViewportSize) const;
bool TryGetMousePositionInWidgetUnits(FVector2D& OutMousePosition) const;
static FVector2D ClampPromptAnchorToViewport(
	const FVector2D& AnchorPosition,
	const FVector2D& ViewportSize,
	const FVector2D& DesiredSize,
	const FVector2D& Alignment,
	const FVector2D& Padding);
```

helper 이름과 분할은 기존 style에 맞게 조정해도 된다.

## `UFocusPromptWidget` CPP 동작

필요 include 예:

```cpp
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Widget.h"
```

### `NativeConstruct()`

기존 focus binding 동작은 유지한다.

추가 정책:

- runtime 시작 시 `Collapsed` 유지
- 필요하면 widget tick이 실제로 호출되도록 engine-supported API를 사용한다.
- `PromptContent`의 slot이 `UCanvasPanelSlot`인지 확인하되, 없다고 crash하지 않는다.

### `SetPromptData(const FFocusPromptData& InPromptData)`

기존 text/visibility 갱신을 유지한다.

변경:

- valid prompt를 `Visible`로 만든 뒤 `UpdatePromptPosition()`을 호출한다.
- invalid prompt는 기존처럼 `Collapsed`로 숨긴다.

권장 흐름:

```cpp
CurrentPromptData = InPromptData;

if (!CurrentPromptData.bIsValid)
{
	SetVisibility(ESlateVisibility::Collapsed);
	OnPromptDataApplied(CurrentPromptData, false);
	return;
}

TargetNameText->SetText(...);
KeyText->SetText(...);
SetVisibility(ESlateVisibility::Visible);
UpdatePromptPosition();
OnPromptDataApplied(CurrentPromptData, true);
```

### `NativeTick()`

visible prompt만 위치를 반복 갱신한다.

권장:

```cpp
Super::NativeTick(MyGeometry, InDeltaTime);

if (CurrentPromptData.bIsValid && GetVisibility() != ESlateVisibility::Collapsed)
{
	UpdatePromptPosition();
}
```

`Hidden`/`Collapsed`/비표시 상태 판정은 더 엄밀하게 조정해도 된다.

### 위치 계산

좌표계는 반드시 하나로 통일한다.

권장:

- viewport size와 mouse position을 UMG layout 단위로 변환한다.
- raw pixel 좌표를 그대로 `CanvasPanelSlot::SetPosition()`에 넣지 않는다.
- `UWidgetLayoutLibrary::GetViewportScale()`을 사용하거나 `GetMousePositionScaledByDPI()`/동등 API를 사용한다.

권장 기준점:

```text
ScreenCenter:
  ViewportSize * 0.5 + ScreenCenterOffset

MouseCursor:
  MousePosition + MouseCursorOffset
```

`PromptContent` 크기:

- text 변경 후 `ForceLayoutPrepass()`를 호출하고 `PromptContent->GetDesiredSize()`를 사용한다.
- 원하는 크기를 얻지 못하면 clamp 없이 anchor 위치를 사용하거나 안전한 fallback을 둔다.

clamp:

- `PromptContent`가 `UCanvasPanelSlot`에 있어야 한다.
- slot alignment를 반영한다.
- 현재 WBP의 vertical center alignment를 유지할 수 있도록 `CanvasSlot->GetAlignment()` 값을 사용한다.

권장 clamp 공식:

```text
MinAnchor = ViewportPadding + DesiredSize * Alignment
MaxAnchor = ViewportSize - ViewportPadding - DesiredSize * (1 - Alignment)
ClampedAnchor.X = Clamp(Anchor.X, MinAnchor.X, MaxAnchor.X)
ClampedAnchor.Y = Clamp(Anchor.Y, MinAnchor.Y, MaxAnchor.Y)
```

content가 viewport보다 커서 `MaxAnchor < MinAnchor`가 되는 경우도 crash 없이 처리한다.

마지막 적용:

```cpp
CanvasSlot->SetPosition(ClampedAnchor);
```

## Blueprint 에셋 작업

`Content/UI/WBP_FocusPrompt`만 수정한다.

필수:

1. parent class는 계속 `UFocusPromptWidget`으로 유지한다.
2. 현재 prompt 전체를 감싸는 root content widget을 `PromptContent`로 이름 변경한다.
   - 현재 구조 기준으로 `CanvasPanel`의 direct child인 기존 `VerticalBox_52`가 대상이다.
3. `PromptContent`는 variable로 노출되어 C++ `BindWidget`과 연결되어야 한다.
4. `TargetNameText`, `KeyText` 이름은 유지한다.
5. `PromptContent`는 `CanvasPanelSlot` 아래에 있어야 한다.
6. `PromptContent` slot은 runtime 위치 제어에 맞게 설정한다.
   - Anchors: top-left `(0, 0)` / `(0, 0)`
   - Auto Size: true 유지 권장
   - Alignment: 기존 vertical center 느낌을 유지하려면 `(0, 0.5)` 권장
7. 기존 prompt text/visibility EventGraph 로직은 다시 추가하지 않는다.

`BP_BeekeeperCharacter`의 `CreateWidget(WBP_FocusPrompt)` / `AddToViewport` 흐름은 변경하지 않는다.

## 문서 갱신

구현 후 아래 문서를 갱신한다.

### `.md/0_ARCHITECTURE.md`

- `FFocusPromptData`에 prompt anchor mode가 포함된다고 기록한다.
- 전체 Focus prompt는 screen center 기준, PartFocus prompt는 mouse cursor 기준으로 표시된다고 기록한다.
- 위치 갱신은 `UFocusPromptWidget`이 담당한다고 기록한다.

### `.md/Architecture/FocusSystem.md`

- Focus prompt data source는 계속 `UBeekeeperFocusComponent`라고 기록한다.
- `UFocusTargetComponent`가 만드는 일반 focus prompt는 `ScreenCenter` mode라고 기록한다.
- `UCursorPartFocusScopeComponent`가 engaged prompt override를 만들 때 `MouseCursor` mode로 변환한다고 기록한다.
- Focus system은 widget 위치를 직접 조작하지 않는다고 기록한다.

### `.md/Architecture/UISystem.md`

- `UFocusPromptWidget`이 `FFocusPromptData::AnchorMode`에 따라 prompt 위치를 갱신한다고 기록한다.
- `PromptContent`, `ScreenCenterOffset`, `MouseCursorOffset`, `ViewportPadding` 계약을 기록한다.
- `NativeTick()` 위치 갱신은 PartFocus cursor follow를 위한 UI 표시 책임이라고 기록한다.

## 검색 검증

```powershell
rg "EFocusPromptAnchorMode|AnchorMode|ScreenCenterOffset|MouseCursorOffset|ViewportPadding|PromptContent|UpdatePromptPosition" Source/BeekeepingSim .md
rg "SetEngagedFocusPromptOverride|BroadcastPartPrompt|GetPromptData" Source/BeekeepingSim/Public/Focus Source/BeekeepingSim/Private/Focus
rg "FocusPromptWidget|WBP_FocusPrompt|TargetNameText|KeyText|PromptContent" Source/BeekeepingSim Content/UI .md
```

확인할 것:

- `FFocusPromptData`에 `AnchorMode`가 추가되었다.
- 일반 focus prompt는 기본 `ScreenCenter`를 사용한다.
- PartFocus prompt override는 `MouseCursor`로 설정된다.
- `PromptContent`, `TargetNameText`, `KeyText`가 모두 C++ `BindWidget`과 Blueprint designer 이름으로 일치한다.
- `WBP_FocusPrompt`의 EventGraph가 prompt binding/text/visibility/position 책임을 다시 갖지 않는다.
- `UBeekeepingSimFocusSettings`에 prompt offset/padding 값이 추가되지 않았다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## Blueprint 검증

가능하면 Unreal Editor 또는 commandlet에서 `WBP_FocusPrompt`를 compile/save한다.

확인할 것:

1. `WBP_FocusPrompt` parent class가 `UFocusPromptWidget`이다.
2. `PromptContent`, `TargetNameText`, `KeyText`가 정상 bind된다.
3. Blueprint compile error가 없다.
4. `PromptContent`가 `CanvasPanelSlot` 아래에 있고 runtime 위치 이동이 가능하다.
5. `BP_BeekeeperCharacter`가 기존처럼 `WBP_FocusPrompt`를 생성하고 viewport에 추가한다.

## 런타임 시나리오 검증

가능하면 PIE에서 확인한다.

1. 플레이 직후 focus target이 없으면 prompt는 숨김 상태다.
2. 전체 Focus target에 진입하면 prompt가 화면 중앙 근처에 표시된다.
3. 전체 Focus target에서 벗어나면 prompt가 숨김 상태가 된다.
4. FocusEngaged host 내부 PartFocus hover 대상에 진입하면 prompt가 마우스 커서 근처에 표시된다.
5. PartFocus hover 대상이 유지된 상태로 마우스를 움직이면 prompt가 매 프레임 커서를 따라간다.
6. 마우스가 viewport edge 근처에 있어도 prompt가 viewport 밖으로 잘리지 않는다.
7. PartFocus hover가 사라지거나 invalid prompt가 브로드캐스트되면 prompt가 숨김 상태가 된다.
8. PIE 종료/위젯 제거 시 delegate 해제 관련 오류가 없다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- `WBP_FocusPrompt`에서 `PromptContent`로 지정할 단일 root content widget을 안정적으로 특정할 수 없는 경우
- `PromptContent`가 `CanvasPanelSlot` 아래에 있지 않아 위치 제어 방식 변경이 필요한 경우
- `NativeTick()`이 호출되지 않는 엔진/asset 설정이 확인되어 별도 tick 활성화 정책을 정해야 하는 경우
- DPI 변환 후 위치가 실제 viewport 좌표와 맞지 않아 좌표계 정책 재결정이 필요한 경우
- `FFocusPromptData`에 enum property를 추가한 뒤 Blueprint compile/save에서 호환성 문제가 발생하는 경우
- `BP_BeekeeperCharacter`의 widget 생성 흐름을 건드리지 않고는 owning player / viewport 좌표를 안정적으로 얻을 수 없는 경우
