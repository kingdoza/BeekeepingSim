다음 문서를 먼저 읽고 그 규칙을 기준으로 작업해:
- Source/CODEX_IMPLEMENTATION.md
- Source/ARCHITECTURE.md

다음 작업을 수행해.

[목표]
Beekeeper 캐릭터가 공중에 있을 때는 이동 상태 기반 카메라 셰이크를 정지하고,
착지하는 순간에는 일회성 착지 카메라 셰이크를 재생하도록 구현한다.

[작업 대상 범위]
오직 아래 경로의 실제 C++ 파일만 수정 대상으로 고려한다.
- Source/BeekeepingSim/Public
- Source/BeekeepingSim/Private

[우선 검토 대상 파일]
- Source/BeekeepingSim/Public/BeekeeperCharacter.h
- Source/BeekeepingSim/Private/BeekeeperCharacter.cpp
- Source/BeekeepingSim/Public/BeekeeperMovementComponent.h
- Source/BeekeepingSim/Private/BeekeeperMovementComponent.cpp
- Source/BeekeepingSim/Public/BeekeeperCameraShakeComponent.h
- Source/BeekeepingSim/Private/BeekeeperCameraShakeComponent.cpp

[구현 요구사항]
1. 기존 구조를 유지한다
2. 카메라 셰이크 제어 책임은 `UBeekeeperCameraShakeComponent` 에 둔다
3. 캐릭터가 공중 상태일 때는 idle / walk / sprint 카메라 셰이크를 모두 정지한다
4. 캐릭터가 공중에서 지면으로 착지하는 순간 1회성 착지 카메라 셰이크를 재생한다
5. 착지 셰이크는 반복 재생되면 안 되고, 실제 착지 순간에만 1회 재생되어야 한다
6. 로컬 플레이어에게만 셰이크가 재생되도록 유지한다
7. null 체크와 상태 전이 체크를 포함한다
8. 불필요한 구조 변경은 하지 않는다
9. 추측이 필요한 내용은 반드시 "추측"이라고 명시한다

[상세 동작 조건]
- `UBeekeeperCameraShakeComponent` 는 현재 이동 상태 셰이크를 관리하는 기존 책임을 유지한다
- 추가로 이전 프레임의 공중 여부를 기억한다
- 현재 프레임이 공중 상태라면:
  - 현재 재생 중인 이동 셰이크를 정지한다
  - 이동 상태 기반 셰이크를 새로 시작하지 않는다
- 이전 프레임은 공중 상태였고 현재 프레임은 지상 상태라면:
  - 착지로 간주한다
  - 착지용 카메라 셰이크를 1회 재생한다
- 지상 상태로 돌아온 이후에는 기존 idle / walk / sprint 상태 계산 로직이 다시 정상 동작해야 한다

[구현 방향]
- 가능하면 `UCharacterMovementComponent::IsFalling()` 을 활용한다
- `UBeekeeperCameraShakeComponent` 내부에 이전 공중 상태를 저장하는 변수를 둔다
- 착지 셰이크용 프로퍼티를 추가한다
  - 예: `LandingCameraShakeClass`
- 이동 셰이크 정지와 착지 셰이크 재생은 함수로 분리한다
- 기존 상태 전환 로직과 충돌하지 않게 정리한다
- 착지 직후 이동 상태 셰이크가 필요하면 기존 상태 계산 흐름에서 다시 적용되게 한다

[권장 구현 포인트]
- `BeginPlay` 에서 초기 공중 상태를 저장한다
- `TickComponent` 에서 다음 순서로 처리한다
  1. 로컬 플레이어 여부 확인
  2. 현재 falling 상태 확인
  3. 공중이면 이동 셰이크 정지
  4. 이전 프레임 대비 착지 여부 판정
  5. 착지면 landing shake 1회 재생
  6. 지상일 때만 idle / walk / sprint 상태 계산 및 적용
  7. 마지막에 이전 공중 상태 갱신
- 착지 셰이크는 `StartCameraShake` 로 1회 재생하고, 이동 셰이크의 지속 상태와 분리한다

[주의사항]
- 공중 상태 진입 시마다 착지 셰이크가 재생되면 안 된다
- 걷기/달리기 셰이크가 공중에서도 유지되면 안 된다
- 비로컬 플레이어에서 셰이크 관련 Tick 이 불필요하게 돌지 않도록 한다
- 기존 `IdleCameraShakeClass`, `WalkCameraShakeClass`, `SprintCameraShakeClass` 동작을 깨지 않는다

[작업 절차]
1. 작업 범위를 요약한다
2. 영향 받는 파일 목록을 제시한다
3. 필요한 신규 프로퍼티/상태 변수/함수 계획을 제시한다
4. 승인 후 구현한다
5. 구현 완료 후 변경 사항을 요약한다
6. 구조 변경이 있으면 `Source/ARCHITECTURE.md` 에 변경된 부분만 반영한다
7. 인간 검토가 필요한 로직을 식별한다
8. 코드 리뷰용 프롬프트를 생성한다

[완료 후 보고 형식]
- [상태] 완료
- [요약] 수행 내용
- [영향 파일] 변경 파일 목록
- [ARCHITECTURE.md 반영 여부] 반영 내용 요약
- [검토 필요 로직] 인간 검토 필요 항목
- [코드 리뷰 프롬프트] 복사 가능한 리뷰 요청 문장
- [다음] 추가 지시 대기
