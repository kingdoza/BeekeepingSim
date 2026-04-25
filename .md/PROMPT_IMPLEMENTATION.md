# 구현 프롬프트: ItemPresentation 커서 정렬 방식 개선

## 작업 목표

`EngagedFocus` 상태에서 `AItemPresentationActor` 가 화면 커서에 더 정확히 붙어 보이도록 `UBeekeeperHeldItemVisualizerComponent::UpdateCursorPresentation()` 의 위치 계산 방식을 변경한다.

현재 방식은 viewport 좌표를 `-1.0 ~ 1.0` 로 정규화한 뒤 카메라 local `Y/Z` offset 에 단순 반영한다. 이 방식은 FOV, aspect ratio, 카메라 블렌드 상태, actor pivot 차이에 따라 실제 화면 커서와 presentation actor 가 어긋날 수 있다.

이번 작업에서는 화면 커서 위치를 월드 ray 로 deproject 한 뒤, 카메라 앞 고정 거리 평면과 교차시켜 actor 를 배치한다. 또한 현재 공통으로 쓰는 `MeshRelativeScale` 을 `InHand` 와 `OnCursor` 모드별 scale 로 분리한다.

## 대상 파일 목록

수정 대상:

- `Source/BeekeepingSim/Public/BeekeeperHeldItemVisualizerComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`
- `.md/0_ARCHITECTURE.md`

필요 시 확인 대상:

- `Source/BeekeepingSim/Public/ItemPresentationActor.h`
- `Source/BeekeepingSim/Private/ItemPresentationActor.cpp`
- `Source/BeekeepingSim/Public/BeekeeperHotbarComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHotbarComponent.cpp`

## 구현 요구사항

1. `UBeekeeperHeldItemVisualizerComponent` 의 scale 설정을 모드별로 분리한다.
   - 기존 `MeshRelativeScale` 단일 값을 더 이상 핵심 경로에서 공통 사용하지 않는다.
   - 신규 property 를 추가한다.
     - `InHandRelativeScale`
     - `OnCursorRelativeScale`
   - 기본값은 둘 다 기존 `MeshRelativeScale` 과 같은 `FVector(1.0f, 1.0f, 1.0f)` 로 둔다.
   - 기존 asset/Blueprint 호환을 고려해 `MeshRelativeScale` 제거가 위험하면 deprecated 용도로 남기고, 새 값들이 실제 적용 경로에서 사용되게 한다.

2. `InHand` 표시에서는 `InHandRelativeScale` 을 적용한다.
   - `EHotbarPresentationMode::InHand` 일 때:
     - relative location: `InHandLocalOffset`
     - relative rotation: `InHandLocalRotation`
     - relative scale: `InHandRelativeScale`

3. `OnCursor` 표시에서는 `OnCursorRelativeScale` 을 적용한다.
   - `EHotbarPresentationMode::OnCursor` 일 때:
     - cursor 위치 계산 후 actor 위치 갱신
     - rotation: `OnCursorLocalRotation` 기준
     - scale: `OnCursorRelativeScale`

4. `UpdateCursorPresentation()` 의 위치 계산 방식을 deprojection 기반으로 변경한다.
   - `APlayerController::GetMousePosition()` 으로 cursor screen 좌표를 얻는다.
   - `APlayerController::DeprojectScreenPositionToWorld()` 로 cursor ray 를 얻는다.
   - `OwnerCamera` 의 위치와 forward vector 를 기준으로 카메라 앞 평면을 만든다.
   - cursor ray 와 카메라 앞 평면의 교차점을 계산한다.
   - `HeldPresentationActor` 를 해당 world location 으로 이동시킨다.

5. cursor 평면 거리를 설정 가능한 property 로 둔다.
   - 신규 property 예시:
     - `OnCursorPlaneDistance`
   - 기본값은 기존 `OnCursorBaseLocalOffset.X` 와 같은 의미의 `55.0f` 로 둔다.
   - `OnCursorBaseLocalOffset` 은 기존 asset 호환 때문에 제거하지 않는다.
   - 새 방식에서 `OnCursorBaseLocalOffset.X` 는 핵심 거리값으로 직접 사용하지 않고, 필요 시 migration/fallback 참고값 정도로만 둔다.

6. cursor 위치에 추가 보정 offset 을 유지한다.
   - deproject 교차점만 사용하면 actor pivot 이 cursor 에 바로 붙는다.
   - 기존처럼 아이템을 cursor 에서 약간 띄우거나 아래/옆으로 보정할 수 있도록 local offset property 를 둔다.
   - 기존 `OnCursorBaseLocalOffset.Y/Z`, `CursorHorizontalOffsetRange`, `CursorVerticalOffsetRange` 는 더 이상 정규화 커서 범위 계산에 사용하지 않는다.
   - 신규 property 예시:
     - `OnCursorLocalOffset`
   - 이 offset 은 카메라 local space 기준으로 교차점에 더한다.
   - 기본값은 `FVector::ZeroVector` 또는 기존 감각을 유지하려면 `FVector(0.0f, 0.0f, -8.0f)` 로 둔다. 코드 변경 전 실제 값 선택을 명확히 주석 또는 property 이름으로 드러낸다.

7. 회전 적용 방식을 명확히 한다.
   - actor 가 카메라에 attach 된 상태라면 world location 을 직접 설정하더라도 relative rotation/scale 과 충돌하지 않도록 처리한다.
   - 추천 방식:
     - actor 는 계속 `OwnerCamera` 에 attach 한다.
     - deproject 로 얻은 world location 을 `OwnerCamera` local space 로 변환해 `SetActorRelativeLocation()` 에 넣는다.
     - rotation 은 기존처럼 `SetActorRelativeRotation(OnCursorLocalRotation)` 을 사용한다.
   - world transform 직접 적용 방식을 선택할 경우 attach 상태와 relative transform 갱신 방식이 꼬이지 않도록 한 가지 기준만 사용한다.

## 권장 구현 상세

`UpdateCursorPresentation()` 의 권장 흐름:

```cpp
float CursorX = 0.0f;
float CursorY = 0.0f;
if (!PlayerController->GetMousePosition(CursorX, CursorY))
{
    return;
}

FVector RayOrigin;
FVector RayDirection;
if (!PlayerController->DeprojectScreenPositionToWorld(CursorX, CursorY, RayOrigin, RayDirection))
{
    return;
}

const FVector CameraLocation = OwnerCamera->GetComponentLocation();
const FVector CameraForward = OwnerCamera->GetForwardVector();
const FVector PlanePoint = CameraLocation + CameraForward * OnCursorPlaneDistance;

const FVector TargetWorldLocation = FMath::LinePlaneIntersection(
    RayOrigin,
    RayOrigin + RayDirection * 100000.0f,
    PlanePoint,
    CameraForward);

const FVector WorldOffset =
    OwnerCamera->GetRightVector() * OnCursorLocalOffset.Y
    + OwnerCamera->GetUpVector() * OnCursorLocalOffset.Z
    + OwnerCamera->GetForwardVector() * OnCursorLocalOffset.X;

const FVector FinalWorldLocation = TargetWorldLocation + WorldOffset;
const FVector RelativeLocation = OwnerCamera->GetComponentTransform().InverseTransformPosition(FinalWorldLocation);

HeldPresentationActor->SetActorRelativeLocation(RelativeLocation);
HeldPresentationActor->SetActorRelativeRotation(OnCursorLocalRotation);
HeldPresentationActor->SetActorRelativeScale3D(OnCursorRelativeScale);
```

주의:

- 위 코드는 방향성 예시다. 실제 프로젝트 스타일과 include 상태에 맞춰 작성한다.
- line-plane 교차가 비정상인 경우를 방어한다.
- `OnCursorPlaneDistance` 는 0 이하가 되지 않도록 clamp metadata 를 둔다.

## 구현 원칙

- 이번 작업은 `UBeekeeperHeldItemVisualizerComponent` 의 presentation 위치/scale 계산만 다룬다.
- `UBeekeeperHotbarComponent` 의 선택/필터링/표시 모드 정책은 변경하지 않는다.
- `UBeekeeperFocusComponent` 의 engaged focus 흐름은 변경하지 않는다.
- `AItemPresentationActor` 의 책임은 확장하지 않는다.
- `ItemDefinition`, `ItemInstance`, pickup 획득 흐름은 변경하지 않는다.
- Blueprint asset 호환을 고려해 기존 property 제거는 신중히 한다.
- 기존 Public/Private 구조와 Unreal coding style 을 유지한다.

## 문서 반영 요구사항

구현 완료 후 `.md/0_ARCHITECTURE.md` 의 변경된 부분만 갱신한다.

반영할 내용:

- `UBeekeeperHeldItemVisualizerComponent` 의 `OnCursor` 위치 계산이 viewport 정규화 offset 방식이 아니라 deproject cursor ray + camera plane 교차 방식임을 명시한다.
- `InHandRelativeScale`, `OnCursorRelativeScale` 로 모드별 scale 이 분리되었음을 명시한다.
- 기존 `OnCursorBaseLocalOffset`, `CursorHorizontalOffsetRange`, `CursorVerticalOffsetRange` 를 유지하거나 deprecated 처리했다면 현재 역할을 문서에 정확히 적는다.

## 검증 요구사항

- 가능하면 Unreal Build Tool 로 C++ 빌드를 시도한다.
- 빌드가 불가능하면 최소한 변경 파일의 include, forward declaration, UPROPERTY metadata, compile 가능성을 정적으로 점검한다.
- 직접 실행 검증이 가능하다면 `EngagedFocus` 상태에서 cursor 와 presentation actor 의 화면상 위치가 기존보다 정확히 맞는지 확인한다.

## 출력 요구사항

작업 완료 보고는 아래 형식을 따른다.

```
[상태] 완료
[요약] 커서 정렬 방식과 scale 분리 변경 요약
[변경 파일] 수정한 파일 목록
[검증] 빌드/정적 확인/실행 확인 결과
[주의] Blueprint asset 에서 새 scale/offset 값을 조정해야 하는 항목
```
