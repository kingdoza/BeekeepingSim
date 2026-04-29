import json
from pathlib import Path


ROOT = Path("C:/UnrealProjects/BeekeepingSim")
CONTENT_ROOT = ROOT / "Content"

TARGETS = {
    "ItemDragVisualWidget": {
        "kind": "class",
        "aliases": ["ItemDragVisualWidget", "UItemDragVisualWidget"],
        "qna": [3],
    },
    "ItemVisualWidget": {
        "kind": "class",
        "aliases": ["ItemVisualWidget", "UItemVisualWidget"],
        "qna": [3],
    },
    "ItemSlotWidget": {
        "kind": "class",
        "aliases": ["ItemSlotWidget", "UItemSlotWidget"],
        "qna": [2, 5],
    },
    "StorageBoxWidget": {
        "kind": "class",
        "aliases": ["StorageBoxWidget", "UStorageBoxWidget"],
        "qna": [2, 5],
    },
    "StorageSlotDragDropOperation": {
        "kind": "class",
        "aliases": ["StorageSlotDragDropOperation", "UStorageSlotDragDropOperation"],
        "qna": [2, 6],
    },
    "EStorageSlotContainerType": {
        "kind": "enum",
        "aliases": ["EStorageSlotContainerType"],
        "qna": [6],
    },
    "EItemSlotDragMode": {
        "kind": "enum",
        "aliases": ["EItemSlotDragMode"],
        "qna": [6],
    },
    "FItemSlotMoveResult": {
        "kind": "struct",
        "aliases": ["FItemSlotMoveResult", "ItemSlotMoveResult"],
        "qna": [6],
    },
    "InitializeDragVisual": {
        "kind": "function",
        "aliases": ["InitializeDragVisual"],
        "qna": [3],
    },
    "OnDragVisualInitialized": {
        "kind": "event",
        "aliases": ["OnDragVisualInitialized"],
        "qna": [3],
    },
    "InitializeSlotContext": {
        "kind": "function",
        "aliases": ["InitializeSlotContext"],
        "qna": [2],
    },
    "RefreshVisual": {
        "kind": "function",
        "aliases": ["RefreshVisual"],
        "qna": [2],
    },
    "ShouldHideItemVisualForCurrentDrag": {
        "kind": "function",
        "aliases": ["ShouldHideItemVisualForCurrentDrag"],
        "qna": [2],
    },
    "GetDragPreviewDisplayStackCount": {
        "kind": "function",
        "aliases": ["GetDragPreviewDisplayStackCount"],
        "qna": [2],
    },
    "RefreshDragPreviewFromOperation": {
        "kind": "function",
        "aliases": ["RefreshDragPreviewFromOperation"],
        "qna": [2],
    },
    "IsPartialDragPreviewActive": {
        "kind": "function",
        "aliases": ["IsPartialDragPreviewActive"],
        "qna": [2],
    },
    "GetPartialDragPreviewDisplayStackCount": {
        "kind": "function",
        "aliases": ["GetPartialDragPreviewDisplayStackCount"],
        "qna": [2],
    },
    "RefreshPartialDragPreviewFromOperation": {
        "kind": "function",
        "aliases": ["RefreshPartialDragPreviewFromOperation"],
        "qna": [2],
    },
    "InitializeStorageWidget": {
        "kind": "function",
        "aliases": ["InitializeStorageWidget"],
        "qna": [2],
    },
    "MoveHotbarItemToStorage": {
        "kind": "function",
        "aliases": ["MoveHotbarItemToStorage"],
        "qna": [2, 5],
    },
    "MoveStorageItemToHotbar": {
        "kind": "function",
        "aliases": ["MoveStorageItemToHotbar"],
        "qna": [2, 5],
    },
    "SwapStorageSlots": {
        "kind": "function",
        "aliases": ["SwapStorageSlots"],
        "qna": [2, 5],
    },
    "SwapHotbarAndStorage": {
        "kind": "function",
        "aliases": ["SwapHotbarAndStorage"],
        "qna": [2, 5],
    },
    "OnStorageWidgetInitialized": {
        "kind": "event",
        "aliases": ["OnStorageWidgetInitialized"],
        "qna": [2],
    },
    "bClearFocusOnConfirm": {
        "kind": "property",
        "aliases": ["bClearFocusOnConfirm", "ClearFocusOnConfirm"],
        "qna": [4],
    },
    "ShouldClearFocusOnConfirm": {
        "kind": "function",
        "aliases": ["ShouldClearFocusOnConfirm"],
        "qna": [4],
    },
    "FocusTargetComponent": {
        "kind": "class",
        "aliases": ["FocusTargetComponent", "UFocusTargetComponent"],
        "qna": [4],
    },
    "SetMoveQuantityClamped": {
        "kind": "function",
        "aliases": ["SetMoveQuantityClamped"],
        "qna": [6],
    },
    "AdjustMoveQuantity": {
        "kind": "function",
        "aliases": ["AdjustMoveQuantity"],
        "qna": [6],
    },
    "InitializeMoveQuantity": {
        "kind": "function",
        "aliases": ["InitializeMoveQuantity"],
        "qna": [6],
    },
    "OnMoveQuantityChanged": {
        "kind": "delegate_property",
        "aliases": ["OnMoveQuantityChanged"],
        "qna": [6],
    },
    "SourceType": {
        "kind": "property",
        "aliases": ["SourceType"],
        "qna": [6],
    },
    "SourceIndex": {
        "kind": "property",
        "aliases": ["SourceIndex"],
        "qna": [6],
    },
    "MoveQuantity": {
        "kind": "property",
        "aliases": ["MoveQuantity"],
        "qna": [6],
    },
    "MaxMoveQuantity": {
        "kind": "property",
        "aliases": ["MaxMoveQuantity"],
        "qna": [6],
    },
    "DragVisualWidget": {
        "kind": "property",
        "aliases": ["DragVisualWidget"],
        "qna": [6],
    },
}


def asset_files():
    suffixes = {".uasset", ".umap"}
    return sorted(path for path in CONTENT_ROOT.rglob("*") if path.suffix.lower() in suffixes)


def contains(data, text):
    raw = text.encode("utf-8")
    wide = text.encode("utf-16le")
    return raw in data or wide in data


def main():
    files = asset_files()
    results = {}
    module_marker = "/Script/BeekeepingSim"
    for target_name, info in TARGETS.items():
        hit_paths = []
        module_hit_paths = []
        alias_hits = {}
        for path in files:
            data = path.read_bytes()
            matched_aliases = [alias for alias in info["aliases"] if contains(data, alias)]
            if matched_aliases:
                rel = str(path.relative_to(ROOT)).replace("\\", "/")
                hit_paths.append(rel)
                alias_hits[rel] = matched_aliases
                if contains(data, module_marker):
                    module_hit_paths.append(rel)
        results[target_name] = {
            "kind": info["kind"],
            "qna": info["qna"],
            "aliases": info["aliases"],
            "hit_count": len(hit_paths),
            "hits": hit_paths,
            "module_hit_count": len(module_hit_paths),
            "hits_with_beekeeping_module_marker": module_hit_paths,
            "alias_hits": alias_hits,
        }

    data = {
        "content_asset_file_count": len(files),
        "targets": results,
    }

    out = ROOT / ".md" / "blueprint_symbol_scan.json"
    out.write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")


if __name__ == "__main__":
    main()
