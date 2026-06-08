# Architecture QnA

## 미해결 질문

- 없음

## 해결 완료

### [소비장 밀랍 plane 표시]

1. 소비장 full honey 시 밀랍 plane authoring 경로
- 질문 내용: 소비장의 꿀이 full 상태가 되었을 때 앞/뒤 밀랍 plane을 어떤 경로로 소유하고 표시할 것인가?
- 필요한 이유: 현재 C++에는 `ABeehiveCombActor`의 `FrontHoneyPlane`/`BackHoneyPlane`만 있고, 별도 wax/capping plane native component는 없다. 기능 구현은 `BP_BeehiveComb` 같은 Content/Editor 수동 설정이 필요할 수 있으므로 native component 추가와 BP-authored component 바인딩 중 하나를 확정해야 한다.
- 선택지
  - 옵션 A: `ABeehiveCombActor`에 `FrontWaxCappingPlane`/`BackWaxCappingPlane` native component를 추가하고, `IsHoneyFull()` 파생 상태로 hidden-in-game을 제어한다.
  - 옵션 B: 새 component 없이 기존 `FrontHoneyPlane`/`BackHoneyPlane` material에 wax/capping 표시 scalar parameter를 추가한다.
  - 옵션 C: 밀랍 plane은 Blueprint에서 자유롭게 만들고, C++은 component tag로 front/back wax component를 찾아 visibility만 제어한다.
- 권장 옵션: 옵션 A. 현재 honey plane과 bee plane이 소비장 actor의 native component로 관리되고 있으므로, full honey의 capping 표시도 `ABeehiveCombActor` 내부 파생 visual state로 두는 편이 상태 오너와 표시 경계가 가장 명확하다. 단, 기존 BP에 이미 밀랍 plane component가 있으면 옵션 C가 더 낮은 migration 비용일 수 있다.
- 답변 : 옵션A

### [사용영역 active 중 아이템 내구도 Tick 감소]

1. 내구도 감소 기능의 authoring 위치
- 질문 내용: 특정 아이템이 item-use-area에서 active use 중일 때 내구도를 감소시키는 설정은 어디에 둘 것인가?
- 필요한 이유: `UHoldItemUseAction`은 사용 세션과 효과 Tick을 소유하고, `UItemDefinition`은 durability/max durability를 소유한다. 설정 위치를 정해야 Blueprint/DataAsset authoring 경계와 재사용 범위가 명확해진다.
- 선택지
  - 옵션 A: `UItemDefinition` base class에 active-use durability drain spec을 추가한다.
  - 옵션 B: `UHoldItemUseAction` subclass별 property로 둔다. `USmokerUseAction`, `UDisinfectantUseAction` 같은 action마다 별도 authoring한다.
  - 옵션 C: 특정 concrete item/action에 하드코딩한다.
  - 옵션 D: `UItemDefinition`은 유지하고, active-use 내구도 감소 전용 하위 item definition class를 추가한다.
- 권장 옵션: 옵션 D. 기존 `UItemDefinition`을 범용 설정 창고로 키우지 않으면서, `UPollenPattyItemDefinition`처럼 특정 아이템군 데이터만 subclass에 둘 수 있다.
- 답변: 옵션 D. 예: `UActiveUseDurabilityItemDefinition : public UItemDefinition`에 `DurabilityDrainPerSecond`, `DrainPolicy`, `bRemoveItemWhenDepleted`를 둔다.

2. 내구도 감소가 발생하는 정확한 active 조건
- 질문 내용: 어떤 상태를 “사용영역에 active 중”으로 볼 것인가?
- 필요한 이유: 현재 `UCursorItemUseAreaScopeComponent`는 scope 활성, 선택 아이템 매칭, LMB use session, hovered active descriptor, `ApplyUseEffect` 성공 여부가 각각 분리되어 있다. 아이템에 따라 “target에 실제 사용됐을 때만 닳는 도구”와 “LMB active 동안 계속 닳는 도구”가 모두 필요할 수 있다.
- 선택지
  - 옵션 A: `WhenUseEffectSucceeded` - LMB use session 중이고, matching active use-area 위에서 `ApplyUseEffect`가 `bSucceeded=true`를 반환한 Tick에만 감소한다.
  - 옵션 B: `WhileOverValidUseArea` - LMB use session 중이고, matching active use-area 위에서 `CanApplyUseEffect`가 true이면 `ApplyUseEffect` 성공 여부와 무관하게 감소한다.
  - 옵션 C: `WhileUseSessionActive` - `BeginUse`가 성공해 LMB active use session이 유지되는 동안 use-area hover/target 여부와 무관하게 감소한다.
  - 옵션 D: 전용 item definition에 `DrainPolicy` enum을 두고 아이템별로 옵션 A/B/C 중 하나를 선택한다.
- 권장 옵션: 옵션 D. 내구도 감소 조건은 item definition authoring 정책으로 분리하고, 기본값은 기존 target-bound 동작에 가까운 `WhenUseEffectSucceeded`로 둔다.
- 답변: 옵션 D. `DrainPolicy` 값은 `WhenUseEffectSucceeded`, `WhileOverValidUseArea`, `WhileUseSessionActive`를 제공한다. `WhenUseEffectSucceeded`의 `bSucceeded=true` 의미는 실제 수치 변화가 발생했는지가 아니라, `ApplyUseEffect`가 유효한 action target을 찾아 성공 결과를 반환했는지로 정의한다.

3. 내구도 0 도달 시 처리
- 질문 내용: active-use 내구도 감소로 selected item durability가 0 이하가 되면 어떻게 처리할 것인가?
- 필요한 이유: 현재 hotbar는 stack 소모 시 selected slot을 비울 수 있지만, durability 소진 전용 mutation API는 없다. 소진 시 세션 종료, held presentation 종료, slot 제거 여부를 확정해야 한다.
- 선택지
  - 옵션 A: durability가 0이 되면 selected slot item을 제거하고 active use session을 cancel/end 처리한다.
  - 옵션 B: durability는 0으로 남기고 item은 유지하되 `CanBeginUse`/`CanApplyUseEffect`에서 사용 불가로 판정한다.
  - 옵션 C: stack count를 1 감소시키고 durability를 max로 재설정한다.
  - 옵션 D: 전용 item definition의 `bRemoveItemWhenDepleted` 값에 따른다. true면 옵션 A, false면 옵션 B 정책을 적용한다.
- 권장 옵션: 옵션 D. 1번 항목에서 확정한 `bRemoveItemWhenDepleted`를 소진 처리의 단일 기준으로 삼아 item definition authoring과 runtime 동작을 일치시킨다.
- 답변: 옵션 D. `bRemoveItemWhenDepleted=true`이면 selected slot item을 제거하고 active use session을 종료한다. `false`이면 durability 0 상태로 item을 유지하되 이후 use begin/effect 적용은 차단한다.

4. stack 가능한 durability 아이템 허용 여부
- 질문 내용: active-use 내구도 감소 대상 아이템의 `MaxStack > 1`을 허용할 것인가?
- 필요한 이유: 현재 durability stack 병합은 같은 durability 값끼리만 허용한다. Tick마다 durability를 감소시키는 selected stack은 stack 전체가 같은 durability를 공유하게 되어 “한 개만 사용 중”인지 “스택 전체가 닳는지”가 불명확하다.
- 선택지
  - 옵션 A: active-use durability drain 대상은 `MaxStack == 1`만 허용한다.
  - 옵션 B: `MaxStack > 1`도 허용하고 selected stack 전체 durability를 함께 감소시킨다.
  - 옵션 C: 사용 시작 시 stack에서 1개를 분리해 별도 instance로 만든 뒤 그 instance만 감소시킨다.
- 권장 옵션: 옵션 A. 현재 inventory 모델 변경 없이 명확한 invariant를 유지할 수 있다.
- 답변: 옵션 A. `UActiveUseDurabilityItemDefinition` 사용 아이템은 `bUsesDurability=true`, `MaxDurability>0`, `MaxStack==1`을 유효 설정으로 본다.

5. durability mutation authority와 API
- 질문 내용: active-use 중 내구도 감소를 어떤 계층이 실제로 적용할 것인가?
- 필요한 이유: 기존 원칙은 Inventory/Hotbar가 슬롯 상태 변경 authority이고, Focus scope는 입력/영역 라우터다. action이 `UItemInstance::SetDurability`를 직접 호출하면 delegate broadcast, slot 제거, selection 정리 정책이 흩어진다.
- 선택지
  - 옵션 A: `UBeekeeperHotbarComponent`에 `ApplySelectedItemDurabilityDelta(float)` 같은 public mutation API를 추가하고 scope가 이를 호출한다.
  - 옵션 B: `UHoldItemUseAction`이 owning item instance의 durability를 직접 변경한다.
  - 옵션 C: `FItemActionExecutionResult`에 durability delta 필드를 추가하고 scope가 기존 result 해석 흐름에서 hotbar API로 적용한다.
- 권장 옵션: 옵션 C + 옵션 A. action은 “이번 Tick 감소량”을 결과로 표현하고, 실제 mutation은 Hotbar authority API가 처리한다.
- 답변: 옵션 C + 옵션 A. `FItemActionExecutionResult`는 durability delta를 전달하고, `UCursorItemUseAreaScopeComponent`는 이를 `UBeekeeperHotbarComponent::ApplySelectedItemDurabilityDelta`에 위임한다.

6. 기존 훈연기/소독제/벌솔 정책 변경 여부
- 질문 내용: 현재 존재하는 hold-use item 중 어떤 아이템부터 active-use durability drain을 적용할 것인가?
- 필요한 이유: 기존 문서에는 `USmokerUseAction`이 stack/durability/fuel을 소비하지 않는다고 기록되어 있었다. 특정 기존 아이템에 적용하면 문서와 gameplay 계약이 바뀐다.
- 선택지
  - 옵션 A: 새 opt-in subclass만 추가하고 기존 아이템 DataAsset은 기본값 off로 유지한다.
  - 옵션 B: 훈연기부터 opt-in해 aggression 감소와 함께 durability를 감소시킨다.
  - 옵션 C: 모든 hold-use item에 기본 적용한다.
  - 옵션 D: 훈연기와 소독약만 active-use durability drain을 적용하고, 벌솔은 기존 소모 없음 정책을 유지한다.
- 권장 옵션: 옵션 D. 현재 요구는 특정 item-use 도구에 대한 내구도 소모이며, 훈연기/소독약은 continuous target effect와 함께 자원 소모를 붙이기 쉽다. 벌솔은 벌/여왕벌 상호작용 정책이 별도이므로 이번 적용 대상에서 제외한다.
- 답변: 옵션 D. 훈연기와 소독약만 active-use durability drain을 적용한다.

7. 감소량 단위와 Tick 누적 방식
- 질문 내용: 내구도 감소량은 어떤 단위로 authoring하고, 프레임별 소수점 누적은 어떻게 처리할 것인가?
- 필요한 이유: Tick마다 바로 float durability를 감소시키면 프레임레이트에는 독립적이지만, UI 표시와 0 도달 타이밍이 소수점 단위로 움직인다. 정수형 resource처럼 보이게 할지 float durability로 유지할지 정해야 한다.
- 선택지
  - 옵션 A: `DurabilityDrainPerSecond` float 값을 `DeltaTime`과 곱해 매 Tick float durability를 감소시킨다.
  - 옵션 B: action 내부에 pending accumulator를 두고 정수 단위 이상 누적될 때만 감소시킨다.
  - 옵션 C: 고정 interval timer 방식으로 n초마다 고정량을 감소시킨다.
- 권장 옵션: 옵션 A. 기존 durability가 float이고, `ApplyUseEffect`도 per-second rate를 `DeltaTime`으로 적용하는 패턴을 사용한다.
- 답변: 옵션 A. 별도 accumulator를 두지 않고 `DurabilityDrainPerSecond * DeltaTime`을 그대로 durability delta로 적용한다.
