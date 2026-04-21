다음 작업을 Unreal Editor 내부에서 도와줘.

목표:
- `ABeekeeperCharacter`의 `BeekeeperCameraShake` 컴포넌트에서 사용할 착지 전용 카메라 셰이크 블루프린트를 만든다.
- 이 셰이크는 캐릭터가 공중에서 지면으로 착지하는 순간에만 1회 재생된다.
- 기존 이동 셰이크(`Idle`, `Walk`, `Sprint`)와는 별개의 일회성 착지 반응이어야 한다.

작업 요청:
1. `CameraShakeBase` 기반 블루프린트 1개를 생성해줘.
   - `BP_BeekeeperCameraShake_Landing`
2. 저장 위치는 가능하면 `Content/Beekeeper/Camera/` 폴더로 해줘.
3. 이 셰이크는 1인칭 착지 반응용으로 만들어줘.
4. 느낌은 다음 방향으로 잡아줘.
   - 착지 순간 짧은 아래 방향 충격감
   - 아주 짧은 Pitch 반동
   - 필요하면 약한 위치 Z 반응
5. 과한 멀미를 유발하지 않도록 지속시간은 짧고 강도는 보수적으로 설정해줘.
6. 반복 루프형이 아니라 1회성 셰이크로 설정해줘.
7. `Walk`나 `Sprint` 이동 셰이크보다 순간적인 반응은 느껴지되, 너무 과장된 시네마틱 충격은 피해서 실제 플레이용으로 만들어줘.
8. 가능하면 기본값을 아래 방향으로 제안해줘.
   - 짧은 duration
   - 작은 Pitch kick
   - 작은 Z drop / rebound 느낌
   - 빠른 감쇠
9. 생성 후 `BeekeeperCharacter` 블루프린트에서 `BeekeeperCameraShake` 컴포넌트의 아래 프로퍼티에 연결해줘.
   - `LandingCameraShakeClass` -> `BP_BeekeeperCameraShake_Landing`
10. 기존 `IdleCameraShakeClass`, `WalkCameraShakeClass`, `SprintCameraShakeClass`는 건드리지 말고 유지해줘.

추가 조건:
- C++ 쪽에서는 이미 착지 순간에 `LandingCameraShakeClass`를 1회 재생하도록 구현되어 있다.
- 여기서는 블루프린트 에셋 제작과 컴포넌트 할당만 해주면 된다.
- 실제 플레이용 1인칭 착지 피드백이 목표다.
- 값이 애매하면 보수적인 기본값으로 시작하고, 나중에 더 강하게 튜닝할 수 있게 설명해줘.

최종 출력 형식:
- 생성한 블루프린트 이름
- 저장 경로
- 핵심 설정값 요약
- `BeekeeperCameraShake` 컴포넌트에 어떤 값으로 연결했는지 요약
- 추가 튜닝 포인트 2~3개
