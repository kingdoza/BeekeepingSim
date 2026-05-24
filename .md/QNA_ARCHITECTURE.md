### FocusEngaged Item Use Area QnA

확정 요구사항:

- Item-use area는 벌통 전용 기능이 아니라 FocusEngaged 상태의 host actor가 선택적으로 제공하는 generic 기능이다.
- `ABeehive`는 이 generic 구조를 사용하는 첫 구현 host다.
- FocusEngaged host가 item-use-area scope/provider를 지원하고 대상 아이템이 선택되어 있으면, 해당 아이템에 대응되는 사용영역은 LMB 조작 여부와 무관하게 항상 표시/점멸한다.
- FocusEngaged host가 item-use-area scope/provider를 지원하지 않으면, 선택 아이템이 있더라도 해당 host의 기존 FocusAction/PartFocus 입력 정책을 따른다.
- Anchored cursor FocusEngaged 진입 시 hotbar 선택은 빈손으로 전환한다. item-use-area는 FocusEngaged 진입 후 플레이어가 다시 대상 아이템을 선택했을 때 활성화된다.
- LMB Press/Hold/Release는 "아이템 사용중" 세션과 "실질 아이템사용효과" 적용 여부만 제어한다.
- 사용영역 표시/점멸 조건과 실질 효과 적용 조건은 분리한다.

1. LMB 입력 우선순위
- 질문 내용: FocusEngaged 상태에서 선택 아이템이 있을 때, LMB 입력을 item-use action으로 처리할지 기존 FocusAction/PartFocus 입력으로 처리할지 결정이 필요하다.
- 필요한 이유: 기존 FocusEngaged host는 자체 confirm/cancel/PartFocus 입력 정책을 가질 수 있다. 새 item-use-area 기능은 LMB Press/Hold/Release를 사용하므로, host 지원 여부와 선택 아이템 상태에 따른 우선순위가 명확해야 한다.
- 선택지
  - 옵션 A: 선택 아이템에 사용 가능한 item-use action이 있고 표시 가능한 사용영역이 하나 이상 있으면 LMB는 item-use action으로 소비한다.
  - 옵션 B: 커서가 item-use area 위에 있을 때만 LMB를 item-use action으로 소비하고, 그 외에는 기존 FocusAction/PartFocus 입력으로 처리한다.
  - 옵션 C: item-use action과 기존 FocusAction/PartFocus 입력을 별도 입력으로 분리한다.
  - 옵션 D: engaged host가 item-use-area를 지원하고 선택된 아이템이 있으면 사용영역 유무와 무관하게 LMB는 item-use action으로 처리한다. host가 item-use-area를 지원하지 않거나 선택 아이템이 없으면 기존 FocusAction/PartFocus 입력 정책을 따른다.
답변: 옵션 D

2. 사용영역 커서 trace/collision 정책
- 질문 내용: 마우스 커서가 item-use area mesh 위에 있는지 판정할 때 전용 trace channel을 추가할지, 기존 visibility trace와 component filter를 사용할지 결정이 필요하다.
- 필요한 이유: 사용영역은 투명 메시로 월드에 실제 존재하지만 기본 상태에서는 비활성/비가시 상태다. 표시 중인 사용영역만 안정적으로 hit 판정하려면 collision 정책이 명확해야 한다.
- 선택지
  - 옵션 A: 전용 item-use-area trace channel을 추가하고, 활성 사용영역 mesh만 해당 채널 query에 응답하게 한다.
  - 옵션 B: 기존 visibility trace를 사용하되, active descriptor에 등록된 component인지 추가로 검사한다.
  - 옵션 C: cursor trace는 기존 PartFocus trace 경로를 재사용하고, item-use area descriptor만 별도 필터로 얹는다.
- 권장 옵션: 옵션 B
답변: 옵션 B

3. 사용영역 authoring 위치
- 질문 내용: FocusEngaged host의 사용영역 메시와 descriptor를 C++ 기본 컴포넌트로 고정할지, Blueprint child/component 또는 child actor provider 기반으로 등록할지 결정이 필요하다.
- 필요한 이유: 사용영역은 host actor의 asset 배치/메시 형태와 강하게 연결된다. C++에 고정하면 안정적이지만 디자이너 조정이 둔해지고, Blueprint/provider authoring으로 열면 확장성은 좋아지지만 등록 규칙이 필요하다.
- 선택지
  - 옵션 A: host actor C++에 대표 사용영역 컴포넌트를 기본 구성으로 추가한다.
  - 옵션 B: host Blueprint child에 투명 mesh component를 배치하고, component tag 또는 명시 등록 API로 `FItemUseAreaDescriptor`를 구성한다.
  - 옵션 C: 사용영역 전용 child actor를 두고 host actor가 child actor provider들을 수집해 descriptor를 만든다.
- 권장 옵션: 옵션 B
답변: 옵션 B 확장안. 사용영역 mesh는 host Blueprint child의 직접 하위 component와 host child actor 내부 component를 모두 허용한다. host actor는 direct component tag scan과 child actor provider/interface 수집을 통해 `FItemUseAreaDescriptor`로 정규화한 뒤 item-use-area scope에 등록한다. `ABeehive`는 이 경로의 첫 적용 사례다.

4. 아이템과 사용영역 매칭 방식
- 질문 내용: 선택 아이템이 어떤 사용영역을 활성화할지 item class/type 직접 비교로 정할지, GameplayTag/TagQuery 기반으로 정할지 결정이 필요하다.
- 필요한 이유: 앞으로 아이템과 FocusEngaged host가 추가될 때 기존 host 코드를 덜 수정하려면 아이템과 영역의 연결 방식이 데이터 중심이어야 한다.
- 선택지
  - 옵션 A: item definition 또는 item action의 GameplayTag/TagQuery가 사용 가능한 area tag를 선언한다.
  - 옵션 B: host actor 쪽에서 item class/type별 사용영역 매핑 테이블을 가진다.
  - 옵션 C: 각 사용영역 descriptor가 허용 item tag query를 가지고, scope가 선택 아이템 태그를 검사한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

5. 실질 아이템사용효과의 소유 위치
- 질문 내용: LMB hold 중 커서가 사용영역 위에 있을 때 적용되는 실제 효과를 item action이 소유할지, 사용영역/host actor가 소유할지 결정이 필요하다.
- 필요한 이유: 같은 사용영역을 여러 아이템이 공유할 수 있고, 같은 아이템이 여러 영역에서 다른 효과를 낼 수 있다. 효과의 authority를 정해야 확장 시 중복과 결합을 줄일 수 있다.
- 선택지
  - 옵션 A: item action이 효과 실행의 owner가 되고, 사용영역은 hit target과 target context만 제공한다.
  - 옵션 B: 사용영역 또는 host actor가 효과 실행의 owner가 되고, item은 효과 선택용 데이터만 제공한다.
  - 옵션 C: item action은 공통 lifecycle만 담당하고, 실제 효과는 영역별 Blueprint event로 위임한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

6. 하나의 아이템과 하나의 사용영역의 다대다 허용 여부
- 질문 내용: 하나의 아이템이 여러 사용영역을 활성화할 수 있는지, 하나의 사용영역이 여러 아이템에 의해 공유될 수 있는지 명시 결정이 필요하다.
- 필요한 이유: 요구사항상 두 방향 모두 가능성이 있으며, 초기 설계에서 1:1로 제한하면 이후 도구/처리 아이템/host 확장 시 구조 변경이 커질 수 있다.
- 선택지
  - 옵션 A: 다대다를 허용한다. item action의 area tag query와 area descriptor tag를 매칭한다.
  - 옵션 B: 아이템 1개는 사용영역 1개만 허용하고, 영역 공유만 허용한다.
  - 옵션 C: 사용영역 1개는 아이템 1개만 허용하고, 아이템의 복수 영역만 허용한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

7. 선택 아이템은 있지만 매칭 사용영역이 없을 때의 LMB 처리
- 질문 내용: item-use-area를 지원하는 FocusEngaged host에서 선택 아이템은 있지만 현재 아이템에 매칭되는 사용영역이 없을 때, LMB hold item-use session을 시작할지 결정이 필요하다.
- 필요한 이유: LMB 입력 우선순위는 "host가 item-use-area를 지원하고 선택된 아이템이 있으면 item-use action"으로 결정되었다. 하지만 사용영역이 없을 때도 사용중 연출/사운드 같은 hold session을 허용할지 명확해야 한다.
- 선택지
  - 옵션 A: 선택 아이템이 있으면 매칭 사용영역이 없어도 item-use session은 시작할 수 있다. 단, 실질 아이템사용효과는 발생하지 않는다.
  - 옵션 B: 선택 아이템이 있어도 매칭 사용영역이 없으면 item-use session 자체를 시작하지 않는다.
  - 옵션 C: item action별로 매칭 사용영역이 없어도 사용중 상태를 허용할지 설정한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

8. 여러 사용영역이 커서 아래 겹쳤을 때 우선순위
- 질문 내용: 활성 item-use area mesh가 서로 겹치거나 trace 결과가 여러 후보와 교차할 때 어떤 사용영역을 hover/effect 대상으로 삼을지 결정이 필요하다.
- 필요한 이유: 투명 메시 사용영역은 host 내부 구조나 child actor 배치에 따라 공간상 겹칠 수 있다. 실질 효과 대상은 매 Tick 하나로 정규화되어야 예측 가능하다.
- 선택지
  - 옵션 A: trace hit result에서 가장 가까운 active area component 1개를 우선한다.
  - 옵션 B: descriptor 우선순위 값을 두고 priority가 가장 높은 active area를 우선한다.
  - 옵션 C: 겹친 모든 active area에 실질 효과를 적용한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

9. Item-use area hover 표시와 PartFocus outline 동시 표시 정책
- 질문 내용: item-use area 점멸/hover 표시와 기존 CursorPartFocus outline이 같은 시점에 동시에 보일 수 있는지 결정이 필요하다.
- 필요한 이유: FocusEngaged host 내부에는 기존 PartFocus hover/click과 새 item-use area hover가 함께 존재할 수 있다. 두 표시가 충돌하면 플레이어가 현재 조작 대상을 오해할 수 있다.
- 선택지
  - 옵션 A: 둘 다 표시 가능하되, 선택 아이템이 있을 때는 item-use area hover visual을 주 표시로 본다.
  - 옵션 B: item-use-area를 지원하는 host에서 선택 아이템이 있으면 PartFocus outline을 숨기고 item-use area만 표시한다.
  - 옵션 C: PartFocus outline은 항상 유지하고 item-use area hover는 별도 색상으로만 보조 표시한다.
- 권장 옵션: 옵션 A
답변: 옵션 B

10. 실질 아이템사용효과 적용 주기
- 질문 내용: LMB hold 중 커서가 유효 사용영역 위에 있을 때 실질 아이템사용효과를 어떤 주기로 적용할지 결정이 필요하다.
- 필요한 이유: 벌 수 감소, 내구도 감소, 작업 진행도 증가처럼 지속 효과가 있을 수 있고, 효과별 적용 빈도는 달라질 수 있다.
- 선택지
  - 옵션 A: scope가 매 Tick `ApplyUseEffect(Context, DeltaTime)`를 호출하고, item action이 내부에서 rate limit 또는 누적 시간을 관리한다.
  - 옵션 B: scope가 고정 interval timer로 효과를 호출한다.
  - 옵션 C: LMB Press 순간과 Release 순간에만 효과를 호출한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

11. 사용영역 visual material parameter 계약
- 질문 내용: 사용영역 mesh가 점멸/hover 상태를 표현할 때 C++/Blueprint가 공통으로 기대하는 material parameter 이름을 고정할지 결정이 필요하다.
- 필요한 이유: 사용영역 mesh가 host 직접 하위 component일 수도 있고 child actor 내부 component일 수도 있다. 공통 parameter 계약이 없으면 표시 적용 로직이 asset별 분기나 Blueprint 이벤트에 의존하게 된다.
- 선택지
  - 옵션 A: 공통 material parameter 이름을 고정한다. 예: `UseAreaColor`, `UseAreaOpacity`, `PulseSpeed`, `HoverStrength`.
  - 옵션 B: descriptor마다 parameter 이름을 개별 설정한다.
  - 옵션 C: C++은 visibility/collision만 제어하고 점멸/hover material 처리는 Blueprint event로만 처리한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

12. 사용영역 descriptor rebuild 타이밍
- 질문 내용: FocusEngaged host의 item-use area descriptor를 언제 다시 수집/등록할지 결정이 필요하다.
- 필요한 이유: 사용영역 구조 변경과 선택 아이템 변경은 비용과 의미가 다르다. 선택 아이템이 바뀔 때마다 전체 component/child actor scan을 수행하면 불필요한 비용이 생긴다.
- 선택지
  - 옵션 A: FocusEngaged 진입 시 rebuild하고, host 내부 구조/child actor 변경 같은 구조 변경 시 rebuild한다. 선택 아이템 변경 시에는 기존 descriptor를 유지하고 active area filter만 갱신한다.
  - 옵션 B: 선택 아이템이 바뀔 때마다 descriptor를 rebuild한다.
  - 옵션 C: BeginPlay에서 한 번만 rebuild하고 런타임 구조 변경은 지원하지 않는다.
- 권장 옵션: 옵션 A
답변: 옵션 A

13. 실질 효과 target context 기준
- 질문 내용: item action이 실질 효과를 적용할 대상 객체를 hit component에서 직접 해석할지, descriptor가 명시 target을 제공할지 결정이 필요하다.
- 필요한 이유: host actor 자체, host child actor, 전용 component 등 효과 대상이 host마다 달라질 수 있다. item action이 component 구조를 직접 알면 WorldActors 구조에 과하게 결합된다.
- 선택지
  - 옵션 A: descriptor에 `EffectTargetObject`를 두고 item action은 이 target context를 통해 효과를 적용한다.
  - 옵션 B: item action이 hit component/owner actor를 직접 검사해 target을 찾는다.
  - 옵션 C: 모든 효과 target은 항상 FocusEngaged host actor로 통일하고, host가 내부 대상 분배를 담당한다.
- 권장 옵션: 옵션 A
답변: 옵션 A

14. 로컬 UX와 authority/replication 경계
- 질문 내용: item-use area 표시, LMB hold session, 실질 효과 적용을 현재 단계에서 로컬 UX 기준으로만 설계할지, 서버 authority/replication까지 함께 설계할지 결정이 필요하다.
- 필요한 이유: 현재 정본 문서의 Focus/UI 흐름은 로컬 플레이어 UX 중심이다. 하지만 실질 효과는 게임 상태를 바꿀 수 있으므로 이후 멀티플레이를 고려하면 authority 경계가 필요할 수 있다.
- 선택지
  - 옵션 A: 현재 단계는 로컬 플레이어 UX 기준으로 설계하고, 실제 상태 변경 함수는 이후 authority 경로로 감쌀 수 있게 item action/effect 호출 경계를 분리한다.
  - 옵션 B: 초기 설계부터 서버 RPC/replication 경로를 포함한다.
  - 옵션 C: 완전히 싱글플레이 전용으로 두고 authority 확장성은 고려하지 않는다.
- 권장 옵션: 옵션 A
답변: 옵션 A

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

### DynamicSky TimeOfDay QnA

1. TimeOfDay 소유자 분리 방식
- 질문 내용: `EnvironmentTimeOfDay`가 환경 표현용으로 사용되지 않게 될 때, 게임 시간 `TimeOfDay`를 어떤 객체가 소유할지 결정이 필요하다.
- 필요한 이유: 현재 `AEnvironmentTimeOfDayActor`는 `CurrentHour24`, 시간 진행, `OnTimeOfDayChanged`, 환경 시각 적용을 모두 소유한다. DynamicSky 도입 후에도 벌통 bucket, clock UI, 이후 gameplay time event는 같은 `Hour24` 기준을 계속 사용해야 하므로 시간 source of truth를 명확히 분리해야 한다.
- 선택지
  - 옵션 A: 기존 `AEnvironmentTimeOfDayActor`에서 환경 표현 로직만 제거하고, 시간 소유자/이벤트 broadcaster로 유지한다.
  - 옵션 B: 새 `AGameTimeOfDayActor`를 만들고, 기존 `AEnvironmentTimeOfDayActor`의 시간 소유/진행/이벤트 책임을 이관한다.
  - 옵션 C: `UWorldSubsystem` 기반 시간 관리자로 옮기고, actor 배치 없이 월드 단위로 `TimeOfDay`를 소유한다.
- 권장 옵션: 옵션 B
답변: 옵션 B

2. 기존 `OnTimeOfDayChanged` delegate 호환 정책
- 질문 내용: DynamicSky 도입 시 기존 `OnTimeOfDayChanged(float Hour24, const FTimeOfDayVisualState& VisualState)` 시그니처를 유지할지, 시간 전용 delegate로 새로 만들지 결정이 필요하다.
- 필요한 이유: `FTimeOfDayVisualState`는 기존 Environment 표현 결과를 담는 구조체다. DynamicSky가 표현 책임을 가져가면 시간 source가 visual state를 계산해 넘기는 것은 책임 경계와 맞지 않는다. 하지만 기존 delegate를 바로 변경하면 Blueprint/API 참조, bucket subsystem, clock widget binding에 영향이 있다.
- 선택지
  - 옵션 A: 기존 delegate를 유지하고 `FTimeOfDayVisualState`는 transition 기간 동안만 전달한다.
  - 옵션 B: 새 시간 전용 delegate를 추가한다. 예: `OnGameTimeOfDayChanged(float Hour24)`. 기존 delegate는 deprecated wrapper로 유지한다.
  - 옵션 C: 기존 delegate를 즉시 시간 전용 시그니처로 변경하고 관련 참조를 한 번에 migration한다.
- 권장 옵션: 옵션 B
답변: 옵션 B

3. DynamicSky의 TimeOfDay 연결 방식
- 질문 내용: `ADynamicSky`가 runtime `TimeOfDay`를 어떤 방식으로 받을지 결정이 필요하다.
- 필요한 이유: DynamicSky는 실시간으로 하늘 상태를 갱신해야 하지만 게임 시간을 소유하면 안 된다. concrete actor 직접 참조, interface 구독, subsystem 조회 중 어떤 경로를 공식 계약으로 둘지 정해야 Blueprint authoring과 C++ 의존성이 안정된다.
- 선택지
  - 옵션 A: `ADynamicSky`가 `AGameTimeOfDayActor` concrete reference를 가진다.
  - 옵션 B: `ITimeOfDayProvider` interface를 구현한 actor를 참조하고, `GetCurrentHour24` 및 time changed 구독 API를 통해 연결한다.
  - 옵션 C: `ADynamicSky`가 `UWorldSubsystem` 시간 관리자를 직접 조회해 구독한다.
- 권장 옵션: 옵션 B
답변: 옵션 B

4. 에디터 프리뷰 시작시간의 런타임 반영 주체
- 질문 내용: `ADynamicSky::bStartPlayFromPreviewHour`가 true일 때 preview 시간이 실제 게임 시작 시간에 어떻게 반영될지 결정이 필요하다.
- 필요한 이유: 에디터 프리뷰 시간은 DynamicSky의 visual preview 값이고, runtime `TimeOfDay`는 별도 시간 소유자가 가진다. DynamicSky가 BeginPlay에서 시간 소유자의 값을 직접 변경하면 표현 actor가 domain time을 mutate하는 예외가 생긴다.
- 선택지
  - 옵션 A: `ADynamicSky`가 BeginPlay에서 TimeOfDay owner에 `PreviewHour24`를 직접 설정한다.
  - 옵션 B: TimeOfDay owner가 자체 `InitialHour24`/preview-start 옵션을 소유하고, DynamicSky preview 시간은 visual preview에만 사용한다.
  - 옵션 C: editor utility 또는 level setting에서 DynamicSky preview 값을 TimeOfDay owner 초기값으로 동기화한다.
- 권장 옵션: 옵션 B
답변: 옵션 A 확장안. `ADynamicSky`가 `bUseEditorPreviewTime`과 `bStartGameTimeFromPreviewHour`가 모두 true일 때 게임 시작 시간을 DynamicSky preview 시간과 동기화한다. Canonical `AGameTimeOfDayActor`는 첫 `OnGameTimeOfDayChanged` broadcast 전에 `ADynamicSky::PreviewHour24`를 시작 시간으로 채택한다. Legacy `AEnvironmentTimeOfDayActor` provider 경로에서는 `ADynamicSky`가 BeginPlay에서 `PreviewHour24`를 `SetCurrentHour24`로 주입한다.

5. SkyAtmosphere curve 타입 계약
- 질문 내용: SkyAtmosphere의 `RayleighScattering`과 `MultiScattering`을 어떤 curve 타입으로 authoring할지 결정이 필요하다.
- 필요한 이유: 두 값은 단순 float로 볼 수도 있고, RGB/vector 성분을 가진 색/벡터 값으로 다룰 수도 있다. 타입 계약이 확정되어야 Details 패널 property, 평가 함수, Blueprint asset authoring 방식이 정해진다.
- 선택지
  - 옵션 A: `UCurveFloat`를 사용해 scalar 값만 시간대별로 조절한다.
  - 옵션 B: `UCurveLinearColor` 또는 vector curve를 사용해 채널별 값을 시간대별로 조절한다.
  - 옵션 C: curve asset 대신 `FRuntimeFloatCurve`/inline curve를 actor property로 노출한다.
  - 옵션 D: curve를 사용하지 않고 태양빛 존재시간용/비존재시간용 `RayleighScattering` 색상 2개와 `MultiScattering` float 2개를 Details 값으로 노출한다. 일출/일몰 주변 `GapTime` 전환 구간에서는 두 상태 값을 보간한다.
- 권장 옵션: 옵션 B
답변: 옵션 D. 기존 curve 기반 설계는 폐기한다. `RayleighScatteringCurve`, `MultiScatteringCurve`, curve 평가 로직은 제거하고, 태양빛 존재시간 여부와 gap 보간 alpha로 `RayleighScattering` 색상 2개와 `MultiScattering` float 2개를 평가한다.

6. Sun/Moon 궤도 회전축과 기준 방향
- 질문 내용: "0도에서 -180도까지 회전"을 Unreal `FRotator`에서 어떤 축과 기준 방향으로 해석할지 결정이 필요하다.
- 필요한 이유: DirectionalLight의 빛 방향은 actor rotation convention에 의존한다. Pitch 기준으로 0 -> -180을 적용할지, 기존 east-west yaw 보간과 결합할지, art direction상 해가 뜨는 방향을 어떤 world axis로 둘지 확정해야 레벨/sky material/그림자 방향이 일관된다.
- 선택지
  - 옵션 A: Pitch를 0 -> -180으로 보간하고 Yaw는 고정 `OrbitYaw`를 사용한다.
  - 옵션 B: 기존 EnvironmentTimeOfDay처럼 altitude를 pitch로, 동서 이동을 yaw 보간으로 계산한다.
  - 옵션 C: `USplineComponent` 또는 authoring curve로 sun/moon rotation을 직접 정의한다.
- 권장 옵션: 옵션 A
답변: 옵션 A 확장안. 태양과 달의 회전 기준은 `WorldRotation`이다. 태양과 달은 공통 궤적을 사용하며, 해당 궤적의 `Yaw`는 Details 패널에서 조절 가능해야 한다.
