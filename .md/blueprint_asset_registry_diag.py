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


def main():
    registry = unreal.AssetRegistryHelpers.get_asset_registry()
    paths = [
        "/Game/UI/WBP_ItemSlot",
        "/Game/UI/WBP_ItemVisual",
        "/Game/UI/WBP_Hotbar",
        "/Game/UI/WBP_StorageBox",
        "/Game/Beehive/BP_Beehive",
        "/Game/Stuff/BP_StorageBox",
    ]
    options = unreal.AssetRegistryDependencyOptions()
    options.include_hard_package_references = True
    options.include_soft_package_references = True
    options.include_searchable_names = True
    options.include_soft_management_references = True
    options.include_hard_management_references = True

    data = {
        "registry_type": str(type(registry)),
        "registry_dir_sample": [n for n in dir(registry) if "depend" in n.lower() or "refer" in n.lower() or "asset" in n.lower()],
        "assets": [],
    }

    for package_name in paths:
        info = {"package_name": package_name}
        try:
            asset_data = registry.get_asset_by_object_path(package_name + "." + package_name.rsplit("/", 1)[-1])
            info["asset_data"] = str(asset_data)
            info["asset_class_path"] = stringify(asset_data.asset_class_path)
            info["package_name_from_data"] = stringify(asset_data.package_name)
            try:
                info["export_text_name"] = stringify(asset_data.get_export_text_name())
            except Exception as exc:
                info["export_text_name_error"] = str(exc)
            try:
                info["native_class"] = stringify(asset_data.find_asset_native_class())
            except Exception as exc:
                info["native_class_error"] = str(exc)
            try:
                info["object_path"] = stringify(asset_data.object_path)
            except Exception as exc:
                info["object_path_error"] = str(exc)
            tag_names = [
                "ParentClass",
                "NativeParentClass",
                "GeneratedClass",
                "ImplementedInterfaces",
                "BlueprintType",
                "BlueprintDescription",
            ]
            info["tags"] = {}
            for tag_name in tag_names:
                try:
                    tag_value = asset_data.get_tag_value(tag_name)
                    info["tags"][tag_name] = stringify(tag_value)
                except Exception as exc:
                    info["tags"][tag_name] = "ERR:" + str(exc)
        except Exception as exc:
            info["asset_data_error"] = str(exc)
        try:
            deps = registry.get_dependencies(package_name, options)
            info["dependencies"] = [stringify(v) for v in deps]
        except Exception as exc:
            info["dependencies_error"] = str(exc)
        try:
            refs = registry.get_referencers(package_name, options)
            info["referencers"] = [stringify(v) for v in refs]
        except Exception as exc:
            info["referencers_error"] = str(exc)
        data["assets"].append(info)

    out = "C:/UnrealProjects/BeekeepingSim/.md/blueprint_asset_registry_diag.json"
    with open(out, "w", encoding="utf-8") as handle:
        json.dump(data, handle, ensure_ascii=False, indent=2)
    unreal.log("Blueprint asset registry diag wrote: " + out)


main()
