# QNA_ARCHITECTURE

## 게임 시간 Bucket 이벤트 시스템 설계 QnA

### [질문 1]

1. 중앙 시간 Bucket 시스템 형태
- 질문 내용: 게임 시간 n분 단위마다 여러 액터가 특정 동작을 수행하게 할 중앙 시스템을 Actor로 둘지 Subsystem으로 둘지 확정해야 한다.
- 필요한 이유: 앞으로 벌통 외에도 농작물, 생산기계, NPC 스케줄 등 여러 시스템이 같은 시간 단위 이벤트를 사용할 가능성이 높다. 중앙 시스템 형태는 배치/참조/자동 생성 방식에 영향을 준다.
- 선택지
  - 옵션 A: `AGameTimeBucketEventActor`를 레벨에 배치한다.
  - 옵션 B: `UGameTimeBucketSubsystem`을 `UWorldSubsystem`으로 만든다.
  - 옵션 C: 각 기능별 Actor/Component가 직접 `AEnvironmentTimeOfDayActor` 이벤트를 구독한다.
- 권장 옵션: 옵션 B. 장기 확장성은 `UWorldSubsystem`이 가장 좋다. 다만 에디터에서 수동 참조와 디버깅이 쉬운 초기 구현을 우선하면 옵션 A도 가능하다.
- 답변: 옵션B

---

### [질문 2]

1. `AEnvironmentTimeOfDayActor` 연결 방식
- 질문 내용: 중앙 Bucket 시스템이 현재 시간 원천인 `AEnvironmentTimeOfDayActor`를 어떻게 찾고 연결할지 확정해야 한다.
- 필요한 이유: 시간 Actor가 레벨에 1개 있다는 전제는 있지만, 참조 방식에 따라 에디터 설정 누락/자동 탐색 비용/복수 Actor 오류 처리가 달라진다.
- 선택지
  - 옵션 A: `AEnvironmentTimeOfDayActor`를 UPROPERTY로 수동 지정한다.
  - 옵션 B: BeginPlay에서 `AEnvironmentTimeOfDayActor`를 자동 탐색한다.
  - 옵션 C: 수동 지정값을 우선 사용하고, 비어 있으면 자동 탐색 fallback을 수행한다.
- 권장 옵션: 옵션 C. 명시 참조를 우선하면서도 레벨 설정 누락에 대한 복원력이 있다.
- 답변: 옵션C

---

### [질문 3]

1. 시간 Bucket 구독 방식
- 질문 내용: 여러 Actor가 n분 단위 이벤트를 받는 방식을 인터페이스, 컴포넌트, 중앙 배열 등록 중 무엇으로 할지 확정해야 한다.
- 필요한 이유: 기능이 늘어날수록 각 Actor가 독립적으로 구독 단위와 정책을 가져야 하며, Blueprint Actor도 쉽게 참여할 수 있어야 한다.
- 선택지
  - 옵션 A: `UGameTimeBucketListener` 인터페이스를 Actor가 직접 구현한다.
  - 옵션 B: `UGameTimeBucketListenerComponent`를 필요한 Actor에 붙인다.
  - 옵션 C: 중앙 시스템에 `TargetActors` 배열을 두고 대상 Actor를 직접 등록한다.
- 권장 옵션: 옵션 A. C++/Blueprint Actor가 직접 반응을 정의하기 쉽고, 중앙 시스템은 구독자 탐색/호출만 담당할 수 있다. 다만 Actor 코드를 건드리기 어려운 외부 BP용으로 옵션 B를 보조 확장으로 고려할 수 있다.
- 답변: 옵션A

---

### [질문 4]

1. Blueprint 구독 지원 범위
- 질문 내용: 시간 Bucket 이벤트를 C++ 전용으로 둘지, Blueprint Actor도 바로 구현/수신 가능하게 할지 확정해야 한다.
- 필요한 이유: 게임 시간 n분 단위 행동은 디자이너가 BP에서 빠르게 붙일 가능성이 높다.
- 선택지
  - 옵션 A: C++ virtual/interface 함수만 제공한다.
  - 옵션 B: `BlueprintNativeEvent` 또는 `BlueprintImplementableEvent`로 BP 구현을 지원한다.
- 권장 옵션: 옵션 B. 확장 기능이 많아질 예정이면 BP 구독 지원이 필요하다.
- 답변: 옵션B

---

### [질문 5]

1. Bucket 단위 입력 정책
- 질문 내용: 구독자가 사용할 시간 단위를 1/5/10/30/60분 preset으로 제한할지, 임의의 n분 값을 허용할지 확정해야 한다.
- 필요한 이유: 임의 n분은 유연하지만 검증과 bucket 계산이 필요하다. preset은 단순하지만 요구가 늘면 제한이 된다.
- 선택지
  - 옵션 A: preset enum만 허용한다.
  - 옵션 B: 임의의 `BucketMinutes` 정수를 허용하고 `1~1440` 범위로 clamp한다.
- 권장 옵션: 옵션 B. “게임시간 n분 단위” 요구에 더 직접적으로 맞고 확장성이 좋다.
- 답변: 옵션B, 하루 00:00 기준으로 n분마다 자르고, 마지막은 짧은 bucket 허용

---

### [질문 6]

1. BeginPlay 즉시 호출 정책
- 질문 내용: 플레이 시작 시 현재 게임 시간이 12:35라면 즉시 한 번 이벤트를 보낼지, 다음 10분 경계인 12:40까지 기다릴지 확정해야 한다.
- 필요한 이유: 벌통 SpawnAmount 같은 상태 동기화 기능은 시작 즉시 현재 시간 기준으로 맞춰야 한다.
- 선택지
  - 옵션 A: 모든 listener에 BeginPlay 즉시 호출한다.
  - 옵션 B: listener별 `bApplyImmediatelyOnBeginPlay` 설정에 따라 호출한다.
  - 옵션 C: BeginPlay 즉시 호출하지 않고 다음 bucket 경계부터 호출한다.
- 권장 옵션: 옵션 B. Beehive는 true가 적절하지만, 모든 시스템이 즉시 호출을 원하는 것은 아닐 수 있다.
- 답변: 옵션B

---

### [질문 7]

1. 시간 점프 처리 정책
- 질문 내용: 시간이 12:10에서 15:40으로 점프했을 때 중간 bucket 이벤트를 모두 처리할지, 현재 bucket만 처리할지 확정해야 한다.
- 필요한 이유: 벌통 SpawnAmount는 현재 상태만 맞추면 되지만, 생산/성장 시스템은 중간 시간을 누적 처리해야 할 수 있다.
- 선택지
  - 옵션 A: `LatestOnly`만 지원한다. 점프 후 현재 bucket 이벤트만 보낸다.
  - 옵션 B: `CatchUp`만 지원한다. 지나간 bucket들을 모두 순차 호출한다.
  - 옵션 C: listener별로 `LatestOnly` / `CatchUp` 정책을 선택하게 한다.
- 권장 옵션: 옵션 C. Beehive는 `LatestOnly`, 누적형 시스템은 `CatchUp`이 필요할 수 있다.
- 답변: 옵션C

---

### [질문 8]

1. 여러 Bucket 단위 동시 구독 지원 여부
- 질문 내용: 한 Actor가 5분 이벤트와 60분 이벤트를 동시에 받을 수 있게 할지 확정해야 한다.
- 필요한 이유: 하나의 Actor가 짧은 주기의 시각 갱신과 긴 주기의 생산/성장 갱신을 함께 가질 수 있다.
- 선택지
  - 옵션 A: Actor당 하나의 Bucket 단위만 허용한다.
  - 옵션 B: Actor당 여러 Bucket 단위를 배열로 구독할 수 있게 한다.
- 권장 옵션: 옵션 B. 확장성을 고려하면 여러 단위 동시 구독이 유리하다.
- 답변: 옵션B

---

### [질문 9]

1. 24시 wrap 및 날짜 정보 payload
- 질문 내용: 23:50에서 00:00으로 넘어갈 때 이벤트 payload에 날짜 경과 정보를 포함할지 확정해야 한다.
- 필요한 이유: 하루 경계에서 성장/생산/스케줄 시스템이 별도 처리를 해야 할 수 있다.
- 선택지
  - 옵션 A: `Hour24`, `BucketMinutes`, `BucketIndex`만 전달한다.
  - 옵션 B: `bWrappedDay`를 추가 전달한다.
  - 옵션 C: `DayIndex` 또는 누적 일수까지 관리해 전달한다.
- 권장 옵션: 옵션 B. 현재 시간 시스템이 일수까지 직접 관리하지 않으므로, 우선 하루 wrap 여부만 제공하는 것이 적절하다.
- 답변: 옵션B

---

### [질문 10]

1. 게임 시간 정지 및 수동 시간 변경 처리
- 질문 내용: `bTimeProgressionEnabled=false`일 때 bucket 이벤트를 멈출지, `SetCurrentHour24()` 같은 수동 시간 변경은 즉시 반영할지 확정해야 한다.
- 필요한 이유: 시간 정지와 시간 점프는 서로 다른 의도다. 정지 중에도 수동 변경은 프리뷰/디버그/이벤트 트리거로 사용될 수 있다.
- 선택지
  - 옵션 A: 시간 정지 중에는 모든 bucket 이벤트를 막는다.
  - 옵션 B: Tick 기반 진행은 멈추지만, 수동 시간 변경 broadcast는 bucket 변경으로 처리한다.
  - 옵션 C: 시간 정지 여부와 무관하게 들어오는 모든 시간 broadcast를 처리한다.
- 권장 옵션: 옵션 B. 진행 정지는 존중하면서도 명시적 시간 변경은 반영할 수 있다.
- 답변: 옵션B

---

### [질문 11]

1. Listener 자동 발견 및 런타임 등록
- 질문 내용: 시간 Bucket listener를 BeginPlay에 1회만 자동 탐색할지, 런타임 스폰 Actor까지 자동 등록할지 확정해야 한다.
- 필요한 이유: 초기 구현 복잡도와 런타임 동적 Actor 지원 범위가 달라진다.
- 선택지
  - 옵션 A: BeginPlay에서 1회 자동 탐색만 한다.
  - 옵션 B: BeginPlay 1회 자동 탐색 + `RegisterListener` / `UnregisterListener` API를 제공한다.
  - 옵션 C: 주기적으로 재탐색해 런타임 스폰 Actor까지 자동 감지한다.
- 권장 옵션: 옵션 B. 성능과 확장성의 균형이 좋다.
- 답변: 옵션B

---

### [질문 12]

1. Beehive의 시간 Bucket 수신 방식
- 질문 내용: `ABeehive`가 직접 시간 Bucket listener 인터페이스를 구현할지, 별도 adapter component/actor가 받아서 `ApplyBeeSwarmHour24()`를 호출할지 확정해야 한다.
- 필요한 이유: Beehive의 시간 반응은 벌통 핵심 기능에 가깝지만, 시간 시스템 의존을 Beehive에서 최대한 분리할 수도 있다.
- 선택지
  - 옵션 A: `ABeehive`가 직접 listener 인터페이스를 구현하고 10분 bucket에서 `ApplyBeeSwarmHour24()`를 호출한다.
  - 옵션 B: `UBeehiveTimeBucketListenerComponent`가 listener 역할을 하고 소유 Beehive에 `ApplyBeeSwarmHour24()`를 호출한다.
  - 옵션 C: 중앙 시스템이 `ABeehive`를 특별 취급해 직접 호출한다.
- 권장 옵션: 옵션 A. `ABeehive`에 이미 `ApplyBeeSwarmHour24()` API가 있고, 벌통의 시간 반응이 명확하므로 가장 단순하다. 중앙 시스템의 특별 취급은 피한다.
- 답변: 옵션A
