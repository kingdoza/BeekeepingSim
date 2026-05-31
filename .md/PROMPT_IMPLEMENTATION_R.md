# 구현 수정 프롬프트: WBP_FocusPrompt C++ 이관 리뷰 Findings

## 우선순위

1. Medium: `WBP_FocusPrompt` Blueprint에 남은 `UpdateFocusPrompt`/text/visibility 갱신 그래프 제거 또는 비활성화 확인
2. Low: `UFocusPromptWidget::NativeConstruct()`에서 runtime `Collapsed` 기본 상태를 `Super::NativeConstruct()`보다 먼저 적용
3. 참고: UBT 빌드 실패는 이번 변경 파일 기인이 아닌 기존 unity build 충돌로 분리

## 발견 문제

### 1. `WBP_FocusPrompt` asset에 기존 prompt update 그래프 흔적이 남아 있음

- 대상 파일:
  - `Content/UI/WBP_FocusPrompt.uasset`
- 확인 결과:
  - `NativeParentClass=/Script/BeekeepingSim.FocusPromptWidget` 확인됨
  - `TargetNameText`, `KeyText` 이름 확인됨
  - `UpdateFocusPrompt` 문자열 4건 확인됨
  - `SetText`, `SetVisibility`, `bIsValid`, `DisplayName`, `InteractionKeyText` 문자열 확인됨
  - `OnFocusPromptChanged`, `GetOwningPlayerPawn` 문자열은 0건이라 기존 delegate binding은 제거된 것으로 보임
- 원인:
  - parent class 변경과 delegate binding 제거는 되었지만, 기존 `UpdateFocusPrompt` 함수/노드 또는 stale editor graph/search data가 asset에 남아 있다.
- 영향:
  - 실제 활성 그래프라면 `WBP_FocusPrompt`가 여전히 text/visibility 갱신 책임을 일부 가진다.
  - 단순 stale metadata라도 리뷰 기준상 EventGraph 책임 제거를 문자열 검색만으로 확정할 수 없다.
- 수정 방향:
  - Unreal Editor에서 `WBP_FocusPrompt`를 열고 EventGraph/Functions를 확인한다.
  - `UpdateFocusPrompt` 함수와 prompt text/visibility 갱신 노드를 제거한다.
  - Blueprint는 layout/style과 선택적 `OnPromptDataApplied` 반응만 유지한다.
  - Compile/Save 후 아래 검색에서 `UpdateFocusPrompt`, `SetText`, `SetVisibility` 흔적이 사라지거나, 남는 항목이 의도된 `OnPromptDataApplied` 표시 연출뿐임을 명확히 한다.

### 2. `NativeConstruct()`의 runtime collapse가 `Super::NativeConstruct()` 이후에 실행됨

- 대상 파일:
  - `Source/BeekeepingSim/Private/UI/FocusPromptWidget.cpp`
- 원인:
  - 현재 구현은 `Super::NativeConstruct()` 호출 후 `SetVisibility(ESlateVisibility::Collapsed)`를 호출한다.
  - Unreal의 `UUserWidget::NativeConstruct()`는 Blueprint `Construct` 이벤트를 실행하므로, Blueprint Construct가 먼저 실행될 수 있다.
- 영향:
  - 최종 반환 전에는 C++이 collapse/bind를 적용하지만, "NativeConstruct 시작 시 runtime 기본 상태 Collapsed 보장" 요구와 정확히 일치하지 않는다.
  - Blueprint Construct나 animation이 남아 있으면 C++ 초기 상태보다 먼저 실행될 수 있다.
- 수정 방향:
  - runtime 기본 상태 collapse를 `Super::NativeConstruct()`보다 먼저 적용한다.
  - 이후 `Super::NativeConstruct()` 호출, owning pawn resolve, `BindToFocusComponent()` 순서를 유지한다.

```cpp
void UFocusPromptWidget::NativeConstruct()
{
	SetVisibility(ESlateVisibility::Collapsed);

	Super::NativeConstruct();

	ABeekeeperCharacter* BeekeeperCharacter = Cast<ABeekeeperCharacter>(GetOwningPlayerPawn());
	if (!BeekeeperCharacter)
	{
		return;
	}

	BindToFocusComponent(BeekeeperCharacter->GetBeekeeperFocus());
}
```

## 검증 방법

- 검색:
  - `rg "UFocusPromptWidget|FocusPromptWidget|OnPromptDataApplied|BindToFocusComponent|HandleFocusPromptChanged" Source/BeekeepingSim .md`
  - `rg -a "UpdateFocusPrompt|OnFocusPromptChanged|GetOwningPlayerPawn|SetText|SetVisibility|TargetNameText|KeyText|NativeParentClass" Content/UI/WBP_FocusPrompt.uasset`
  - `rg -a "WBP_FocusPrompt|CreateWidget|AddToViewport|OnFocusPromptChanged|UpdateFocusPrompt" Content/Beekeeper/BP_BeekeeperCharacter.uasset`
- UBT:
  - `BeekeepingSimEditor Win64 Development`
  - 현재 빌드 실패 원인:
    - `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`
    - `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`
    - 두 파일의 anonymous namespace `CenterMouseCursorInViewport` 함수명이 unity build에서 충돌
  - 위 빌드 blocker는 이번 FocusPrompt 변경 파일 기인이 아니지만, 최종 검증 전에 별도 수정이 필요하다.
- Editor/PIE:
  - `WBP_FocusPrompt` parent가 `UFocusPromptWidget`인지 확인
  - EventGraph/Functions에 prompt binding/update 책임이 없는지 확인
  - 플레이 직후 focus target 없음: prompt `Collapsed`
  - focus target 진입: `TargetNameText`, `KeyText` 갱신 + `Visible`
  - focus 이탈/invalid prompt: `Collapsed`
  - engaged prompt override 텍스트 반영
  - PIE 종료 시 delegate 해제 관련 오류 없음

## 아키텍처 문서 반영 필요 여부

- 현재 문서 반영은 대체로 완료.
- 추가 문서 반영은 위 수정이 구조/API를 바꾸지 않는 한 불필요.

## 참고 리뷰 결과

- High: 없음
- Medium:
  - `WBP_FocusPrompt` asset에 기존 `UpdateFocusPrompt`/text/visibility update graph 흔적이 남아 있어 Blueprint 책임 제거를 확정할 수 없음
- Low:
  - `NativeConstruct()` runtime collapse 순서가 요구사항의 "시작 시 Collapsed"와 불일치
- UBT/UHT:
  - UHT 통과
  - UBT 실패: 기존 unity build anonymous namespace 함수명 충돌

---

