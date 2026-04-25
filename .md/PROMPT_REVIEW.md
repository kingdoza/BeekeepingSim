# 코드 리뷰 요청 프롬프트

아래 변경사항을 Unreal C++ 관점에서 리뷰하라.

## 변경 파일

- `Source/BeekeepingSim/Public/BeekeeperHeldItemVisualizerComponent.h`
- `Source/BeekeepingSim/Private/BeekeeperHeldItemVisualizerComponent.cpp`
- `Source/BeekeepingSim/Public/ItemPresentationActor.h`
- `Source/BeekeepingSim/Private/ItemPresentationActor.cpp`
- `.md/0_ARCHITECTURE.md`

## 변경 목적

- `EngagedFocus` 상태의 held presentation cursor 정렬 정확도를 개선한다.
- 기존 viewport 정규화 오프셋 방식 대신, deproject cursor ray + camera plane intersection 방식으로 위치를 계산한다.
- held presentation scale을 모드별(`InHandRelativeScale`, `OnCursorRelativeScale`)로 분리한다.
- 사용하지 않는 구 변수(`OnCursorBaseLocalOffset`, `CursorHorizontalOffsetRange`, `CursorVerticalOffsetRange`, `MeshRelativeScale`)를 제거한다.
- lifecycle 안정성을 위해 visualizer `EndPlay()` 정리(delegate 해제, actor destroy)를 유지한다.
- `AItemPresentationActor::InitializePresentation`을 `BlueprintNativeEvent`로 유지해 BP override 가능하도록 한다.

## 핵심 로직

1. `UpdateCursorPresentation()`:
- `GetMousePosition()` + `DeprojectScreenPositionToWorld()`로 월드 ray 산출
- `OwnerCamera` forward 기준 고정 거리 평면(`OnCursorPlaneDistance`) 구성
- ray-plane 교차점 계산 후 `OnCursorLocalOffset`(camera local 기준) 적용
- 최종 world 위치를 camera relative location으로 변환해 actor 위치 반영

2. 모드별 transform:
- `InHand`: `InHandLocalOffset`, `InHandLocalRotation`, `InHandRelativeScale`
- `OnCursor`: deproject 기반 위치, `OnCursorLocalRotation`, `OnCursorRelativeScale`

3. 안정성:
- local/non-local 전환 시 시각화 복구(`bWasRunningLocally`)
- `EndPlay()`에서 delegate 해제 및 spawned actor 정리

## 리뷰 집중 포인트

- deproject + plane intersection 계산의 수학적/좌표계 안정성(FOV, aspect ratio, camera blend 중)  
- attach 상태(actor가 camera에 붙은 상태)에서 relative transform 적용 일관성  
- `OnCursorPlaneDistance <= 0` 및 ray-plane 특이 케이스 방어 충분성  
- lifecycle/GC 안정성(`EndPlay`, transient 참조, actor destroy 타이밍)  
- Blueprint 확장성(`InitializePresentation` BP override 경로)  
- 기존 에셋 영향(제거된 변수로 인한 BP 데이터 손실 가능성)과 migration 필요 여부  
