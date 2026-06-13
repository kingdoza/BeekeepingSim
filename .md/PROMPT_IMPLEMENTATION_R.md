# 구현 수정 프롬프트: 분봉 테스트 포획 Rate 리뷰 Finding

## 우선순위

1. Major: `UBeeCarrierUseAction`의 `AliveRadius` 감소 rate가 리뷰 프롬프트의 수식과 다름

## 발견 문제

### 1. 벌 운반통 drag bonus 계산이 요구 수식과 다름

- 대상 파일:
  - `Source/BeekeepingSim/Public/Inventory/BeeCarrierUseAction.h`
  - `Source/BeekeepingSim/Private/Inventory/BeeCarrierUseAction.cpp`
  - `.md/Architecture/InventorySystem.md`
- 문제:
  - 리뷰 프롬프트의 기대 수식은 `Distance(CurrentImpactPoint, LastImpactPoint) / DeltaTime`으로 계산한 drag speed를 그대로 bonus rate 계산에 쓰는 형태다.
  - 현재 구현은 `DragSpeedSmoothingAlpha`로 smoothing한 값에서 `MinDragSpeedForBonus`를 뺀 값을 bonus speed로 사용한다.
- 영향:
  - 느린 드래그/빠른 드래그 포획 속도 차이가 프롬프트의 계산과 다르게 나타난다.
  - C++ 구현과 `.md/PROMPT_REVIEW.md`의 기대 구현이 불일치한다.
- 수정 방향:
  - `ApplyUseEffect`에서 둘째 tick 이후 `DragSpeed = Distance(CurrentImpactPoint, LastImpactPoint) / DeltaTime`을 계산한다.
  - 첫 tick 또는 hit point 없음이면 bonus 없이 base rate만 적용한다.
  - rate는 아래 수식을 따른다.

```cpp
Rate = BaseAliveRadiusDecreasePerSecond + BonusSpeed * DragSpeedToAliveRadiusDecreaseScale;
Rate = Clamp(Rate, 0.0f, MaxAliveRadiusDecreasePerSecond);
DeltaAliveRadius = Rate * DeltaTime;
```

  - `BonusSpeed`의 정의가 raw drag speed인지 threshold 적용 값인지 애매하면 구현 전 `.md/QNA_IMPLEMENTATION.md`에 질문한다. 이번 리뷰 프롬프트 기준으로는 smoothing 적용은 제거하는 쪽이 일치한다.
  - smoothing/threshold를 제거하면 관련 UPROPERTY와 `.md/Architecture/InventorySystem.md` 설명도 함께 정리한다.

## 검증 방법

- 정적 검색:
  - `rg -n "SmoothedDragSpeed|DragSpeedSmoothingAlpha|MinDragSpeedForBonus|BeeCarrierUseAction" Source/BeekeepingSim/Public/Inventory Source/BeekeepingSim/Private/Inventory .md/Architecture/InventorySystem.md`
- 빌드:
  - `& "C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\DotNET\AutomationTool\UnrealBuildTool.exe" BeekeepingSimEditor Win64 Development -Project="C:\UnrealProjects\BeekeepingSim\BeekeepingSim.uproject" -WaitMutex -NoHotReloadFromIDE`
- 수동 PIE:
  - 첫 tick은 base rate만 적용되는지 확인한다.
  - 같은 `DeltaTime`에서 더 긴 drag distance가 더 큰 `AliveRadius` 감소량을 만드는지 확인한다.
  - `AliveRadius`가 실제 감소한 tick에만 action result success가 true인지 확인한다.

## 문서 반영 필요 여부

- 필요.
- `UBeeCarrierUseAction` rate 계산을 수정하면 `.md/Architecture/InventorySystem.md`의 Bee Carrier Use Action 설명도 동일 수식으로 맞춘다.
