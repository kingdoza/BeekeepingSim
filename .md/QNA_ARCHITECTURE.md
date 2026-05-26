### Beehive Item Use Effects QnA

확정 요구사항:

- 소독약 사용중 상태는 LMB hold 동안 소독약 분사 Niagara가 재생되는 상태다.
- 소독약 실질 효과는 LMB hold 중 커서가 유효 사용영역 위에 있을 때 매 Tick 대상 벌통의 위생성을 증가시키는 것이다.
- 화분떡 사용중(active) 상태도 LMB hold 기반이며 LMB down 즉시 시작된다.
- 화분떡 실질 효과도 소독약과 같은 `ContinuousWhileHeld` 방식이다. active 상태 중 마우스가 유효 사용영역 위에 있으면 매 Tick 해당 사용영역에 화분떡 부착 효과를 시도한다.
- 화분떡 slot에 이미 화분떡이 있으면 해당 사용영역은 효과 대상에서 막힌다.
- 화분떡은 소독약 같은 지속 분사 Niagara는 없지만, active 시작/부착 성공/취소 상태는 action/Blueprint event로 표시할 수 있어야 한다.
- 해당 사용영역 위치에 화분떡이 이미 있으면 기존 활성 조건과 무관하게 해당 사용영역은 비활성화된다.

1. Item-use action 실행 방식 구분
- 질문 내용: 소독약과 화분떡 모두 LMB hold 중 active 상태를 유지하고, active + valid area 동안 매 Tick 효과를 적용한다. 이 공통 hold-use action 구조를 어떻게 유지할지 결정이 필요하다.
- 필요한 이유: 두 아이템은 같은 item-use-area 매칭 구조와 LMB hold session을 사용한다. 차이는 소독약은 위생성 증가를 반복 적용하고, 화분떡은 빈 slot에 부착을 시도하되 occupied slot에서는 효과가 막힌다는 점이다.
- 선택지
  - 옵션 A: item-use action은 공통 `ContinuousWhileHeld` 모델을 유지한다. item action별 `ApplyUseEffect(Context, DeltaTime)` 구현이 소독약/화분떡 효과 차이를 처리한다.
  - 옵션 B: item-use action에 실행 모드 enum을 둔다. 예: `ContinuousWhileHeld`, `ApplyOnceOnValidAreaWhileHeld`.
  - 옵션 C: 화분떡을 별도 action subclass로 만들고 scope가 subclass 타입별로 분기한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

2. 소독약 위생성 상태의 소유 위치
- 질문 내용: 소독약 효과로 증가하는 위생성 값을 어디에서 소유할지 결정이 필요하다.
- 필요한 이유: 위생성은 벌통 gameplay 상태이며, 이후 질병/해충/생산성 같은 시스템과 연결될 수 있다. item action이 상태를 직접 소유하면 벌통별 상태 관리가 어렵다.
- 선택지
  - 옵션 A: `ABeehive`가 위생성 상태와 증가 API를 소유한다. 예: `SanitationValue`, `MaxSanitationValue`, `IncreaseSanitation(float Delta)`.
  - 옵션 B: 위생성 전용 component를 벌통에 추가하고 해당 component가 상태를 소유한다.
  - 옵션 C: 소독약 item action이 위생성 상태를 임시로 관리한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

3. 소독약 증가량 단위
- 질문 내용: 소독약이 유효 사용영역 위에서 LMB hold 중일 때 위생성을 어떤 단위로 증가시킬지 결정이 필요하다.
- 필요한 이유: 효과 적용은 매 Tick 호출되므로 frame rate와 무관한 초당 증가량 기준이 필요하다.
- 선택지
  - 옵션 A: item action 설정값 `SanitationIncreasePerSecond`를 두고 `Delta = SanitationIncreasePerSecond * DeltaTime`으로 적용한다.
  - 옵션 B: 벌통 설정값 `DisinfectSanitationIncreasePerSecond`를 사용한다.
  - 옵션 C: Tick마다 고정량을 증가시킨다.
- 권장 옵션: 옵션 A
답변: 옵션 A

4. 소독약 분사 Niagara의 소유 위치
- 질문 내용: LMB hold 중 재생되는 소독약 분사 Niagara를 item presentation actor가 소유할지, item action이 spawn/attach할지 결정이 필요하다.
- 필요한 이유: 분사 VFX는 선택 아이템의 사용중 연출이며, 사용영역 위가 아니어도 LMB hold 동안 유지되어야 한다. 소유 위치를 정해야 lifetime과 위치 갱신이 명확해진다.
- 선택지
  - 옵션 A: 소독약 held/on-cursor presentation actor가 Niagara component를 소유하고, item action은 Begin/End use 이벤트로 재생/정지만 요청한다.
  - 옵션 B: item action이 Niagara component 또는 actor를 직접 spawn/attach하고 EndUse에서 제거한다.
  - 옵션 C: 캐릭터가 공용 item-use Niagara component를 소유하고 action별 asset을 바꾼다.
- 권장 옵션: 옵션 A
답변: 옵션 A

5. 화분떡 부착 가능 slot 수
- 질문 내용: 벌통에 화분떡을 몇 개까지 부착할 수 있는지 결정이 필요하다.
- 필요한 이유: slot 수에 따라 descriptor 생성 방식, occupied 상태 저장 방식, UI/사용영역 비활성화 기준이 달라진다.
- 선택지
  - 옵션 A: 벌통당 화분떡 slot 여러 개를 허용한다. 각 slot은 고유 `AreaId`를 가진다.
  - 옵션 B: 벌통당 화분떡 1개만 허용한다.
  - 옵션 C: 소비장 또는 내부 파츠마다 1개씩 허용한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

6. 화분떡 부착 actor/표현 방식
- 질문 내용: 화분떡을 부착할 때 월드에 어떤 형태로 표시할지 결정이 필요하다.
- 필요한 이유: 부착된 화분떡은 이후 제거/소모/상태 표시가 필요할 수 있다. 단순 mesh component인지 actor인지에 따라 lifetime과 확장성이 달라진다.
- 선택지
  - 옵션 A: 전용 `APollenPattyActor` 또는 generic item presentation actor subclass를 spawn해 attach한다.
  - 옵션 B: `ABeehive`가 slot별 static mesh component를 미리 가지고 visibility만 켠다.
  - 옵션 C: Niagara/Decal 등 시각 효과만 표시하고 별도 actor 상태는 두지 않는다.
- 권장 옵션: 옵션 A
답변: 옵션 A

7. 화분떡 occupied 상태 소유 위치
- 질문 내용: 특정 화분떡 사용영역에 이미 화분떡이 있는지 어디서 관리할지 결정이 필요하다.
- 필요한 이유: occupied slot은 기존 item tag/query 활성 조건과 무관하게 사용영역을 비활성화해야 한다. 상태 소유자가 명확해야 descriptor 반환 시점에 일관되게 제외할 수 있다.
- 선택지
  - 옵션 A: `ABeehive`가 `AreaId` 또는 slot id 기준으로 화분떡 actor/occupied 상태를 관리한다.
  - 옵션 B: 각 사용영역 provider/child actor가 자신의 occupied 상태를 관리한다.
  - 옵션 C: item-use-area scope가 occupied 상태를 transient로 관리한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

8. 화분떡 사용영역 비활성화 방식
- 질문 내용: 화분떡이 이미 있는 사용영역을 어떤 방식으로 비활성화할지 결정이 필요하다.
- 필요한 이유: "기존 활성조건 불문 비활성화" 요구사항을 구현하려면 tag query 매칭 이전 단계에서 제외하거나, descriptor에 별도 block 상태를 둬야 한다.
- 선택지
  - 옵션 A: occupied slot은 provider가 descriptor를 반환하지 않는다.
  - 옵션 B: descriptor에 `bBlocked`/`bOccupied` 같은 필드를 추가하고 scope가 active filter에서 제외한다.
  - 옵션 C: descriptor는 반환하되 visual만 숨기고 effect만 막는다.
- 권장 옵션: 옵션 A
답변: 옵션 A

9. 화분떡 아이템 소모 정책
- 질문 내용: 화분떡 부착 성공 시 아이템 stack을 어떻게 처리할지 결정이 필요하다.
- 필요한 이유: 화분떡은 설치형 소모 아이템에 가깝다. 실패/이미 occupied인 경우와 성공 시 소모 조건을 분리해야 한다.
- 선택지
  - 옵션 A: 부착 성공 시 stack count를 1 감소시키고, 실패 시 소모하지 않는다.
  - 옵션 B: LMB hold 시작 즉시 stack count를 1 감소시키며, 실패/취소되어도 소모한다.
  - 옵션 C: 화분떡은 소모하지 않고 재사용 가능 아이템으로 둔다.
- 권장 옵션: 옵션 A
답변: 옵션 A

10. 화분떡 효과 target 기준
- 질문 내용: 화분떡 설치 action이 어떤 target/context를 기준으로 slot을 찾을지 결정이 필요하다.
- 필요한 이유: descriptor의 `EffectTargetObject`가 벌통인지 slot actor인지에 따라 action이 slot을 찾는 방식이 달라진다.
- 선택지
  - 옵션 A: `EffectTargetObject`는 `ABeehive`로 두고, action은 `ItemUseAreaId`를 넘겨 벌통 API에 설치를 요청한다.
  - 옵션 B: `EffectTargetObject`를 slot 전용 actor/component로 두고, action이 그 target에 직접 설치를 요청한다.
  - 옵션 C: action이 hit component 이름으로 slot을 역추적한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

11. 화분떡 ContinuousWhileHeld 효과 발동 조건
- 질문 내용: 화분떡 LMB hold active 상태에서 매 Tick 어떤 조건으로 부착 효과를 시도할지 결정이 필요하다.
- 필요한 이유: 화분떡도 소독약과 같은 ContinuousWhileHeld 모델을 사용한다. 다만 효과가 위생성 누적이 아니라 slot 부착 시도이므로, valid area와 occupied block 조건이 명확해야 stack 소모와 occupied 상태 변경이 안정된다.
- 선택지
  - 옵션 A: LMB down 즉시 active 상태를 시작하고, active + valid area 동안 매 Tick `ApplyUseEffect(Context, DeltaTime)`에서 부착을 시도한다. 해당 slot이 occupied이면 효과는 실패/blocked 처리된다.
  - 옵션 B: LMB release 시점에 마우스가 유효 사용영역 위에 있으면 설치 성공으로 확정한다.
  - 옵션 C: active 중 유효 사용영역 hover를 시작하면 별도 짧은 설치 시간을 누적한 뒤 성공으로 확정한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

12. 화분떡 active 중 target/occupied 판정 정책
- 질문 내용: 화분떡 active 중 마우스가 사용영역 위로 들어오거나 다른 사용영역으로 이동했을 때 어떤 target에 부착을 시도하고, occupied slot을 어떻게 막을지 결정이 필요하다.
- 필요한 이유: 화분떡은 LMB down 시점에 target을 고정하지 않고, active 중 현재 커서 아래 valid area를 매 Tick effect target으로 사용한다. 이미 화분떡이 있는 slot에서는 효과가 반복 적용되면 안 된다.
- 선택지
  - 옵션 A: active 중 현재 커서 아래 valid area를 매 Tick target으로 사용한다. occupied slot은 provider가 descriptor를 반환하지 않거나 `TryInstallPollenPatty(...)`가 실패를 반환해 효과를 막는다.
  - 옵션 B: LMB down 시점에 target을 고정하고, 해당 target이 아니면 효과를 적용하지 않는다.
  - 옵션 C: 같은 LMB hold session에서는 첫 유효 target에만 1회 적용하고 이후 모든 target을 무시한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

13. 화분떡 active 표시/연출 소유 위치
- 질문 내용: 화분떡 LMB hold active 상태와 설치 성공/취소 상태를 어디에서 표시하거나 연출할지 결정이 필요하다.
- 필요한 이유: 화분떡은 소독약처럼 지속 Niagara가 필요한 아이템은 아니지만, active 시작과 부착 성공/blocked/취소를 플레이어에게 보여줄 수 있어야 한다. C++ gameplay state와 Blueprint visual event 경계를 정해야 한다.
- 선택지
  - 옵션 A: item action이 active/applied/blocked/canceled 상태를 소유하고 Blueprint event로 active 시작/부착 성공/blocked/취소를 알린다. 실제 표시 연출은 item presentation actor 또는 host Blueprint가 구현한다.
  - 옵션 B: item-use-area scope가 공통 progress UI/머터리얼 표시를 직접 소유한다.
  - 옵션 C: active 표시를 구현하지 않고 설치 성공 순간에만 actor를 부착한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

14. 화분떡 설치 API와 occupied 갱신 시점
- 질문 내용: ContinuousWhileHeld Tick 효과로 화분떡 부착을 시도할 때 actor 부착과 occupied 상태 갱신을 어떤 API 경로로 처리할지 결정이 필요하다.
- 필요한 이유: item action이 WorldActors 내부 component 구조를 직접 알면 결합도가 커진다. 반대로 벌통 API가 설치/occupied를 소유하면 descriptor 비활성화와 상태 저장이 일관된다.
- 선택지
  - 옵션 A: active + valid area Tick에서 item action이 `EffectTargetObject`를 `ABeehive`로 해석하고 `ItemUseAreaId`를 넘겨 `ABeehive::TryInstallPollenPatty(...)` 같은 API를 호출한다. 성공 시 `ABeehive`가 actor 부착과 occupied 상태를 함께 갱신하고, occupied slot에 대한 이후 효과는 blocked 처리된다.
  - 옵션 B: item action이 descriptor의 hit component에 직접 actor를 spawn/attach하고, occupied 상태도 action이 관리한다.
  - 옵션 C: item-use-area scope가 설치 actor spawn과 occupied 갱신을 모두 수행한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

### Generic Item Placement Slot Actor QnA

이 섹션은 선택된 아이템을 배치하는 재사용 가능한 슬롯 액터 설계에 대한 후속 QnA다.

확정 요구사항:

- 선택된 아이템을 월드의 지정 슬롯에 배치하는 재사용 가능한 슬롯 액터가 필요하다.
- 슬롯 액터 자체가 `ItemUseArea`가 된다.
- 선택된 아이템이 슬롯에 배치되면 해당 슬롯 액터의 `ItemUseArea`는 비활성화된다.
- 방금 구현된 화분떡 배치 슬롯도 이 generic slot actor로 처리한다.
- 따라서 기존 `Beehive Item Use Effects QnA`의 화분떡 slot 소유/target 관련 답변 중 `ABeehive`가 pollen slot을 직접 소유한다는 내용은 이 섹션의 후속 설계가 우선한다.
- 기존 `ABeehive` 직접 소유 화분떡 배치 slot 시스템은 삭제한다. 제거 대상에는 `FPollenPattyInstallSlot` 배열, `ABeehive::TryInstallPollenPatty`, `ABeehive::IsPollenPattySlotOccupied`, 벌통 provider 내부의 pollen patty descriptor 생성, `ABeehive`의 `PollenPattyActorClass` 기반 설치 로직이 포함된다.

설계 초안:

- `AItemPlacementSlotActor` 같은 재사용 가능한 actor를 추가한다.
- 이 actor는 `IItemUseAreaProvider`를 구현해 자기 자신을 `FItemUseAreaDescriptor`로 제공한다.
- descriptor의 `EffectTargetObject`는 slot actor 자신으로 둔다.
- slot actor는 `AreaId`, `AreaTags`, hit/visual 겸용 `SlotMeshComponent`, 인스턴스별 `SlotMeshAsset`, `AttachComponent`, `AttachSocketName`, `PlacedActor` 상태를 가진다.
- `SlotMeshComponent` 하나가 cursor trace hit component와 item-use-area material visual component를 동시에 담당한다.
- slot이 비어 있을 때만 descriptor를 반환한다. 이미 배치된 actor가 있으면 descriptor를 반환하지 않아 item-use-area가 비활성화된다.
- 선택 아이템의 placement action은 `EffectTargetObject`를 slot actor 또는 placement slot interface로 해석해 배치를 요청한다.
- 배치 성공 시 action result는 `bConsumedItem=true`, `StackDelta=-1`을 반환하고, stack mutation은 기존 Hotbar authority 경로가 처리한다.
- 화분떡은 벌통이 slot 상태 배열을 직접 들고 있지 않고, 벌통 내부에 배치된 `AItemPlacementSlotActor` child actor로 표현한다. item-use-area scope는 child actor를 자동 순회하지 않으며, host에 붙은 `UChildItemUseAreaProviderComponent`가 `Component Tags`/class 조건을 통과한 child slot provider 결과만 합쳐 반환한다.

1. 재사용 슬롯의 시스템 소속
- 질문 내용: `AItemPlacementSlotActor`를 어느 시스템 책임으로 둘지 결정이 필요하다.
- 필요한 이유: actor는 월드에 배치되고 `ItemUseArea` provider를 구현하지만, 선택 아이템/stack 소비와도 연결된다. 소속을 잘못 잡으면 Focus/Inventory/WorldActors 경계가 흐려진다.
- 선택지
  - 옵션 A: `WorldActors`에 `AItemPlacementSlotActor`를 둔다. Focus와 Inventory에는 provider/action 계약으로만 연결한다.
  - 옵션 B: `Focus`에 item-use-area 전용 actor로 둔다.
  - 옵션 C: `Inventory`에 item placement actor로 둔다.
- 권장 옵션: 옵션 A
답변: 옵션 A

2. slot actor와 item-use-area provider 방식
- 질문 내용: 슬롯 액터가 item-use-area를 어떤 방식으로 제공할지 결정이 필요하다.
- 필요한 이유: `UCursorItemUseAreaScopeComponent`는 FocusEngaged host의 actor provider, host provider component, host 직접 component tag fallback만 수집한다. 슬롯을 host 내부 child actor로 둘 경우 `UChildItemUseAreaProviderComponent`가 `Component Tags`/class 조건으로 slot provider 결과를 합쳐야 한다.
- 선택지
  - 옵션 A: slot actor가 직접 `IItemUseAreaProvider`를 구현하고, 자신이 비어 있을 때 descriptor 1개를 반환한다.
  - 옵션 B: host actor가 slot actor들을 읽어 descriptor를 대신 생성한다.
  - 옵션 C: slot actor는 component만 제공하고, 별도 manager component가 descriptor를 생성한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

3. placement effect target
- 질문 내용: placement action이 배치 요청을 보낼 target을 무엇으로 볼지 결정이 필요하다.
- 필요한 이유: 기존 화분떡 구현은 `EffectTargetObject=ABeehive`와 `AreaId`로 벌통 API에 요청하는 방향이었다. generic slot actor로 바꾸면 action이 host를 알 필요가 없어야 한다.
- 선택지
  - 옵션 A: descriptor의 `EffectTargetObject`를 slot actor 자신으로 두고, action은 placement slot interface/API를 호출한다.
  - 옵션 B: descriptor의 `EffectTargetObject`는 host actor로 유지하고, host가 slot actor를 찾아 배치한다.
  - 옵션 C: action이 hit component owner를 직접 역추적해 slot actor를 찾는다.
- 권장 옵션: 옵션 A
답변: 옵션 A

4. slot actor 배치 API 형태
- 질문 내용: action이 slot actor에 배치를 요청할 때 concrete class cast를 사용할지 interface를 둘지 결정이 필요하다.
- 필요한 이유: 재사용 가능한 슬롯이면 향후 다른 slot actor subclass가 같은 계약을 구현할 수 있어야 한다.
- 선택지
  - 옵션 A: `IItemPlacementSlot` 같은 인터페이스를 추가하고 `TryPlaceItem(...)`, `IsPlacementOccupied()`를 제공한다.
  - 옵션 B: `AItemPlacementSlotActor` concrete class로 cast해서 `TryPlaceItem(...)`을 호출한다.
  - 옵션 C: `IItemUseAreaProvider`에 placement API까지 추가한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

5. placed actor class의 소유 위치
- 질문 내용: 선택 아이템이 배치될 때 spawn할 actor class를 어디에서 결정할지 필요하다.
- 필요한 이유: slot은 재사용 가능해야 하고, 실제 배치되는 시각/상태 actor는 아이템 종류마다 다르다. 화분떡은 `APollenPattyActor`를 쓰지만 다른 아이템은 다른 actor가 필요할 수 있다.
- 선택지
  - 옵션 A: item action class/default가 `PlacedActorClass`를 가진다. slot은 위치와 occupied 상태만 관리한다.
  - 옵션 B: `UItemDefinition`에 `PlacedActorClass` 필드를 추가한다.
  - 옵션 C: slot actor가 `PlacedActorClass`를 가진다.
- 권장 옵션: 옵션 A
답변: 옵션 A

6. placement action 구조
- 질문 내용: 화분떡 전용 action을 유지할지, generic placement action으로 일반화할지 결정이 필요하다.
- 필요한 이유: 앞으로 같은 슬롯 배치 방식의 아이템이 늘어나면 action 중복이 생길 수 있다. 반대로 너무 빨리 일반화하면 현재 화분떡 요구보다 큰 API가 생긴다.
- 선택지
  - 옵션 A: `UItemPlacementUseAction` generic base/action을 만들고, 화분떡은 이 action class 또는 BP subclass에서 `PlacedActorClass`, `UseAreaTagQuery`를 설정한다.
  - 옵션 B: `UPollenPattyUseAction`을 유지하되 target만 slot actor interface로 바꾼다.
  - 옵션 C: slot actor가 선택 아이템을 직접 읽고 action 없이 배치한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

7. 슬롯 활성 조건과 허용 아이템 필터
- 질문 내용: 어떤 선택 아이템일 때 slot actor의 item-use-area를 활성화할지 결정이 필요하다.
- 필요한 이유: 현재 active area 필터는 item action의 `UseAreaTagQuery`와 descriptor `AreaTags` 매칭이다. slot actor가 generic이면 모든 placement item에 보일지, 특정 아이템에만 보일지 정해야 한다.
- 선택지
  - 옵션 A: slot actor는 `AreaTags`를 제공하고, 선택 아이템 action의 `UseAreaTagQuery`가 이를 매칭할 때만 활성화한다. slot actor의 `TryPlaceItem`에서도 동일 계열 tag/조건을 한번 더 검증할 수 있다.
  - 옵션 B: slot actor가 `AcceptedItemTags`를 가지고 선택 아이템 tag를 직접 검사해 descriptor 반환 여부를 결정한다.
  - 옵션 C: item-use-area scope가 selected item tag와 slot accepted tag를 공통으로 검사한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

8. 배치 성공과 stack 소비 순서
- 질문 내용: slot에 actor를 배치하는 시점과 선택 아이템 stack 감소 시점을 어떻게 묶을지 결정이 필요하다.
- 필요한 이유: 배치 actor spawn/attach가 실패했는데 stack이 먼저 감소하면 아이템이 사라진다. 반대로 stack 감소 실패 후 actor가 남으면 월드 상태와 인벤토리가 불일치한다.
- 선택지
  - 옵션 A: action이 slot `TryPlaceItem` 성공 후에만 `bConsumedItem=true`, `StackDelta=-1`을 반환한다. Hotbar stack delta 적용 실패 시 slot rollback은 별도 QnA로 둔다.
  - 옵션 B: stack을 먼저 감소시킨 뒤 slot 배치를 시도하고 실패하면 stack을 복구한다.
  - 옵션 C: slot 배치와 stack 감소를 하나의 Inventory/World transaction service로 묶는다.
- 권장 옵션: 옵션 A
답변: 옵션 A

9. stack 소비 실패 시 rollback
- 질문 내용: slot actor 배치는 성공했지만 Hotbar stack delta 적용이 실패할 경우 어떻게 처리할지 결정이 필요하다.
- 필요한 이유: 현재 item-use-area result routing은 action result 이후 Hotbar authority API가 stack을 감소시킨다. 성공/실패가 분리되어 있어 rollback 정책이 없으면 placed actor만 남을 수 있다.
- 선택지
  - 옵션 A: scope가 stack delta 적용 실패를 감지하면 slot actor의 placement rollback API를 호출한다.
  - 옵션 B: stack delta 적용 실패 가능성을 사전 조건으로 제거한다. 예: Begin/Apply 전에 selected item stack > 0을 강제하고, 실패 시 ensure/log만 남긴다.
  - 옵션 C: stack 소비 실패는 허용하고 actor 배치를 유지한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

10. placed actor lifetime과 제거 정책
- 질문 내용: 한 번 배치된 actor를 언제 제거하거나 다시 빈 slot으로 되돌릴 수 있는지 결정이 필요하다.
- 필요한 이유: 현재 요구는 배치되면 ItemUseArea가 비활성화되는 것뿐이다. 이후 화분떡 소모, 회수, 시간 경과 제거가 생기면 slot occupied state와 actor lifetime API가 필요하다.
- 선택지
  - 옵션 A: 1차 구현은 영구 occupied로 두고, `ClearPlacedItem()` 같은 explicit API만 제공한다.
  - 옵션 B: 시간/효과가 끝나면 slot actor가 자동으로 placed actor를 제거하고 다시 활성화한다.
  - 옵션 C: 배치된 actor가 destroy되면 slot actor가 자동 감지해 빈 상태로 돌아간다.
- 권장 옵션: 옵션 A
답변: 옵션 A

11. 벌통 내부 slot authoring 방식
- 질문 내용: 화분떡용 placement slot actor를 `BP_Beehive` 내부에 어떤 방식으로 배치할지 결정이 필요하다.
- 필요한 이유: `UCursorItemUseAreaScopeComponent`는 FocusEngaged host 기준으로 actor/provider component를 수집한다. slot actor를 host의 child actor로 두면 에디터 authoring은 단순하지만, `UChildItemUseAreaProviderComponent`의 tag/class 필터 조건을 만족해야 descriptor가 노출된다.
- 선택지
  - 옵션 A: `BP_Beehive`에 `ChildActorComponent`로 `AItemPlacementSlotActor`를 배치한다.
  - 옵션 B: 레벨에 독립 actor로 배치하고 벌통에 attach한다.
  - 옵션 C: `ABeehive`가 construction에서 slot actor를 자동 spawn한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

12. attach transform 책임
- 질문 내용: placed actor의 최종 위치/회전/스케일 보정을 slot actor가 데이터로 가질지, placed actor BP 내부 relative transform으로 처리할지 결정이 필요하다.
- 필요한 이유: slot은 재사용 위치이고, 아이템별 visual offset은 actor class마다 다를 수 있다. 둘을 섞으면 authoring 기준이 흐려진다.
- 선택지
  - 옵션 A: slot actor는 `AttachComponent/AttachSocketName`만 제공하고, 아이템별 보정은 placed actor BP의 root/mesh relative transform으로 처리한다.
  - 옵션 B: slot actor가 item별 transform offset map을 가진다.
  - 옵션 C: item action이 transform offset을 가진다.
- 권장 옵션: 옵션 A
답변: 옵션 A

13. slot hit/visual component 통합
- 질문 내용: `AItemPlacementSlotActor`에서 cursor trace 판정용 component와 item-use-area material 표시용 component를 분리할지, 하나의 mesh component로 통합할지 결정이 필요하다.
- 필요한 이유: `ChildActorComponent`로 배치된 slot actor의 내부 component를 parent BP 인스턴스에서 직접 `VisualComponents` 배열에 지정하는 워크플로우가 불안정하다. 또한 slot마다 사용영역 모양이 달라야 하므로 인스턴스별 mesh asset 지정 경로가 필요하다.
- 선택지
  - 옵션 A: slot actor가 `UStaticMeshComponent` 하나를 `SlotMeshComponent`로 소유하고, 이를 descriptor의 `HitComponent`와 `VisualComponents[0]`에 모두 사용한다. slot actor에는 인스턴스별 `SlotMeshAsset` property를 두어 slot 모양을 바꾼다.
  - 옵션 B: `HitBox`와 `VisualMesh`를 분리하고, BP 내부에서 자동 태그 수집으로 visual component를 찾는다.
  - 옵션 C: parent BP가 child actor 내부 component 참조를 직접 설정한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

### Beehive Comb Drag QnA

확정 요구사항:

- 소비장 hover 상태에서 LMB 좌우 drag는 소비장을 180도 뒤집는 동작 후보로 본다.
- 소비장 hover 상태에서 LMB 상하 drag 반복은 소비장을 털어내는 동작 후보로 본다.
- click과 drag는 같은 LMB 입력을 공유하므로, click cancel threshold 초과 후 drag 가능 대상에서만 drag 동작을 시작한다.
- 오브젝트별로 drag 기능이 없을 수 있으며, drag 기능이 없는 대상은 threshold 초과 시 click만 취소한다.

1. 소비장 180도 뒤집기 회전축
- 질문 내용: 좌우 drag로 소비장을 180도 뒤집을 때 어떤 축과 transform 기준을 사용할지 결정이 필요하다.
- 필요한 이유: 현재 소비장 actor는 벌통 slot `UChildActorComponent`에 의해 들기/내리기 이동을 받고, 내부 `CombMesh`와 front/back attach point를 가진다. slot transform을 직접 돌리면 lift component와 충돌할 수 있고, mesh 기준 회전축을 잘못 잡으면 front/back 의미가 뒤집히지 않을 수 있다.
- 선택지
  - 옵션 A: `ABeehiveCombActor` 내부 visual root 또는 `CombMesh` 기준 local yaw 180도를 적용한다.
  - 옵션 B: `ABeehive`가 소유한 comb slot `UChildActorComponent` relative rotation에 180도를 적용한다.
  - 옵션 C: `ABeehiveCombActor`에 별도 `CombPivotRoot`를 추가하고 mesh/Niagara/honey plane/attach point를 그 아래로 옮긴 뒤 pivot root를 180도 회전한다.
- 권장 옵션: 옵션 C
- 답변 : 옵션 C

2. 뒤집힌 상태와 front/back gameplay 의미
- 질문 내용: 소비장이 뒤집힌 뒤 front/back face, queen attach point, honey plane, bee Niagara의 gameplay 의미도 함께 바뀌어야 하는지 결정이 필요하다.
- 필요한 이유: 단순 시각 회전만 하면 플레이어가 보는 앞면과 `FrontFaceBeeNiagara`/`QueenFrontAttachPoint` 같은 내부 의미가 어긋날 수 있다. 여왕벌 위치 갱신, 꿀 시각, 추후 face별 상호작용이 front/back 상태를 어떻게 해석할지 명확해야 한다.
- 선택지
  - 옵션 A: 뒤집기는 시각 상태만 바꾸고 기존 front/back gameplay 데이터 의미는 유지한다.
  - 옵션 B: 뒤집힌 상태에서는 visible face만 바뀌며, helper API가 현재 보이는 face를 `Front/Back`으로 반환한다. 기존 저장 데이터 이름은 유지한다.
  - 옵션 C: flip 시 front/back 데이터 자체를 swap한다.
- 권장 옵션: 옵션 B
- 답변 : 옵션 B

3. 소비장 drag 가능 조건
- 질문 내용: 소비장 drag를 언제 허용할지 결정이 필요하다.
- 필요한 이유: 현재 소비장 PartFocus는 `Beehive.LidOpen` required tag와 `Beehive.CombLift` exclusive group을 가진다. drag가 단순 hover 상태에서도 가능한지, 소비장을 이미 들어 올린 engaged 상태에서만 가능한지에 따라 click/lift/drag 관계가 달라진다.
- 선택지
  - 옵션 A: lid open 상태에서 소비장 hover만 되면 drag를 허용한다.
  - 옵션 B: 소비장 part action이 engaged되어 해당 소비장이 lifted 상태일 때만 drag를 허용한다.
  - 옵션 C: lid open 상태에서는 좌우 flip만 허용하고, 상하 털기는 lifted 상태에서만 허용한다.
- 권장 옵션: 옵션 B
- 답변 : 옵션 B

4. 좌우 flip gesture 판정 기준
- 질문 내용: drag delta를 어떤 기준으로 좌우 flip gesture로 확정할지 결정이 필요하다.
- 필요한 이유: 상하 털기와 좌우 뒤집기가 같은 LMB drag를 사용한다. 초기 movement만으로 mode가 잘못 확정되면 플레이어가 의도하지 않은 동작이 실행될 수 있다.
- 선택지
  - 옵션 A: 누적 X 이동량이 `CombFlipDragThresholdPixels` 이상이고 `Abs(X) > Abs(Y) * HorizontalDominanceRatio`이면 flip mode로 확정한다.
  - 옵션 B: release 시점의 전체 drag vector가 좌우 우세이면 flip을 실행한다.
  - 옵션 C: 좌우 방향 전환 횟수를 기준으로 flip을 실행한다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 A. `Abs(X) > Abs(Y) * HorizontalDominanceRatio`는 가로 이동량이 세로 이동량보다 설정 비율 이상 뚜렷하게 클 때만 좌우 flip gesture로 인정한다는 의미다. `CombFlipDragThresholdPixels`는 충분한 이동량을 판정하고, `HorizontalDominanceRatio`는 대각선/애매한 drag가 flip으로 오인되지 않게 하는 방향 우세 조건이다.

5. 상하 털기 gesture 판정 기준
- 질문 내용: 상하 n회 drag 반복을 어떤 방식으로 stroke count로 계산할지 결정이 필요하다.
- 필요한 이유: 단순 Y 누적량만 보면 위아래 반복과 한 방향 긴 이동을 구분하기 어렵다. 소비장 털기는 반복 동작이므로 방향 전환과 stroke 임계값이 필요하다.
- 선택지
  - 옵션 A: Y 이동이 `CombShakeStrokeThresholdPixels` 이상 누적된 뒤 방향이 반전될 때 stroke count를 증가시키고, `RequiredShakeStrokeCount`에 도달하면 털기를 실행한다.
  - 옵션 B: release 시점까지 누적된 총 Y 이동거리 합이 threshold를 넘으면 털기를 실행한다.
  - 옵션 C: 일정 시간 안에 마우스 Y 속도 peak가 n회 발생하면 털기를 실행한다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 A

6. 한 drag session 안의 mode lock 정책
- 질문 내용: 좌우 flip과 상하 털기 중 하나가 확정된 뒤 같은 drag session에서 다른 mode로 전환할 수 있는지 결정이 필요하다.
- 필요한 이유: mode 전환을 허용하면 flip 후 shake 또는 shake 도중 flip이 한 입력에서 동시에 발생할 수 있다. 반대로 mode lock을 하면 입력 해석은 단순하지만 실수 보정은 줄어든다.
- 선택지
  - 옵션 A: threshold 초과 후 먼저 확정된 mode로 session을 lock하고 release까지 다른 mode로 전환하지 않는다.
  - 옵션 B: drag 중 dominance가 바뀌면 mode를 전환할 수 있다.
  - 옵션 C: flip은 즉시 실행하고 이후 같은 hold에서 shake도 허용한다.
- 권장 옵션: 옵션 A 
- 답변 : 옵션 A. mode가 확정되지 않은 채 release되면 flip/shake fallback을 실행하지 않고 no-op을 허용한다. click cancel threshold를 넘었지만 flip/shake threshold나 dominance 조건을 만족하지 못한 애매한 drag는 의도적으로 아무 동작도 발동하지 않는다.

7. 소비장 털기 효과 범위
- 질문 내용: 소비장 털기 성공 시 어떤 gameplay 상태를 변경할지 결정이 필요하다.
- 필요한 이유: 현재 `ABeehiveCombActor`는 `TargetBeeCount` 감소 API와 honey 상태를 가진다. 털기가 벌을 떨어뜨리는 동작인지, 꿀/위생/아이템 수확과도 연결되는지에 따라 WorldActors/Inventory/ItemAction 경계가 달라진다.
- 선택지
  - 옵션 A: 1차 구현은 `ABeehiveCombActor::ReduceTargetBeeCountByRatio`만 호출해 소비장 표면 벌 수를 줄인다.
  - 옵션 B: 벌 수 감소와 함께 꿀 또는 위생 상태도 변경한다.
  - 옵션 C: 털기 성공 이벤트만 발생시키고 실제 효과는 Blueprint에서만 구현한다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 A

8. flip/shake 애니메이션 소유 위치
- 질문 내용: 소비장 뒤집기와 털기 시각 애니메이션을 C++에서 보간할지 Blueprint 이벤트로 위임할지 결정이 필요하다.
- 필요한 이유: `ABeehiveCombActor`는 현재 Tick을 사용하지 않고, 소비장 lift 이동은 별도 component가 담당한다. 새 애니메이션을 어디서 소유할지 정해야 Tick 비용과 BP authoring 경계가 명확해진다.
- 선택지
  - 옵션 A: `ABeehiveCombActor`에 Tick 기반 flip/shake 보간을 추가한다.
  - 옵션 B: C++은 상태 변경과 Blueprint event만 제공하고, 실제 애니메이션은 BP에서 처리한다.
  - 옵션 C: 전용 `UBeehiveCombDragMotionComponent`를 추가해 flip/shake motion을 소유한다.
- 권장 옵션: 옵션 B
- 답변 : 옵션 B

9. drag threshold/tuning 값 소유 위치
- 질문 내용: 소비장 flip/shake gesture에 필요한 threshold와 효과 수치를 어디에서 조정할지 결정이 필요하다.
- 필요한 이유: Focus 공통 click cancel threshold와 소비장 전용 gesture threshold는 의미가 다르다. 전역 settings에 둘지 action component property로 둘지에 따라 재사용성과 authoring 편의가 달라진다.
- 선택지
  - 옵션 A: `UBeehiveCombPartFocusActionComponent`의 Details property로 둔다.
  - 옵션 B: `UBeekeepingSimFocusSettings`에 Focus 공통 drag tuning으로 둔다.
  - 옵션 C: `ABeehive` actor property로 둔다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 A
