# WorldActors System

## Scope

- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `Source/BeekeepingSim/Public/WorldActors/WorldItemPickup.h`
- `Source/BeekeepingSim/Private/WorldActors/WorldItemPickup.cpp`
- `Source/BeekeepingSim/Public/WorldActors/StorageBox.h`
- `Source/BeekeepingSim/Private/WorldActors/StorageBox.cpp`

## Responsibilities

- 월드 상호작용 actor 구성(메시/포커스 타겟/액션 컴포넌트)
- 아이템 pickup, 보관함 actor의 도메인 상태 오너 역할
- focus 시스템과 inventory 시스템 연결 지점 제공

## Key Classes

- `ABeehive`: 앵커 기반 focus 상호작용 예시 actor
- `AWorldItemPickup`: 단일 아이템 획득 actor
- `AStorageBox`: storage 상호작용 actor

## Dependencies

- Focus
- Interaction
- Inventory
- UI

## Refactoring Notes

- actor 이름/클래스 유지, 컴포넌트 조합 구조 유지
- focus target + action component 조합 방식 유지
- storage actor는 `UStorageBoxComponent` 오너 구조 유지

## Manual Review Points

- pickup 성공/실패 시 actor 생명주기 처리
- storage actor의 active component 연결/해제 타이밍
- beehive focus enter/confirm/cancel 연동 이벤트 정상 동작
