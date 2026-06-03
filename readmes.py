import os
import json
path = os.path


def readme_template(id: str, title: str, has_dark: bool):
    dark_image = '<img src="./assets/screenshot~dark.png" />' if has_dark else ''
    return f"""# pebble-watchface-{id}


| Theme            | Light | Dark |
| ---------------- | :------: | :------: |
| {title}  | <img src="./assets/screenshot.png" /> | {dark_image} |
"""

def generate_project_readme(project: str):
    project_root = project
    project_readme_path = path.join(project_root, "README.md")
    project_config_path = path.join(project_root, "package.json")
    if not os.path.exists(project_config_path):
        return

    project_config = json.loads(open(project_config_path, "r").read())
    open(project_readme_path, "w").write(
        readme_template(
            id=project_config["name"],
            title=project_config["pebble"]["displayName"],
            has_dark=os.path.exists(path.join(project_root, "assets", "screenshot~dark.png")),
        )
    )




for project in os.listdir("."):
    if not os.path.isdir(project):
        continue
    generate_project_readme(project)
