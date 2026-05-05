# Beehive Comb Lift TargetRoot Transform 기준 구현 수정 프롬프트

## 목표

`UBeehiveCombLiftComponent`의 소비장 들어올림 목표 transform 계산을 단순화한다.

기존에 논의했던 아래 기능은 전부 제거한다.

```text
카메라 방향을 향하는 회전 계산
CameraFacingLocalYawDegrees
AutoCameraYawQuat
CombLiftLocalRotationDelta
FacingAxis / UpAxis
FindLookAtRotation 기반 회전 보정
BaseTargetWorldRotation
```

앞으로 소비장 들어올림 목표 위치와 목표 회전은 모두 `CombLiftTargetRoot` 컴포넌트의 transform을 기준으로 한다.

```text
Lift Target Location = CombLiftTargetRoot world location
Lift Target Rotation = CombLiftTargetRoot world rotation
```

즉 디자이너/개발자가 에디터에서 `CombLiftTargetRoot`를 이동/회전시키면, 들어올려진 소비장은 그 위치와 회전을 목표로 보간된다.

## 수정 대상

구현 전 다음 파일을 확인한다.

- `Source/BeekeepingSim/Public/WorldActors/BeehiveCombLiftComponent.h`
- `Source/BeekeepingSim/Private/WorldActors/BeehiveCombLiftComponent.cpp`
- `Source/BeekeepingSim/Public/WorldActors/Beehive.h`
- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- `.md/Architecture/WorldActorsSystem.md`
- `.md/USER_UNREAL.md`

## 제거할 변수/설계

다음 변수나 설계가 있다면 제거한다.

```text
CombLiftLocalRotationDelta
CombLiftFacingAxis
CombLiftUpAxis
CameraFacingLocalYawDegrees
저장된 CameraWorldLocation 기반 회전 재계산
BaseTargetWorldRotation
```

헤더, cpp, 블루프린트 노출값, 문서에서 모두 정리한다.

## 최종 목표 transform 계산

들어올림 Begin 시점 또는 target 갱신 시 다음 방식으로 목표 transform을 계산한다.

```cpp
const FTransform TargetWorldTransform = CombLiftTargetRoot->GetComponentTransform();
const FTransform CombRackWorldTransform = CombRackRoot->GetComponentTransform();
const FTransform TargetRelativeTransform =
    TargetWorldTransform.GetRelativeTransform(CombRackWorldTransform);
```

slot 보간 목표 transform:

```text
Location = TargetRelativeTransform.GetLocation()
Rotation = TargetRelativeTransform.GetRotation()
Scale = RestRelativeTransform.GetScale3D()
```

주의:

```text
Scale은 CombLiftTargetRoot의 scale을 따르지 않는다.
소비장 slot의 기존 rest scale을 유지한다.
```

## 들어올림 동작

소비장 PartFocus Begin 또는 lift 요청 시:

```text
1. 현재 소비장이 관리 중인 slot index를 찾는다.
2. 해당 slot의 rest relative transform을 확보한다.
3. CombLiftTargetRoot의 world location과 world rotation을 가져온다.
4. 이를 CombRackRoot 기준 relative transform으로 변환한다.
5. slot ChildActorComponent를 rest transform에서 target relative transform으로 보간한다.
```

보간 시간은 기존 `CombLiftMoveDuration`을 유지한다.

## 내리기 동작

소비장 PartFocus Cancel 또는 lower 요청 시:

```text
1. lifted slot의 저장된 rest relative transform을 목표로 사용한다.
2. 현재 lifted transform에서 rest relative transform으로 보간한다.
3. 완료 후 lifted 상태를 해제한다.
```

내리기는 `CombLiftTargetRoot` 회전을 사용하지 않는다.  
원래 slot 위치와 회전으로 돌아가는 것이 기준이다.

## 다른 소비장 클릭 시

기존 정책을 유지한다.

```text
소비장 A가 lifted 상태에서 소비장 B를 PartFocus Begin하면:
1. 소비장 A는 rest transform으로 내려간다.
2. 소비장 B는 CombLiftTargetRoot transform으로 올라간다.
```

동시 보간 처리 방식은 기존 구현 정책을 따른다.

## layout refresh 후 재적용

layout refresh 후 lifted transform을 재적용할 때:

```text
1. 새 rest layout을 적용한다.
2. lifted slot이 있으면 rest relative transform을 새 slot rest transform으로 갱신한다.
3. CombLiftTargetRoot world transform을 다시 읽는다.
4. CombRackRoot 기준 relative transform으로 변환한다.
5. lifted slot에 target relative transform을 즉시 재적용한다.
```

카메라 방향, rotation delta, 저장된 camera yaw는 더 이상 사용하지 않는다.

## 기존 동작 유지

이번 수정은 들어올림 목표 회전 정책을 `CombLiftTargetRoot` 기준으로 바꾸는 것이다.

아래 정책은 유지한다.

- 소비장 actor detach 금지
- `UChildActorComponent` slot 자체 이동/회전
- `CombLiftTargetRoot` 위치를 목표 위치로 사용
- `CombLiftTargetRoot` 회전을 목표 회전으로 사용
- `CombLiftMoveDuration`으로 올리기/내리기 공통 보간
- 내리기 목표는 slot rest relative transform
- layout refresh 후 lifted transform 즉시 재적용

## Unreal Editor 문서 갱신

`.md/USER_UNREAL.md`를 갱신한다.

문서에서 제거:

```text
CombLiftLocalRotationDelta
CombLiftFacingAxis
CombLiftUpAxis
카메라 방향 회전 보정 설정법
로컬 Z yaw 보정 설명
```

문서에 추가:

```text
CombLiftTargetRoot
- 소비장 들어올림 목표 위치와 목표 회전을 모두 결정한다.
- 에디터에서 CombLiftTargetRoot를 원하는 위치로 이동하고 원하는 회전으로 돌린다.
- 소비장을 들어올리면 해당 transform으로 보간된다.
```

에디터 설정 기준:

```text
CombLiftTargetRoot의 위치 = 들어올려진 소비장이 놓일 위치
CombLiftTargetRoot의 회전 = 들어올려진 소비장이 가져야 할 회전
```

소비장 정면이 카메라를 향해야 한다면, 코드에서 계산하지 말고 `CombLiftTargetRoot`를 에디터에서 직접 원하는 회전으로 배치한다.

## 검증 기준

- 빌드 성공
- `CombLiftLocalRotationDelta`가 더 이상 Details에 노출되지 않는다.
- `FacingAxis/UpAxis` 관련 코드와 문서가 제거된다.
- 카메라 위치나 방향을 바꿔도 lifted 소비장의 목표 회전이 달라지지 않는다.
- `CombLiftTargetRoot`를 에디터에서 회전시키면 lifted 소비장의 목표 회전도 동일하게 바뀐다.
- 소비장 위치 보간, 내리기, layout refresh 후 lifted transform 재적용 정책은 기존과 동일하게 동작한다.

## QnA 필요 여부

추가 QnA는 필요 없다.

확정 기준:

```text
소비장 들어올림 목표 위치와 목표 회전은 CombLiftTargetRoot가 전부 결정한다.
카메라 방향 회전 계산과 rotation delta 기능은 삭제한다.
```
