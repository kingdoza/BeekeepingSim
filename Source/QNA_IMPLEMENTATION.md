### [WorldItemPickup Prompt Update]

1. pickup 프롬프트 수량 갱신 방식
- `AWorldItemPickup` 의 focus prompt 에 `DisplayName + Quantity` 를 반영하려면 현재 `UFocusTargetComponent` 에 동적 프롬프트 갱신용 공개 API가 필요합니다.
- 현재 수정 대상 목록에는 `UFocusTargetComponent` 가 포함되어 있지 않아, 설계 범위 내 처리 방식 확인이 필요합니다.
- 선택지

- 옵션 A: `UFocusTargetComponent` 를 최소 확장해 prompt 텍스트 setter 또는 override API를 추가한다.
- 옵션 B: `UFocusTargetComponent` 는 수정하지 않고 pickup 프롬프트는 고정 텍스트로 유지한다.
- 옵션 C: 다른 승인된 방식으로 지정한다.

답변:
- 사용자 결정: `AWorldItemPickup` 는 항상 수량 1의 단일 pickup 으로 단순화한다.
- 후속 반영: 부분 획득과 수량 감소 흐름은 제거하고, item display name 반영을 위해 `UFocusTargetComponent` 에 최소 display name setter 만 추가한다.
