# Focus Prompt ActionName Blueprint 노출 구현 프롬프트

## 목표

Focus prompt multi-entry 설계에서 사용할 action name source를 Blueprint/Details에서 설정 가능하게 노출한다.

이번 작업은 **ActionName 노출 API만 구현**한다.

구현하지 않는 것:

- `FFocusPromptEntry` 추가
- `FFocusPromptData::Entries` 추가
- `AppendFocusPromptEntries(...)`
- `AppendPartFocusPromptEntries(...)`
- `OnPromptEntriesApplied(...)`
- hotbar `PreviewAcquireItemBySpec(...)`
- prompt row 생성/수직 정렬/disabled alpha
- pickup/retrieve availability 계산
- `WBP_FocusPrompt` UI row 작업

## 반드시 읽을 문서

- `.md/AGENT_IMPLEMENTATION.md`
- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/CoreSystem.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_IMPLEMENTATION.md`

## 구현 대상

Source:

- `Source/BeekeepingSim/Public/Focus/FocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/FocusActionComponent.cpp`
- `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusActionComponent.cpp`
- `Source/BeekeepingSim/Private/Interaction/PickupFocusActionComponent.cpp`
- `Source/BeekeepingSim/Private/Interaction/StorageBoxFocusActionComponent.cpp`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombPartFocusActionComponent.cpp`
- 필요 시 관련 header

문서:

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

수정 금지:

- `FFocusPromptData` 구조 변경
- `UFocusPromptWidget` 변경
- `WBP_FocusPrompt` 변경
- multi-entry prompt 수집/렌더링 구현
- BlueprintCallable/Public API 삭제 또는 rename
- UCLASS/USTRUCT/UENUM rename
- Core Redirect가 필요한 변경

## 전역 Focus ActionName API

`UFocusActionComponent`에 Blueprint authoring 가능한 prompt action name API를 추가한다.

권장 header:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Action|Prompt")
FText PromptActionText;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Focus Action|Prompt")
FText EngagedPromptActionText;

UFUNCTION(BlueprintCallable, Category = "Focus Action|Prompt")
void SetPromptActionText(const FText& NewText);

UFUNCTION(BlueprintPure, Category = "Focus Action|Prompt")
FText GetPromptActionText() const;

UFUNCTION(BlueprintCallable, Category = "Focus Action|Prompt")
void SetEngagedPromptActionText(const FText& NewText);

UFUNCTION(BlueprintPure, Category = "Focus Action|Prompt")
FText GetEngagedPromptActionText() const;

UFUNCTION(BlueprintPure, Category = "Focus Action|Prompt")
virtual FText ResolveFocusPromptActionText() const;
```

기본 동작:

- `ResolveFocusPromptActionText()`는 `IsActionEngaged()`가 true이고 `EngagedPromptActionText`가 비어 있지 않으면 `EngagedPromptActionText`를 반환한다.
- 그 외에는 `PromptActionText`를 반환한다.
- 이 resolver는 나중에 multi-entry prompt의 `ActionText` source로 사용된다.

기본값:

- `UPickupFocusActionComponent`: `PromptActionText = "획득"`
- `UStorageBoxFocusActionComponent`: `PromptActionText = "열기"`

주의:

- 이번 작업에서 기존 focus prompt 표시 텍스트를 이 값으로 바꾸지 않는다.
- 현재 UI가 단일 `InteractionKeyText`만 표시하더라도, 이 API는 future multi-entry `ActionText` source로만 준비한다.

## PartFocus ActionName API

`UCursorPartFocusActionComponent`에 Blueprint authoring 가능한 primary action name API를 추가한다.

권장 header:

```cpp
UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus|Prompt")
FText PrimaryPromptActionText;

UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor Part Focus|Prompt")
FText EngagedPrimaryPromptActionText;

UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Prompt")
void SetPrimaryPromptActionText(const FText& NewText);

UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Prompt")
FText GetPrimaryPromptActionText() const;

UFUNCTION(BlueprintCallable, Category = "Cursor Part Focus|Prompt")
void SetEngagedPrimaryPromptActionText(const FText& NewText);

UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Prompt")
FText GetEngagedPrimaryPromptActionText() const;

UFUNCTION(BlueprintPure, Category = "Cursor Part Focus|Prompt")
virtual FText ResolvePrimaryPromptActionText() const;
```

기본 동작:

- `ResolvePrimaryPromptActionText()`는 `IsPartActionEngaged()`가 true이고 `EngagedPrimaryPromptActionText`가 비어 있지 않으면 `EngagedPrimaryPromptActionText`를 반환한다.
- 그 외에는 `PrimaryPromptActionText`를 반환한다.
- 이 resolver는 나중에 PartFocus primary entry의 `ActionText` source로 사용된다.

기본값/authoring:

- `ABeehive` native lid action:
  - `PrimaryPromptActionText = "열기"`
  - `EngagedPrimaryPromptActionText = "닫기"`
- `UBeehiveCombPartFocusActionComponent`:
  - `PrimaryPromptActionText = "들기"`
  - `EngagedPrimaryPromptActionText = "넣기"`

주의:

- `FCursorPartFocusPromptData`에는 action name을 추가하지 않는다.
- `FCursorPartFocusPromptData::DisplayName`은 대상 이름이다.
- `FCursorPartFocusPromptData::InteractionKeyText`는 입력 키 텍스트다.
- action name은 실행 주체인 action component가 소유한다.

## 구현 세부 정책

- 새 property는 `EditAnywhere, BlueprintReadWrite`로 Details/BP에서 조정 가능하게 둔다.
- setter/getter는 단순 assign/return으로 충분하다.
- resolver는 virtual로 둬 복잡한 상태 기반 명칭이 필요한 subclass가 override할 수 있게 한다.
- 이번 작업에서는 resolver override를 새로 만들 필요가 없다.
- `ABeehive`의 lid action은 현재 공통 `UCursorPartFocusActionComponent`를 사용하므로 constructor에서 기본 text를 설정한다.
- `UBeehiveCombPartFocusActionComponent`는 constructor에서 소비장 기본 text를 설정한다.

## 문서 갱신

구현 후 실제 API 이름과 일치하도록 아래 문서를 확인/갱신한다.

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InteractionSystem.md`
- `.md/Architecture/WorldActorsSystem.md`

문서에는 다음을 남긴다.

- 전역 Focus action name은 `UFocusActionComponent`가 소유한다.
- PartFocus primary action name은 `UCursorPartFocusActionComponent`가 소유한다.
- 공통 PartFocus component는 engaged 상태에 따라 `PrimaryPromptActionText` / `EngagedPrimaryPromptActionText`를 전환한다.
- 뚜껑은 `열기`/`닫기`, 소비장은 `들기`/`넣기` 기본 authoring을 가진다.

## 검색 검증

```powershell
rg "PromptActionText|EngagedPromptActionText|ResolveFocusPromptActionText" Source/BeekeepingSim .md
rg "PrimaryPromptActionText|EngagedPrimaryPromptActionText|ResolvePrimaryPromptActionText" Source/BeekeepingSim .md
rg "획득|열기|닫기|들기|넣기" Source/BeekeepingSim/Private/Interaction Source/BeekeepingSim/Private/WorldActors .md
```

확인할 것:

- `UFocusActionComponent`에 action name API가 노출되었다.
- `UCursorPartFocusActionComponent`에 primary action name API가 노출되었다.
- pickup/storage 기본값이 설정되었다.
- beehive lid 기본값이 설정되었다.
- beehive comb 기본값이 설정되었다.
- 기존 prompt 표시/위치 동작은 변경되지 않았다.
- multi-entry 관련 API가 이번 작업에서 추가되지 않았다.

## 빌드 검증

가능하면 UBT 빌드를 수행한다.

```powershell
& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE
```

엔진 경로가 없으면 임의 경로로 대체하지 말고 최종 보고에 빌드 미수행 사유를 적는다.

## 중단 조건

아래 상황이면 구현을 멈추고 `.md/QNA_IMPLEMENTATION.md`에 질문한다.

- ActionName property를 추가하려면 기존 Blueprint-exposed property/API rename이 필요해지는 경우
- resolver에 context 인자가 없으면 구현 목적을 달성할 수 없는 기존 코드 경로가 발견되는 경우
- `ABeehive` lid action 기본값을 native constructor에서 안정적으로 설정할 수 없는 경우
- Content asset 수정/compile/save가 필요해지는 경우
- multi-entry 구현 없이는 ActionName 노출 API를 둘 위치가 불명확해지는 경우
