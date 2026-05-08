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
