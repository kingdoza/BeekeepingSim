다음 요구사항에 따라 BeekeepingSim 프로젝트에 아이템 획득 기능을 구현하라.

## 작업 목표

- 현재 `UBeekeeperHotbarComponent` 기반 구조에 아이템 획득 기능을 추가한다.
- 별도 인벤토리 컴포넌트는 만들지 않는다.
- 월드에서 획득한 아이템은 hotbar에 직접 저장한다.
- 같은 아이템을 다시 획득하면 기존 stack에 `MaxStack` 한도까지 우선 병합한다.
- 병합 후 남는 수량이 있으면 첫 번째 빈 hotbar 슬롯에 자동 배치한다.
- hotbar에 빈 슬롯이 없고 기존 stack도 모두 가득 찬 경우 획득은 실패 처리하고 월드 아이템은 그대로 둔다.
- 획득 대상은 `AWorldItemPickup` 계열 액터로 구현하고, 기존 focus 구조(`UFocusTargetComponent` + `UFocusActionComponent`)를 재사용한다.
- 획득 실패 피드백은 최소 구현으로 로그 또는 디버그 메시지 수준만 제공한다.

## 대상 파일 목록

- 새 파일 생성 대상
  - `Source/BeekeepingSim/Public/WorldItemPickup.h`
  - `Source/BeekeepingSim/Private/WorldItemPickup.cpp`
  - `Source/BeekeepingSim/Public/PickupFocusActionComponent.h`
  - `Source/BeekeepingSim/Private/PickupFocusActionComponent.cpp`
- 기존 파일 수정 대상
  - `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
  - `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`
  - `Source/BeekeepingSim/Public/ItemDefinition.h`
  - `Source/BeekeepingSim/Private/ItemInstance.cpp`
  - `Source/ARCHITECTURE.md`

## 구현 요구사항

### 1. Hotbar 직접 저장 구조 유지

- 아이템 소유 데이터는 `UBeekeeperHotbarComponent` 슬롯에 직접 저장한다.
- 별도 `UBeekeeperInventoryComponent` 는 추가하지 않는다.
- 슬롯 데이터는 기존처럼 `UObject* ItemInstance` 기반을 유지하되, 실제 런타임 획득 아이템은 `UItemInstance` 를 사용한다.

### 2. Hotbar 획득 API 추가

- `UBeekeeperHotbarComponent` 에 아이템 획득 전용 공개 API를 추가한다.
- 최소한 아래 역할을 수행할 수 있어야 한다.
  - `UItemDefinition` 과 획득 수량을 받아 획득 시도
  - 기존 동일 아이템 stack 병합
  - 첫 번째 빈 슬롯 자동 배치
  - 성공/실패 여부 반환
  - 일부 수량만 처리된 경우 남은 수량 계산
- 획득 결과는 구현자가 바로 사용할 수 있는 결과 구조체로 정리한다.
  - 최소 포함 권장 항목
    - 성공 여부
    - 요청 수량
    - 실제 추가 수량
    - 남은 수량
    - 실패/부분 처리 메시지

### 3. Stack 처리 규칙

- 동일 아이템 판단 기준은 `UItemDefinition` 참조 동일성 기준으로 처리한다.
- 같은 아이템을 획득하면 기존 `UItemInstance` 중 아직 `MaxStack` 에 도달하지 않은 stack부터 채운다.
- `UItemInstance::SetStackCount()` 의 기존 clamp 규칙을 유지한다.
- stack 병합 후 남는 수량만 새 `UItemInstance` 로 생성한다.
- 새 인스턴스도 `Definition`, `StackCount`, `Durability`, `InstanceId`, `Actions` 초기화 규칙을 따른다.

### 4. 빈 슬롯 자동 배치 규칙

- 새 stack 또는 새 아이템은 첫 번째 빈 hotbar 슬롯에 자동 배치한다.
- hotbar는 고정 8슬롯 구조를 유지한다.
- 빈 슬롯이 없으면 획득 실패로 간주한다.
- 일부 수량은 stack 병합되었지만 나머지가 들어가지 못하는 경우:
  - 병합된 수량은 반영한다.
  - 남은 수량은 월드 아이템에 남겨둔다.
  - 결과 구조체에 부분 성공 정보를 남긴다.

### 5. 월드 획득 액터 구조

- `AWorldItemPickup` 액터를 추가한다.
- 최소 구성 권장:
  - `USceneComponent` root
  - 시각화를 위한 `UStaticMeshComponent`
  - `UFocusTargetComponent`
  - `UPickupFocusActionComponent`
- `AWorldItemPickup` 는 최소 아래 데이터를 가진다.
  - `UItemDefinition* ItemDefinition`
  - `int32 Quantity`
- `ItemDefinition` 의 `WorldMesh` 를 월드 메시 표시 기본값으로 사용할 수 있게 구성한다.
- `Quantity <= 0` 이면 유효하지 않은 pickup으로 간주한다.

### 6. Pickup 전용 Focus Action

- `UPickupFocusActionComponent` 는 `UFocusActionComponent` 파생으로 구현한다.
- focus confirm 시 아래 순서로 동작한다.
  - owner `AWorldItemPickup` 유효성 확인
  - 상호작용한 캐릭터/소유 캐릭터 확인
  - 캐릭터의 `UBeekeeperHotbarComponent` 획득 API 호출
  - 성공 시 pickup 수량 감소
  - 수량이 0이 되면 월드 pickup 액터 제거
  - 일부만 획득되면 남은 수량 유지
  - 실패 시 아무것도 제거하지 않음
- 이 액션은 앵커 이동형 상호작용이 아니므로 `UAnchoredFocusActionComponent` 를 재사용하지 말고 `UFocusActionComponent` 직접 파생으로 구현한다.

### 7. 피드백 정책

- 획득 실패 또는 부분 성공 피드백은 최소 구현으로 처리한다.
- HUD delegate, UI 위젯, 사운드 시스템은 추가하지 않는다.
- 최소 아래 중 하나 이상으로 남긴다.
  - `UE_LOG`
  - `GEngine->AddOnScreenDebugMessage`

### 8. Hotbar 기존 규칙 유지

- `UBeekeeperHotbarComponent` 의 기존 책임은 유지한다.
  - 슬롯 선택/해제
  - focus 기반 필터링
  - presentation mode 계산
  - `OnHotbarChanged` 브로드캐스트
- 획득 기능 추가로 기존 focus 필터링 로직을 깨지 않는다.
- 획득 후 슬롯 상태가 바뀌면 기존과 동일하게 `OnHotbarChanged` 를 방송한다.

### 9. Item 구조와의 정합성 유지

- `UItemDefinition`, `UItemInstance`, `UItemAction` 의 기존 구조를 깨지 않는다.
- 새로 생성되는 `UItemInstance` 는 반드시 `InitializeFromDefinition()` 경로를 사용한다.
- action 객체의 outer 는 계속 `UItemInstance` 여야 한다.
- hotbar는 concrete class 고정 없이 `IHotbarItemInterface` 기반 처리 구조를 유지한다.

## 구현 원칙

- 기존 `Public` / `Private` 폴더 구조를 유지한다.
- Actor는 상태 중심, 기능은 Component 분리 원칙을 유지한다.
- 획득 로직은 hotbar 컴포넌트와 pickup action 컴포넌트로 분리한다.
- focus 구조와 충돌하지 않도록 재사용 가능한 `UFocusActionComponent` 흐름 위에 구현한다.
- 추후 인벤토리 시스템이 생기더라도 현재 hotbar 직접 저장 구조를 쉽게 교체할 수 있게, 획득 API를 컴포넌트 경계 안에 모은다.
- Tick은 추가하지 않는다. 획득은 confirm 시점 이벤트 기반으로만 처리한다.

## 세부 기능 설명

### 1. `UBeekeeperHotbarComponent`

- 기존 슬롯 관리 기능 위에 획득/병합/자동 배치 기능을 추가한다.
- 추천 공개 함수 예시
  - `TryAcquireItem(UItemDefinition* ItemDefinition, int32 Quantity)`
- 추천 내부 보조 함수 예시
  - 동일 `ItemDefinition` stack 탐색
  - 빈 슬롯 탐색
  - 새 `UItemInstance` 생성
  - stack 가능 수량 계산
- 일부만 획득된 경우에도 hotbar 변경 사항은 즉시 반영한다.

### 2. `AWorldItemPickup`

- 월드에 놓여 있는 획득 가능 아이템 표현용 액터다.
- focus prompt 텍스트는 `ItemDefinition` 의 `DisplayName` 과 현재 `Quantity` 를 반영해 구성할 수 있게 한다.
- pickup 수량이 줄면 prompt/표시도 갱신 가능하도록 구조를 잡는다.

### 3. `UPickupFocusActionComponent`

- `CanBeginFocusAction()` 에서 pickup 유효성, 수량, 아이템 정의 존재 여부를 검사한다.
- `BeginFocusAction()` 에서 즉시 획득 시도를 수행한다.
- 성공/실패 후 별도 engaged 상태를 오래 유지하지 않는 단발성 상호작용으로 구현한다.
- 필요 시 action 완료 후 `AbortFocusAction()` 또는 적절한 종료 흐름으로 focus 상태를 정리한다.

### 4. world pickup 수량 처리

- pickup 액터는 다중 수량을 가질 수 있다.
- 일부만 획득된 경우 `Quantity` 를 남은 수량으로 갱신한다.
- 전부 획득된 경우 액터를 제거한다.

## Unreal 관련 제약 조건

- `UItemInstance`, `UItemAction`, `UFocusActionComponent` 파생은 모두 UObject lifecycle과 GC를 고려한다.
- 새로 만든 `UItemInstance` 의 outer 는 hotbar owner 또는 명확한 소유 주체 아래에서 안전하게 유지되도록 설계한다.
  - 최소한 GC로 사라지지 않도록 hotbar 슬롯 참조와 outer 관계를 함께 고려한다.
- `AWorldItemPickup` 는 Blueprint 배치와 확장이 가능하도록 `BlueprintType` / `Blueprintable` 노출을 적절히 고려한다.
- Blueprint는 표현/배치 확장 용도로 두고, 핵심 획득 로직은 C++ 에 둔다.

## 출력 요구사항

- 작업 완료 후 아래 형식으로 보고한다.
  - 수정/생성한 파일 목록
  - `UBeekeeperHotbarComponent` 에 추가된 획득 API 요약
  - stack 병합 및 자동 배치 규칙 요약
  - `AWorldItemPickup` / `UPickupFocusActionComponent` 역할 요약
  - 부분 획득/획득 실패 처리 방식 요약
- `Source/ARCHITECTURE.md` 에는 이번 작업으로 실제 변경된 구조만 반영한다.
- 외부 엔진 코드, 테스트 코드, 불필요한 리팩터링은 포함하지 않는다.
