import json
import unreal


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    asset_data = registry.get_asset_by_object_path("/Game/UI/WBP_ItemSlot.WBP_ItemSlot")
    data = {
        "asset_data": str(asset_data),
        "dir": dir(asset_data),
    }
    out = "C:/UnrealProjects/BeekeepingSim/.md/asset_data_dir_diag.json"
    with open(out, "w", encoding="utf-8") as handle:
        json.dump(data, handle, ensure_ascii=False, indent=2)
    unreal.log("Asset data dir diag wrote: " + out)


main()
