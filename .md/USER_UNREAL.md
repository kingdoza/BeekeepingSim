# Cursor Part Focus - Unreal Editor 작업 목록

## AVfxItemPresentationActor 사용 절차 (2026-05-26)

1. `AVfxItemPresentationActor` 기반 BP를 생성한다. (예: `BP_DisinfectantVfxPresentation`)
2. BP의 `UseVfxComponent`에서 Niagara System Asset을 지정한다.
3. `UseVfxComponent`의 상대 위치/회전/스케일을 아이템 노즐 위치에 맞춘다.
4. `UseVfxComponent`의 `Auto Activate`는 false로 유지한다.
5. 필요 시 actor 옵션:
   - `bResetVfxOnStart`
   - `bDeactivateImmediatelyOnEnd`
6. 아이템 정의의 held presentation actor class를 해당 BP로 지정한다.
7. 소독약 hold-use 시작/종료 시 VFX가 자동으로 시작/정지되는지 PIE에서 확인한다.

## Held Disinfectant VFX Hookup (2026-05-26)

- 소독약 held presentation BP의 부모를 `AItemPresentationActor` 기반으로 둔다.
- BP에 분사 VFX/오디오 컴포넌트(예: Niagara, Audio)를 추가한다.
- `ReceiveItemUseActiveStarted`에서 VFX 시작, `ReceiveItemUseActiveEnded(bool bCanceled)`에서 VFX 정지를 구현한다.
- 소독약 아이템 정의에서 held presentation actor class를 위 BP로 지정한다.
- 소독약 action class는 `UDisinfectantUseAction`(또는 서브클래스)로 유지한다.
- 런타임 경로:
  - `UDisinfectantUseAction` -> `UBeekeeperHeldItemVisualizerComponent` -> 현재 `AItemPresentationActor`

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
  - 화면 외곽 취소 영역 두께는 컴포넌트별 값이 아니라 공통 설정값을 사용한다.
  - `Project Settings > Beekeeping Sim Focus > Cursor Part Focus > ScreenEdgeCancelRegionThickness`에서 조정한다.

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
  - 기본적으로 `Esc`와 동일한 cancel 우선순위로 동작한다.
  - 단, 유효한 PartFocus hover target 위에서 클릭한 경우에는 edge cancel보다 target click 처리가 우선한다.

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

## 9. FocusEngaged Item Use Area 설정

- `BP_Beehive`에서 `ItemUseAreaScope` 컴포넌트가 존재하는지 확인한다.
- `PartFocusClickAction`(LMB)에 `Started`와 `Completed`가 모두 연결되어 있는지 확인한다.
  - `Started`: item-use press 또는 part-focus click
  - `Completed`: item-use release
- Gameplay Tag 등록:
  - `Beehive.UseArea.Lid`
  - `Beehive.UseArea.Comb`
- item-use area 시각 표현용 머티리얼 파라미터를 사용영역 mesh(material index 0)에 준비한다.
  - `UseAreaColor`
  - `UseAreaOpacity`
  - `PulseSpeed`
  - `HoverStrength`
- 권장 authoring:
  - 실제 gameplay mesh는 hit 용도
  - 반투명 가상 mesh는 visual 용도
  - 둘 다 어려우면 현재 mesh를 hit+visual로 임시 사용 가능

## 10. FocusEngaged Item Use Area PIE 검증

- item-use-area 미지원 host에서는 기존 PartFocus 동작이 유지되어야 한다.
- 벌통 FocusEngaged + 빈손: 기존 PartFocus click 동작 유지
- 벌통 FocusEngaged + hold-use action 아이템 선택:
  - 대응 영역이 LMB와 무관하게 표시/점멸
  - LMB hold 중 영역 밖: `BeginUse/TickUse`만, 실질 효과 없음
  - LMB hold 중 영역 위: `ApplyUseEffect`가 Tick 기반 호출
  - LMB release/cancel/abort 시 session이 종료되어야 함
- 선택 아이템이 있을 때 벌통 PartFocus hover outline/prompt가 숨겨지는지 확인한다.

## 11. Generic Item Placement Slot Authoring (2026-05-26)

- `BP_ItemPlacementSlotActor`를 `AItemPlacementSlotActor` 기반으로 만든다.
  - 기본 구성: `Root`, `SlotMeshComponent`, `AttachComponent`
  - `SlotMeshComponent`가 hit 영역 + use-area visual 표시를 동시에 담당한다.
- 슬롯 설정:
  - `AreaId` (예: `PollenPattySlot_0`)
  - `AreaTags`에 `Item.UseArea.Beehive.PollenPatty`
  - `SlotMeshAsset`으로 slot 인스턴스별 mesh 지정 가능
  - `SlotMeshMaterial`로 item-use-area 표시 material 지정 가능
  - `SlotMeshRelativeTransform`으로 hit/visual mesh의 local 위치/회전/스케일 조정
  - `AttachRelativeTransform`으로 placed actor 부착 위치/회전/스케일 조정
  - `AttachSocketName`이 필요하면 socket 이름 지정
- `BP_Beehive`에서 화분떡 위치마다 `ChildActorComponent`를 추가하고 class를 `BP_ItemPlacementSlotActor`로 지정한다.
- child actor 인스턴스 transform으로 슬롯 actor 자체 위치를 배치하고, `SlotMeshAsset`/`SlotMeshRelativeTransform`으로 표시/판정 mesh 모양과 local 위치를 조정한다.
- `AttachRelativeTransform`으로 화분떡 actor가 실제로 붙을 local attach point를 조정한다.
- `BP_Beehive`에는 native `ItemUseAreaMeshProvider`가 기본으로 존재한다.
- slot용 `ChildActorComponent`에 필요한 경우 `Component Tags`를 추가하고, `ItemUseAreaMeshProvider.RequiredChildActorComponentTag`로 필터링한다.
- C++ scope는 actor-level descriptor override를 호출하지 않고 `ItemUseAreaMeshProvider`가 수집한 `UItemUseAreaMeshComponent`만 등록한다.
- 화분떡 아이템 action:
  - `UItemPlacementUseAction` 또는 `UPollenPattyUseAction` 사용
  - `UseAreaTagQuery`는 `Item.UseArea.Beehive.PollenPatty` 매칭
  - `PlacedActorClass`는 `APollenPattyActor` 기반 BP 지정
- occupied 상태(`PlacedActor` 유효)가 되면 슬롯 actor가 descriptor를 반환하지 않으므로 use-area가 자동 비활성화된다.

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
## DynamicSky / GameTimeOfDay (2026-05-24)

- Level placement:
  - Place one `AGameTimeOfDayActor` in the level.
  - Place one `ADynamicSky` in the level.
- `ADynamicSky` asset wiring:
  - Assign `SkySphereMeshAsset` and `SkySphereMaterial`.
  - Set `SunLightRayleighScattering` and `NoSunLightRayleighScattering` colors.
  - Set `SunLightMultiScattering` and `NoSunLightMultiScattering` float values.
  - Curve assets are no longer used for DynamicSky Rayleigh/MultiScattering.
  - Verify `SunDirectionalLight` uses Atmosphere Sun Light Index `0`.
  - Verify `MoonDirectionalLight` uses Atmosphere Sun Light Index `1`.
  - Sun/Moon directional light component visibility is not toggled by DynamicSky at runtime.
- Start time from preview:
  - Enable `bUseEditorPreviewTime`.
  - Set `PreviewHour24`.
  - Keep `bStartGameTimeFromPreviewHour` enabled to start PIE from the DynamicSky preview hour.
- Transition cleanup:
  - Existing `AEnvironmentTimeOfDayActor` should not be used as the primary sky visual actor for new levels.
- Validation checklist:
  - Toggle `bUseEditorPreviewTime` on `ADynamicSky`, change `PreviewHour24`, verify immediate visual updates.
  - In PIE, verify clock widget updates from provider time.
  - Verify bucket-driven world gameplay still reacts on schedule.
- Blueprint compile/save:
  - Recompile/save any level Blueprint or actor Blueprint that references old time actor wiring if warnings appear.

## Beekeeper Flashlight Toggle (2026-05-24)

- Input assets:
  - Create `IA_FlashlightToggle` input action.
  - Add it to the player mapping context.
  - Map key `T` to `IA_FlashlightToggle`.
- Character blueprint wiring:
  - Assign `IA_FlashlightToggle` to `BP_BeekeeperCharacter.FlashlightToggleAction`.
- Flashlight tuning:
  - In `BeekeeperFlashlight` component details, tune:
    - `Intensity`
    - `AttenuationRadius`
    - `InnerConeAngle`
    - `OuterConeAngle`
    - `bCastShadows`
    - `RelativeLocation` / `RelativeRotation`
- Validation:
  - Press `T` to toggle On/Off in PIE.
  - Verify flashlight direction follows first-person camera rotation.
  - Verify toggle still works while focus interaction input is locked.
- Save/compile:
  - Compile/save `BP_BeekeeperCharacter` and related input assets.

## Placed Item Retrieve + Focus Secondary Input (2026-05-27)

1. 입력 액션/매핑
- `IA_FocusSecondaryAction` input action asset을 생성한다.
- Player 입력 매핑 컨텍스트에 `IA_FocusSecondaryAction`을 추가하고 `RMB`에 매핑한다.
- `BP_BeekeeperCharacter`에서 `FocusSecondaryAction` 프로퍼티에 `IA_FocusSecondaryAction`을 할당한다.

2. Generic placed item BP 생성
- `APlacedItemActor` 기반 BP를 생성한다. (예: `BP_PlacedItem_Generic`)
- 필요 시 `ReceivePlacedItemInitialized`에서 추가 시각 설정을 구현한다.

3. 화분떡 배치 class 교체
- 화분떡 placement action(`UItemPlacementUseAction`/`UPollenPattyUseAction`)의 `PlacedActorClass`를 `APlacedItemActor` 기반 BP로 지정한다.
- 기존 `APollenPattyActor` native class rename은 하지 않는다.

4. PIE 검증
- 배치 성공 시 hotbar stack이 1 감소하고 slot이 occupied 상태가 되는지 확인한다.
- 배치된 actor hover 중 RMB 입력 시 hotbar에 아이템 1개가 추가되고 actor/slot 점유가 해제되는지 확인한다.
- hotbar 공간이 없을 때 RMB 회수가 실패하고 placed actor/slot 점유가 유지되는지 확인한다.

## PartFocus Provider 기반 Placed Item Retrieve (2026-05-27)

1. 벌통 child slot 수집 태그
- 벌통의 placement slot용 `ChildActorComponent`에 `ItemUseAreaChild` 태그를 유지한다.
- 같은 component에 `PartFocusChild` 태그를 추가한다. (child part provider 수집용)

2. Placed item BP authoring
- `APlacedItemActor` 기반 BP를 생성한다.
- 기본 mesh authoring은 BP에서 가능하며, item definition의 `WorldMesh`가 있으면 runtime 초기화 시 해당 mesh로 override된다.
- 필요 시 `ReceivePlacedItemInitialized` 이벤트로 추가 표시 연출을 구현한다.

3. 입력 매핑
- `IA_FocusSecondaryAction`을 생성해 `RMB`에 매핑한다.
- `BP_BeekeeperCharacter.FocusSecondaryAction`에 `IA_FocusSecondaryAction`을 할당한다.

4. 검증 포인트
- empty slot: item-use-area 표시가 보이고 PartFocus target은 없어야 한다.
- occupied slot: item-use-area 표시는 사라지고 placed item이 PartFocus hover/outline 대상이어야 한다.
- placed item hover + RMB: hotbar 1개 반환 + slot empty 복귀
- hotbar 공간 없음: 회수 실패 + placed actor/slot 유지

## Hotbar Middle Click Selection Toggle (2026-05-27)

1. 입력 액션/매핑
- `IA_HotbarToggleSelection` input action asset을 생성한다.
- Value Type은 Digital(Boolean) 계열로 설정한다.
- Player 입력 매핑 컨텍스트에 `IA_HotbarToggleSelection`을 추가하고 `Middle Mouse Button`(Wheel Click)에 매핑한다.

2. 캐릭터 BP 할당
- `BP_BeekeeperCharacter`에서 `HotbarToggleSelectionAction` 프로퍼티에 `IA_HotbarToggleSelection`을 할당한다.

3. PIE 검증
- 슬롯 선택 상태에서 middle click 시 전체 미선택으로 전환되는지 확인한다.
- 미선택 상태에서 middle click 시 마지막 선택 슬롯(기본 1번 슬롯)이 재선택되는지 확인한다.
- FocusEngaged에서 hotbar slot input block 상태일 때 middle click이 무시되는지 확인한다.

## BeeBrush Lifted Comb ItemUseArea (2026-05-27)

1. Gameplay Tag 확인
- `Project Settings > Gameplay Tags`에서 `Item.UseArea.Beehive.BeeBrush`가 등록되어 있는지 확인한다.

2. `BP_BeehiveComb` 설정
- `BeeBrushUseAreaMesh` 컴포넌트(`UItemUseAreaMeshComponent`)에 소비장 전체를 덮는 mesh를 지정한다.
- `BeeBrushUseAreaMesh`에 item-use-area 표시 material을 지정한다.
- 소비장 전체를 커버하도록 상대 위치/회전/스케일을 조정한다.
- `BeeBrushUseAreaMesh.AreaTags`에 `Item.UseArea.Beehive.BeeBrush`를 추가한다.
- `BeeBrushUseAreaMesh.EffectTargetPolicy`는 `ComponentOwner`로 유지한다.
- 필요하면 `BeeBrushUseAreaMesh.VisualSettings`에서 표시 색상/opacity/pulse/hover strength를 조정한다.

3. material parameter 확인
- 아래 파라미터를 material에서 사용하도록 구성한다.
  - `UseAreaColor`
  - `UseAreaOpacity`
  - `PulseSpeed`
  - `HoverStrength`

4. BeeBrush item definition 설정
- BeeBrush item definition ActionSpec에 `UBeeBrushUseAction`을 추가한다.
- 필요하면 held presentation actor BP를 별도로 지정한다.

5. PIE 검증
- BeeBrush 선택 전/또는 comb 미-lift 상태: BeeBrush use-area 표시 없음
- comb lift 상태 + BeeBrush 선택: lifted comb의 `BeeBrushUseAreaMesh`만 표시
- hover + LMB hold: 해당 comb의 `TargetBeeCount`만 감소
- `ColonyBeeCount`/hotbar stack은 감소하지 않아야 함
- comb return/cancel/abort 시 BeeBrush use-area가 즉시 사라져야 함

## ItemUseAreaMeshComponent 전환 체크리스트 (2026-05-27)

1. `BP_Beehive`
- `ItemUseAreaMeshProvider` 컴포넌트 존재 확인
- child actor 필터가 필요하면 `RequiredChildActorComponentTag` 설정

2. placement slot BP
- `SlotMeshComponent` 타입이 `UItemUseAreaMeshComponent`인지 확인
- `AreaId/AreaTags/VisualSettings/EffectTargetPolicy`는 actor 프로퍼티가 아니라 `SlotMeshComponent` details에서 설정
- `EffectTargetPolicy=ComponentOwner` 유지

3. deprecated 경로 주의
- `Get Item Use Area Descriptors` BP override는 새 runtime 경로에서 사용하지 않는다.
- `Component Tags = ItemUseArea` fallback은 사용하지 않는다.

## Generic Placement Occupant + Beehive Comb Slot 전환 (2026-05-28)

1. `BP_Beehive` / 벌통 BP
- comb rack child actor component의 class를 `ABeehiveCombSlotActor` 기반 BP로 설정한다. (예: `BP_BeehiveCombSlot`)
- 기존 comb actor 직접 child actor 방식은 사용하지 않는다.
- 필요 시 `ABeehive.CombSlotActorClass`를 slot BP class로 지정한다.
- `InitialCombCount`로 초기 배치할 소비장 수를 지정한다. 이 값은 `0..MaxCombCount`로 제한되며 PIE/런타임 중 details에서 수정하지 않는다.

2. `BP_BeehiveCombSlot`
- `SlotMeshComponent` mesh/material/transform을 comb item 배치 위치에 맞게 설정한다.
- `SlotMeshComponent.AreaTags`를 comb placement item의 `UseAreaTagQuery`와 일치시킨다.
- preplaced comb를 사용할 경우:
  - `InitialOccupantActor` 지정
  - `bAttachInitialOccupantToSlot`, `bSnapInitialOccupantToAttachPoint` 옵션 확인

3. `BP_BeehiveComb`
- `PlacementOccupant` 컴포넌트(실제 class: `UBeehiveCombPlacementOccupantComponent`) 존재를 확인한다.
- `PlacementRetrieveAction` 컴포넌트 존재를 확인한다.
- secondary retrieve는 comb part action bridge로 처리되므로 descriptor action handler를 기존 comb part action으로 유지한다.
- authored fallback 회수가 필요하면 `PlacementOccupant.AuthoredReturnItemDefinition`을 설정한다.

4. 소비장 아이템 정의(`UItemDefinition`)
- comb 배치 action(`UItemPlacementUseAction`)의 `PlacedActorClass`를 `ABeehiveCombActor` 기반 BP로 설정한다.
- `UseAreaTagQuery`가 comb slot area tags와 매칭되는지 확인한다.
- comb item은 상태 보존 계약을 위해 `MaxStack=1`을 필수로 유지한다. (`MaxStack>1`이면 회수 차단)

5. 기존 placed item BP
- `APlacedItemActor` 기반 BP에 `PlacementOccupant` + generic retrieve component가 붙어 있는지 확인한다.
- 기존 `UPlacedItemRetrievePartFocusActionComponent`는 wrapper(deprecated 경로)로 남아 있으므로 신규 경로는 generic component 기준으로 확인한다.

6. PIE 검증 체크리스트
- generic slot:
  - empty slot 배치 성공 / occupied 재배치 차단
  - secondary 회수 성공 시 hotbar +1, slot clear
  - hotbar 공간 부족 시 회수 실패, actor/slot 유지
- preplaced slot:
  - `InitialOccupantActor` claim 성공
  - `AuthoredReturnItemDefinition` fallback 회수 성공
- comb slot:
  - comb item 배치 성공
  - LMB lift/return 기존 동작 유지
  - secondary retrieve:
    - `TargetBeeCount == 0` + queen 미부착 -> 회수 성공
    - `TargetBeeCount > 0` 또는 queen 부착 -> 회수 실패, slot/actor 유지
  - 회수 후 재배치 시 꿀 양/visible face가 item state를 통해 복원되는지 확인
