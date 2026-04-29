# BeekeepingSim Architecture Map

## 프로젝트 개요

- 이 문서는 `Source/BeekeepingSim/Public`, `Source/BeekeepingSim/Private` 기준의 **전체 지도 문서**다.
- 상세 클래스 설명과 실행 흐름은 시스템별 문서로 분리한다.
- 현재 리팩토링 기준 시스템 분할:
  - Character
  - Camera
  - Focus
  - Interaction
  - Inventory
  - UI
  - WorldActors
  - Core(문서 경계 전용)

## 분석 범위

- 주 분석 범위: 위 7개 시스템(소스 파일 이동 대상 60개)
- 문서 경계만 존재: Core
- 분석 범위 밖 실제 코드(동일 모듈 내 병존):
  - `Source/BeekeepingSim` 루트 템플릿/기본 클래스
  - Variant 계열 코드
- 리팩토링 중 분석 범위 밖 코드는 **include 경로 보정** 외 변경하지 않는다.

## 시스템 문서 링크

- [CharacterSystem.md](Architecture/CharacterSystem.md)
- [CameraSystem.md](Architecture/CameraSystem.md)
- [FocusSystem.md](Architecture/FocusSystem.md)
- [InteractionSystem.md](Architecture/InteractionSystem.md)
- [InventorySystem.md](Architecture/InventorySystem.md)
- [UISystem.md](Architecture/UISystem.md)
- [WorldActorsSystem.md](Architecture/WorldActorsSystem.md)
- [CoreSystem.md](Architecture/CoreSystem.md)

## 시스템 간 주요 의존 관계

- Character -> Camera, Focus, Inventory, UI
- Camera -> Character
- Focus -> Character, Camera, Inventory, UI
- Interaction -> Focus, Inventory, WorldActors
- Inventory -> Focus(규칙 반영), UI
- UI -> Inventory, Focus, Character(Controller drag 상태)
- WorldActors -> Focus, Interaction, Inventory
- Core -> 공통 규칙/문서 허브(전용 소스 없음)

## 리팩토링 후 Source 구조 요약

```text
Source/BeekeepingSim/
  Public/
    Character/
    Camera/
    Focus/
    Interaction/
    Inventory/
    UI/
    WorldActors/
  Private/
    Character/
    Camera/
    Focus/
    Interaction/
    Inventory/
    UI/
    WorldActors/
```

- `Core`는 문서상 경계만 유지하며 빈 소스 폴더를 만들지 않는다.
