# Source Scope Refactoring Candidates

## 목적

이 문서는 현재 `Source/BeekeepingSim/Public` 및 `Source/BeekeepingSim/Private` 기준으로 확인된 리팩토링 후보를 정리한다.

이 문서는 구현 지시서가 아니다. 실제 리팩토링을 수행할 때는 현재 Architecture 문서, Blueprint 참조 상태, 빌드 가능 상태를 다시 확인한 뒤 별도 구현 범위를 확정해야 한다.

## 분석 기준

- 분석 대상:
  - `Source/BeekeepingSim/Public/**/*.h`
  - `Source/BeekeepingSim/Private/**/*.cpp`
  - `Source/BeekeepingSim/Private/**/*.h`
- 제외 대상:
  - `Content/`
  - `Config/`
  - `Intermediate/`
  - `Saved/`
  - `Binaries/`
  - Unreal Engine 내부 코드
  - 플러그인 코드
- 현재 C++ 소스 규모:
  - `Source/BeekeepingSim/Public` + `Source/BeekeepingSim/Private`: 약 `10,689 LOC`
  - 주 압력 지점: `WorldActors`, `Focus`, `Inventory`

## 우선순위 요약

| 우선순위 | 대상 | 성격 | 권장 시점 |
| --- | --- | --- | --- |
| P1 | `ABeehive` 책임 분리 | 대형 Actor 책임 축소 | WorldActors 기능 추가 전 |
| P1 | Inventory 이동/QuickMove 정책 정리 | 중복 로직 제거, UI 책임 축소 | Inventory/UI 동작 변경 전 |
| P2 | Cursor scope 내부 helper 추출 | Focus scope 가독성 개선 | Cursor 기반 상호작용 추가 시 |
| P3 | `AnchoredFocusCursorActionComponent` 조회 정리 | 반복 조회/보일러플레이트 축소 | 작은 정리 작업으로 처리 가능 |
| P3 | Legacy Blueprint wrapper 제거 검토 | 호환성 정리 | Blueprint 참조 감사 이후 |

## P1. `ABeehive` 책임 분리

### 대상 파일

- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`

### 현재 상태

`ABeehive`는 현재 가장 큰 단일 gameplay actor이며, 다음 책임을 함께 가진다.

- 벌통 actor lifecycle
- bee swarm actor/component 설정
- colony population 계산
- honey 생산/저장 계산
- queen 위치 및 bucket 반응
- comb slot layout 계산
- comb actor spawn/refresh
- part focus descriptor 제공
- item-use area descriptor 제공
- comb lift delegate 처리
- time bucket listener 등록/해제

이 구조는 기능이 늘어날수록 `ABeehive`가 WorldActors 전체의 변경 집중 지점이 될 가능성이 높다.

### 권장 방향

초기 단계에서는 `ABeehive`의 Blueprint-facing 역할을 유지하고 내부 구현만 줄인다.

권장 분리 후보:

- swarm 설정 적용 helper
  - dual swarm, attraction target, control mode 관련 계산을 분리한다.
- colony/honey runtime helper
  - bee population, honey production, bucket transition 반응 계산을 분리한다.
- comb slot layout helper
  - slot transform, spawn count, slot descriptor 계산을 분리한다.
- comb event routing 정리
  - comb lift 관련 delegate 바인딩/해제를 작은 내부 함수군으로 묶는다.

### 주의 사항

- `UCLASS`, `UPROPERTY`, component 이름 변경은 Blueprint 직렬화에 영향을 줄 수 있다.
- 파일명/class rename은 별도 Core Redirect 계획 없이 진행하지 않는다.
- 첫 단계는 private helper 또는 non-UObject helper 중심이 안전하다.
- 신규 `UActorComponent`를 도입할 경우 Blueprint 노출 여부와 serialization 영향을 별도로 검토한다.

## P1. Inventory 이동/QuickMove 정책 정리

### 대상 파일

- `Source/BeekeepingSim/Private/Inventory/StorageBoxComponent.cpp`
- `Source/BeekeepingSim/Private/Inventory/BeekeeperHotbarComponent.cpp`
- `Source/BeekeepingSim/Private/UI/ItemSlotWidget.cpp`
- `Source/BeekeepingSim/Private/Inventory/ItemStackMoveUtils.cpp`

### 현재 상태

Inventory 이동 로직은 일부 공통 계산이 `ItemStackMoveUtils`로 분리되어 있지만, container 간 partial move와 quick move target 선택에는 여전히 반복 패턴이 남아 있다.

반복되는 정책:

- source/target slot 검증
- 같은 item definition stack merge
- 빈 slot spill
- source stack 차감
- target 변경 broadcast
- hotbar selected slot/focus 재평가
- storage widget refresh 유도

특히 `UItemSlotWidget::TryQuickMove`는 UI 레이어에서 다음 정책을 직접 판단한다.

- Hotbar -> Storage target 선택
- Storage -> Hotbar target 선택
- 같은 item definition 우선 병합
- 빈 slot fallback

이 로직은 UI 표시 책임보다 Inventory domain 책임에 가깝다.

### 권장 방향

1. QuickMove target 선택을 Inventory 계층으로 이동한다.
2. Hotbar/Storage 간 이동 알고리즘을 slot accessor 기반 helper 또는 작은 transfer service로 통합한다.
3. 기존 public Blueprint API는 유지하고 내부 구현만 helper로 위임한다.
4. `UItemSlotWidget`은 사용자 입력을 domain API 호출로 전달하고 결과에 따라 UI만 갱신한다.

### 주의 사항

- Hotbar는 selected slot 및 held item/focus 재평가 부작용이 있다.
- Storage는 storage-specific broadcast와 widget refresh 흐름이 있다.
- helper 추출 시 container별 post-move hook을 명확히 분리해야 한다.
- 동작 변경 없이 중복 제거를 목표로 해야 한다.

## P2. Cursor Scope 내부 helper 추출

### 대상 파일

- `Source/BeekeepingSim/Private/Focus/CursorItemUseAreaScopeComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/CursorPartFocusScopeComponent.cpp`
- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`

### 현재 상태

Cursor 기반 Focus scope는 기능 자체의 응집도는 유지하고 있으나, 각 component 내부에서 여러 하위 책임이 함께 처리된다.

`UCursorItemUseAreaScopeComponent` 주요 책임:

- scope activate/deactivate/tick
- item-use area descriptor 수집
- provider actor/child actor/component tag 기반 descriptor rebuild
- cursor hover trace
- active item/action filtering
- visual material state 적용
- collision state cache/apply/restore
- part focus outline suppression
- item action context 구성

`UCursorPartFocusScopeComponent` 주요 책임:

- scope activate/deactivate/tick
- cursor trace
- part descriptor registration
- hover state 관리
- outline 적용
- prompt broadcast
- cancel cascade/action stack
- host focus cancel

### 권장 방향

당장 공통 base class를 만들기보다는, 각 component 내부의 반복 책임을 private helper로 먼저 줄인다.

분리 후보:

- cursor trace resolution helper
- descriptor registration/rebuild helper
- visual/outline state helper
- item-use area collision state helper
- cancel/edge-exit 처리 helper

### 주의 사항

- Part focus와 item-use area는 비슷해 보여도 입력 의미와 cancel 흐름이 다르다.
- 성급한 공통 base class는 오히려 조건 분기를 늘릴 수 있다.
- 먼저 private helper로 반복을 줄이고, 실제 공통 규칙이 안정되면 그때 shared abstraction을 검토한다.

## P3. `AnchoredFocusCursorActionComponent` 조회 정리

### 대상 파일

- `Source/BeekeepingSim/Private/Focus/AnchoredFocusCursorActionComponent.cpp`

### 현재 상태

owner actor에서 cursor scope component를 찾기 위해 `FindComponentByClass` 호출이 여러 lifecycle/input 경로에 반복된다.

### 권장 방향

- `GetItemUseAreaScope()` / `GetPartFocusScope()` 형태의 private resolver를 둔다.
- activation 시점에 weak pointer cache를 구성하고, component invalidation 가능성을 방어한다.
- 기능 변경 없이 반복 조회와 null-check 패턴만 줄인다.

### 주의 사항

- component 동적 추가/제거 가능성이 있다면 cache invalidation을 고려한다.
- 성능 개선보다는 가독성 개선 성격이 강하다.

## P3. Legacy Blueprint wrapper 제거 검토

### 대상 파일

- `Source/BeekeepingSim/Public/WorldActors/BeeSplineSwarmActor.h`
- `Source/BeekeepingSim/Private/WorldActors/BeeSplineSwarmActor.cpp`
- `Source/BeekeepingSim/Public/UI/ItemSlotWidget.h`
- `Source/BeekeepingSim/Private/UI/ItemSlotWidget.cpp`

### 현재 상태

일부 API는 Blueprint compatibility를 위해 legacy wrapper로 유지되고 있다.

예시:

- `BeeSplineSwarmActor` override settings 계열 no-op wrapper
- `ItemSlotWidget` legacy drag/slot context wrapper

### 권장 방향

Blueprint 참조 감사 후 다음 중 하나로 결정한다.

- Blueprint 참조가 있으면 유지하거나 Blueprint migration 후 제거한다.
- 참조가 없으면 deprecation 기간을 거쳐 제거한다.
- public Blueprint API 제거가 필요한 경우 별도 구현 프롬프트와 검증 절차를 둔다.

### 주의 사항

- Blueprint asset은 C++ grep만으로 참조 여부를 확정할 수 없다.
- 제거 전 Editor load/compile/save 검증이 필요하다.
- UFUNCTION rename/remove는 Content asset 파손 가능성이 있다.

## 당장 리팩토링 우선순위가 낮은 영역

다음 영역은 현재 LOC 대비 응집도가 비교적 양호하므로, 별도 기능 추가나 버그 수정이 없다면 우선순위를 낮게 둔다.

- `Camera`
- `Interaction`
- `Environment`

`EnvironmentTimeOfDayActor`와 `GameTimeBucketSubsystem`은 라인 수가 어느 정도 있으나, 현재는 시간 progression과 bucket dispatch라는 명확한 책임 안에 있다.

## 구현 착수 전 체크리스트

실제 리팩토링 구현 전에 다음을 확인한다.

- `.md/0_ARCHITECTURE.md`와 관련 `.md/Architecture/*System.md`를 먼저 읽는다.
- Blueprint-facing API 변경 여부를 구분한다.
- `UCLASS`, `USTRUCT`, `UENUM`, `UPROPERTY`, `UFUNCTION` 이름 변경 여부를 확인한다.
- Content asset 변경이 필요한 작업인지 확인한다.
- 현재 빌드가 통과하는지 확인한다.
- 리팩토링 단위를 작게 나누고 각 단위마다 빌드 검증한다.

## 권장 진행 순서

1. Inventory 이동/QuickMove 정책을 내부 helper로 정리한다.
2. `ABeehive`의 계산성 private helper를 먼저 분리한다.
3. Cursor scope component 내부 helper를 추출한다.
4. `AnchoredFocusCursorActionComponent` 반복 조회를 정리한다.
5. Blueprint 참조 감사를 거쳐 legacy wrapper 제거 여부를 결정한다.

이 순서는 위험도와 검증 난이도를 고려한 기본 순서다. WorldActors 신규 기능을 먼저 추가해야 한다면 `ABeehive` 분리를 선행할 수 있다.
