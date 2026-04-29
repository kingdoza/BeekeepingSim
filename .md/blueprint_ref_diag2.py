import json
import unreal


def stringify(v):
    try:
        if v is None:
            return None
        if hasattr(v, "get_name"):
            return v.get_name() + " | " + str(v)
        return str(v)
    except Exception as e:
        return "ERR_STR:" + str(e)


def attr(obj, name):
    try:
        v = getattr(obj, name)
        if callable(v):
            return stringify(v())
        return stringify(v)
    except Exception as e:
        return "ERR_ATTR:" + str(e)


def call(obj, name):
    try:
        return stringify(getattr(obj, name)())
    except Exception as e:
        return "ERR_CALL:" + str(e)


def inspect(path):
    asset = unreal.load_asset(path)
    info = {"path": path, "loaded": bool(asset)}
    if not asset:
        return info
    info["asset"] = stringify(asset)
    info["class"] = stringify(asset.get_class())
    for name in ["generated_class", "skeleton_generated_class", "parent_class", "blueprint_type"]:
        info[name + "_attr"] = attr(asset, name)
    for name in ["get_class", "get_path_name", "get_full_name", "get_name", "get_outer", "get_outermost"]:
        info[name + "_call"] = call(asset, name)

    try:
        types = unreal.get_blueprint_generated_types(asset)
        info["get_blueprint_generated_types"] = [stringify(v) for v in types]
    except Exception as e:
        info["get_blueprint_generated_types"] = "ERR:" + str(e)

    gen = None
    try:
        gen_attr = getattr(asset, "generated_class")
        gen = gen_attr() if callable(gen_attr) else gen_attr
    except Exception:
        gen = None
    if gen:
        info["generated_class_name"] = stringify(gen)
        info["generated_class_dir_sample"] = [n for n in dir(gen) if "super" in n.lower() or "class" in n.lower() or "default" in n.lower() or "property" in n.lower()]
        try:
            info["generated_super_class"] = stringify(gen.get_super_class())
        except Exception as e:
            info["generated_super_class"] = "ERR:" + str(e)
        cdo = unreal.get_default_object(gen)
        info["cdo"] = stringify(cdo)
        info["cdo_class"] = stringify(cdo.get_class()) if cdo else None
        if cdo:
            names = [n for n in dir(cdo) if "drag" in n.lower() or "partial" in n.lower() or "storage" in n.lower() or "slot" in n.lower() or "visual" in n.lower()]
            info["cdo_relevant_dir"] = names
            values = {}
            for n in names:
                if n.startswith("_"):
                    continue
                try:
                    values[n] = stringify(getattr(cdo, n))
                except Exception as e:
                    values[n] = "ERR:" + str(e)
            info["cdo_relevant_values"] = values
    return info


def main():
    paths = [
        "/Game/UI/WBP_ItemSlot.WBP_ItemSlot",
        "/Game/UI/WBP_ItemVisual.WBP_ItemVisual",
        "/Game/UI/WBP_Hotbar.WBP_Hotbar",
        "/Game/UI/WBP_StorageBox.WBP_StorageBox",
    ]
    data = [inspect(p) for p in paths]
    out = "C:/UnrealProjects/BeekeepingSim/.md/blueprint_ref_diag2.json"
    with open(out, "w", encoding="utf-8") as f:
        json.dump(data, f, ensure_ascii=False, indent=2)
    unreal.log("Blueprint ref diag2 wrote: " + out)


main()
