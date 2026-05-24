# FocusEngaged Item Use Area 세션 유지 문서

## 목적

다음 세션에서 FocusEngaged item-use-area 기능과 소독약/화분떡 효과 설계를 바로 이어가기 위한 작업 메모다.

이 문서는 정본 아키텍처 문서를 대체하지 않는다. 구현/설계 재개 시 아래 문서를 먼저 읽는다.

- `.md/0_ARCHITECTURE.md`
- `.md/Architecture/FocusSystem.md`
- `.md/Architecture/InventorySystem.md`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/QNA_ARCHITECTURE.md`
- `.md/PROMPT_IMPLEMENTATION.md`

## 현재 기능 방향

- Item-use area는 벌통 전용 기능이 아니라 FocusEngaged 상태의 host actor가 선택적으로 제공하는 generic 기능이다.
- `ABeehive`는 generic 구조를 사용하는 첫 구현 host다.
- FocusEngaged host가 item-use-area scope/provider를 지원하고 대상 아이템이 선택되어 있으면, 해당 아이템에 대응되는 사용영역은 LMB 조작 여부와 무관하게 항상 표시/점멸한다.
- Anchored cursor FocusEngaged 진입 시 hotbar 선택은 빈손으로 전환한다.
- Item-use area는 FocusEngaged 진입 후 플레이어가 다시 대상 아이템을 선택했을 때 활성화된다.
- 선택 아이템이 없거나 host가 item-use-area를 지원하지 않으면 기존 FocusAction/PartFocus 입력 정책을 따른다.

## 현재 구현 상태

구현된 주요 타입/파일:

- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaTypes.h`
  - `FItemUseAreaVisualSettings`
  - `FItemUseAreaDescriptor`
- `Source/BeekeepingSim/Public/Focus/ItemUseAreaProvider.h`
  - `IItemUseAreaProvider`
- `Source/BeekeepingSim/Public/Focus/CursorItemUseAreaScopeComponent.h`
- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Public/Inventory/HoldItemUseAction.h`
- `Source/BeekeepingSim/Private/Inventory/HoldItemUseAction.cpp`

수정된 주요 흐름:

- `UAnchoredFocusCursorActionComponent`
  - FocusEngaged 시작 시 `UCursorItemUseAreaScopeComponent`를 activate한다.
  - Focus return/abort 시 item-use-area scope를 deactivate한다.
  - LMB pressed는 선택 아이템이 있을 때 item-use-area scope에 우선 전달한다.
  - LMB released는 active item-use session 종료에 사용한다.
  - `ShouldClearHotbarSelectionOnFocusEngaged()`는 `true`다.
- `UBeekeeperFocusComponent`
  - LMB release hook이 engaged action으로 전달되는 경로가 추가되어 있다.
- `UBeekeeperHotbarComponent`
  - FocusEngaged 진입 시 active focus action 정책에 따라 선택 해제한다.

최근 빌드:

- `BeekeepingSimEditor Win64 Development` 빌드 성공.

## 중요한 구현 디테일

### 사용영역 표시와 collision

현재 scope는 material parameter로 표시 상태를 바꾼다.

- `UseAreaColor`
- `UseAreaOpacity`
- `PulseSpeed`
- `HoverStrength`

Inactive use area가 뒤쪽 사용영역/PartFocus trace를 막지 않도록 최근 수정했다.

- active descriptor의 `HitComponent`만 cursor trace channel에 `Block`
- inactive use area와 visual-only component는 cursor trace channel에 `Ignore`
- scope deactivate/rebuild 시 원래 collision 설정 복구

에디터에서 사용영역 mesh는 다음 기준이 안전하다.

- `Visible = true`
- `Hidden In Game = false`
- collision은 scope가 trace response를 조정한다.
- material opacity 기본값이 0 또는 낮은 값이어도 component 자체를 숨기지는 않는다.

### Blueprint descriptor 등록 방식

Blueprint에서는 `ItemUseAreaProvider` 인터페이스를 구현한다.

노드 기준:

- `Class Settings -> Implemented Interfaces -> ItemUseAreaProvider`
- `Get Item Use Area Descriptors`
- `Make Item Use Area Descriptor`
- `Make Array`
- Return node의 `Out Descriptors`

고정 개수 영역이면 `Out Descriptors`에 `Add`를 직접 연결하지 않는다. Return pin은 수정 가능한 배열 입력이 아니므로 `Make Array`로 반환하는 것이 가장 단순하다.

예: `BP_Beehive`

- `LidUseAreaMesh`
- `CombUseAreaMesh`
- `InnerUseAreaMesh`

각 descriptor:

- `AreaId`
- `AreaTags`
- `OwnerActor = Self`
- `HitComponent = 사용영역 mesh`
- `VisualComponents = MakeArray(사용영역 mesh)`
- `EffectTargetObject = Self` 또는 실제 효과 대상
- `VisualSettings`는 비워두면 C++ struct 기본값 사용

### VisualSettings 기본값

`VisualSettings`를 비워두면 `FItemUseAreaVisualSettings` C++ 기본값이 적용된다.

현재 기본:

- `UseAreaColor = FLinearColor(0.2f, 0.8f, 1.0f, 0.35f)`
- `UseAreaOpacity = 0.35f`
- `PulseSpeed = 2.0f`
- `HoverStrength = 1.0f`

전역 기본값을 에디터에서 바꾸는 설정은 아직 없다.

## 현재 알려진 설계/구현 문제

### HoldItemUseAction Blueprint 이벤트 부재

현재 `UHoldItemUseAction`의 함수들은 `BlueprintCallable`일 뿐 Blueprint override 이벤트가 아니다.

함수:

- `CanBeginUse`
- `BeginUse`
- `TickUse`
- `EndUse`
- `CanApplyUseEffect`
- `ApplyUseEffect`

현재 Blueprint에서 이 함수들을 이벤트로 구현할 수 없다.

필요한 보완 후보:

- `BlueprintNativeEvent`로 변경
- 또는 `ReceiveCanBeginUse`, `ReceiveBeginUse`, `ReceiveTickUse`, `ReceiveEndUse`, `ReceiveCanApplyUseEffect`, `ReceiveApplyUseEffect` 같은 `BlueprintImplementableEvent` wrapper 추가

사용자가 직접 수정하라고 하기 전까지 구현하지 않는다. 다음 구현 요청이 오면 이 부분이 우선 후보다.

### 화분떡은 hold-use와 다름

화분떡은 LMB hold 지속 효과가 아니라 유효 사용영역 위 LMB click 1회 효과다.

현재 `UHoldItemUseAction` 이름/구조가 지속 사용 중심이므로 아래 실행 모드 도입이 필요할 수 있다.

- `ContinuousWhileHeld`
- `InstantOnPress`

QnA에 결정 항목을 추가해 두었고 아직 답변은 없다.

## 소독약/화분떡 설계 요구사항

사용자 요구:

- 소독약 사용중(LMB hold): 소독약 분사 Niagara 재생 상태
- 소독약 영향(사용영역 hover + LMB hold): Tick마다 대상 벌통의 위생성 증가
- 화분떡 사용중: 없음
- 화분떡 영향(사용영역 hover + LMB click): 해당 사용영역 자리에 화분떡 부착
- 해당 자리에 화분떡이 있으면 기존 활성조건과 무관하게 사용영역 비활성화

작성된 QnA 섹션:

- `.md/QNA_ARCHITECTURE.md`
- `### Beehive Item Use Effects QnA`

현재 미답변 항목:

1. Item-use action 실행 방식 구분
2. 소독약 위생성 상태의 소유 위치
3. 소독약 증가량 단위
4. 소독약 분사 Niagara의 소유 위치
5. 화분떡 부착 가능 slot 수
6. 화분떡 부착 actor/표현 방식
7. 화분떡 occupied 상태 소유 위치
8. 화분떡 사용영역 비활성화 방식
9. 화분떡 아이템 소모 정책
10. 화분떡 효과 target 기준

현재 권장안 요약:

- action 실행 모드 enum 추가: `ContinuousWhileHeld`, `InstantOnPress`
- 위생성 상태는 `ABeehive`가 소유
- 소독약 증가량은 item action 설정값 `SanitationIncreasePerSecond * DeltaTime`
- 소독약 Niagara는 held/on-cursor presentation actor가 소유하고 action이 재생/정지만 요청
- 화분떡 slot은 벌통당 여러 개 허용
- 화분떡은 전용 actor 또는 item presentation actor subclass를 spawn/attach
- occupied 상태는 `ABeehive`가 `AreaId` 또는 slot id 기준으로 관리
- occupied slot은 provider가 descriptor를 반환하지 않는 방식으로 비활성화
- 화분떡은 부착 성공 시 stack count 1 감소, 실패 시 소모 없음
- 화분떡 action은 `EffectTargetObject = ABeehive`, `ItemUseAreaId`로 벌통 API에 설치 요청

## 다음 세션 추천 순서

1. `.md/QNA_ARCHITECTURE.md`의 `Beehive Item Use Effects QnA` 답변 확정
2. 확정 답변을 `.md/PROMPT_IMPLEMENTATION.md`에 반영
3. 필요 시 `.md/Architecture/FocusSystem.md`, `.md/Architecture/InventorySystem.md`, `.md/Architecture/WorldActorsSystem.md`에 설계 반영
4. 구현 요청이 명시되면:
   - `UHoldItemUseAction` Blueprint event hook 보완
   - item-use action 실행 모드 추가
   - `ABeehive` 위생성 상태/API 추가
   - 소독약 action/presentation 연동
   - 화분떡 slot/occupied/attach API 설계 및 구현

## 주의

- 사용자가 설계/문서 요청만 했을 때 C++ 코드를 수정하지 않는다.
- Content asset은 직접 수정하지 않는다.
- Blueprint API rename/delete는 피한다.
- UCLASS/USTRUCT/UENUM rename이 필요하면 Core Redirect/QnA가 먼저다.
- 사용자가 “직접 코드 수정”을 명시한 경우에만 구현한다.
