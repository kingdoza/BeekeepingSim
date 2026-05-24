### Beehive Item Use Effects QnA

확정 요구사항:

- 소독약 사용중 상태는 LMB hold 동안 소독약 분사 Niagara가 재생되는 상태다.
- 소독약 실질 효과는 LMB hold 중 커서가 유효 사용영역 위에 있을 때 매 Tick 대상 벌통의 위생성을 증가시키는 것이다.
- 화분떡은 별도 사용중 연출이 없다.
- 화분떡 실질 효과는 유효 사용영역 위에서 LMB click 시 해당 사용영역 위치에 화분떡을 부착하는 것이다.
- 해당 사용영역 위치에 화분떡이 이미 있으면 기존 활성 조건과 무관하게 해당 사용영역은 비활성화된다.

1. Item-use action 실행 방식 구분
- 질문 내용: 소독약은 LMB hold 지속 효과이고, 화분떡은 LMB click 1회 효과다. 현재 hold-use action 구조에서 두 방식을 어떻게 구분할지 결정이 필요하다.
- 필요한 이유: 두 아이템은 같은 item-use-area 매칭 구조를 사용하지만 입력/효과 적용 주기가 다르다. action별 실행 모드를 명시하지 않으면 화분떡도 Tick 기반 hold 효과처럼 처리될 수 있다.
- 선택지
  - 옵션 A: item-use action에 실행 모드 enum을 둔다. 예: `ContinuousWhileHeld`, `InstantOnPress`.
  - 옵션 B: 화분떡을 별도 action subclass로 만들고 scope가 subclass 타입별로 분기한다.
  - 옵션 C: 모든 item-use action은 hold 기반으로 유지하고 화분떡 action 내부에서 첫 Tick에만 적용한다.
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
  - 옵션 B: LMB click 시도 즉시 stack count를 1 감소시키며, 실패해도 소모한다.
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
