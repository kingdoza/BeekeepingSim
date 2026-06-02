# Core System

## Scope

- Core 전용 C++ 소스 폴더는 없다.
- 이 문서는 시스템 경계, 공통 문서 규칙, Core Redirect 정책을 정의한다.
- 정본 문서 경로:
  - `.md/0_ARCHITECTURE.md`
  - `.md/Architecture/*.md`

## Responsibilities

- Character/Camera/Focus/Interaction/Inventory/UI/WorldActors/Environment 시스템 경계 유지
- Source 구조와 문서 구조 동기화
- Blueprint migration 및 Core Redirect 같은 cross-system 변경 기록
- legacy 문서가 현재 구조와 충돌하지 않도록 정본 경로 안내

## Current Core Redirects

`Config/DefaultEngine.ini`의 `[CoreRedirects]`는 현재 다음 호환 경로를 포함한다.

- 기존 template/class rename:
  - `Beekeeper` -> `BeekeeperCharacter`
  - `ProfessorMovement` -> `BeekeeperMovement`
- 보류 리팩토링 완료 후 drag/drop rename:
  - `StorageSlotDragDropOperation` -> `ItemSlotDragDropOperation`
  - `EStorageSlotContainerType` -> `EItemSlotContainerType`
- placed item actor migration:
  - `PollenPattyActor` -> `PlacedItemActor`

Core Redirect는 Blueprint/asset 직렬화 호환 목적이다. 새 rename을 추가할 때는 Editor 재시작, Blueprint compile/save, 심볼 재검사를 같이 수행한다.

## Dependency Rules

- 시스템 폴더명은 include 경로의 첫 segment로 유지한다.
- `#include "Public/..."` 형태는 사용하지 않는다.
- `Private` 내부 helper는 가능한 한 public header로 노출하지 않는다.
- UObject/UCLASS rename은 Core Redirect 계획 없이는 진행하지 않는다.
- Content 참조가 확인된 Blueprint API는 대체 노드 migration 전까지 삭제하지 않는다.
- Environment는 다른 gameplay 시스템에 의존하지 않는다. 다른 시스템이 시간에 반응할 때는 가능하면 `IGameTimeBucketListener` + `UGameTimeBucketSubsystem`을 사용한다.
- `ABeekeeperController`가 runtime clock 표시를 위해 `ITimeOfDayProvider`를 resolve하는 경로는 Character-side local UI binding 예외다. 이 경로는 `AGameTimeOfDayActor`를 우선하고 `AEnvironmentTimeOfDayActor`는 compatibility fallback으로만 사용한다. Gameplay actor가 이 경로를 따라 Environment actor를 polling하면 안 된다.
- Editor-only 보조 코드는 `#if WITH_EDITOR`와 editor dependency 조건을 명확히 둔다. 예: `BeehiveDualSwarmActorCustomization`은 runtime gameplay source of truth가 아니라 details panel 노출 제어용이다.
- `UDeveloperSettings` 기반 tuning 값은 해당 system 문서에 source of truth를 기록한다. 예: Focus screen-edge cancel 두께는 `UBeekeepingSimFocusSettings`가 소유한다.

## Legacy Document Policy

- `Source/ARCHITECTURE.md`는 오래된 단일 대형 문서였고 현재는 `.md/0_ARCHITECTURE.md`로 연결하는 안내 역할만 한다.
- `Source/QNA_ARCHITECTURE.md`는 과거 설계 질의/답변 기록으로만 본다.
- 새 설계 문서는 `.md/Architecture/*.md`에 추가한다.

## Manual Review Points

- 새 시스템이 생길 때 Source 폴더와 Architecture 문서가 같이 추가되는지 확인한다.
- Core Redirect가 누적될 경우 현재 에셋에 더 이상 필요 없는 redirect인지 별도 migration 후 판단한다.
- legacy 문서나 prompt 파일의 과거 내용이 현재 정본과 충돌하지 않도록 최종 링크를 정리한다.
- 새 system 간 예외 의존을 추가할 때는 `.md/0_ARCHITECTURE.md`의 의존 방향과 관련 system 문서를 함께 갱신한다.
- editor-only customization이 runtime module/package 경계나 shipping build에 영향을 주지 않는지 확인한다.
