# Character System

## Scope

- `Source/BeekeepingSim/Public/Character/BeekeeperCharacter.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperCharacter.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperController.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperController.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperMovementComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperMovementComponent.cpp`
- `Source/BeekeepingSim/Public/Character/BeekeeperHeldItemVisualizerComponent.h`
- `Source/BeekeepingSim/Private/Character/BeekeeperHeldItemVisualizerComponent.cpp`

## Responsibilities

- 플레이어 입력 라우팅(이동/시점/점프/스프린트/focus/hotbar)
- 캐릭터 이동 상태 관리(걷기/질주)
- 로컬 플레이어 기준 아이템 표시 actor 갱신
- 포커스 카메라 오버라이드 진입/복귀 제어

## Key Classes

- `ABeekeeperCharacter`: 입력, 카메라 기준점, 하위 컴포넌트 오너
- `ABeekeeperController`: 입력 매핑 및 drag 연동 상태 관리
- `UBeekeeperMovementComponent`: 스프린트/감속/전방 입력 기반 이동 상태
- `UBeekeeperHeldItemVisualizerComponent`: 선택 아이템의 held/on-cursor 시각화

## Dependencies

- Camera
- Focus
- Inventory
- UI

## Refactoring Notes

- 클래스명/파일명/public API 유지
- 캐릭터가 focus/hotbar 시그널의 중심 라우터라는 구조는 유지
- 로컬/비로컬 visualizer 동작 분기와 Tick 정책 유지

## Manual Review Points

- focus camera override 시작/종료 시 입력 잠금 복구 순서
- hotbar 휠 입력과 drag 수량 조절 분기
- 비로컬에서 visualizer actor 정리 및 Tick 간격 저하 동작
