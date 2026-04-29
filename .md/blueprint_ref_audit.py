import json
import traceback

import unreal


TARGETS = {
    "classes": [
        "ItemDragVisualWidget",
        "ItemVisualWidget",
        "ItemSlotWidget",
        "StorageBoxWidget",
        "StorageSlotDragDropOperation",
        "FocusTargetComponent",
        "StorageBoxFocusActionComponent",
        "PickupFocusActionComponent",
    ],
    "functions": [
        "ShouldHideItemVisualForCurrentDrag",
        "GetDragPreviewDisplayStackCount",
        "RefreshDragPreviewFromOperation",
        "IsPartialDragPreviewActive",
        "GetPartialDragPreviewDisplayStackCount",
        "RefreshPartialDragPreviewFromOperation",
        "MoveHotbarItemToStorage",
        "MoveStorageItemToHotbar",
        "SwapStorageSlots",
        "SwapHotbarAndStorage",
        "ShouldClearFocusOnConfirm",
    ],
    "properties": [
        "bClearFocusOnConfirm",
        "ContainerType",
        "SlotIndex",
        "DragVisualWidgetClass",
    ],
    "types": [
        "EStorageSlotContainerType",
        "FItemSlotMoveResult",
        "EItemSlotDragMode",
    ],
}


def safe_get_editor_property(obj, name):
    try:
        return obj.get_editor_property(name)
    except Exception:
        return None


def get_name(value):
    try:
        if value is None:
            return ""
        if hasattr(value, "get_name"):
            return value.get_name()
        return str(value)
    except Exception:
        return ""


def object_text(obj):
    parts = [get_name(obj), str(type(obj))]
    for prop_name in [
        "function_reference",
        "variable_reference",
        "member_name",
        "member_parent",
        "pin_type",
        "default_value",
        "default_object",
        "parent_class",
        "generated_class",
        "skeleton_generated_class",
    ]:
        value = safe_get_editor_property(obj, prop_name)
        if value is not None:
            parts.append(prop_name + "=" + get_name(value))
            parts.append(str(value))
    return " ".join(parts)


def target_hits(text):
    hits = []
    for category, names in TARGETS.items():
        for name in names:
            if name in text:
                hits.append({"category": category, "name": name})
    return hits


def inspect_loaded_asset(asset_path, asset):
    results = []
    package = asset.get_outermost()

    parent_class = safe_get_editor_property(asset, "parent_class")
    if parent_class:
        text = "parent_class=" + get_name(parent_class) + " " + str(parent_class)
        hits = target_hits(text)
        if hits:
            results.append({
                "asset": asset_path,
                "object": get_name(asset),
                "object_class": asset.get_class().get_name(),
                "kind": "asset_parent_class",
                "text": text,
                "hits": hits,
            })

    generated_class = safe_get_editor_property(asset, "generated_class")
    if generated_class:
        text = "generated_class=" + get_name(generated_class) + " " + str(generated_class)
        hits = target_hits(text)
        if hits:
            results.append({
                "asset": asset_path,
                "object": get_name(asset),
                "object_class": asset.get_class().get_name(),
                "kind": "asset_generated_class",
                "text": text,
                "hits": hits,
            })

    try:
        nested = unreal.ObjectLibrary.get_objects_from_outer(package, True)
    except Exception:
        nested = []

    for obj in nested:
        text = object_text(obj)
        hits = target_hits(text)
        if hits:
            results.append({
                "asset": asset_path,
                "object": get_name(obj),
                "object_class": obj.get_class().get_name() if obj else "",
                "kind": "nested_object",
                "text": text,
                "hits": hits,
            })

    return results


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    registry.search_all_assets(True)

    asset_paths = unreal.EditorAssetLibrary.list_assets("/Game", True, False)
    all_results = []
    errors = []

    for asset_path in asset_paths:
        try:
            asset = unreal.EditorAssetLibrary.load_asset(asset_path)
            if not asset:
                continue
            all_results.extend(inspect_loaded_asset(asset_path, asset))
        except Exception:
            errors.append({
                "asset": asset_path,
                "error": traceback.format_exc(),
            })

    output = {
        "asset_count": len(asset_paths),
        "hit_count": len(all_results),
        "error_count": len(errors),
        "hits": all_results,
        "errors": errors,
    }

    output_path = "C:/UnrealProjects/BeekeepingSim/.md/blueprint_ref_audit.json"
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(output, f, ensure_ascii=False, indent=2)

    unreal.log("Blueprint ref audit wrote: " + output_path)
    unreal.log("Blueprint ref audit assets: %d hits: %d errors: %d" % (len(asset_paths), len(all_results), len(errors)))


main()
