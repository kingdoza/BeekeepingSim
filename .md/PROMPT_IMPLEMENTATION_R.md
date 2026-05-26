# 구현 수정 프롬프트: PartFocus Placed Item Retrieve 리뷰 Findings

## 우선순위

1. Medium: placed item retrieve PartFocus descriptor가 LMB persistent action으로도 동작하는 문제 수정

## 발견 문제

### 1. placed item retrieve action이 기본 `PersistentAction`으로 등록되어 LMB no-op action을 시작할 수 있음

- 대상 파일:
  - `Source/BeekeepingSim/Private/WorldActors/PlacedItemActor.cpp`
  - `Source/BeekeepingSim/Private/WorldActors/ItemPlacementSlotActor.cpp`
  - `Source/BeekeepingSim/Public/Focus/CursorPartFocusActionComponent.h`
- 원인:
  - `UCursorPartFocusActionComponent`의 기본 `EngageMode`는 `PersistentAction`이다.
  - `APlacedItemActor` 생성자에서 `RetrieveAction`의 engage mode를 override하지 않는다.
  - `AItemPlacementSlotActor::GetCursorPartFocusDescriptors_Implementation()`은 descriptor engage mode를 `Descriptor.ActionHandler->GetEngageMode()` 그대로 사용한다.
- 영향:
  - 회수는 RMB secondary 경로가 의도인데, placed item을 LMB 클릭하면 retrieve action이 persistent part action stack에 들어간다.
  - 이후 RMB 회수로 actor가 destroy/clear되면 scope의 active action stack에 destroyed placed item component reference가 남을 수 있다.
  - 현재 기능상 즉시 이중 회수는 보이지 않지만, 입력 의미와 lifecycle이 설계와 어긋난다.
- 수정 방향:
  - placed item retrieve descriptor는 LMB begin/cancel 대상이 되지 않도록 `PreviewOnly`로 등록한다.
  - 선택지 A: `APlacedItemActor` 생성자에서 `RetrieveAction->SetEngageMode(ECursorPartFocusEngageMode::PreviewOnly)` 호출
  - 선택지 B: `AItemPlacementSlotActor::GetCursorPartFocusDescriptors_Implementation()`에서 placed item descriptor의 `EngageMode`를 `PreviewOnly`로 고정
  - 권장: 선택지 A. action 자체의 의도를 component 생성 시점에 명시해 다른 provider가 재사용해도 같은 정책을 유지한다.

## 검증 방법

- UBT:
  - `BeekeepingSimEditor Win64 Development`
- 검색:
  - `rg -n "SetEngageMode|GetEngageMode|PlacedItem" Source/BeekeepingSim/Private/WorldActors Source/BeekeepingSim/Public/WorldActors`
- PIE:
  - occupied placed item hover 상태에서 LMB 클릭이 no-op인지 확인
  - occupied placed item hover 상태에서 RMB 회수가 한 번만 실행되는지 확인
  - 회수 후 slot empty item-use-area descriptor가 다시 나타나는지 확인
  - 회수 후 FocusCancel/host cancel에서 destroyed placed item action 관련 경고가 없는지 확인

## 문서 반영 필요 여부

- 불필요.
- 설계 문서는 이미 placed item 회수를 FocusEngaged 내부 PartFocus secondary action으로 정의하고 있으며, 수정은 구현 정책 보정이다.
