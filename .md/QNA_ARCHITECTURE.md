# QNA_ARCHITECTURE

## Beehive 구심점 비행 Niagara 설계 QnA

### [질문 1]

1. 구심점 Niagara 소유 위치
- 질문 내용: 구심점을 기준으로 비행하는 신규 Niagara를 `ABeehive`가 직접 컴포넌트로 소유할지, 별도 child actor로 분리할지 확정해야 한다.
- 필요한 이유: 신규 Niagara는 벌통 내부 벌 수와 벌통 설정값에 직접 반응하며, 별도 경로나 독립 동작이 요구되지 않는다. 소유 위치에 따라 파라미터 적용 책임과 에디터 편집 방식이 달라진다.
- 선택지
  - 옵션 A: `ABeehive`에 `UNiagaraComponent* AttractionSwarmNiagara`를 직접 추가한다.
  - 옵션 B: 별도 `ABeehiveAttractionSwarmActor`를 만들고 `UChildActorComponent`로 소유한다.
  - 옵션 C: 레벨 독립 Actor로 배치하고 `ABeehive`가 참조한다.
- 권장 옵션: 옵션 A. `ABeehive`의 `ColonyBeeCount`와 시간 bucket 이벤트를 바로 사용하므로 가장 단순하고 책임이 명확하다.
- 답변: 옵션A

---

### [질문 2]

1. 구심점 위치 표현 방식
- 질문 내용: Niagara가 비행 기준으로 삼을 구심점을 어떤 방식으로 표현할지 확정해야 한다.
- 필요한 이유: User Parameter 목록에는 별도 center vector/object가 없다. 따라서 NiagaraComponent 자체 위치를 구심점으로 쓸지, SceneComponent를 따로 노출할지 정해야 한다.
- 선택지
  - 옵션 A: `AttractionSwarmNiagara` 컴포넌트의 위치 자체를 구심점으로 사용한다.
  - 옵션 B: `USceneComponent* AttractionSwarmCenter`를 추가하고 Niagara를 그 하위에 attach한다.
  - 옵션 C: Niagara User Parameter에 Center Position을 새로 추가한다.
- 권장 옵션: 옵션 B. 에디터에서 구심점만 이동/회전하기 쉽고, Niagara는 해당 SceneComponent에 attach되어 위치를 따라간다. User Parameter 추가 없이 요구사항을 만족한다.
- 답변: 옵션A

---

### [질문 3]

1. 신규 Niagara System asset 지정 방식
- 질문 내용: `AttractionSwarmNiagara`가 사용할 Niagara System asset을 어디서 지정할지 확정해야 한다.
- 필요한 이유: C++에서 Content asset path를 하드코딩하면 Content 의존이 생긴다. BP에서 지정하면 에디터 작업이 필요하지만 C++ 경계가 안전하다.
- 선택지
  - 옵션 A: `AttractionSwarmNiagara` 컴포넌트를 노출하고 BP_Beehive에서 Niagara System asset을 지정한다.
  - 옵션 B: `TObjectPtr<UNiagaraSystem> AttractionSwarmNiagaraSystem`을 `ABeehive`에 노출하고 C++에서 컴포넌트에 적용한다.
  - 옵션 C: C++에서 Niagara System asset path를 직접 로드한다.
- 권장 옵션: 옵션 A. 기존 Unreal 컴포넌트 편집 방식과 맞고 C++ Content path 의존을 피할 수 있다.
- 답변: 옵션A

---

### [질문 4]

1. Beehive 노출 설정 구조
- 질문 내용: `AttractionPower`, `NoisePower`, `SpawnSphereRadius`, SpawnAmount 계산 상수를 개별 UPROPERTY로 둘지 구조체로 묶을지 확정해야 한다.
- 필요한 이유: 관련 값들이 하나의 기능 설정이므로 구조체로 묶으면 확장과 문서화가 쉽다. 반면 개별 property는 에디터에서 바로 찾기 쉽다.
- 선택지
  - 옵션 A: `FBeehiveAttractionSwarmSettings` 구조체로 묶는다.
  - 옵션 B: `ABeehive`에 개별 UPROPERTY로 둔다.
- 권장 옵션: 옵션 A. 신규 Niagara 설정이 한 기능 단위이므로 구조체가 더 적합하다.
- 답변: 옵션A

---

### [질문 5]

1. `SpawnAmount` 계산식과 반올림 정책
- 질문 내용: Niagara `User.SpawnAmount(int32)`를 `ColonyBeeCount * 상수`로 계산할 때 상수 타입과 반올림 방식을 확정해야 한다.
- 필요한 이유: 계산 결과가 float일 수 있으므로 floor/round/ceil 중 하나를 정해야 결과가 안정적이다.
- 선택지
  - 옵션 A: `SpawnAmount = RoundToInt(ColonyBeeCount * SpawnAmountScale)`
  - 옵션 B: `SpawnAmount = FloorToInt(ColonyBeeCount * SpawnAmountScale)`
  - 옵션 C: `SpawnAmount = CeilToInt(ColonyBeeCount * SpawnAmountScale)`
- 권장 옵션: 옵션 A. 가장 직관적인 비율 계산이며 극단적으로 낮거나 높게 치우치지 않는다.
- 답변: 옵션A

---

### [질문 6]

1. `SpawnAmount` 상한값 필요 여부
- 질문 내용: `ColonyBeeCount * SpawnAmountScale` 결과를 제한하는 `MaxSpawnAmount`를 둘지 확정해야 한다.
- 필요한 이유: 벌 개체수가 커지거나 scale이 잘못 설정되면 Niagara spawn이 과도해질 수 있다.
- 선택지
  - 옵션 A: `MaxSpawnAmount`를 두고 최종값을 `0~MaxSpawnAmount`로 clamp한다.
  - 옵션 B: 상한 없이 계산값을 그대로 적용한다.
- 권장 옵션: 옵션 A. 성능 안전장치가 필요하다.
- 답변: 옵션A

---

### [질문 7]

1. `AttractionPower`, `NoisePower`, `SpawnSphereRadius` 즉시 적용 시점
- 질문 내용: 액터 단계에서 노출한 값 변경 시 언제 Niagara User Parameter에 반영할지 확정해야 한다.
- 필요한 이유: 에디터 변경 즉시 반영 요구가 있으므로 Construction/PropertyChange/BeginPlay 경로를 정해야 한다.
- 선택지
  - 옵션 A: `OnConstruction`, `PostEditChangeProperty`, `BeginPlay`, 명시적 `ApplyAttractionSwarmSettings()`에서 적용한다.
  - 옵션 B: `Tick`에서 계속 적용한다.
  - 옵션 C: BeginPlay에서만 적용한다.
- 권장 옵션: 옵션 A. 즉시 반영 요구를 만족하면서 Tick 비용을 만들지 않는다.
- 답변: 옵션A

---

### [질문 8]

1. `SpawnAmount` 자동 시간 업데이트 사용 여부
- 질문 내용: 신규 Attraction swarm의 `SpawnAmount`를 특정 시간 경계마다 자동 갱신할지, 시간 기반 자동 갱신 없이 벌통 설정/벌 개체수 변경 시점에만 갱신할지 확정해야 한다.
- 필요한 이유: 사용자가 특정 시간마다 `SpawnAmount`를 자동 업데이트하는 기능을 취소했다. 따라서 bucket listener나 시간 delegate에 신규 Attraction swarm `SpawnAmount` 갱신을 연결하면 안 된다.
- 선택지
  - 옵션 A: 특정 시간 경계 자동 업데이트를 사용하지 않는다. `OnConstruction`, `PostEditChangeProperty`, `BeginPlay`, 설정 변경, `ColonyBeeCount` 변경 시점에만 `SpawnAmount`를 재계산/적용한다.
  - 옵션 B: 기존 `BeeSwarm` 시간 bucket 이벤트에서 함께 갱신한다.
  - 옵션 C: `AttractionSwarm` 별도 subscription tag를 추가해 시간 bucket 이벤트로 갱신한다.
- 권장 옵션: 옵션 A. 최신 결정이 시간 기반 자동 업데이트 취소이므로 Attraction swarm은 시간 bucket 시스템에 연결하지 않는다.
- 답변: 옵션A

---

### [질문 9]

1. Niagara User Parameter 직접 수정 UI 숨김 방식
- 질문 내용: `AttractionSwarmNiagara`의 User Parameter를 모두 숨기는 방식을 확정해야 한다.
- 필요한 이유: NiagaraComponent details의 User Parameter UI를 완전히 숨기려면 details customization이 필요할 수 있다. 단순히 Beehive 값으로 덮어쓰는 것만으로는 UI 노출이 남는다.
- 선택지
  - 옵션 A: Custom details customization으로 `AttractionSwarmNiagara` User Parameter UI를 숨기고, Beehive 적용 경로에서도 항상 덮어쓴다.
  - 옵션 B: UI 숨김은 하지 않고, Beehive 적용 경로에서 항상 덮어쓴다.
  - 옵션 C: NiagaraComponent 자체를 최대한 private/VisibleAnywhere로 숨기되 엔진 기본 User Parameter UI는 별도 제어하지 않는다.
- 권장 옵션: 옵션 A. 요구사항이 “모두 숨김”이므로 숨김과 source-of-truth 강제를 같이 적용한다.
- 답변: 옵션A

---

### [질문 10]

1. `SpawnAmount` User Parameter 타입 적용 방식
- 질문 내용: Niagara User Parameter `SpawnAmount`가 `int32`라고 했으므로 C++에서 어떤 Niagara setter를 사용할지 확정해야 한다.
- 필요한 이유: 기존 spline swarm의 `SpawnAmount`는 float였다. 신규 Niagara는 int32이므로 float setter를 쓰면 Niagara parameter 타입과 불일치할 수 있다.
- 선택지
  - 옵션 A: `UNiagaraComponent::SetVariableInt(TEXT("User.SpawnAmount"), SpawnAmount)`를 사용한다.
  - 옵션 B: 기존처럼 float로 변환해 `SetVariableFloat`를 사용한다.
- 권장 옵션: 옵션 A. Niagara parameter 타입과 C++ setter 타입을 맞춘다.
- 답변: 옵션 A.

---

### [질문 11]

1. `ColonyBeeCount` 변경 시 `SpawnAmount` 즉시 반영 여부
- 질문 내용: `ColonyBeeCount` 자체가 변경될 때 Attraction swarm `SpawnAmount`를 즉시 재계산/적용할지 확정해야 한다.
- 필요한 이유: 시간 기반 자동 업데이트가 취소되었으므로, 벌 개체수 변경 시 즉시 반영하지 않으면 신규 Attraction swarm의 `SpawnAmount`가 오래된 값으로 유지될 수 있다.
- 선택지
  - 옵션 A: `ColonyBeeCount` 변경 시에도 즉시 `SpawnAmount`를 재계산/적용한다.
  - 옵션 B: `ColonyBeeCount` 변경은 저장만 하고 다음 10분 경계에서 반영한다.
- 권장 옵션: 옵션 A. 설정 변경 즉시 피드백이 자연스럽고, 10분 경계는 시간 흐름에 따른 정기 갱신으로 유지하면 된다.
- 답변: 옵션A

---

### [질문 12]

1. 기존 Outgoing/Ingoing spline swarm과의 관계
- 질문 내용: 신규 구심점 Niagara가 기존 dual spline swarm과 동시에 동작할지, 특정 상태에서 대체할지 확정해야 한다.
- 필요한 이유: 동시에 동작하면 벌통 주변 밀도가 증가하고, 대체 동작이면 기존 swarm 활성/비활성 제어가 필요하다.
- 선택지
  - 옵션 A: 기존 Outgoing/Ingoing spline swarm과 항상 동시에 동작한다.
  - 옵션 B: 구심점 swarm이 켜져 있으면 기존 spline swarm 일부 또는 전체를 끈다.
  - 옵션 C: Beehive 상태에 따라 선택적으로 동작한다.
- 권장 옵션: 옵션 A. 현재 요구에는 대체/상태 전환 조건이 없으므로 동시에 동작하는 부가 효과로 설계한다.
- 답변: 옵션 A
