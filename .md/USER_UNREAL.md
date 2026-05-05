# Cursor Part Focus - Unreal Editor 작업 목록

이 문서는 Cursor Part Focus C++ 구현 후 Unreal Editor에서 수동으로 설정/확인해야 할 항목만 정리한다.

## 1. Input 설정

- `FocusCancel` 입력에서 `F` 키를 제거한다.
- `FocusCancel`은 키보드 기준 `Esc`만 유지한다.
- `F` 키가 남아 있어야 한다면 cancel이 아니라 confirm/interact 계열 입력으로만 사용한다.
- 아래 PartFocus 전용 InputAction을 추가/할당한다.
  - `IA_PartFocusClick` -> `LMB`
  - `IA_PartFocusPreviewR` -> `R`
  - `IA_PartFocusPreviewF` -> `F`
  - `IA_PartFocusPreviewC` -> `C`
- PIE에서 확인:
  - `F`로 FocusCancel이 발생하지 않아야 한다.
  - Host FocusEngaged 상태에서 `LMB`로 PartFocus Begin/Cancel 토글이 동작해야 한다.
  - Host FocusEngaged 상태에서 `R/F/C`가 hover된 PartFocus 대상의 추가 동작으로 dispatch되어야 한다.
  - `Esc`는 PartFocusAction stack을 먼저 cancel하고, stack이 비어 있으면 Host FocusEngaged를 cancel해야 한다.

## 2. Gameplay Tag 등록

Cursor Part Focus 관계 정책에 사용할 태그를 프로젝트 Gameplay Tags에 등록한다.

- `Beehive.LidOpen`
  - 뚜껑이 열린 상태를 나타낸다.
  - 뚜껑 PartFocusAction의 `ProvidedStateTags`에 사용한다.

- `Beehive.CombLift`
  - 동시에 하나만 유지되는 소비장 들어올림 exclusive group이다.
  - 소비장 PartFocusAction의 `ExclusiveGroup`에 사용한다.

## 3. BP_Beehive 컴포넌트 설정

`BP_Beehive`에서 다음 컴포넌트/값을 설정한다.

- `CursorPartFocusScopeComponent`
  - Host 내부 파츠 Focus Scope
  - 화면 외곽 취소 영역 두께 기본값: `64px`

- 뚜껑 PartFocusAction (`UCursorPartFocusActionComponent`)
  - `EngageMode`: `PersistentAction`
  - `ProvidedStateTags`: `Beehive.LidOpen`
  - `RequiredStateTags`: 없음
  - `ExclusiveGroup`: 없음
  - BP 구현(Owner Actor 바인딩 권장):
    - `LidPartFocusAction` 컴포넌트 선택
    - Details > Events에서 `On Part Focus Begin`, `On Part Focus Cancel`, `On Part Focus Abort` 이벤트 추가
    - `On Part Focus Begin`: 뚜껑 열기 + 필요 시 `SetLidOpenForPartFocus(true)`
    - `On Part Focus Cancel`: 뚜껑 닫기 + 필요 시 `SetLidOpenForPartFocus(false)`
    - `On Part Focus Abort`: 뚜껑 닫기 보장 + 필요 시 `SetLidOpenForPartFocus(false)`

- 뚜껑 PartFocus descriptor
  - 대상 component: `LidMesh`
  - action handler: 뚜껑 PartFocusAction
  - outline 대상: `LidMesh`
  - prompt는 필요 시만 설정
  - preview key 사용이 필요하면 action component에서 `R/F/C` enable 토글 후 BP preview key 이벤트 구현
  - `Receive Part Focus ...` 이벤트는 component subclass 구현 경로다.
  - `BP_Beehive` 같은 액터 BP에서는 component delegate인 `On Part Focus ...` 이벤트 바인딩 경로를 사용한다.

## 4. BP_BeehiveComb 설정

소비장 Blueprint가 있다면 다음을 설정한다.

- 소비장 PartFocusAction (`UCursorPartFocusActionComponent`)
  - `EngageMode`: `PersistentAction`
  - `RequiredStateTags`: `Beehive.LidOpen`
  - `ExclusiveGroup`: `Beehive.CombLift`
  - BP 구현(Owner Actor 바인딩 권장):
    - 소비장 PartFocusAction 컴포넌트 선택
    - Details > Events에서 `On Part Focus Begin`, `On Part Focus Cancel`, `On Part Focus Abort` 이벤트 추가
    - `On Part Focus Begin`: 소비장 들어올리기
    - `On Part Focus Cancel`: 소비장 원래 슬롯에 삽입
    - `On Part Focus Abort`: 소비장 원상복귀 보장
  - preview key 사용이 필요하면 action component에서 `R/F/C` enable 토글 후 BP preview key 이벤트 구현

- outline 대상
  - 소비장 mesh component를 outline 대상으로 지정한다.
  - 기존 `UFocusTargetComponent`와 동일하게 CustomDepth outline이 보이도록 mesh render 설정을 확인한다.

## 5. 배치 아이템 파츠 설정

벌통 내부에 배치되는 화분떡, 말벌트랩 같은 아이템은 1차 구현에서 PreviewOnly로 설정한다.

- `EngageMode`: `PreviewOnly`
- prompt: 없음
- `LMB click`: no-op
- 필요 시 action handler를 연결하고 `R/F/C` key enable을 켜서 preview key 동작만 구현 가능
- outline 대상: 해당 아이템 mesh component

## 6. Outline 확인

PartFocus outline은 기존 `UFocusTargetComponent`와 같은 CustomDepth 기반 표현을 사용한다.

확인할 것:

- outline 대상 mesh가 `Render CustomDepth` 토글에 반응하는지 확인한다.
- stencil 값은 기본 `1`을 사용한다.
- 필요한 경우 outline 대상 component tag를 지정한다.
- `RequiredStateTags`가 만족되는 파츠에 hover할 때만 outline이 보여야 한다.
- PartFocusEngaged 상태라는 이유만으로 outline이 계속 유지되면 안 된다.

## 7. PIE 동작 검증

벌통 FocusEngaged 상태에서 다음을 확인한다.

- 뚜껑이 닫힌 상태
  - 소비장 hover outline이 나오지 않아야 한다.
  - 소비장 click이 동작하지 않아야 한다.

- 뚜껑 hover
  - `LidMesh`에 outline이 보여야 한다.

- 뚜껑 click
  - 입력: `LMB`
  - 뚜껑이 열린다.
  - Host FocusEngaged는 유지된다.

- 뚜껑이 열린 상태에서 소비장 hover
  - 소비장 outline이 보여야 한다.

- 소비장 click
  - 입력: `LMB`
  - 소비장이 들어올려진다.

- 같은 소비장 다시 click
  - 입력: `LMB`
  - 소비장이 원래 위치로 내려간다.

- 소비장 A를 들어올린 상태에서 소비장 B click
  - 입력: `LMB`
  - 소비장 A가 원래 위치로 내려간다.
  - 소비장 B가 들어올려진다.

- 소비장을 들어올린 상태에서 뚜껑 cancel
  - 소비장이 먼저 원상복귀한다.
  - 그 다음 뚜껑이 닫힌다.

- 화면 외곽 취소 영역 click
  - `Esc`와 동일한 cancel 우선순위로 동작한다.

- 배치 아이템 hover
  - outline만 표시된다.
  - prompt는 표시되지 않는다.
  - `LMB` click해도 아무 동작이 없어야 한다.
  - enabled된 `R/F/C`만 동작 가능해야 한다.

## 8. 저장/컴파일

- 관련 Blueprint compile
  - `BP_Beehive`
  - `BP_BeehiveComb`
  - 배치 아이템 Blueprint가 있다면 해당 Blueprint
- 테스트 레벨에서 저장
- 에디터 재시작 후 다시 열어 컴포넌트 참조/GameplayTag 설정이 유지되는지 확인

## Beehive Comb Delegate 위임

- `BP_Beehive`에서 `Receive Comb Part Focus Begin/Cancel/Abort` 이벤트를 구현해 comb actor별 연출(들기/내리기/강제복귀)을 처리한다.
- 이 이벤트는 `ABeehive`가 active comb로 등록한 actor에서만 들어오며, 독립 배치 `ABeehiveCombActor`는 전달되지 않는다.
- `BP_BeehiveComb`에서 Beehive를 직접 찾아 위임하는 그래프는 만들지 않는다.

## Beehive Comb Lift 설정

- `BP_Beehive`에서 `CombLiftTargetRoot` 위치를 소비장 검사/작업 기준 위치로 조정한다.
- `CombLiftComponent` 설정:
  - `CombLiftMoveDuration`: 들기/내리기 공통 보간 시간
- `CombLiftTargetRoot`가 소비장 들어올림 목표 위치와 목표 회전을 모두 결정한다.
  - `CombLiftTargetRoot` 위치 = 들어올려진 소비장 목표 위치
  - `CombLiftTargetRoot` 회전 = 들어올려진 소비장 목표 회전
- 원하는 최종 방향이 있으면 코드 보정 대신 `CombLiftTargetRoot`를 에디터에서 직접 회전시켜 맞춘다.
- 소비장 기본 이동/복귀는 C++ `UBeehiveCombLiftComponent`가 처리한다.
- `ReceiveCombPartFocusBegin/Cancel/Abort`는 추가 연출(사운드/이펙트) 용도로 사용한다.
