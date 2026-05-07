### Beehive Honey System QnA

1. 소비장 HoneyAmount가 1.0에 도달했을 때 초과 생산량 처리
- 질문 내용: 60분 꿀 생산 업데이트에서 소비장별 증가량을 적용하다가 특정 소비장의 `HoneyAmount`가 1.0을 초과하면 초과분을 어떻게 처리할지 결정이 필요하다.
- 필요한 이유: 요구사항은 "소비장별 증가량에는 편차가 있지만 총합은 벌통의 꿀 증가량 총합과 같다"이므로, 단순 clamp로 초과분을 버리면 총합 보존 조건이 깨진다.
- 선택지
  - 옵션 A: 초과분을 버린다. 구현은 가장 단순하지만 실제 적용 총합이 계산된 총합보다 작아질 수 있다.
  - 옵션 B: 초과분을 아직 덜 찬 소비장들에 재분배한다. 총합 보존 요구에 가장 잘 맞지만 분배 로직이 조금 복잡해진다.
  - 옵션 C: 초과분을 벌통 단위 저장고에 누적한다. 장기 확장성은 좋지만 벌통 저장고 개념이 추가된다.
- 권장 옵션: 옵션 A

2. HoneyAmount 단위
- 질문 내용: 소비장이 내부에 유지하는 꿀 양 변수 `HoneyAmount`를 실제 저장량으로 볼지, 표시용 정규화 값으로 볼지 결정이 필요하다.
- 필요한 이유: 현재 요구사항은 `0~1` 변수라고 명시되어 있으나, 이후 수확/아이템화/소비장별 최대 용량 확장이 들어오면 내부 단위 설계가 달라질 수 있다.
- 선택지
  - 옵션 A: `HoneyAmount` 자체를 `0.0~1.0` 실제 저장량이자 표시량으로 사용한다.
  - 옵션 B: 내부 저장량은 별도 절대값으로 두고, 표시만 `CurrentHoney / MaxHoney`로 `0.0~1.0`을 계산한다.
- 권장 옵션: 옵션 B

3. 소비장별 꿀 증가 편차 방식
- 질문 내용: 꿀 증가량 총합을 소비장 수로 나눌 때 어떤 방식으로 소비장별 편차를 줄지 결정이 필요하다.
- 필요한 이유: 총합 보존을 유지하려면 개별 증가량을 무작위로 더하는 방식보다 가중치 생성 후 정규화하는 방식이 안정적이다.
- 선택지
  - 옵션 A: 모든 active 소비장에 `RandomRange(1-DeviationRatio, 1+DeviationRatio)` 가중치를 주고, 가중치 합으로 정규화한다.
  - 옵션 B: 중앙 소비장에 추가 가중치를 주고, 그 위에 랜덤 편차를 더한다.
  - 옵션 C: 이전 업데이트 분포를 저장해 다음 업데이트에서도 비슷한 분포를 유지한다.
- 권장 옵션: 옵션 A

4. 꿀 생산 BeginPlay 즉시 적용 여부
- 질문 내용: 꿀 생산 60분 bucket 구독 시 BeginPlay에서 즉시 1회 적용할지 결정이 필요하다.
- 필요한 이유: 현재 colony population은 기본적으로 BeginPlay 즉시 적용하지 않는다. 꿀 생산도 같은 정책을 따를지 확정해야 한다.
- 선택지
  - 옵션 A: `bApplyHoneyProductionOnBeginPlayBucket=false` 기본, 첫 60분 경과 후 적용한다.
  - 옵션 B: `bApplyHoneyProductionOnBeginPlayBucket=true` 기본, BeginPlay 시 즉시 1회 적용한다.
- 권장 옵션: 옵션 A

5. 꿀 plane 위치 표현 기준
- 질문 내용: 꿀 양에 따라 Front/Back 꿀 plane 위치를 어떻게 보간할지 기준이 필요하다.
- 필요한 이유: mesh 방향과 에셋 pivot은 Blueprint/asset마다 달라질 수 있으므로 C++에서 특정 축을 고정하면 에셋 변경에 취약하다.
- 선택지
  - 옵션 A: `FrontHoneyEmptyRelativeLocation`, `FrontHoneyFullRelativeLocation`, `BackHoneyEmptyRelativeLocation`, `BackHoneyFullRelativeLocation`을 C++ 변수로 노출하고 에디터에서 조정한다.
  - 옵션 B: 하나의 축과 거리 값만 노출하고 C++에서 위치를 계산한다.
  - 옵션 C: 위치 이동은 Blueprint 이벤트로만 처리하고 C++은 `HoneyAmount`만 전달한다.
- 권장 옵션: 옵션 A

6. 꿀 plane 머티리얼 파라미터 적용 대상
- 질문 내용: 꿀 업데이트 시 `HoneyAmount` scalar parameter를 어떤 material slot 또는 material instance에 적용할지 결정이 필요하다.
- 필요한 이유: Front/Back plane이 단일 material slot만 쓸 수도 있고, 복수 slot을 가질 수도 있다. C++ 적용 범위를 확정해야 누락이 없다.
- 선택지
  - 옵션 A: Front/Back plane의 material index 0에만 dynamic material instance를 만들고 `HoneyAmount`를 적용한다.
  - 옵션 B: Front/Back plane의 모든 material slot에 dynamic material instance를 만들고 `HoneyAmount`를 적용한다.
  - 옵션 C: material parameter 적용은 Blueprint에서 처리하고 C++은 이벤트만 호출한다.
- 권장 옵션: 옵션 A
- 추가 결정: material parameter 이름은 `HoneyAmount`를 유지하되, 값은 내부 절대 꿀 양이 아니라 `CurrentHoney / MaxHoneyPerComb`로 계산한 정규화된 `0.0~1.0` fill ratio를 적용한다.

7. 같은 bucket에서 꿀 생산과 벌 수 업데이트 처리 순서
- 질문 내용: 꿀 생산 bucket과 colony population bucket이 같은 주기와 같은 경계에 걸렸을 때 어떤 업데이트를 먼저 처리할지 결정이 필요하다.
- 필요한 이유: 꿀 생산량은 `ColonyBeeCount * HoneyProductionCoefficient`이므로, 같은 60분 경계에서 벌 수 업데이트 전/후 중 어느 값을 기준으로 삼는지에 따라 생산량이 달라진다.
- 결정: 같은 bucket에서는 현재 벌 수로 꿀 업데이트를 먼저 처리하고, 그 다음 벌 수 업데이트를 처리한다.
- 구현 기준: `ABeehive::GetGameTimeBucketSubscriptions()`에서 `HoneyProduction` subscription을 `ColonyPopulation` subscription보다 먼저 `OutSubscriptions`에 추가한다.
- 결과: 60분 경계의 꿀 생산은 업데이트 직전 `ColonyBeeCount`를 기준으로 계산되고, 이후 colony population이 다음 주기용 벌 수로 갱신된다.



8. MaxHoneyPerComb 기본값
- 질문 내용: 내부 꿀 양을 절대값으로 유지하기 때문에 소비장 1개가 담을 수 있는 최대 꿀 양 기본값을 결정해야 한다.
- 필요한 이유: `HoneyFillRatio = CurrentHoney / MaxHoneyPerComb`로 꿀 plane 위치와 material parameter `HoneyAmount`를 계산하므로, 최대 용량이 없으면 정규화 기준이 없다.
- 선택지
  - 옵션 A: `MaxHoneyPerComb = 1.0`. 기존 0~1 스케일과 가장 유사하고 초기 구현이 단순하다.
  - 옵션 B: `MaxHoneyPerComb = 10.0`. 내부 절대량 확장 여지를 조금 더 둔다.
  - 옵션 C: `MaxHoneyPerComb = 100.0`. 수확/아이템 수량과 직접 연결하기 쉽지만 초기 계수 튜닝 폭이 커진다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 C

9. HoneyProductionCoefficient 기본값
- 질문 내용: 60분마다 `TotalHoneyIncrease = ColonyBeeCount * HoneyProductionCoefficient`로 계산할 때 사용할 기본 계수를 결정해야 한다.
- 필요한 이유: 기본값이 없으면 에디터 배치 직후 꿀 생산 속도를 판단하기 어렵고, 너무 큰 값은 소비장을 즉시 가득 채울 수 있다.
- 선택지
  - 옵션 A: `HoneyProductionCoefficient = 0.001`. 벌 100마리 기준 60분에 총 0.1만큼 생산한다.
  - 옵션 B: `HoneyProductionCoefficient = 0.005`. 벌 100마리 기준 60분에 총 0.5만큼 생산한다.
  - 옵션 C: `HoneyProductionCoefficient = 0.01`. 벌 100마리 기준 60분에 총 1.0만큼 생산한다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 C

10. HoneyDistributionDeviationRatio 기본값
- 질문 내용: 소비장별 꿀 증가량 편차를 만들 때 사용할 기본 편차 비율을 결정해야 한다.
- 필요한 이유: 편차가 0이면 모든 소비장이 동일하게 차고, 너무 크면 특정 소비장 쏠림이 강해진다.
- 선택지
  - 옵션 A: `HoneyDistributionDeviationRatio = 0.2`. 각 소비장 가중치가 `0.8~1.2` 범위로 변한다.
  - 옵션 B: `HoneyDistributionDeviationRatio = 0.5`. 각 소비장 가중치가 `0.5~1.5` 범위로 변한다.
  - 옵션 C: `HoneyDistributionDeviationRatio = 0.0`. 편차 없이 균등 분배한다.
- 권장 옵션: 옵션 A
- 답변 : 옵션 B