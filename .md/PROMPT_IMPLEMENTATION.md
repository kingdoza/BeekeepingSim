# 구현 프롬프트: Comb Slot 중앙 기준 배치 변경

## 목표

`ABeehive`의 소비장 slot 배치 기준을 변경한다.

현재 소비장은 slot index 기준으로 local `-Y` 방향부터 채워지는 정책을 유지한다. 단, `MaxCombCount`로 만들 수 있는 전체 소비장 slot 위치 배열의 중앙이 `CombRackRoot` origin과 일치해야 한다.

## 변경 대상

- `Source/BeekeepingSim/Private/WorldActors/Beehive.cpp`
- 필요 시 `.md/Architecture/WorldActorsSystem.md`
- 필요 시 `.md/0_ARCHITECTURE.md`
- 필요 시 `.md/USER_UNREAL.md`

## 현재 문제

현재 구현은 slot 0을 `CombRackRoot` origin에 두고, 이후 slot을 local `-Y` 방향으로 나열하는 구조일 수 있다.

이 방식은 `MaxCombCount`가 바뀔 때 전체 소비장 묶음의 중앙이 이동한다.

## 요구 동작

- `CombRackRoot` local space 기준은 유지한다.
- `CombSlotSpacing` 사용은 유지한다.
- 활성 소비장은 기존처럼 slot 0부터 `CurrentCombCount - 1`까지 채운다.
- slot 0은 전체 slot 배열에서 가장 local `-Y` 쪽 위치다.
- `MaxCombCount` 기준 전체 slot 배열의 중앙은 항상 `CombRackRoot` origin과 일치한다.

slot 위치 공식:

```cpp
const int32 SafeMaxCombCount = FMath::Max(0, MaxCombCount);
const float HalfSpan = static_cast<float>(SafeMaxCombCount - 1) * 0.5f * CombSlotSpacing;
const float SlotY = -HalfSpan + (static_cast<float>(Index) * CombSlotSpacing);
const FVector RelativeLocation(0.0f, SlotY, 0.0f);
```

`SafeMaxCombCount <= 0`인 경우 slot이 없어야 하므로 위치 계산을 적용하지 않는다.

## 예시

- `MaxCombCount = 5`, `CombSlotSpacing = S`
  - slot Y: `-2S, -S, 0, S, 2S`
  - `CurrentCombCount = 3`이면 활성 위치: `-2S, -S, 0`

- `MaxCombCount = 4`, `CombSlotSpacing = S`
  - slot Y: `-1.5S, -0.5S, 0.5S, 1.5S`
  - `CurrentCombCount = 2`이면 활성 위치: `-1.5S, -0.5S`

## 유지해야 할 것

- `CurrentCombCount` clamp 정책은 변경하지 않는다.
- `SpawnAmount` / `TargetBeeCount` 계산과 초기화 정책은 변경하지 않는다.
- `CombActorClass`, `CombPlaneSize`, Niagara parameter 적용 정책은 변경하지 않는다.
- 기존 outgoing/ingoing swarm, attraction swarm 동작은 변경하지 않는다.
- `Content/` asset은 수정하지 않는다.

## 검증

- `MaxCombCount`를 1, 2, 3, 4, 5로 바꿨을 때 전체 slot 배열 중앙이 `CombRackRoot` origin에 맞는지 확인한다.
- `CurrentCombCount`를 테스트 API로 줄여도 활성 소비장이 local `-Y` 쪽부터 채워지는지 확인한다.
- `CombSlotSpacing` 변경 시 중앙 기준이 유지되는지 확인한다.
- 가능하면 UBT 빌드를 수행한다.
