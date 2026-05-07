# Cursor Part Focus Scope 미결 QnA

### [질문 항목]

1. RequiredStateTags가 만족되지 않는 파츠를 PreviewFocus 대상으로 보여줄 것인가?
- 질문 내용: 예를 들어 뚜껑이 닫혀 있어 `Beehive.LidOpen` 상태가 없을 때, `RequiredStateTags={Beehive.LidOpen}`인 소비장을 마우스 hover 대상으로 잡을지 결정이 필요하다.
- 필요한 이유: Begin은 거부하면 되지만, Preview 단계에서 아예 대상에서 제외할지, 비활성 prompt를 보여줄지에 따라 UX가 달라진다.
- 선택지
  - 옵션 A: RequiredStateTags가 만족되지 않는 파츠는 PreviewFocus 대상에서도 제외한다.
  - 옵션 B: PreviewFocus는 허용하되 prompt를 비활성/조건 미충족 상태로 표시하고 click begin은 거부한다.
  - 옵션 C: PreviewFocus와 click begin 모두 시도하되, begin 실패 시 별도 피드백만 준다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A

2. 이미 들어올린 소비장을 다시 클릭했을 때의 동작은 무엇인가?
- 질문 내용: 소비장 A가 이미 `PartFocusEngaged`로 들어올려진 상태에서 같은 소비장 A를 다시 클릭했을 때, 이를 cancel/원상복귀로 볼지, no-op으로 볼지 결정이 필요하다.
- 필요한 이유: 소비장 조작은 `FocusCancel -> 원상복귀`가 기본이지만, 같은 소비장을 다시 클릭해서 내려놓는 UX를 허용할지 확정해야 한다.
- 선택지
  - 옵션 A: 같은 소비장 재클릭은 해당 소비장 FocusCancel과 동일하게 처리해 원상복귀한다.
  - 옵션 B: 같은 소비장 재클릭은 no-op이며 Esc/외곽 취소 영역으로만 원상복귀한다.
  - 옵션 C: 같은 소비장 재클릭은 별도 action complete로 처리한다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A. 벌통 뚜껑 클릭도 마찬가지로 Engaged <-> Cancel 토글

3. 배치된 아이템 PreviewFocus prompt는 어떤 정보를 보여줄 것인가?
- 질문 내용: 벌통 내부에 배치된 화분떡, 말벌트랩은 현재 `PreviewOnly` 대상인데, hover 시 어떤 prompt를 보여줄지 결정이 필요하다.
- 필요한 이유: click action이 없는 PreviewOnly 대상이라도 이름만 보여줄지, 상태/잔량 같은 정보를 함께 보여줄지에 따라 필요한 데이터 계약이 달라진다.
- 선택지
  - 옵션 A: 아이템 이름만 표시한다.
  - 옵션 B: 아이템 이름 + 간단 상태값을 표시한다. 예: 잔량, 설치 상태, 작동 여부.
  - 옵션 C: 1차 구현에서는 prompt 없이 outline만 표시한다.
- 권장 옵션: 옵션 C
- 답변: 옵션 C

4. 화면 외곽 취소 영역 두께의 기본값은 어떻게 할 것인가?
- 질문 내용: 화면 외곽 클릭을 `FocusCancel`과 동일하게 처리하기로 했으므로, 외곽 취소 영역의 기본 두께를 정해야 한다.
- 예외 규칙: 현재 커서가 유효한 Part Focus hover target(`RegisteredParts` 유효 + `IsDescriptorPreviewAllowed`) 위에 있으면 edge cancel보다 target click 처리를 우선한다.
- 필요한 이유: 값이 너무 작으면 클릭하기 어렵고, 너무 크면 파츠 클릭과 충돌한다. Details 노출값으로 조정 가능하더라도 기본값은 필요하다.
- 선택지
  - 옵션 A: 48px
  - 옵션 B: 64px
  - 옵션 C: 96px
- 권장 옵션: 옵션 B
- 답변: 옵션 B

5. PartFocus outline은 preview와 engaged에서 어떻게 구분할 것인가?
- 질문 내용: 파츠 hover preview 상태와 PartFocusEngaged 상태의 outline 표현을 구분할지 결정이 필요하다.
- 필요한 이유: 여러 PersistentAction이 stack에 있을 수 있으므로, hover 중인 파츠와 이미 engaged된 파츠를 시각적으로 구분해야 할 수 있다.
- 선택지
  - 옵션 A: Preview와 Engaged outline을 같은 방식으로 표시한다.
  - 옵션 B: Preview outline과 Engaged outline을 다른 색/강도로 구분한다.
  - 옵션 C: Preview는 outline, Engaged는 별도 mesh 위치/애니메이션만으로 구분하고 outline은 제거한다.
- 권장 옵션: 옵션 B
- 답변: PartFocusEngaged 대상 여부와 무관하게, `RequiredStateTags`를 만족하는 PartFocus 대상에 마우스를 hover하면 해당 hover 대상에 외곽선을 표시한다. Engaged 상태라는 이유만으로 별도 outline을 계속 유지하지 않는다. 즉 outline은 기본적으로 현재 hover preview 대상 표시이며, 이미 engaged된 파츠라도 마우스 hover 중일 때만 outline 대상이 된다.

## Beehive Comb Lift C++ QnA

### [질문 항목]

1. 소비장 올리기 목표 기준점은 슬롯별 개별 기준점인가, 벌통 공통 기준점 하나인가?
- 질문 내용: 소비장을 들어올릴 때 목표 위치를 계산할 기준점 컴포넌트를 `ABeehive`에 하나만 둘지, 소비장 슬롯별로 별도 기준점을 둘지 결정이 필요하다.
- 필요한 이유: 공통 기준점 하나면 구현과 디자이너 조정이 단순하다. 슬롯별 기준점이면 각 소비장의 목표 위치를 세밀하게 다르게 만들 수 있지만 컴포넌트 관리가 커진다.
- 선택지
  - 옵션 A: `CombLiftTargetRoot` 같은 공통 `USceneComponent` 하나를 `ABeehive`에 추가하고 모든 소비장은 해당 위치로 올라간다.
  - 옵션 B: 슬롯별 lift target component를 생성한다.
  - 옵션 C: 별도 component 없이 offset 값만으로 목표 위치를 계산한다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A

2. 소비장 들어올림 회전은 시작 시점 카메라 기준으로 고정할 것인가?
- 질문 내용: 소비장을 올릴 때 플레이어 카메라 정면을 바라보도록 회전한다고 했을 때, begin 시점의 카메라 회전을 목표로 고정할지, 이동 중에도 카메라가 움직이면 계속 따라갈지 결정이 필요하다.
- 필요한 이유: 이동 중 카메라를 계속 추적하면 보간 목표가 매 프레임 바뀌어 흔들림이 생길 수 있다. 시작 시점 고정은 안정적이지만 이동 중 카메라를 돌려도 소비장이 따라오지는 않는다.
- 선택지
  - 옵션 A: PartFocus Begin 시점의 카메라 forward를 기준으로 목표 회전을 한 번 계산해 고정한다.
  - 옵션 B: 보간 중 매 tick 현재 카메라 forward를 다시 반영한다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A

3. 소비장이 들어올려진 상태에서 슬롯 layout refresh가 발생하면 어떻게 처리할 것인가?
- 질문 내용: `RefreshCombSlotTransforms()`가 호출되면 슬롯 transform을 rest 위치로 다시 세팅한다. 소비장이 올라간 상태에서 MaxCombCount/CurrentCombCount/CombSlotSpacing 변경 등으로 refresh가 발생하면 어떻게 처리할지 결정이 필요하다.
- 필요한 이유: 올려진 슬롯을 그냥 덮어쓰면 소비장이 갑자기 원위치로 이동한다. 반대로 무시하면 layout 변경 후 상태가 어긋날 수 있다.
- 선택지
  - 옵션 A: layout refresh 전 active lifted comb를 먼저 cancel/원상복귀한 뒤 refresh한다.
  - 옵션 B: active lifted slot은 refresh transform 적용에서 제외한다.
  - 옵션 C: refresh 후 lift target을 다시 계산해 들어올림 상태를 유지한다.
- 권장 옵션: 옵션 A
- 답변: 옵션C

## Queen Bee Actor QnA

### [질문 항목]

1. 여왕벌 위치 업데이트 시 yaw 랜덤 범위는 어떻게 할 것인가?
- 질문 내용: 60분 단위 위치 업데이트에서 여왕벌을 선택된 소비장 Front/Back 면 중앙에 부착한 뒤, 여왕벌 actor yaw에 어떤 범위의 랜덤값을 적용할지 결정이 필요하다.
- 필요한 이유: Tick yaw 떨림과 별개로 위치 갱신 순간의 기준 회전을 정해야 하며, 범위에 따라 연출 다양성과 면 부착 방향 일관성이 달라진다.
- 선택지
  - 옵션 A: `0~360도` 완전 랜덤 yaw를 적용한다.
  - 옵션 B: `-n2~n2도` 범위 랜덤 yaw를 적용하고 `n2`를 Details에 노출한다.
  - 옵션 C: 위치 업데이트 시에는 yaw 랜덤을 적용하지 않고 attach point 회전만 사용한다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A

2. 위치 업데이트 yaw는 어떤 회전을 기준으로 적용할 것인가?
- 질문 내용: 여왕벌을 소비장 면 중앙에 부착할 때, 랜덤 yaw를 선택된 attach point 기준 회전에 더할지, 기존 여왕벌 world rotation을 유지한 상태에서 yaw만 바꿀지 결정이 필요하다.
- 필요한 이유: Front/Back 면에 붙는 방향이 소비장 기준과 일관되어야 하는지, 기존 actor 회전 연속성을 우선할지에 따라 attach transform 계산 방식이 달라진다.
- 선택지
  - 옵션 A: 선택된 Front/Back attach point의 회전을 기준으로 yaw 랜덤값을 추가한다.
  - 옵션 B: 여왕벌 actor의 기존 world rotation을 유지한 뒤 yaw 랜덤값만 추가한다.
  - 옵션 C: 소비장 actor 회전을 기준으로 yaw 랜덤값을 추가하고 attach point의 회전은 무시한다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A

3. Tick yaw 떨림은 누적 회전인가, 기준 yaw 주변 흔들림인가?
- 질문 내용: 여왕벌 actor가 Tick마다 `AddActorLocalRotation`으로 yaw 떨림을 적용할 때, 매 Tick 변화량을 계속 누적할지, 위치 업데이트 시 정해진 기준 yaw 주변에서만 흔들리도록 보정할지 결정이 필요하다.
- 필요한 이유: 누적 방식은 요구사항의 `AddActorLocalRotation`과 직접 일치하지만 장시간 플레이 시 기준 방향이 계속 떠다닌다. 기준 yaw 주변 흔들림은 안정적이지만 구현상 기준 회전 상태를 별도로 보관해야 한다.
- 선택지
  - 옵션 A: 매 Tick `AddActorLocalRotation`으로 랜덤 yaw를 누적한다.
  - 옵션 B: 위치 업데이트 때 설정한 기준 yaw를 보관하고, Tick에서는 기준 yaw 주변의 비누적 offset으로 흔들린다.
  - 옵션 C: Tick yaw 떨림은 비활성화하고 위치 업데이트 시 랜덤 yaw만 사용한다.
- 권장 옵션: 옵션 B
- 답변: 옵션 A, 장시간 플레이 시 기준 방향이 계속 떠다니는게 원래 구현 목표

4. 여왕벌이 붙은 소비장이 들어올려질 때 어떻게 처리할 것인가?
- 질문 내용: 여왕벌이 부착된 소비장이 `UBeehiveCombLiftComponent`에 의해 들어올려지는 경우, 여왕벌을 소비장에 붙인 채 같이 이동시킬지, 즉시 다른 소비장으로 재배치할지 결정이 필요하다.
- 필요한 이유: 위치 업데이트 후보에서는 들어올려진 소비장을 제외하지만, 이미 여왕벌이 붙은 소비장이 나중에 들어올려지는 상황은 별도 정책이 필요하다.
- 선택지
  - 옵션 A: 여왕벌은 해당 소비장에 attach된 상태로 유지되어 소비장과 함께 이동한다.
  - 옵션 B: 소비장이 들어올려지는 순간 여왕벌을 즉시 다른 후보 소비장으로 재배치한다.
  - 옵션 C: 여왕벌을 일시적으로 숨기고 다음 60분 위치 업데이트 때 다시 배치한다.
- 권장 옵션: 옵션 A
- 답변: 옵션 A
