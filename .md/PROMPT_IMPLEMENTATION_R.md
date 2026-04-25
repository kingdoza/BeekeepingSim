# 코드 개선 요청 프롬프트

아래 리뷰 결과를 기준으로 Unreal C++ 코드를 수정하라.

## 대상 파일

- `Source/BeekeepingSim/Public/BeekeeperHeldItemVisualizerComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`

## 전제

- 삭제된 UPROPERTY(`OnCursorBaseLocalOffset`, `CursorHorizontalOffsetRange`, `CursorVerticalOffsetRange`, `MeshRelativeScale`)를 참조하는 기존 BP 데이터는 migration 대상이 아니라 제거 대상이다.
- 따라서 deprecated property 유지나 `PostLoad()` migration은 수행하지 않는다.

## 치명적 문제

없음.

## 중요 문제 1: `OnCursorPlaneDistance <= 0` 방어가 실제로는 부족함

### 문제
`UpdateCursorPresentation()`에서 `OnCursorPlaneDistance`를 `FMath::Max(0.0f, OnCursorPlaneDistance)`로만 보정한다. 값이 0이면 평면이 카메라 위치에 생성되어 presentation actor가 카메라 원점 또는 near plane 쪽으로 붙을 수 있다.

### 영향
디자이너가 값을 0으로 설정하거나 기본값이 잘못 들어간 경우, cursor presentation이 화면에 보이지 않거나 카메라 클리핑/깜빡임이 발생할 수 있다.

### 수정 방향
`OnCursorPlaneDistance`는 0이 아니라 최소 양수 거리로 clamp해야 한다. `ClampMin` metadata도 0보다 큰 값으로 바꾸고, 런타임에서도 최소값을 적용한다.

### 수정 코드
파일 경로: `Source/BeekeepingSim/Public/BeekeeperHeldItemVisualizerComponent.h`

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Held Item", meta = (ClampMin = "1.0", UIMin = "1.0"))
float OnCursorPlaneDistance = 55.0f;
```

파일 경로: `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`

```cpp
constexpr float MinCursorPlaneDistance = 1.0f;
const float PlaneDistance = FMath::Max(MinCursorPlaneDistance, OnCursorPlaneDistance);
const FVector PlanePoint = CameraLocation + CameraForward * PlaneDistance;
```

## 중요 문제 2: ray-plane 교차가 ray의 뒤쪽 평면도 허용할 수 있음

### 문제
`DirectionDot`을 `IsNearlyZero()`로만 검사한다. dot이 음수이면 deproject ray 방향 기준 뒤쪽 평면과도 교차 계산이 진행된다.

### 영향
일시적인 카메라/view mismatch, camera blend, split view 또는 비정상 deproject 상황에서 actor가 카메라 뒤쪽 상대 위치로 튈 수 있다.

### 수정 방향
카메라 전방 평면을 쓰는 정책이므로 `DirectionDot`은 양수 임계값 이상일 때만 교차를 허용한다. 또한 `LinePlaneIntersection` 대신 ray parameter `T`를 직접 계산하면 특이 케이스를 더 명확히 방어할 수 있다.

### 수정 코드
파일 경로: `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`

```cpp
const float DirectionDot = FVector::DotProduct(SafeRayDirection, CameraForward);
constexpr float MinPlaneDot = KINDA_SMALL_NUMBER;
if (DirectionDot <= MinPlaneDot)
{
	return;
}

const float RayDistanceToPlane = FVector::DotProduct(PlanePoint - RayOrigin, CameraForward) / DirectionDot;
if (RayDistanceToPlane <= 0.0f)
{
	return;
}

const FVector TargetWorldLocation = RayOrigin + SafeRayDirection * RayDistanceToPlane;
```

## 개선 제안 1: plane 계산에 같은 camera basis 값을 재사용

### 문제
`CameraForward`를 변수로 저장해놓고 offset 계산에서 다시 `OwnerCamera->GetForwardVector()`를 호출한다.

### 영향
동일 프레임 내 차이는 거의 없지만, camera override/blend 중 basis 일관성을 명확히 하는 편이 유지보수에 좋다.

### 수정 코드
파일 경로: `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`

```cpp
const FVector CameraRight = OwnerCamera->GetRightVector();
const FVector CameraUp = OwnerCamera->GetUpVector();

const FVector WorldOffset =
	CameraForward * OnCursorLocalOffset.X +
	CameraRight * OnCursorLocalOffset.Y +
	CameraUp * OnCursorLocalOffset.Z;
```

## BP 정리 작업

- `BP_BeekeeperCharacter` 등 기존 Blueprint에서 삭제된 property 참조/serialized override는 제거한다.
- 새 값은 `OnCursorPlaneDistance`, `OnCursorLocalOffset`, `InHandRelativeScale`, `OnCursorRelativeScale` 기준으로 수동 재설정한다.
- C++에서 deleted property migration 코드는 추가하지 않는다.

## 검증 항목

- `OnCursorPlaneDistance`를 0으로 설정할 수 없거나, 런타임에서 1.0 이상으로 보정되는지 확인한다.
- 커서가 viewport 중앙/모서리에 있을 때 actor가 카메라 앞 평면 위에서 안정적으로 이동하는지 확인한다.
- EngagedFocus 카메라 blend 중 cursor presentation이 튀지 않는지 확인한다.
- 삭제된 property 참조가 BP에서 제거되었는지 확인한다.
- 새 scale 값 `InHandRelativeScale`, `OnCursorRelativeScale`이 모드별로 적용되는지 확인한다.
- `InitializePresentation` Blueprint override가 class 기반 presentation에서 호출되는지 확인한다.

## 0_ARCHITECTURE.md 반영 필요 여부

불필요. 삭제된 UPROPERTY의 BP 참조 제거는 에셋 정리 작업이며 현재 구조 문서 변경 사항은 아니다.
