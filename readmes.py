import os
import json
path = os.path


def readme_template(id: str, title: str):
    return f"""# pebble-watchface-{id}


| Theme            | Light | Dark |
| ---------------- | :------: | :------: |
| {title}  | <img src="./assets/screenshot.png" /> | <img src="./assets/screenshot~dark.png" /> |
"""

def generate_project_readme(project: str):
    project_root = project
    project_readme_path = path.join(project_root, "README.md")
    project_config_path = path.join(project_root, "package.json")
    project_config = json.loads(open(project_config_path, "r").read())

    if not os.path.exists(project_config_path):
        return

    open(project_readme_path, "w").write(
        readme_template(
            id=project_config["name"],
            title=project_config["pebble"]["displayName"],
        )
    )




for project in os.listdir("."):
    if not os.path.isdir(project):
        continue
    generate_project_readme(project)
