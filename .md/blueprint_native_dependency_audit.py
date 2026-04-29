import json
import unreal


def stringify(value):
    try:
        if value is None:
            return None
        if hasattr(value, "to_string"):
            return value.to_string()
        return str(value)
    except Exception as exc:
        return "ERR_STR:" + str(exc)


def tag(asset_data, tag_name):
    try:
        return stringify(asset_data.get_tag_value(tag_name))
    except Exception as exc:
        return "ERR:" + str(exc)


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    options = unreal.AssetRegistryDependencyOptions()
    options.include_hard_package_references = True
    options.include_soft_package_references = True
    options.include_searchable_names = True
    options.include_soft_management_references = True
    options.include_hard_management_references = True

    all_assets = registry.get_all_assets()
    native_assets = []
    native_parent_assets = []
    dependency_errors = []

    for asset_data in all_assets:
        package_name = stringify(asset_data.package_name)
        if not package_name or not package_name.startswith("/Game/"):
            continue

        deps = []
        try:
            deps = [stringify(v) for v in registry.get_dependencies(package_name, options)]
        except Exception as exc:
            dependency_errors.append({"package_name": package_name, "error": str(exc)})

        tags = {
            "ParentClass": tag(asset_data, "ParentClass"),
            "NativeParentClass": tag(asset_data, "NativeParentClass"),
            "GeneratedClass": tag(asset_data, "GeneratedClass"),
            "ImplementedInterfaces": tag(asset_data, "ImplementedInterfaces"),
            "BlueprintType": tag(asset_data, "BlueprintType"),
        }
        depends_on_module = "/Script/BeekeepingSim" in deps
        native_parent = tags["NativeParentClass"]
        has_native_parent = bool(native_parent and "/Script/BeekeepingSim." in native_parent)

        if depends_on_module or has_native_parent:
            item = {
                "package_name": package_name,
                "asset_name": stringify(asset_data.asset_name),
                "asset_class_path": stringify(asset_data.asset_class_path),
                "tags": tags,
                "dependencies": deps,
                "referencers": [],
            }
            try:
                item["referencers"] = [stringify(v) for v in registry.get_referencers(package_name, options)]
            except Exception as exc:
                item["referencers_error"] = str(exc)
            native_assets.append(item)
            if has_native_parent:
                native_parent_assets.append(item)

    data = {
        "game_asset_count": len([a for a in all_assets if stringify(a.package_name).startswith("/Game/")]),
        "beekeeping_sim_dependency_count": len(native_assets),
        "native_parent_count": len(native_parent_assets),
        "native_parent_assets": native_parent_assets,
        "beekeeping_sim_dependency_assets": native_assets,
        "dependency_errors": dependency_errors,
    }

    out = "C:/UnrealProjects/BeekeepingSim/.md/blueprint_native_dependency_audit.json"
    with open(out, "w", encoding="utf-8") as handle:
        json.dump(data, handle, ensure_ascii=False, indent=2)
    unreal.log("Blueprint native dependency audit wrote: " + out)


main()
