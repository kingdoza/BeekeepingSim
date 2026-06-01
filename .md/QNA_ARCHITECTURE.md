### [훈연기와 벌통 공격성]

1. 벌통 공격성의 초기값을 어떻게 둘지?
- 질문 내용
  - 훈연기는 벌통 공격성을 낮추는 hold-use item으로 설계한다. 이때 `ABeehive`의 초기 `AggressionValue`를 어떤 값으로 시작할지 결정이 필요하다.
- 필요한 이유
  - 초기 공격성이 낮으면 훈연기 효과가 체감되지 않고, 초기 공격성이 최대이면 "훈연기로 낮춘다"는 gameplay loop가 명확해진다.
- 선택지
  - 옵션 A: `AggressionValue`를 `MaxAggressionValue`와 동일하게 시작한다. 기본값은 100/100.
  - 옵션 B: `AggressionValue`를 0으로 시작하고 외부 이벤트로 증가시킨다.
  - 옵션 C: `InitialAggressionValue`를 별도 authoring 값으로 둔다.
- 권장 옵션: 옵션 A. 벌통은 기본적으로 공격성을 가진 상태이고, 훈연기로 낮추는 흐름이 가장 직관적이다.
- 답변 : 옵션A

2. 훈연으로 낮아진 공격성이 자동 회복되어야 하는지?
- 질문 내용
  - 훈연기 사용 후 낮아진 벌통 공격성이 유지되는지, 시간이 지나며 회복되는지 결정이 필요하다.
- 필요한 이유
  - 자동 회복은 시간 bucket/상태 갱신 설계가 추가로 필요하고, 회복 없음은 소독약 효과 방식과 동일한 단순 누적 상태 변경으로 처리할 수 있다.
- 선택지
  - 옵션 A: 자동 회복 없음. 훈연으로 낮춘 공격성 값이 유지된다.
  - 옵션 B: `UGameTimeBucketSubsystem` bucket 이벤트로 일정 시간마다 회복한다.
  - 옵션 C: Tick 기반으로 연속 회복한다.
- 권장 옵션: 옵션 A. 기존 소독약 효과 방식과 동일하게 먼저 구현하고, 회복은 실제 공격 시스템이 구체화될 때 추가한다.
- 답변 : 옵션A

3. 공격성에서 실제 공격력을 계산하는 공식을 어떻게 둘지?
- 질문 내용
  - `AggressionValue`가 내려갈 때 해당 벌통의 실제 공격력이 어떤 방식으로 낮아지는지 결정이 필요하다.
- 필요한 이유
  - 공격성이 0일 때 완전히 무력화할지, 최소 공격력은 남길지에 따라 밸런스와 API가 달라진다.
- 선택지
  - 옵션 A: `EffectiveAttackPower = BaseAttackPower * AggressionRatio`
  - 옵션 B: `EffectiveAttackPower = BaseAttackPower * Lerp(MinAttackMultiplier, 1.0, AggressionRatio)`
  - 옵션 C: `AggressionValue` 자체를 공격력으로 사용한다.
- 권장 옵션: 옵션 B. `MinAttackMultiplier` 기본값을 0으로 두면 옵션 A와 동일하게 동작하면서, 이후 최소 공격력을 쉽게 조정할 수 있다.
- 답변 : 옵션D. 이 설계는 공격성 수치와 훈연기 동작에만 집중. 공격력 관련 구현은 X.

4. 훈연기 적용 use-area를 어디에 둘지?
- 질문 내용
  - 훈연기가 벌통의 어느 item-use-area에서 동작해야 하는지 결정이 필요하다.
- 필요한 이유
  - 기존 item-use-area mesh/tag 계약을 재사용할지, 훈연기 전용 hit 영역을 추가할지에 따라 Blueprint 작업 범위가 달라진다.
- 선택지
  - 옵션 A: 기존 벌통 body/lid 등 host use-area에 `Item.UseArea.Beehive.Smoker` 태그를 추가한다.
  - 옵션 B: 훈연기 전용 `UItemUseAreaMeshComponent`를 추가한다.
  - 옵션 C: 모든 벌통 use-area에서 훈연기를 허용하도록 host actor fallback을 둔다.
- 권장 옵션: 옵션 A. 소독약과 같은 item-use-area 효과 방식이므로 태그만 추가하는 편이 가장 일관적이다.
- 답변 : 옵션A

5. 훈연기 사용 중 자원을 소모할지?
- 질문 내용
  - 훈연기 사용 중 stack, durability, fuel 같은 자원을 소모할지 결정이 필요하다.
- 필요한 이유
  - 자원 소모를 넣으면 item stack/durability/remaining 정책과 UI 표시 범위가 함께 커진다.
- 선택지
  - 옵션 A: 1차 구현에서는 자원 소모 없음.
  - 옵션 B: durability를 감소시킨다.
  - 옵션 C: 별도 fuel/remaining 시스템을 추가한다.
- 권장 옵션: 옵션 A. 기존 소독약 효과 방식과 동일하게 hold-use 효과만 먼저 구현하고, 훈연기 연료는 별도 설계로 나중에 붙인다.
- 답변 : 옵션A
