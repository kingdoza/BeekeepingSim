import json
import unreal


def safe_prop(obj, name):
    try:
        v = obj.get_editor_property(name)
        return str(v)
    except Exception as e:
        return "ERR:" + str(e)


def main():
    paths = [
        "/Game/UI/WBP_ItemSlot.WBP_ItemSlot",
        "/Game/UI/WBP_ItemVisual.WBP_ItemVisual",
        "/Game/UI/WBP_Hotbar.WBP_Hotbar",
        "/Game/UI/WBP_StorageBox.WBP_StorageBox",
    ]
    data = {
        "unreal_object_names": [n for n in dir(unreal) if "object" in n.lower() or "outer" in n.lower()],
        "unreal_graph_names": [n for n in dir(unreal) if "graph" in n.lower() or "blueprint" in n.lower() or "k2" in n.lower()],
        "assets": [],
    }

    for path in paths:
        asset = unreal.load_asset(path)
        info = {
            "path": path,
            "loaded": bool(asset),
        }
        if asset:
            info["name"] = asset.get_name()
            info["class"] = asset.get_class().get_name()
            info["dir_sample"] = [n for n in dir(asset) if "graph" in n.lower() or "class" in n.lower() or "parent" in n.lower() or "property" in n.lower()]
            for prop in [
                "parent_class",
                "generated_class",
                "skeleton_generated_class",
                "ubergraph_pages",
                "function_graphs",
                "macro_graphs",
                "delegate_signature_graphs",
                "new_variables",
                "simple_construction_script",
            ]:
                info[prop] = safe_prop(asset, prop)
        data["assets"].append(info)

    output_path = "C:/UnrealProjects/BeekeepingSim/.md/blueprint_ref_diag.json"
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    unreal.log("Blueprint ref diag wrote: " + output_path)


main()
